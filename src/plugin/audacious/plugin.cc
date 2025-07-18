// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <cassert>
#include <cctype>
#include <cstring>
#include <string>

#include <libaudcore/audstrings.h>
#include <libaudcore/drct.h>
#include <libaudcore/plugin.h>
#include <libaudcore/runtime.h>
#include <libaudcore/vfs.h>

#include "config.h"
#include "common/logger.h"
#include "common/songend.h"
#include "common/strings.h"
#include "common/std/optional.h"
#include "prefs.h"
#include "player/extensions.h"
#include "player/player.h"
#include "songend/precalc.h"
#include "songdb/songdb.h"
#include "plugin/common/copyright.h"
#include "3rdparty/xxhash/xxhash.h"

using namespace std;

namespace {

struct Info {
    player::Player player;
    string format;
    int channels;
    int minsubsong;
    int maxsubsong;
    optional<songdb::MetaData> metadata;
};

const set<string> extensions = [](){
    set<string> exts;
    for (const auto ext : player::exts)
        if (ext) exts.insert(ext);
    return exts;
}();

void fseek0(VFSFile &file) {
    if (file.fseek(0, VFS_SEEK_SET)) {
        WARN("uade_plugin could not seek %s\n", file.filename());
    }
}

Index<char> read_all(VFSFile &file) {
    fseek0(file);
    Index<char> buf = file.read_all();
    if (buf.len() < file.fsize()) {
        file.set_limit_to_buffer(false);
        const Index<char> buf2 = file.read_all();
        buf.insert(buf2.begin(), buf.len(), buf2.len());
    }
    return buf;
}

string xxh32hash(VFSFile &file) {
    const Index<char> buf = file.read_all();
    const auto xxh32 = XXH32(buf.begin(),min(songdb::XXH_MAX_BYTES, (size_t)buf.len()), 0);
    return common::to_hex(xxh32) + common::to_hex((uint16_t)(file.fsize() & 0xFFFF));
}

vector<player::Player> check_player(VFSFile &file, const string &path, bool check_all) {
    fseek0(file);
    // note: this only reads 256k or so, see read_all(file) above to read full file
    const Index<char> buf = file.read_all();
    return player::check(path.c_str(), buf.begin(), buf.len(), file.fsize(), check_all);
}

int parse_uri(const char *uri, string &path, string &ext) {
    int subsong;
    const char *sub, *tmpExt;

    uri_parse(uri, nullptr, &tmpExt, &sub, &subsong);
#ifdef __MINGW32__
    string p = string(uri_to_filename(uri));
    replace(p.begin(), p.end(), '\\', '/');
    const char *tmpPath = p.c_str();
#else
    const char *tmpPath = uri_to_filename(uri);
#endif
    path = string(tmpPath, strlen(tmpPath) - strlen(sub));
    ext = string(tmpExt, strlen(tmpExt) - strlen(sub));

    return strlen(sub) > 0 ? subsong : -1;
}

void update_tuple_song_end(Tuple &tuple, const common::SongEnd &songend, const optional<string> &format) {
    const auto status = songend.status;
    const auto comment = "songend=" + songend.status_string();
    tuple.set_str(Tuple::Comment, comment.c_str());
    if (songend.length > 0 && status != common::SongEnd::NOSOUND &&
        (status != common::SongEnd::ERROR ||
        (format && songend::precalc::allow_songend_error(format.value())))) {
        tuple.set_int(Tuple::Length, songend.length);
    }
}

void update_tuple_subsong_range(Tuple &tuple, int minsubsong, int maxsubsong) {
    // provide mappings to uade subsong numbers
    Index<short> subtunes;
    for (int i = minsubsong; i <= maxsubsong; ++i) {
        subtunes.append(i);
    }
    tuple.set_subtunes(subtunes.len(), subtunes.begin());
}

void update_tuple_songdb(Tuple &tuple, const string &path, const songdb::SubSongInfo &songinfo, const Info &info, const string &hash) {
    TRACE("Found songlength data for %s:%d %s, length:%d, status:%s\n", hash.c_str(), songinfo.subsong, path.c_str(), songinfo.songend.length, songinfo.songend.status_string().c_str());
    const auto set_str = [&tuple](Tuple::Field field, const string &value) {
        if (!tuple.is_set(field) && !value.empty()) {
            tuple.set_str(field, value.c_str());
        }
    };
    const auto set_int = [&tuple](Tuple::Field field, const int value) {
        if (!tuple.is_set(field) && value > 0) {
            tuple.set_int(field, value);
        }
    };

    set_int(Tuple::Length, songinfo.songend.length);
    set_str(Tuple::Comment, "songend="+songinfo.songend.status_string());

    if (!info.metadata) {
        TRACE("No metadata for %s %s\n", hash.c_str(), path.c_str());
        return;
    }

    const auto &data = info.metadata.value();
    TRACE("Found metadata for %s:%d %s, author:%s, album:%s, publisher:%s, year:%d\n", hash.c_str(), songinfo.subsong, path.c_str(), data.author.c_str(), data.album.c_str(), data.publisher.c_str(), data.year);
    set_str(Tuple::Artist, data.author);
    set_str(Tuple::Album, data.album);
#if AUDACIOUS_HAS_PUBLISHER
// since Audacious 4.3
    set_str(Tuple::Publisher, data.publisher);
#else
    set_str(Tuple::Copyright, data.publisher);
#endif
    set_int(Tuple::Year, data.year);
}

bool update_tuple(Tuple &tuple, const string &path, int subsong, const Info &info, const string &modulehash) {
    /* prefer filename as title for now, maybe make configurable in future
    bool modulename_ok = strnlen(info->modulename, 256) > 0 &&
                            !is_blacklisted_title(info);
    const char *title = modulename_ok ? info->modulename : name;
    */
    tuple.set_str(Tuple::Title, common::split(path, "/").back().c_str());
    const auto player = player::name(info.player);
    string codec = info.format;
    transform(codec.begin(), codec.end(), codec.begin(), ::tolower);
    if (codec != player) {
        codec = info.format + " [" + player.data() + "]";
    } else {
        codec = info.format; // undo tolower
    }
    tuple.set_str(Tuple::Codec, codec.c_str());
    tuple.set_str(Tuple::Quality, "sequenced");
#if AUDACIOUS_HAS_CHANNELS
    if (info.channels > 0) {
        tuple.set_int(Tuple::Channels, info.channels);
    }
#endif
    const int subsongs = info.maxsubsong - info.minsubsong + 1;
    // initial probe
    if (subsong == -1) {
        update_tuple_subsong_range(tuple, info.minsubsong, info.maxsubsong);
    } else {
        if (subsongs > 1) {
            tuple.set_int(Tuple::NumSubtunes, subsongs);
            tuple.set_int(Tuple::Subtune, subsong);
            tuple.set_int(Tuple::Track, subsong);
        }
        const auto songinfo = songdb::lookup(modulehash, subsong);
        if (songinfo) {
            update_tuple_songdb(tuple, path, songinfo.value(), info, modulehash);
            return true;
        } else {
            TRACE("No songlength data for %s:%d %s\n", modulehash.c_str(), subsong, path.c_str());
        }
    }
    return false;
}

common::SongEnd precalc_song_end(
    player::Player player,
    VFSFile &file,
    const string &path,
    const string &hash,
    const int subsong
) {
    const Index<char> buf = read_all(file);
    const auto modinfo = player::parse(path.c_str(), buf.begin(), buf.len(), player);
    if (!modinfo) {
        return { common::SongEnd::ERROR, 0 };
    }
    return songend::precalc::precalc_song_end(modinfo.value(), buf.begin(), buf.len(), subsong, hash);
};

optional<Info> parse_info(VFSFile &file, const string &path, const string &hash) {
    const auto players = check_player(file, path, true);
    if (players.empty()) {
        return {};
    }
    const auto songdbinfo = songdb::lookup(hash);
#if PLAYER_all
    if (songdbinfo && songdbinfo->modinfo && !songdbinfo->subsongs.empty() && players.size() == 1) {
        const auto player = players.front();
        const auto minsubsong = songdbinfo->subsongs.front().subsong;
        const auto maxsubsong = songdbinfo->subsongs.back().subsong;
        return Info {
            player,
            songdbinfo->modinfo->format,
            songdbinfo->modinfo->channels,
            minsubsong,
            maxsubsong,
            songdbinfo->metadata,
        };
    }
#endif
    optional<player::ModuleInfo> modinfo;
    const Index<char> buf = read_all(file);
    for (const auto &p : players) {
        modinfo = player::parse(path.c_str(), buf.begin(), buf.len(), p);
        if (modinfo) break;
    }
    if (!modinfo) {
        return {};
    }
    return Info {
        modinfo->player,
        modinfo->format,
        modinfo->channels,
        modinfo->minsubsong,
        modinfo->maxsubsong,
        songdbinfo ? songdbinfo->metadata : optional<songdb::MetaData>{},
    };
}

player::Filter player_filter(const int filter) {
    switch(filter) {
        case 1: return player::Filter::A500;
        case 2: return player::Filter::A1200;
        case 3: return player::Filter::NONE;
        default: return player::Filter::A1200;
    }
}

player::uade::Resampler uade_resampler(const int resampler) {
    switch(resampler) {
        case 0: return player::uade::Resampler::DEFAULT;
        case 1: return player::uade::Resampler::SINC;
        case 2: return player::uade::Resampler::NONE;
        default: return player::uade::Resampler::SINC;
    }
}

player::uade::UADEConfig get_uade_config(const player::PlayerConfig &config) {
    const int filter = aud_get_int(PLUGIN_NAME, "filter");
    const bool force_led_enabled = aud_get_bool(PLUGIN_NAME, "force_led_enabled");
    const int force_led = aud_get_int(PLUGIN_NAME, "force_led");
    const int resampler = aud_get_int(PLUGIN_NAME, "resampler");
    const float panning = aud_get_double(PLUGIN_NAME, "panning");
    const bool headphones = aud_get_bool(PLUGIN_NAME, "headphones");
    const bool headphones2 = aud_get_bool(PLUGIN_NAME, "headphones2");
    const float gain = aud_get_double(PLUGIN_NAME, "gain");
    const int subsong_timeout = aud_get_int(PLUGIN_NAME, "subsong_timeout");
    const int silence_timeout = aud_get_int(PLUGIN_NAME, "silence_timeout");

    player::uade::UADEConfig conf(config);

    conf.filter = player_filter(filter);
    conf.resampler = uade_resampler(resampler);

    if (force_led_enabled)
        conf.force_led = force_led ? true : false;
    else
        conf.force_led = {};

    conf.panning = panning;
    conf.gain = gain;

    conf.headphones = headphones;
    conf.headphones2 = headphones2;

    conf.subsong_timeout = subsong_timeout;
    conf.silence_timeout = silence_timeout;

    return conf;
}

player::it2play::Driver it2play_driver(const int driver) {
    switch(driver) {
        case 0: return player::it2play::Driver::HQ;
        case 1: return player::it2play::Driver::SB16MMX;
        case 2: return player::it2play::Driver::SB16;
        case 3: return player::it2play::Driver::WAVWRITER;
        default: return player::it2play::Driver::HQ;
    }
}

player::it2play::IT2PlayConfig get_it2play_config(const player::PlayerConfig &config) {
    const int driver = aud_get_int(PLUGIN_NAME, "it2play_driver");

    player::it2play::IT2PlayConfig conf(config);
    conf.driver = it2play_driver(driver);
    return conf;
}

player::libopenmpt::LibOpenMPTConfig get_libopenmpt_config(const player::PlayerConfig &config) {
    const int filter = aud_get_int(PLUGIN_NAME, "filter");
    const float panning = aud_get_double(PLUGIN_NAME, "panning");

    player::libopenmpt::LibOpenMPTConfig conf(config);
    conf.filter = player_filter(filter);
    conf.panning = panning;

    return conf;
}

player::libxmp::LibXMPConfig get_libxmp_config(const player::PlayerConfig &config) {
    const int filter = aud_get_int(PLUGIN_NAME, "filter");
    const float panning = aud_get_double(PLUGIN_NAME, "panning");

    player::libxmp::LibXMPConfig conf(config);
    conf.filter = player_filter(filter);
    conf.panning = panning;

    return conf;
}

// hack for files which actually contain ? in their name, e.g. MOD.louzy-house?2 or MOD.how low can we go?1
// which conflicts with audacious subsong uri scheme
// XXX audacious also does not support subsongs for "prefix" formats by default
// see https://redmine.audacious-media-player.org/boards/2/topics/1354
int applySubsongHack(int subsong, const char *uri, const string &hash, const bool slowProbe, const char* tag) {
    const auto &subsongs = songdb::subsong_range(hash);
    if (!subsongs) {
        if (!slowProbe)
            ERR("SUBSONGS AND METADATA NOT AVAILABLE: Enable \"Probe content of files with no recognized file name extension\" in preferences to fix!\n");
        else
            ERR("%s could not determine subsong for %s\n", tag, uri);
        return -1;
    }
    if (!slowProbe)
        ERR("SUBSONGS AND METADATA NOT AVAILABLE: Enable \"Probe content of files with no recognized file name extension\" in preferences to fix!\n");
    else if (subsongs->first == subsongs->second)
        DEBUG("%s enforced subsong %d (was %d) for %s\n", tag, subsongs->first, subsong, uri);
    else 
        WARN("%s enforced subsong %d (was %d) for %s\n", tag, subsongs->first, subsong, uri);

    return subsongs->first;
}

} // namespace {}

