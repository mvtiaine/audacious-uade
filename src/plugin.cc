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

#include "config.h"
extern "C" {
#include "../uade/src/frontends/include/uade/options.h"
#include "../uade/src/frontends/include/uade/uadeconfstructure.h"
#include "../uade/src/frontends/include/uade/uade.h"
}
#include "common.h"
#include "extensions.h"
#include "hacks.h"
#include "songdb.h"
#include "prefs.h"
#include "uade_common.h"
#include "uade_config.h"
#include "converter/converter.h"
#include "player/player.h"
#include "songend/songend.h"
#include "3rdparty/md5.h"

using namespace std;

namespace {

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

bool is_our_file_uade(VFSFile &file, const string &fname, uade_state *state) {
    fseek0(file);
    const Index<char> buf = file.read_all();
    return uade_is_our_file_from_buffer(fname.c_str(), buf.begin(), buf.len(), state) != 0;
}

bool is_our_file_player(VFSFile &file) {
    char magic[player::MAGIC_SIZE];
    fseek0(file);
    if (file.fread(magic, 1, player::MAGIC_SIZE) < player::MAGIC_SIZE)  {
        WARN("uade_plugin could not read player magic for %s\n", file.filename());
        return false;
    }
    return player::is_our_file(magic, player::MAGIC_SIZE);
}

int parse_uri(const char *uri, string &path, string &name, string &ext) {
    int subsong;
    const char *tmpName, *sub, *tmpExt;
    const char *tmpPath = uri_to_filename(uri);

    uri_parse(tmpPath, &tmpName, &tmpExt, &sub, &subsong);

    path = string(tmpPath, strlen(tmpPath) - strlen(sub));
    name = string(tmpName, strlen(tmpName) - strlen(sub));
    ext = string(tmpExt, strlen(tmpExt) - strlen(sub));

    return strlen(sub) > 0 ? subsong : -1;
}

void update_tuple_song_end(Tuple &tuple, const song_end &song_end, const struct uade_song_info* info) {
    const auto status = song_end.status;
    const auto comment = "songend=" + song_end.status_string();
    tuple.set_str(Tuple::Comment, comment.c_str());
    if (song_end.length > 0 && status != song_end::NOSOUND && (status != song_end::ERROR || allow_songend_error(info))) {
        tuple.set_int(Tuple::Length, song_end.length);
    }
}

void update_tuple_song_end_player(Tuple &tuple, const song_end &song_end) {
    const auto status = song_end.status;
    const auto comment = "songend=" + song_end.status_string();
    tuple.set_str(Tuple::Comment, comment.c_str());
    if (song_end.length > 0 && status != song_end::NOSOUND && status != song_end::ERROR) {
        tuple.set_int(Tuple::Length, song_end.length);
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

// TODO formatname to songdb
void update_tuple_songdb(Tuple &tuple, const SongInfo &songinfo, const string &name, const string &formatname) {
    TRACE("Found songlength data for %s:%d %s, length:%d, status:%s\n", songinfo.md5.c_str(), songinfo.subsong, name.c_str(), songinfo.length, songinfo.status.c_str());
    if (songinfo.length > 0)
        tuple.set_int(Tuple::Length, songinfo.length);
    if (songinfo.status.size())
        tuple.set_str(Tuple::Comment, ("songend="+songinfo.status).c_str());
    if (songinfo.modland_data.has_value()) {
        const auto data = songinfo.modland_data.value();
        TRACE("Found Modland data for %s:%d %s, format:%s, author:%s, album:%s, filename:%s\n", songinfo.md5.c_str(), songinfo.subsong, name.c_str(), data.format.c_str(), data.author.c_str(), data.album.c_str(), data.filename.c_str());
        if (!data.author.empty())
            tuple.set_str(Tuple::Artist, data.author.c_str());
        // prefer UADE codec names, but fall back to modland if not available
        if (string_view(formatname) == UNKNOWN_CODEC)
            tuple.set_str(Tuple::Codec, data.format.c_str());
        if (!data.album.empty())
            tuple.set_str(Tuple::Album, data.album.c_str());
    } else if (songinfo.amp_data.has_value()) {
        const auto data = songinfo.amp_data.value();
        TRACE("Found AMP data for %s:%d %s, author:%s, filename:%s\n", songinfo.md5.c_str(), songinfo.subsong, name.c_str(), data.author.c_str(), data.filename.c_str());
        if (!data.author.empty())
            tuple.set_str(Tuple::Artist, data.author.c_str());
    } else if (songinfo.unexotica_data.has_value()) {
        const auto data = songinfo.unexotica_data.value();
        TRACE("Found UnExotica data for %s:%d %s, author:%s, album:%s, filename:%s\n", songinfo.md5.c_str(), songinfo.subsong, name.c_str(),  data.author.c_str(), data.album.c_str(), data.filename.c_str());
        if (!data.author.empty())
            tuple.set_str(Tuple::Artist, data.author.c_str());
        if (!data.album.empty())
            tuple.set_str(Tuple::Album, data.album.c_str());
    } else {
        TRACE("No Modland/AMP/UnExotica data for %s %s\n", songinfo.md5.c_str(), name.c_str());
    }
}

bool update_tuple_player(Tuple &tuple, const string &name, int subsong, const player::ModuleInfo &info, const string &modulemd5) {
    tuple.set_str(Tuple::Title, name.c_str()); 
    tuple.set_str(Tuple::Codec, info.format.c_str());
    tuple.set_str(Tuple::Quality, "sequenced");
    if (info.channels > 0) {
        tuple.set_int(Tuple::Channels, info.channels);
    }
    // initial probe
    if (subsong == -1) {
        update_tuple_subsong_range(tuple, 0, info.maxsubsong);
    } else {
        if (info.maxsubsong > 0) {
            tuple.set_int(Tuple::NumSubtunes, info.maxsubsong + 1);
            tuple.set_int(Tuple::Subtune, subsong);
            tuple.set_int(Tuple::Track, subsong);
        }
        const auto songinfo = songdb_lookup(modulemd5.c_str(), subsong, info.fname);
        if (songinfo.has_value()) {
            update_tuple_songdb(tuple, songinfo.value(), name, info.format);
            return true;
        } else {
            TRACE("No songlength data for %s %s\n", modulemd5.c_str(), name.c_str());
        }
    }
    return false;
}

bool update_tuple(Tuple &tuple, const string &name, int subsong, const struct uade_song_info* info,
                  const string &modulemd5, const string &formatname) {
    /* prefer filename as title for now, maybe make configurable in future
    bool modulename_ok = strnlen(info->modulename, 256) > 0 &&
                            !is_blacklisted_title(info);
    const char *title = modulename_ok ? info->modulename : name;
    */
    tuple.set_str(Tuple::Title, name.c_str());

    tuple.set_str(Tuple::Codec, formatname.c_str());
    tuple.set_str(Tuple::Quality, "sequenced");
    //tuple.set_int(Tuple::Channels, 4); // TODO multichannel support

    // avoid uade_subsong_control: Assertion `subsong >= 0 && subsong < 256' failed.
    const int subsongs = min(info->subsongs.max - info->subsongs.min + 1, 255);

    // initial probe
    if (subsong == -1) {
        update_tuple_subsong_range(tuple, info->subsongs.min, min(255, info->subsongs.max));
    } else {
        if (subsongs > 1) {
            tuple.set_int(Tuple::NumSubtunes, subsongs);
            tuple.set_int(Tuple::Subtune, subsong);
            tuple.set_int(Tuple::Track, subsong);
        }
        const auto songinfo = songdb_lookup(modulemd5.c_str(), subsong, info->modulefname);
        if (songinfo.has_value()) {
            update_tuple_songdb(tuple, songinfo.value(), name, formatname);
            return true;
        } else {
            TRACE("No songlength data for %s %s\n", modulemd5.c_str(), name.c_str());
        }
        // UADE contentdb doesn't support separate lengths for subsongs
        if (subsongs == 1 && info->duration > 0 && tuple.get_int(Tuple::Length) <= 0) {
            tuple.set_str(Tuple::Comment, "songend=contentdb");
            tuple.set_int(Tuple::Length, info->duration * 1000);
            return false;
        }
    }
    return false;
}

bool needs_conversion(VFSFile &file) {
    char magic[converter::MAGIC_SIZE];
    fseek0(file);
    if (file.fread(magic, 1, converter::MAGIC_SIZE) < converter::MAGIC_SIZE)  {
        WARN("uade_plugin could not read converter magic for %s\n", file.filename());
        return false;
    }
    return converter::needs_conversion(magic, converter::MAGIC_SIZE);
}

int play_uade(
    const char *uri,
    VFSFile &file,
    const string &path,
    const string &name,
    int subsong,
    uade_state *state,
    optional<string> &formatname
) {
    if (needs_conversion(file)) {
        const Index<char> buf = read_all(file);
        auto res = converter::convert(buf.begin(), buf.len());
        if (res.success) {
            formatname = res.format;
            DEBUG("uade_plugin converted %s to format %s\n", uri, res.format.c_str());
            return uade_play_from_buffer(name.c_str(), res.data.data(), res.data.size(), subsong, state);
        } else {
            WARN("uade_plugin conversion failed for %s reason: %s\n", uri, res.reason_failed.c_str());
            return 0;
        }
    } else {
        return uade_play(path.c_str(), subsong, state);
    }
};

optional<player::PlayerState> play_player(
    VFSFile &file,
    const string &fname,
    int subsong,
    int frequency
) {
    const Index<char> buf = read_all(file);
    return player::play(fname.c_str(), buf.begin(), buf.len(), subsong, frequency);
};

void stop_uade(probe_state *probe_state, const char *uri) {
    TRACE("stop_uade id %d - %s\n", probe_state->id, uri);
    if (uade_stop(probe_state->state)) {
        WARN("uade_stop failed for id %d - %s\n", probe_state->id, uri);
        cleanup_uade_state(probe_state->state, probe_state->id, uri);
        probe_state->state = create_uade_probe_state();
    }
}

const optional<player::ModuleInfo> player_parse(VFSFile &file, const string &fname) {
    const Index<char> buf = read_all(file);
    return player::parse(fname.c_str(), buf.begin(), buf.len());
}

const struct uade_song_info *get_song_info(const uade_state *state) {
    const struct uade_song_info *info = uade_get_song_info(state);
#if DEBUG_TRACE
    const uade_subsong_info *subsong = &info->subsongs;
    const uade_detection_info *detection = &info->detectioninfo;
    char infotext[16384];
    TRACE("uade_song_info " 
          " - subsong cur:%d, min:%d, def:%d, max:%d"
          " - detection custom:%d, content:%d, ext:%s"
          " - module bytes:%lu, md5:%s, fname:%s, name:%s"
          " - duration:%f, subsongbytes:%zd, songbytes:%zd"
          " - player fname:%s, name:%s, format:%s"
          "\n",
          subsong->cur, subsong->min, subsong->def, subsong->max,
          detection->custom, detection->content, detection->ext,
          info->modulebytes, info->modulemd5, info->modulefname, info->modulename,
          info->duration, (ssize_t)info->subsongbytes, (ssize_t)info->songbytes,
          info->playerfname, info->playername, info->formatname);
    if (!uade_song_info(infotext, size(infotext), info->modulefname, UADE_MODULE_INFO)) {
        TRACE("uade_song_info - module info:\n%s\n", infotext);
    } else {
        TRACE("uade_song_info - no module info\n");
    }
#endif
    return info;
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
        "Simplistic Binary Streams 1.0.3\n"
        "Copyright (C) 2014-2019, Wong Shao Voon\n"
        "https://opensource.org/licenses/MIT\n"
        "\n"
        "HivelyTracker 1.9\n"
        "Copyright (c) 2006-2018, Pete Gordon\n"
        "https://opensource.org/license/bsd-3-clause\n"
        "\n"
        "libdigibooster3 1.2\n"
        "Copyright (c) 2014, Grzegorz Kraszewski\n"
        "https://opensource.org/license/bsd-2-clause\n",
        &plugin_prefs
    };

    constexpr UADEPlugin() : InputPlugin(info, InputInfo(FlagSubtunes)
        .with_priority(_AUD_PLUGIN_DEFAULT_PRIO - 1) // preempt openmpt, modplug, mpg123 etc. plugins (mpg123 has many false positives)
        .with_exts(plugin_extensions)
        .with_mimes(plugin_mimes)) {}

    bool init();
    void cleanup();

    bool is_our_file(const char *uri, VFSFile &file);
    bool read_tag(const char *uri, VFSFile &file, Tuple &tuple, Index<char> *image);
    bool play(const char *uri, VFSFile &file);

private:
    pair<song_end,bool> playback_loop(uade_state* state, int timeout);
    pair<song_end,bool> playback_loop_player(player::PlayerState &state, int timeout);

}; // class UADEPlugin

#define EXPORT __attribute__((visibility("default")))
EXPORT UADEPlugin aud_plugin_instance;

bool UADEPlugin::init() {
    DEBUG("uade_plugin_init\n");
    aud_config_set_defaults (PLUGIN_NAME, plugin_defaults);
    aud_config_set_defaults (PLUGIN_NAME, uade_defaults);
    for (int i = 0; i < MAX_PROBES; ++i) {
        probes[i] = {};
    }
    player::init();
    songdb_init();
    return true;
}

void UADEPlugin::cleanup() {
    DEBUG("uade_plugin_cleanup\n");
    for (int i = 0; i < MAX_PROBES; ++i) {
        if (probes[i].state != nullptr) {
            cleanup_uade_state(probes[i].state, probes[i].id, "");
        }
    }
}

bool UADEPlugin::is_our_file(const char *uri, VFSFile &file) {
    TRACE("uade_plugin_is_our_file %s\n", uri);

    bool is_our_file = false;
    string path, name, ext;

    parse_uri(uri, path, name, ext);

    if (songdb_exists(name, file.fsize())) {
        TRACE("uade_plugin_is_our_file accepted from songdb (%s,%zd) %s\n", name.c_str(), (ssize_t)file.fsize(), uri);
        return true;
    } else {
        //TRACE("uade_plugin_is_our_file NOT accepted from songdb (%s,%lld) %s\n", name.c_str(), file.fsize(), uri);
    }

    if (is_blacklisted_extension(name, ext)) {
        TRACE("uade_plugin_is_our_file blacklisted %s\n", uri);
        return false;
    }
    
    if (needs_conversion(file)) {
        DEBUG("uade_plugin_is_our_file needs conversion: %s\n", uri);
        // don't try uade_play yet
        return true;
    }

    if (is_our_file_player(file)) {
        TRACE("uade_plugin_is_our_file accepted (player) %s\n", uri);
        return true;
    }

    probe_state *probe_state = get_probe_state();
    TRACE("uade_plugin_is_our_file using probe id %d - %s\n", probe_state->id, uri);

    if (!is_our_file_uade(file, name, probe_state->state)) {
        TRACE("uade_plugin_is_our_file rejected %s\n", uri);
        release_probe_state(probe_state);
        return false;
    }

    const string &md5 = md5hex(file);

    // add to playlist, but call uade_play() on-demand (may hang UADE/audacious completely)
    if (is_blacklisted_md5(md5)) {
        DEBUG("uade_plugin_is_our_file blacklisted md5 %s\n", uri);
        release_probe_state(probe_state);
        return true;
    }

    // assume our file if found in songdb
    if (songdb_subsong_range(md5).has_value()) {
        TRACE("uade_plugin_is_our_file accepted (songdb) %s\n", uri);
        release_probe_state(probe_state);
        return true;
    }

    switch (uade_play(path.c_str(), -1, probe_state->state)) {
        case 1:
            TRACE("uade_plugin_is_our_file accepted %s\n", uri);
            stop_uade(probe_state, uri);
            is_our_file = true;
            break;
        case -1:
            WARN("uade_plugin_is_our_file fatal error on %s path %s\n", uri, path.c_str());
            cleanup_uade_state(probe_state->state, probe_state->id, uri);
            probe_state->state = create_uade_probe_state();
            break;
        default:
            DEBUG("uade_plugin_is_our_file cannot play %s\n", uri);
            stop_uade(probe_state, uri);
            break;
    }
    
    release_probe_state(probe_state);

    return is_our_file;
}

bool UADEPlugin::read_tag(const char *uri, VFSFile & file, Tuple &tuple, Index<char> *image) {
    TRACE("uade_plugin_read_tag %s\n", uri);

    string path, name, ext;
    optional<string> formatname;
    int subsong;
    bool success = false;

    subsong = parse_uri(uri, path, name, ext);

    const string &md5 = md5hex(file);

    // add to playlist, but call uade_play() on-demand (may hang UADE/audacious completely)
    if (is_blacklisted_md5(md5)) {
        DEBUG("uade_plugin_read_tag blacklisted md5 %s\n", uri);
        return true;
    }

    // try read subsongs directly from songdb
    const auto &subsongs = songdb_subsong_range(md5);
    if (subsongs.has_value() && subsong < 0) {
        TRACE("uade_plugin_read_tag read subsong range from songdb for md5 %s uri %s\n", md5.c_str(), uri);
        const auto &minmax = subsongs.value();
        update_tuple_subsong_range(tuple, minmax.first, minmax.second);
        return true;
    }

    // hack for files which actually contain ? in their name, e.g. MOD.louzy-house?2 or MOD.how low can we go?1
    // which conflicts with audacious subsong uri scheme
    bool needfix = subsong >= 0 && string(uri).find_last_of("?") == string::npos;
    if (needfix) {
        const string &md5 = md5hex(file);
        const auto &subsongs = songdb_subsong_range(md5);
        if (subsongs.has_value() && subsongs.value().first == subsongs.value().second) {
            WARN("uade_plugin_read_tag enforced subsong %d (was %d) for %s\n", subsongs.value().first, subsong, uri);
            subsong = subsongs.value().first;
            path = uri_to_filename(uri);
            name = split(path, "/").back();
        } else {
            WARN("uade_plugin_read_tag could not determine subsong for %s\n", uri);
        }
    }

    const auto playback_file = aud_drct_get_filename();
    const auto for_playback = playback_file && string(playback_file) == string(uri);

    // non-UADE
    if (is_our_file_player(file)) {
        const auto info = player_parse(file, path);
        if (!info.has_value()) {
            WARN("uade_plugin_read_tag could not parse module %s\n", uri);
            return false;
        }
        TRACE("uade_plugin_read_tag fname %s format %s maxsubsong %d channels %d\n", info->fname.c_str(), info->format.c_str(), info->maxsubsong, info->channels);
        if (subsong < 0) {
            update_tuple_subsong_range(tuple, 0, info->maxsubsong);
            return true;
        } else {
            bool has_db_entry = update_tuple_player(tuple, name, subsong, info.value(), md5);
            const bool do_precalc = !has_db_entry && !for_playback &&
                tuple.get_int(Tuple::Length) <= 0 && aud_get_bool(PLUGIN_NAME, PRECALC_SONGLENGTHS);
            if (do_precalc) {
                auto state = play_player(file, path, subsong, songend::PRECALC_FREQ_PLAYER);
                if (!state.has_value()) {
                    return false;
                }
                const auto songend = precalc_song_length_player(state.value(), path.c_str());
                update_tuple_song_end_player(tuple, songend);
                player::stop(state.value());
                // update songdb (runtime only) so next read_tag call doesn't precalc again
                const SongInfo info = { md5, subsong, songend.length, songend.status_string(), file.fsize()};
                songdb_update(info);
            }
            return true;
        }
    }

    // UADE
    probe_state *probe_state = get_probe_state();
    TRACE("uade_plugin_read_tag using probe id %d - %s\n", probe_state->id, uri);

    switch (play_uade(uri, file, path, name, subsong, probe_state->state, formatname)) {
        case 1: {
            const struct uade_song_info* uade_info = get_song_info(probe_state->state);
            const bool has_db_entry = update_tuple(tuple, name, subsong, uade_info, md5,
                formatname.has_value() ? formatname.value() : parse_codec(uade_info));
            const bool do_precalc = subsong != -1 && !has_db_entry && !for_playback &&
                tuple.get_int(Tuple::Length) <= 0 && aud_get_bool(PLUGIN_NAME, PRECALC_SONGLENGTHS);
            if (do_precalc) {
                const auto songend = precalc_song_length(probe_state->state, uade_info);
                update_tuple_song_end(tuple, songend, uade_info);
                if (songend.status == song_end::ERROR) {
                    cleanup_uade_state(probe_state->state, probe_state->id, uri);
                    probe_state->state = create_uade_probe_state();
                 } else {
                    stop_uade(probe_state, uri);
                }
                // update songdb (runtime only) so next read_tag call doesn't precalc again
                const SongInfo info = { md5, subsong, songend.length, songend.status_string(), file.fsize()};
                songdb_update(info);
            } else {
                stop_uade(probe_state, uri);
            }
            success = true;
            break;
        }
        case -1:
            WARN("uade_plugin_read_tag fatal error on %s path %s\n", uri, path.c_str());
            cleanup_uade_state(probe_state->state, probe_state->id, uri);
            probe_state->state = create_uade_probe_state();
            break;
        default:
            WARN("uade_plugin_read_tag cannot play %s\n", uri);
            stop_uade(probe_state, uri);
            break;
    }
    release_probe_state(probe_state);

    return success;
}

pair<song_end, bool> UADEPlugin::playback_loop(uade_state* state, int timeout) {
    char buffer[4096];
    song_end songend;
    songend.status = song_end::TIMEOUT;
    songend.length = timeout;
    bool stopped = false;
    bool seeked = false;
    const size_t bytespersec = UADE_BYTES_PER_FRAME * uade_get_sampling_rate(state);
    // UADE plays some mods for hours or possibly forever (with always_ends default)
    size_t maxbytes = timeout > 0 ? timeout * bytespersec / 1000 : songend::PRECALC_TIMEOUT * bytespersec;
    size_t totalbytes = 0;

    while (!(stopped = check_stop()) && totalbytes < maxbytes) {
        int seek_millis = check_seek();
        if (seek_millis >= 0) {
            seeked = true;
            if (uade_seek(UADE_SEEK_SUBSONG_RELATIVE, seek_millis / 1000.0, -1, state)) {
                ERR("Could not seek to %d\n", seek_millis);
            } else {
                DEBUG("Seek to %d\n", seek_millis);
                totalbytes = seek_millis * bytespersec / 1000;
            };
        }
        const auto res = render_audio(buffer, sizeof buffer, state);
        // ignore "tail bytes" to avoid pop in end of audio if song restarts
        // messing up with silence/volume trimming etc.
        if (res.second > 0 && (res.first == song_end::NONE || totalbytes == 0)) {
            write_audio(buffer, res.second);
            totalbytes += res.second;
        }
        if (res.first == song_end::ERROR) {
            ERR("Playback error.\n");
            songend.status = song_end::ERROR;
            break;
        } else if (res.first == song_end::TIMEOUT) {
            TRACE("Song end (timeout).\n");
            songend.status = song_end::TIMEOUT;
            break;
        } else if (res.first == song_end::UADE_SILENCE) {
            TRACE("Song end (silence).\n");
            songend.status = song_end::UADE_SILENCE;
            break;
        } else if (res.first == song_end::PLAYER) {
            TRACE("Song end.\n");
            songend.status = song_end::PLAYER;
            break;
        }
    }
    if (stopped) {
        songend.status = song_end::STOP;
    } else if (songend.length <= 0 && songend.status == song_end::TIMEOUT) {
        songend.length = aud_get_int(PLUGIN_NAME, "subsong_timeout") * 1000;
    } else if (!seeked) {
        songend.length = totalbytes * 1000 / bytespersec;
    }
    if (songend.status == song_end::UADE_SILENCE) {
        int uade_silence = aud_get_int(PLUGIN_NAME, "silence_timeout") * 1000;
        songend.length -= uade_silence;
        if (uade_silence > songend::MAX_SILENCE) {
            songend.length += songend::MAX_SILENCE;
        }
    }
    return pair(songend, seeked);
}

pair<song_end, bool> UADEPlugin::playback_loop_player(player::PlayerState &state, int timeout) {
    char buffer[player::MIXBUFSIZE];
    song_end songend;
    songend.status = song_end::TIMEOUT;
    songend.length = timeout;
    bool stopped = false;
    bool seeked = false;
    const int frequency = aud_get_int(PLUGIN_NAME, "frequency");
    const size_t bytespersec = 4 * frequency;
    size_t maxbytes = timeout > 0 ? timeout * bytespersec / 1000 : songend::PRECALC_TIMEOUT * bytespersec;
    size_t totalbytes = 0;

    while (!(stopped = check_stop()) && totalbytes < maxbytes) {
        int seek_millis = check_seek();
        if (seek_millis >= 0) {
            seeked = true;
            DEBUG("Seek to %d\n", seek_millis);
            if (player::seek(state, seek_millis)) {
                songend.status = song_end::PLAYER;
                break;
            }
            totalbytes = seek_millis * bytespersec / 1000;
        }
        const auto res = render_audio_player(buffer, sizeof buffer, state);
        if (res.second > 0) {
            write_audio(buffer, res.second);
            totalbytes += res.second;
        }
        if (res.first == song_end::ERROR) {
            ERR("Playback error.\n");
            songend.status = song_end::ERROR;
            break;
        } else if (res.first == song_end::PLAYER) {
            TRACE("Song end.\n");
            songend.status = song_end::PLAYER;
            break;
        }
    }
    if (stopped) {
        songend.status = song_end::STOP;
    } else if (songend.length <= 0 && songend.status == song_end::TIMEOUT) {
        songend.length = aud_get_int(PLUGIN_NAME, "subsong_timeout") * 1000;
    } else if (!seeked) {
        songend.length = totalbytes * 1000 / bytespersec;
    }
    return pair(songend, seeked);
}

bool UADEPlugin::play(const char *uri, VFSFile &file) {
    TRACE("uade_plugin_play %s\n", uri);

    string path, name, ext;
    optional<string> formatname;
    int subsong, rate;
    bool ret = false;
    uade_state *state = nullptr;
    Tuple tuple = get_playback_tuple();

    // skip play if known invalid file
    int known_timeout = tuple.get_int(Tuple::Length);
    const auto comment = tuple.get_str(Tuple::Comment);
    if (known_timeout <= 0 && comment && string(comment).find("songend=") == 0) {
        return true;
    }

    subsong = parse_uri(uri, path, name, ext);

    // hack for files which actually contain ? in their name, e.g. MOD.louzy-house?2 or MOD.how low can we go?1
    // which conflicts with audacious subsong uri scheme
    bool needfix = subsong >= 0 && string(uri).find_last_of("?") == string::npos;
    if (needfix) {
        const string &md5 = md5hex(file);
        const auto &subsongs = songdb_subsong_range(md5);
        if (subsongs.has_value() && subsongs.value().first == subsongs.value().second) {
            WARN("uade_plugin_play enforced subsong %d (was %d) for %s\n", subsongs.value().first, subsong, uri);
            subsong = subsongs.value().first;
            path = uri_to_filename(uri);
            name = split(path, "/").back();
        } else {
            WARN("uade_plugin_play could not determine subsong for %s\n", uri);
        }
    }

    rate = aud_get_int(PLUGIN_NAME, "frequency");
    open_audio(FMT_S16_NE, rate, 2);

    // non-UADE
    if (is_our_file_player(file)) {
        auto state = play_player(file, path, subsong, rate);
        if (!state.has_value()) {
            return false;
        }
        const auto [songend, seeked] = playback_loop_player(state.value(), known_timeout);
        if (songend.status == song_end::ERROR) {
            ERR("Error playing %s\n", uri);
            ret = false;
        } else {
            TRACE("Playback status for %s - %d\n", uri, songend.status);
            ret = true;
        }
        if (known_timeout <= 0 && songend.status != song_end::STOP && !seeked) {
            update_tuple_song_end_player(tuple, songend);
            set_playback_tuple(tuple.ref());
        }
        player::stop(state.value());
        return ret;
    }

    // UADE
    state = create_uade_state(known_timeout);
    if (!state) {
        ERR("Could not init uade state\n");
        return false;
    }

    switch (play_uade(uri, file, path, name, subsong, state, formatname)) {
        case 1: {
            const auto [songend, seeked] = playback_loop(state, known_timeout);
            const auto *songinfo = get_song_info(state);
            if (songend.status == song_end::ERROR && !allow_songend_error(songinfo)) {
                ERR("Error playing %s\n", uri);
                ret = false;
            } else {
                TRACE("Playback status for %s - %d\n", uri, songend.status);
                ret = true;
            }
            if (known_timeout <= 0 && songend.status != song_end::STOP && !seeked) {
                update_tuple_song_end(tuple, songend, songinfo);
                set_playback_tuple(tuple.ref());
            }
            break;
        }
        default:
            ERR("Could not play %s\n", uri);
            ret = false;
            break;
    }
    cleanup_uade_state(state, -1, uri);
    
    return ret;
}

