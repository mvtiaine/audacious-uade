// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <pthread.h>
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

#include <uade/uade.h>

#include "config.h"
#include "common.h"
#include "extensions.h"
#include "hacks.h"
#include "modland.h"
#include "prefs.h"
#include "converter/converter.h"
#include "3rdparty/md5.h"

using namespace std;

namespace {

struct probe_state {
    struct uade_state *state;
    bool initialized = false;
    bool available = true;
};
static pthread_mutex_t probe_mutex = PTHREAD_MUTEX_INITIALIZER;
constexpr int MAX_PROBES = 8;
static probe_state probes[MAX_PROBES];

void cleanup_uade_state(struct uade_state *state) {
    // avoid concurrent contentdb updates
    pthread_mutex_lock (&probe_mutex);
    uade_cleanup_state(state);
    pthread_mutex_unlock(&probe_mutex);
}

struct probe_state *get_probe_state() {
    pthread_mutex_lock (&probe_mutex);
    struct probe_state *state = nullptr;
    for (int i = 0; i < MAX_PROBES; ++i) {
        if (probes[i].available) {
            probes[i].available = false;
            if (!probes[i].initialized) {
                probes[i].state = create_uade_probe_state();
                probes[i].initialized = true;
            }
            state = &probes[i];
            break;
        }
    }
    pthread_mutex_unlock(&probe_mutex);
    assert(state);
    assert(state->state);
    return state;
}

void release_probe_state(struct probe_state *probe_state) {
    pthread_mutex_lock (&probe_mutex);
    probe_state->available = true;
    pthread_mutex_unlock(&probe_mutex);
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

constexpr string_view TYPE_PREFIX = "type: ";
constexpr string_view UNKNOWN_CODEC = "UADE";

const string parse_codec(const struct uade_song_info *info) {
    const string_view formatname(info->formatname);
    const string_view playername(info->playername);
    if (!formatname.empty()) {
        // remove "type: " included in some formats
        if (formatname.find(TYPE_PREFIX) == 0) {
            string name (formatname.substr(TYPE_PREFIX.length()));
            return name.c_str();
        } else {
            return info->formatname;
        }
    } else if (!playername.empty()) {
        return info->playername;

    } else {
        return UNKNOWN_CODEC.begin();
    }
}

void update_tuple(Tuple &tuple, const string &name, int subsong, const struct uade_song_info* info, const string &modulemd5, const string &formatname) {
    /* prefer filename as title for now, maybe make configurable in future
    bool modulename_ok = strnlen(info->modulename, 256) > 0 &&
                            !is_blacklisted_title(info);
    const char *title = modulename_ok ? info->modulename : name;
    */
    tuple.set_str(Tuple::Title, name.c_str());

    tuple.set_str(Tuple::Codec, formatname.c_str());

    const int subsongs = info->subsongs.max - info->subsongs.min + 1;

    // UADE contentdb doesn't support separate lengths for subsongs
    if (subsongs == 1 && info->duration > 0 && tuple.get_int(Tuple::Length) <= 0) {
        tuple.set_int(Tuple::Length, info->duration * 1000);
    }

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

    const auto ml_entry = modland_lookup(modulemd5.c_str(), name);
    if (ml_entry.has_value()) {
        const auto ml_data = ml_entry.value();
        TRACE("Found modland data for %s, format:%s, author:%s, album:%s, filename:%s\n", modulemd5.c_str(), ml_data.format.c_str(), ml_data.author.c_str(), ml_data.album.c_str(), ml_data.filename.c_str());
        if (!ml_data.author.empty()) {
            tuple.set_str(Tuple::Artist, ml_data.author.c_str());
        }
        // prefer UADE codec names, but fall back to modland if not available
        const string uade_codec = string(tuple.get_str(Tuple::Codec));
        if (uade_codec == UNKNOWN_CODEC) {
            tuple.set_str(Tuple::Codec, ml_data.format.c_str());
        }
        if (!ml_data.album.empty()) {
            tuple.set_str(Tuple::Album, ml_data.album.c_str());
        }
    } else {
        TRACE("No modland data for %s\n", modulemd5.c_str());
    }
}

ssize_t render_audio(void *buffer, struct uade_state *state) {
    struct uade_notification n;
    ssize_t nbytes = uade_read(buffer, sizeof buffer, state);
    while (uade_read_notification(&n, state)) {
        switch (n.type) {
            case UADE_NOTIFICATION_MESSAGE:
                TRACE("Amiga message: %s\n", n.msg);
                break;
            case UADE_NOTIFICATION_SONG_END: {
                TRACE("%s: %s\n", n.song_end.happy ? "song end" : "bad song end", n.song_end.reason);
                constexpr string_view reason_timeout1 = "song timeout";
                constexpr string_view reason_timeout2 = "subsong timeout";
                constexpr string_view reason_silence = "silence";
                if (n.song_end.happy) {
                    string reason = n.song_end.reason;
                    nbytes = 0;
                    if (reason == reason_timeout1 || reason == reason_timeout2)
                        nbytes = -2;
                    else if (reason == reason_silence)
                        nbytes = -3;
                } else {
                    nbytes = -1;
                }
                break;
            }
            default:
                WARN("Unknown notification type from libuade\n");
                break;
        }
        uade_cleanup_notification(&n);
    }
    return nbytes;
}

int precalc_songlength(struct uade_state *state) {
    char buffer[4096];
    ssize_t nbytes;
    uint64_t totalbytes = 0;
    const int bytespersec = UADE_BYTES_PER_FRAME * uade_get_sampling_rate(state);
    // UADE plays some mods for hours or possibly forever (with always_ends default)
    uint64_t maxbytes = aud_get_int(PLUGIN_NAME, PRECALC_TIMEOUT) * bytespersec;
    while ((nbytes = render_audio(buffer, state)) > 0) {
        totalbytes += nbytes;
        if (totalbytes >= maxbytes) {
            return maxbytes * 1000 / bytespersec;
        }
    }
    if (nbytes != -1) {
        // UADE does not update info->duration, use songbytes instead
        const struct uade_song_info* info = uade_get_song_info(state);
        return info->songbytes * 1000 / bytespersec;
    }
    return 0;
}

bool needs_conversion(const char *uri, VFSFile &file) {
    char magic[converter::MAGIC_SIZE];
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
    optional<string> &formatname,
    optional<string> &modulemd5
) {
    if (needs_conversion(uri, file)) {
        if (file.fseek(0, VFS_SEEK_SET)) {
            WARN("uade_plugin could not seek %s\n", uri);
            return 0;
        }
        const auto &buf = file.read_all();
        auto res = converter::convert(buf.begin(), buf.len());
        if (res.success) {
            formatname = res.format;
            DEBUG("uade_plugin converted %s to format %s\n", uri, res.format.c_str());
            MD5 md5; md5.update((const unsigned char*)buf.begin(), buf.len()); md5.finalize();
            modulemd5 = md5.hexdigest();
            return uade_play_from_buffer(name.c_str(), res.data.data(), res.data.size(), subsong, state);
        } else {
            WARN("uade_plugin conversion failed for %s reason: %s\n", uri, res.reason_failed.c_str());
            return 0;
        }
    } else {
        return uade_play(path.c_str(), subsong, state);
    }
};

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
    bool playback_loop(struct uade_state* state);

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
    return true;
}

void UADEPlugin::cleanup() {
    DEBUG("uade_plugin_cleanup\n");
    for (int i = 0; i < MAX_PROBES; ++i) {
        if (probes[i].state != nullptr) {
            cleanup_uade_state(probes[i].state);
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
    if (is_blacklisted_filename(name)) {
        DEBUG("uade_plugin_is_our_file blacklisted filename %s\n", uri);
        return true;
    }

    if (needs_conversion(uri, file)) {
        DEBUG("uade_plugin_is_our_file needs conversion: %s\n", uri);
        // don't try uade_play yet
        return true;
    }

    struct probe_state *probe_state = get_probe_state();

    switch (uade_play(path.c_str(), -1, probe_state->state)) {
        case 1:
            TRACE("uade_plugin_is_our_file accepted %s\n", uri);
            uade_stop(probe_state->state);
            is_our_file = true;
            break;
        case -1:
            WARN("uade_plugin_is_our_file fatal error on %s path %s\n", uri, path.c_str());
            cleanup_uade_state(probe_state->state);
            probe_state->state = create_uade_probe_state();
            break;
        default:
            DEBUG("Cannot play %s\n", uri);
            uade_stop(probe_state->state);
            break;
    }
    release_probe_state(probe_state);

    return is_our_file;
}

bool UADEPlugin::read_tag(const char *uri, VFSFile & file, Tuple &tuple, Index<char> *image) {
    TRACE("uade_plugin_read_tag %s\n", uri);

    string path, name, ext;
    optional<string> formatname, modulemd5;
    int subsong;
    bool success = false;

    subsong = parse_uri(uri, path, name, ext);

    if (is_blacklisted_extension(ext)) {
        DEBUG("uade_plugin_read_tag blacklisted %s\n", uri);
        return false;
    }

    // add to playlist, but call uade_play() on-demand (may hang UADE/audacious completely)
    if (is_blacklisted_filename(name)) {
        DEBUG("uade_plugin_read_tag blacklisted filename %s\n", uri);
        return true;
    }

    struct probe_state *probe_state = get_probe_state();

    switch (play_uade(uri, file, path, name, subsong, probe_state->state, formatname, modulemd5)) {
        case 1: {
            const struct uade_song_info* info = uade_get_song_info(probe_state->state);
            update_tuple(tuple, name, subsong, info,
                modulemd5.has_value() ? modulemd5.value(): info->modulemd5,
                formatname.has_value() ? formatname.value() : parse_codec(info));
            bool do_cleanup = false;
            if (subsong != -1 && tuple.get_int(Tuple::Length) <= 0 &&
                aud_get_bool(PLUGIN_NAME, PRECALC_SONGLENGTHS)) {
                int length = precalc_songlength(probe_state->state);
                if (length > 0) {
                    tuple.set_int(Tuple::Length, abs(length));
                } else {
                    do_cleanup = true;
                }
            }
            if (do_cleanup) {
                cleanup_uade_state(probe_state->state);
                probe_state->state = create_uade_probe_state();
            } else {
                uade_stop(probe_state->state);
            }
            success = true;
            break;
        }
        case -1:
            WARN("uade_plugin_read_tag fatal error on %s path %s\n", uri, path.c_str());
            cleanup_uade_state(probe_state->state);
            probe_state->state = create_uade_probe_state();
            break;
        default:
            WARN("uade_plugin_read_tag cannot play %s\n", uri);
            uade_stop(probe_state->state);
            break;
    }
    release_probe_state(probe_state);

    return success;
}

bool UADEPlugin::playback_loop(struct uade_state* state) {
    char buffer[4096];
    bool wasSeeked = false;
    while (!check_stop()) {
        int seek_value = check_seek();
        if (seek_value >= 0) {
            wasSeeked = true;
            if (uade_seek(UADE_SEEK_SUBSONG_RELATIVE, seek_value / 1000.0, -1, state)) {
                ERROR("Could not seek to %d\n", seek_value);
            } else {
                DEBUG("Seek to %d\n", seek_value);
            };
        }
        ssize_t nbytes = render_audio(buffer, state);
        if (nbytes == -1) {
            ERROR("Playback error.\n");
            return false;
        } else if (nbytes == -2) {
            TRACE("Song end (timeout).\n");
            break;
        } else if (nbytes == -3) {
            TRACE("Song end (silence).\n");
            break;
        } else if (nbytes == 0) {
            TRACE("Song end.\n");
            // update length in playlist
            Tuple tuple = get_playback_tuple();
            if (!wasSeeked && abs(tuple.get_int(Tuple::Length) - aud_drct_get_time()) > 250) {
                tuple.set_int(Tuple::Length, aud_drct_get_time());
                set_playback_tuple(tuple.ref());
            }
            break;
        }
        write_audio(buffer, nbytes);
    }
    return true;
}

bool UADEPlugin::play(const char *uri, VFSFile &file) {
    TRACE("uade_plugin_play %s\n", uri);

    string path, name, ext;
    optional<string> formatname, modulemd5;
    int subsong, rate;
    bool ret = false;
    struct uade_state *state = nullptr;

    subsong = parse_uri(uri, path, name, ext);

    state = create_uade_state();
    if (!state) {
        ERROR("Could not init uade state\n");
        return false;
    }

    rate = uade_get_sampling_rate(state);

    open_audio(FMT_S16_NE, rate, 2);

    switch (play_uade(uri, file, path, name, subsong, state, formatname, modulemd5)) {
        case 1:
            // update tuple at playtime for blacklisted filenames
            if (is_blacklisted_filename(name)) {
                Tuple tuple = get_playback_tuple();
                const struct uade_song_info* info = uade_get_song_info(state);
                update_tuple(tuple, name, subsong, info,
                    modulemd5.has_value() ? modulemd5.value().c_str() : info->modulemd5,
                    formatname.has_value() ? formatname.value().c_str() : parse_codec(info));
                set_playback_tuple(tuple.ref());
            }
            ret = playback_loop(state);
            break;
        default:
            ERROR("Could not play %s\n", uri);
            ret = false;
            break;
    }

    cleanup_uade_state(state);

    return ret;
}