class UADEPlugin : public InputPlugin
{

public:
    static constexpr PluginInfo info = {
        "UADE Plugin",
        "audacious-uade",
        plugin_copyright,
        &plugin_prefs
    };

    constexpr UADEPlugin() : InputPlugin(info, InputInfo(FlagSubtunes)
#if AUDACIOUS_HAS_DEFAULT_PRIO
        .with_priority(_AUD_PLUGIN_DEFAULT_PRIO - 1) // preempt openmpt, modplug, mpg123 etc. plugins (mpg123 has many false positives)
#else
        .with_priority(-1)
#endif
        .with_exts(player::exts)
        .with_mimes(player::mimetypes)) {}

    bool init();
    void cleanup();

    bool is_our_file(const char *uri, VFSFile &file);
    bool read_tag(const char *uri, VFSFile &file, Tuple &tuple, Index<char> *image);
    bool play(const char *uri, VFSFile &file);

private:

}; // class UADEPlugin

#define EXPORT __attribute__((visibility("default")))
EXPORT UADEPlugin aud_plugin_instance;

bool UADEPlugin::init() {
    DEBUG("uade_plugin_init\n");

    aud_config_set_defaults (PLUGIN_NAME, plugin_defaults);
    aud_config_set_defaults (PLUGIN_NAME, uade_defaults);

    player::init();
    songdb::init(UADEDIR "/songdb");
    return true;
}

