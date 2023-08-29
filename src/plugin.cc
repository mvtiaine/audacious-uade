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
#if SYSTEM_LIBUADE
#include <uade/uade.h>
#else
#include "../uade/src/frontends/include/uade/options.h"
#include "../uade/src/frontends/include/uade/uadeconfstructure.h"
#include "../uade/src/frontends/include/uade/uade.h"
#endif

#include "common.h"
#include "extensions.h"
#include "hacks.h"
#include "songdb.h"
#include "prefs.h"
#include "uade_common.h"
#include "uade_config.h"
#include "converter/converter.h"
#include "songend/songend.h"
#include "3rdparty/md5.h"

using namespace std;

namespace {

void fseek0(VFSFile &file) {
    if (file.fseek(0, VFS_SEEK_SET)) {
        WARN("uade_plugin could not seek %s\n", file.filename());
    }
}

string md5hex(VFSFile &file) {
    fseek0(file);
    const auto &buf = file.read_all();
    MD5 md5; md5.update((const unsigned char*)buf.begin(), buf.len()); md5.finalize();
    return md5.hexdigest();
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
        // provide mappings to uade subsong numbers
        Index<short> subtunes;
        for (int i = 0; i < subsongs; ++i) {
            subtunes.append(info->subsongs.min + i);
        }
        tuple.set_subtunes(subtunes.len(), subtunes.begin());
    } else if (subsongs > 1) {
        // convert to playlist 1/x numbering
        int pl_subsong = subsong - info->subsongs.min + 1;
        tuple.set_int(Tuple::NumSubtunes, subsongs);
        tuple.set_int(Tuple::Subtune, pl_subsong);
        tuple.set_int(Tuple::Track, pl_subsong);
    }
    
    if (subsong != -1) {
        const auto songinfo = songdb_lookup(modulemd5.c_str(), subsong, name);
        if (songinfo.has_value()) {
            const auto si = songinfo.value();
            TRACE("Found songlength data for %s:%d %s, length:%d, status:%s\n", modulemd5.c_str(), subsong, name.c_str(), si.length, si.status.c_str());
            if (si.length > 0)
                tuple.set_int(Tuple::Length, si.length);
            if (si.status.size())
                tuple.set_str(Tuple::Comment, ("songend="+si.status).c_str());
            if (si.modland_data.has_value()) {
                const auto ml_data = si.modland_data.value();
                TRACE("Found modland data for %s:%d %s, format:%s, author:%s, album:%s, filename:%s\n", modulemd5.c_str(), subsong, name.c_str(), ml_data.format.c_str(), ml_data.author.c_str(), ml_data.album.c_str(), ml_data.filename.c_str());
                if (!ml_data.author.empty())
                    tuple.set_str(Tuple::Artist, ml_data.author.c_str());
                // prefer UADE codec names, but fall back to modland if not available
                if (string_view(formatname) == UNKNOWN_CODEC)
                    tuple.set_str(Tuple::Codec, ml_data.format.c_str());
                if (!ml_data.album.empty())
                    tuple.set_str(Tuple::Album, ml_data.album.c_str());
            } else {
                TRACE("No modland data for %s %s\n", modulemd5.c_str(), name.c_str());
            }
            return true;
        } else {
            TRACE("No songlength data for %s %s\n", modulemd5.c_str(), name.c_str());
        }
        // UADE contentdb doesn't support separate lengths for subsongs
        if (subsongs == 1 && info->duration > 0 && tuple.get_int(Tuple::Length) <= 0) {
            tuple.set_str(Tuple::Comment, "songend=contentdb");
            tuple.set_int(Tuple::Length, info->duration * 1000);
            return true;
        }
    }
    return false;
}

