// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <cassert>
#include <cstring>
#include <optional>
#include <string>

#include <libaudcore/audstrings.h>
#include <libaudcore/drct.h>
#include <libaudcore/i18n.h>
#include <libaudcore/plugin.h>
#include <libaudcore/runtime.h>
#include <libaudcore/vfs.h>

extern "C" {
#include "../uade/src/frontends/include/uade/options.h"
}

#include "config.h"
#include "common/common.h"
#include "common/extensions.h"
#include "common/logger.h"
#include "common/md5.h"
#include "prefs.h"
#include "player/player.h"
#include "songend/precalc.h"
#include "songdb/songdb.h"

using namespace std;

namespace {

struct Info {
    player::Player player;
    std::string format;
    int channels;
    int minsubsong;
    int maxsubsong;
};

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

string md5hex(VFSFile &file) {
    const Index<char> buf = read_all(file);
    MD5 md5; md5.update((const unsigned char*)buf.begin(), buf.len()); md5.finalize();
    return md5.hexdigest();
}

player::Player check_player(VFSFile &file, const string &path) {
    fseek0(file);
    // note: this only reads 256k or so, see read_all(file) above to read full file
    const Index<char> buf = file.read_all();
    return player::check(path.c_str(), buf.begin(), buf.len());
}

int parse_uri(const char *uri, string &path, string &ext) {
    int subsong;
    const char *sub, *tmpExt;
    const char *tmpPath = uri_to_filename(uri);

    uri_parse(tmpPath, nullptr, &tmpExt, &sub, &subsong);

    path = string(tmpPath, strlen(tmpPath) - strlen(sub));
    ext = string(tmpExt, strlen(tmpExt) - strlen(sub));

    return strlen(sub) > 0 ? subsong : -1;
}

void update_tuple_song_end(Tuple &tuple, const common::SongEnd &songend, const string &format) {
    const auto status = songend.status;
    const auto comment = "songend=" + songend.status_string();
    tuple.set_str(Tuple::Comment, comment.c_str());
    if (songend.length > 0 && status != common::SongEnd::NOSOUND &&
        (status != common::SongEnd::ERROR || songend::precalc::allow_songend_error(format))) {
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

void update_tuple_songdb(Tuple &tuple, const string &path, const songdb::SongInfo &songinfo, const Info &info, const string &md5) {
    TRACE("Found songlength data for %s:%d %s, length:%d, status:%s\n", md5.c_str(), songinfo.subsong, path.c_str(), songinfo.songlength, songinfo.songend.c_str());
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

    set_int(Tuple::Length, songinfo.songlength);
    set_str(Tuple::Comment, "songend="+songinfo.songend);

    if (!songinfo.demozoo_data && !songinfo.modland_data && !songinfo.amp_data && !songinfo.unexotica_data) {
        TRACE("No Demozoo/Modland/UnExotica/AMP data for %s %s\n", md5.c_str(), path.c_str());
        return;
    }

    if (songinfo.unexotica_data) {
        const auto &data = songinfo.unexotica_data.value();
        TRACE("Found UnExotica data for %s:%d %s, author:%s, album:%s, publisher:%s, year:%d\n", md5.c_str(), songinfo.subsong, path.c_str(), data.author.c_str(), data.album.c_str(), data.publisher.c_str(), data.year);
        //set_author(data.author); // prefer other sources
        set_str(Tuple::Album, data.album);
#if AUDACIOUS_HAS_PUBLISHER
// since Audacious 4.3
        set_str(Tuple::Publisher, data.publisher);
#else
        set_str(Tuple::Copyright, data.publisher);
#endif
        set_int(Tuple::Year, data.year);
    }
    if (songinfo.demozoo_data) {
        const auto &data = songinfo.demozoo_data.value();
        TRACE("Found Demozoo data for %s:%d %s, author:%s, album:%s, publisher:%s, year:%d\n", md5.c_str(), songinfo.subsong, path.c_str(), data.author.c_str(), data.album.c_str(), data.publisher.c_str(), data.year);
        //set_author(data.author); // prefer other sources
        set_str(Tuple::Album, data.album);
#if AUDACIOUS_HAS_PUBLISHER
// since Audacious 4.3
        set_str(Tuple::Publisher, data.publisher);
#else
        set_str(Tuple::Copyright, data.publisher);
#endif
        set_int(Tuple::Year, data.year);
    }

    const auto set_author = [&tuple](const string &author) {
        if ((!tuple.is_set(Tuple::Artist) && !author.empty())) {
            tuple.set_str(Tuple::Artist, author.c_str());
        } else {
            const char *oldartist = tuple.get_str(Tuple::Artist);
            const int oldcount = oldartist ? common::split(oldartist, " & ").size() : 0;
            const int newcount = common::split(author, " & ").size();
            if ((!oldartist || oldartist == songdb::UNKNOWN_AUTHOR) && !author.empty() && author != songdb::UNKNOWN_AUTHOR) {
                tuple.set_str(Tuple::Artist, author.c_str());
            } else if (!author.empty() && author != songdb::UNKNOWN_AUTHOR && newcount > oldcount) {
                tuple.set_str(Tuple::Artist, author.c_str());
            }
        }
    };

    auto tokens = common::split(path,"/");
    reverse(tokens.begin(), tokens.end());

    const auto has_author = [&tokens](const string& author, const int max_tokens) {
        const auto authors = common::split(author, " & ");
        const auto end = min(tokens.begin() + max_tokens, tokens.end());
        for (const auto &author : authors) {
            if (find(tokens.begin(), end, author) != end) {
                return true;
            }
        }
        return false;
    };

    bool modland_path = false;
    if (songinfo.modland_data) {
        const auto &data = songinfo.modland_data.value();
        TRACE("Found Modland data for %s:%d %s, author:%s, album:%s\n", md5.c_str(), songinfo.subsong, path.c_str(), data.author.c_str(), data.album.c_str());
        set_str(Tuple::Album, data.album);
        modland_path = has_author(data.author, 4);
    }
    bool amp_path = false;
    if (songinfo.amp_data) {
        const auto &data = songinfo.amp_data.value();
        TRACE("Found AMP data for %s:%d %s, author:%s\n", md5.c_str(), songinfo.subsong, path.c_str(), data.author.c_str());
        amp_path = has_author(data.author, 2);
    }
    bool unexotica_path = false;
    if (songinfo.unexotica_data) {
        const auto &data = songinfo.unexotica_data.value();
        const auto author_path = songdb::unexotica::author_path(data.author);
        unexotica_path = has_author(author_path, 4);
    }
    if (modland_path) {
        set_author(songinfo.modland_data->author);
    } else if (amp_path) {
        set_author(songinfo.amp_data->author);
    } else if (unexotica_path) {
        set_author(songinfo.unexotica_data->author);
    } else {
        if (songinfo.modland_data) set_author(songinfo.modland_data->author);
        if (songinfo.amp_data) set_author(songinfo.amp_data->author);
        if (songinfo.demozoo_data) set_author(songinfo.demozoo_data->author);
        if (songinfo.unexotica_data) set_author(songinfo.unexotica_data->author);
    }
}

bool update_tuple(Tuple &tuple, const string &path, int subsong, const Info &info, const string &modulemd5) {
    /* prefer filename as title for now, maybe make configurable in future
    bool modulename_ok = strnlen(info->modulename, 256) > 0 &&
                            !is_blacklisted_title(info);
    const char *title = modulename_ok ? info->modulename : name;
    */
    tuple.set_str(Tuple::Title, common::split(path, "/").back().c_str());
    if (info.format.size()) {
        tuple.set_str(Tuple::Codec, info.format.c_str());
    }
    tuple.set_str(Tuple::Quality, "sequenced");
    if (info.channels > 0) {
        tuple.set_int(Tuple::Channels, info.channels);
    }
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
        const auto songinfo = songdb::lookup(modulemd5.c_str(), subsong);
        if (songinfo) {
            update_tuple_songdb(tuple, path, songinfo.value(), info, modulemd5);
            return true;
        } else {
            TRACE("No songlength data for %s %s\n", modulemd5.c_str(), path.c_str());
        }
    }
    return false;
}

common::SongEnd precalc_song_end(
    VFSFile &file,
    const string &path,
    const string &md5,
    const int subsong
) {
    const Index<char> buf = read_all(file);
    const auto modinfo = player::parse(path.c_str(), buf.begin(), buf.len());
    if (!modinfo) {
        return { common::SongEnd::ERROR, 0 };
    }
    return songend::precalc::precalc_song_end(modinfo.value(), buf.begin(), buf.len(), subsong, md5);
};

optional<Info> parse_info(VFSFile &file, const string &path, const string &md5, const int subsong) {
    const auto &infos = songdb::lookup_all(md5);
    if (infos.size() > 0) {
        const auto player = check_player(file, path);
        const auto &info = infos.front();
        const auto minsubsong = info.subsong;
        const auto maxsubsong = infos.back().subsong;
        return Info {
            player,
            info.format,
            info.channels,
            minsubsong,
            maxsubsong,
        };
    }
    const Index<char> buf = read_all(file);
    const auto modinfo = player::parse(path.c_str(), buf.begin(), buf.len());
    if (modinfo) {
        return Info {
            modinfo->player,
            modinfo->format,
            modinfo->channels,
            modinfo->minsubsong,
            modinfo->maxsubsong,
        };
    }
    return {};
}

player::uade::Filter uade_filter(const int filter) {
    switch(filter) {
        case 2: return player::uade::Filter::A1200;
        case 3: return player::uade::Filter::NONE;
        default: return player::uade::Filter::A500;
    }
}

player::uade::Resampler uade_resampler(const int resampler) {
    switch(resampler) {
        case 1: return player::uade::Resampler::SINC;
        case 2: return player::uade::Resampler::NONE;
        default: return player::uade::Resampler::DEFAULT;
    }
}

player::uade::UADEConfig get_uade_config(int frequency, int known_timeout) {
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

    player::uade::UADEConfig conf;
    conf.frequency = frequency;
    conf.probe = false;
    conf.known_timeout = known_timeout;
    conf.player = player::Player::uade;

    conf.filter = uade_filter(filter);
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

} // namespace {}

class UADEPlugin : public InputPlugin
{

public:
    static constexpr PluginInfo info = {
        "UADE Plugin",
        "audacious-uade",
        "Audacious UADE plugin " PACKAGE_VERSION "\n"
        "Written by Matti Tiainen <mvtiaine@cc.hut.fi>\n"
        PACKAGE_URL"\n"
        "\n"
        "UADE: https://zakalwe.fi/uade/\n"
        "\n"
        "Using bundled libuade " UADE_VERSION "\n"
        "\n"
        "Simplistic Binary Streams 1.0.3 (MIT)\n"
        "Copyright (C) 2014-2019, Wong Shao Voon\n"
        "\n"
        "HivelyTracker 1.9 (BSD-3-Clause)\n"
        "Copyright (c) 2006-2018, Pete Gordon\n"
        "\n"
        "libdigibooster3 1.2 (BSD-2-Clause)\n"
        "Copyright (c) 2014, Grzegorz Kraszewski\n",
        &plugin_prefs
    };

    constexpr UADEPlugin() : InputPlugin(info, InputInfo(FlagSubtunes)
        .with_priority(_AUD_PLUGIN_DEFAULT_PRIO - 1) // preempt openmpt, modplug, mpg123 etc. plugins (mpg123 has many false positives)
        .with_exts(common::plugin_extensions)
        .with_mimes(common::plugin_mimes)) {}

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

    if (songdb::blacklist::is_blacklisted_extension(path, ext)) {
        TRACE("uade_plugin_is_our_file blacklisted %s\n", uri);
        return false;
    }
    
    if (check_player(file, path) != player::Player::NONE) {
        TRACE("uade_plugin_is_our_file accepted %s\n", uri);
        return true;
    }

    TRACE("uade_plugin_is_our_file rejected %s\n", uri);
    return false;
}

bool UADEPlugin::read_tag(const char *uri, VFSFile & file, Tuple &tuple, Index<char> *image) {
    TRACE("uade_plugin_read_tag %s\n", uri);
    string path, ext;
    int subsong = parse_uri(uri, path, ext);

    const string &md5 = md5hex(file);

    // add to playlist, but call uade_play() on-demand (may hang UADE/audacious completely)
    if (songdb::blacklist::is_blacklisted_md5(md5)) {
        DEBUG("uade_plugin_read_tag blacklisted md5 %s\n", uri);
        return true;
    }

    // try read subsongs directly from songdb
    const auto &subsongs = songdb::subsong_range(md5);
    if (subsongs && subsong < 0) {
        TRACE("uade_plugin_read_tag read subsong range from songdb for md5 %s uri %s\n", md5.c_str(), uri);
        const auto &minmax = subsongs.value();
        update_tuple_subsong_range(tuple, minmax.first, minmax.second);
        return true;
    }

    // hack for files which actually contain ? in their name, e.g. MOD.louzy-house?2 or MOD.how low can we go?1
    // which conflicts with audacious subsong uri scheme
    bool needfix = subsong >= 0 && string(uri).find_last_of("?") == string::npos;
    if (needfix) {
        const auto &subsongs = songdb::subsong_range(md5);
        if (subsongs && subsongs->first == subsongs->second) {
            WARN("uade_plugin_read_tag enforced subsong %d (was %d) for %s\n", subsongs->first, subsong, uri);
            subsong = subsongs->first;
        } else {
            WARN("uade_plugin_read_tag could not determine subsong for %s\n", uri);
        }
    }

    const auto playback_file = aud_drct_get_filename();
    const auto for_playback = playback_file && string(playback_file) == string(uri);

    const auto info = parse_info(file, path, md5, subsong);
    if (!info) {
        WARN("uade_plugin_read_tag could not parse module %s\n", uri);
        return false;
    }
    TRACE("uade_plugin_read_tag path %s format %s minsubsong %d maxsubsong %d channels %d\n", path.c_str(), info->format.c_str(), info->minsubsong, info->maxsubsong, info->channels);
    if (subsong < 0) {
        update_tuple_subsong_range(tuple, info->minsubsong, info->maxsubsong);
    } else {
        bool has_db_entry = update_tuple(tuple, path, subsong, info.value(), md5);
        const bool do_precalc = !has_db_entry && !for_playback &&
            tuple.get_int(Tuple::Length) <= 0 && aud_get_bool(PLUGIN_NAME, PRECALC_SONGLENGTHS);
        if (do_precalc) {
            const auto &songend = precalc_song_end(file, path, md5, subsong);
            update_tuple_song_end(tuple, songend, info->format);
            // update songdb (runtime only) so next read_tag call doesn't precalc again
            songdb::update(md5, subsong, songend.length, songend.status, info->format, info->channels);
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
    if (known_timeout <= 0 && comment && string(comment).starts_with("songend=")) {
        WARN("uade_plugin_play skipped known invalid file %s\n", uri);
        return true;
    }

    string path, ext;
    int subsong = parse_uri(uri, path, ext);

    // hack for files which actually contain ? in their name, e.g. MOD.louzy-house?2 or MOD.how low can we go?1
    // which conflicts with audacious subsong uri scheme
    bool needfix = subsong >= 0 && string(uri).find_last_of("?") == string::npos;
    if (needfix) {
        const string &md5 = md5hex(file);
        const auto &subsongs = songdb::subsong_range(md5);
        if (subsongs && subsongs->first == subsongs->second) {
            WARN("uade_plugin_play enforced subsong %d (was %d) for %s\n", subsongs->first, subsong, uri);
            subsong = subsongs->first;
        } else {
            WARN("uade_plugin_play could not determine subsong for %s\n", uri);
        }
    }
    
    int frequency = aud_get_int(PLUGIN_NAME, "frequency");

    const auto player = check_player(file, path);
    const auto uade_config = get_uade_config(frequency, known_timeout);
    const player::PlayerConfig player_config = {frequency, known_timeout};
    const auto &config = player == player::Player::uade ? uade_config : player_config;
    
    const auto check_stop_ = []() { return check_stop(); };
    const auto check_seek_ = []() { return check_seek(); };
    const auto write_audio_ = [](char *buf, int bytes) { write_audio(buf, bytes); };

    const Index<char> buf = read_all(file);
    auto state = player::play(path.c_str(), buf.begin(), buf.len(), subsong, config);
    if (!state) {
        ERR("Could not play %s", uri);
        return false;
    }

    // XXX FMT_S16_NE does not seem to work on big endian
    open_audio(endian::native == endian::big ? FMT_S16_BE : FMT_S16_LE, frequency, 2);

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
        update_tuple_song_end(tuple, res.songend, state->info.format);
        set_playback_tuple(tuple.ref());
    }

    return true;
}