void UADEPlugin::cleanup() {
    DEBUG("uade_plugin_cleanup\n");

    player::shutdown();
}

bool UADEPlugin::is_our_file(const char *uri, VFSFile &file) {
    TRACE("uade_plugin_is_our_file %s\n", uri);
    string path, ext;
    parse_uri(uri, path, ext);

    if (songdb::blacklist::is_blacklisted_extension(path, ext, extensions)) {
        TRACE("uade_plugin_is_our_file blacklisted %s\n", uri);
        return false;
    }
    
    if (check_player(file, path, false).empty()) {
        TRACE("uade_plugin_is_our_file rejected %s\n", uri);
        return false;
    }

    TRACE("uade_plugin_is_our_file accepted %s\n", uri);
    return true;
}

bool UADEPlugin::read_tag(const char *uri, VFSFile & file, Tuple &tuple, Index<char> *image) {
    TRACE("uade_plugin_read_tag %s\n", uri);
    string path, ext;
    int subsong = parse_uri(uri, path, ext);

    const string hash = xxh32hash(file);

#if PLAYER_uade
    // add to playlist, but call uade_play() on-demand (may hang UADE/audacious completely)
    if (songdb::blacklist::is_blacklisted_hash(hash)) {
        WARN("uade_plugin_read_tag blacklisted hash %s (%s)\n", uri, hash.c_str());
        return true;
    }
#endif
    // try read subsongs directly from songdb
#if PLAYER_all
    const auto subsongs = subsong < 0 ? songdb::subsong_range(hash) : optional<pair<int, int>>();
    if (subsongs) {
        TRACE("uade_plugin_read_tag read subsong range from songdb for hash %s uri %s\n", hash.c_str(), uri);
        const auto &minmax = subsongs.value();
        update_tuple_subsong_range(tuple, minmax.first, minmax.second);
        return true;
    }
#endif
    bool needfix = subsong >= 0 && string(uri).find_last_of("?") == string::npos;
    if (needfix) {
        subsong = applySubsongHack(subsong, uri, hash, aud_get_bool(nullptr, "slow_probe"), "uade_plugin_read_tag");
        if (subsong < 0)
            return false;
    }    

    const auto playback_file = aud_drct_get_filename();
    const auto for_playback = playback_file && string(playback_file) == string(uri);

    const auto info = parse_info(file, path, hash);
    if (!info) {
        WARN("uade_plugin_read_tag could not parse module %s\n", uri);
        return false;
    }
    TRACE("uade_plugin_read_tag path %s format %s minsubsong %d maxsubsong %d channels %d\n", path.c_str(), info->format.c_str(), info->minsubsong, info->maxsubsong, info->channels);
    if (subsong < 0) {
        update_tuple_subsong_range(tuple, info->minsubsong, info->maxsubsong);
    } else {
        bool has_db_entry = update_tuple(tuple, path, subsong, info.value(), hash);
        const bool do_precalc = !has_db_entry && !for_playback &&
            tuple.get_int(Tuple::Length) <= 0 && aud_get_bool(PLUGIN_NAME, PRECALC_SONGLENGTHS);
        if (do_precalc) {
            const auto &songend = precalc_song_end(info->player, file, path, hash, subsong);
            update_tuple_song_end(tuple, songend, info->format);
            // update songdb (runtime only) so next read_tag call doesn't precalc again
            songdb::update(hash, songdb::SubSongInfo{static_cast<uint8_t>(subsong), {songend.status, songend.length}}, info->minsubsong, info->maxsubsong);
            songdb::update(hash, songdb::ModInfo{info->format, static_cast<uint8_t>(info->channels)});
        }
    }
    return true;
}