bool needs_conversion(const char *uri, VFSFile &file) {
    char magic[converter::MAGIC_SIZE];
    fseek0(file);
    if (file.fread(magic, 1, converter::MAGIC_SIZE) < converter::MAGIC_SIZE)  {
        WARN("uade_plugin could not read magic for %s\n", uri);
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
    if (needs_conversion(uri, file)) {
        fseek0(file);
        const auto &buf = file.read_all();
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

void stop_uade(probe_state *probe_state, const char *uri) {
    TRACE("stop_uade id %d - %s\n", probe_state->id, uri);
    if (uade_stop(probe_state->state)) {
        WARN("uade_stop failed for id %d - %s\n", probe_state->id, uri);
        cleanup_uade_state(probe_state->state, probe_state->id, uri);
        probe_state->state = create_uade_probe_state();
    }
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
          " - duration:%f, subsongbytes:%lld, songbytes:%lld"
          " - player fname:%s, name:%s, format:%s"
          "\n",
          subsong->cur, subsong->min, subsong->def, subsong->max,
          detection->custom, detection->content, detection->ext,
          info->modulebytes, info->modulemd5, info->modulefname, info->modulename,
          info->duration, info->subsongbytes, info->songbytes,
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
        "Audacious UADE plugin " VERSION "\n"
        "Written by Matti Tiainen <mvtiaine@cc.hut.fi>\n"
        "\n"
        "UADE: https://zakalwe.fi/uade/\n"
        "\n"
#if SYSTEM_LIBUADE
        "Using system libuade " UADE_VERSION "\n"
#else
        "Using bundled libuade " UADE_VERSION "\n"
#endif
        "\n"
        "Simplistic Binary Streams 1.0.3\n"
        "Copyright (C) 2014 - 2019\n"
        "by Wong Shao Voon (shaovoon@yahoo.com)\n"
        "https://opensource.org/licenses/MIT\n",
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

    if (is_blacklisted_extension(ext)) {
        DEBUG("uade_plugin_is_our_file blacklisted %s\n", uri);
        return false;
    }

    // add to playlist, but call uade_play() on-demand (may hang UADE/audacious completely)
    if (is_blacklisted_md5(md5hex(file))) {
        DEBUG("uade_plugin_is_our_file blacklisted md5 %s\n", uri);
        return true;
    }

    if (needs_conversion(uri, file)) {
        DEBUG("uade_plugin_is_our_file needs conversion: %s\n", uri);
        // don't try uade_play yet
        return true;
    }

    probe_state *probe_state = get_probe_state();
    TRACE("uade_plugin_is_our_file using probe id %d - %s\n", probe_state->id, uri);

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

    if (is_blacklisted_extension(ext)) {
        DEBUG("uade_plugin_read_tag blacklisted %s\n", uri);
        return false;
    }

    const string &md5 = md5hex(file);

    // add to playlist, but call uade_play() on-demand (may hang UADE/audacious completely)
    if (is_blacklisted_md5(md5)) {
        DEBUG("uade_plugin_read_tag blacklisted md5 %s\n", uri);
        return true;
    }

    const auto playback_file = aud_drct_get_filename();
    const auto for_playback = playback_file && string(playback_file) == string(uri);

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
                const SongInfo info = { md5, subsong, songend.length, songend.status_string()};
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

    while (!(stopped = check_stop()) && (totalbytes < maxbytes || seeked)) {
        int seek_value = check_seek();
        if (seek_value >= 0) {
            seeked = true;
            if (uade_seek(UADE_SEEK_SUBSONG_RELATIVE, seek_value / 1000.0, -1, state)) {
                ERROR("Could not seek to %d\n", seek_value);
            } else {
                DEBUG("Seek to %d\n", seek_value);
            };
        }
        const auto res = render_audio(buffer, sizeof buffer, state);
        if (res.second > 0) {
            write_audio(buffer, res.second);
            totalbytes += res.second;
        }
        if (res.first == song_end::ERROR) {
            ERROR("Playback error.\n");
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

    state = create_uade_state(known_timeout);
    if (!state) {
        ERROR("Could not init uade state\n");
        return false;
    }
    rate = uade_get_sampling_rate(state);

    open_audio(FMT_S16_NE, rate, 2);

    switch (play_uade(uri, file, path, name, subsong, state, formatname)) {
        case 1: {
            const auto [songend, seeked] = playback_loop(state, known_timeout);
            const auto *songinfo = get_song_info(state);
            if (songend.status == song_end::ERROR && !allow_songend_error(songinfo)) {
                ERROR("Error playing %s\n", uri);
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
            ERROR("Could not play %s\n", uri);
            ret = false;
            break;
    }
    cleanup_uade_state(state, -1, uri);
    
    return ret;
}