bool UADEPlugin::play(const char *uri, VFSFile &file) {
    TRACE("uade_plugin_play %s\n", uri);

    Tuple tuple = get_playback_tuple();

    // skip play if known invalid file
    int known_timeout = tuple.get_int(Tuple::Length);
    const auto comment = tuple.get_str(Tuple::Comment);
    if (known_timeout <= 0 && comment && common::starts_with(string(comment), "songend=")) {
        DEBUG("uade_plugin_play skipped known invalid file/subsong %s\n", uri);
        return true;
    }

    string path, ext;
    int subsong = parse_uri(uri, path, ext);

    bool needfix = subsong < 0 || (subsong >= 0 && string(uri).find_last_of("?") == string::npos);
    if (needfix) {
        const string hash = xxh32hash(file);
        subsong = applySubsongHack(subsong, uri, hash, aud_get_bool(nullptr, "slow_probe"), "uade_plugin_play");
        if (subsong < 0)
            return false;
    }
    
    int frequency = aud_get_int(PLUGIN_NAME, "frequency");

    const auto players = check_player(file, path, true);
    if (players.empty()) {
        ERR("uade_plugin_play no player found for %s\n", uri);
        return false;
    }
    const Index<char> buf = read_all(file);
    auto player = players.front();
    if (players.size() > 1) {
        for (const auto &p : players) {
            if (player::parse(path.c_str(), buf.begin(), buf.len(), p)) {
                player = p;
                break;
            }
        }
    }
    const player::PlayerConfig player_config = {player, frequency, known_timeout};
    const auto uade_config = get_uade_config(player_config);
    const auto it2play_config = get_it2play_config(player_config);
    const auto libopenmpt_config = get_libopenmpt_config(player_config);
    const auto libxmp_config = get_libxmp_config(player_config);
    const auto &config =
        player == player::Player::uade ? uade_config :
        player == player::Player::it2play ? it2play_config :
        player == player::Player::libopenmpt ? libopenmpt_config :
        player == player::Player::libxmp ? libxmp_config :
        player_config;
    
    const auto check_stop_ = []() { return check_stop(); };
    const auto check_seek_ = []() { return check_seek(); };
    const auto write_audio_ = [](char *buf, int bytes) { write_audio(buf, bytes); };

    auto state = player::play(path.c_str(), buf.begin(), buf.len(), subsong, config);
    if (!state) {
        ERR("Could not play %s", uri);
        return false;
    }

    // XXX FMT_S16_NE does not seem to work on big endian
    open_audio(endian::native == endian::big ? FMT_S16_BE : FMT_S16_LE, state->frequency, 2);

    const auto res = player::support::playback_loop(state.value(), config, check_stop_, check_seek_, write_audio_);

    if (!player::stop(state.value())) {
        WARN("Could not stop %s\n", uri);
    }

    if (res.songend.status == common::SongEnd::ERROR) {
        ERR("Error playing %s\n", uri);
        return false;
    }

    TRACE("Playback status for %s - %d\n", uri, res.songend.status);

    if (known_timeout <= 0 && !res.stopped && !res.seeked) {
        update_tuple_song_end(tuple, res.songend, {});
        set_playback_tuple(tuple.ref());
    }

    return true;
}

