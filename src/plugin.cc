/*
    Copyright (C) 2018  Matti Tiainen <mvtiaine@cc.hut.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <libgen.h>
#include <pthread.h>
#include <cstdlib>
#include <cstring>
#include <cmath>
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

using namespace std;

namespace {

    static pthread_mutex_t probe_mutex = PTHREAD_MUTEX_INITIALIZER;
    struct uade_state *probe_state;

    struct uade_state *create_uade_state(struct uade_config *uc) {
/* TODO
#ifdef DEBUG_TRACE
        if (!uc) {
            uc = uade_new_config();
        }
        uade_config_toggle_boolean(uc, UC_VERBOSE);
#endif
*/
        struct uade_state *state = uade_new_state(uc);
        // should not be needed anymore (modland now uses prefix for TFMX)
        // uade_set_amiga_loader(amiga_loader_wrapper, NULL, state);
        return state;
    }

    int parse_uri(const char *uri, StringBuf *path, StringBuf *name, StringBuf *ext) {
        int subsong;
        const char *tmpName, *sub, *tmpExt;
        const char *tmpPath = uri_to_filename(uri);

        uri_parse(tmpPath, &tmpName, &tmpExt, &sub, &subsong);

        if (path) {
            strncpy(*path, tmpPath, strlen(tmpPath) - strlen(sub));
            path->resize(strlen(tmpPath) - strlen(sub));
        }

        if (name) {
            strncpy(*name, tmpName, strlen(tmpName) - strlen(sub));
            name->resize(strlen(tmpName) - strlen(sub));
        }

        if (ext) {
            strncpy(*ext, tmpExt, strlen(tmpExt) - strlen(sub));
            ext->resize(strlen(tmpExt) - strlen(sub));
        }

        return strnlen(sub, FILENAME_MAX) > 0 ? subsong : -1;
    }

    #define TYPE_PREFIX "type: "
    #define UNKNOWN_CODEC "UADE"
    #define ERRORED_CODEC "UADE ERROR"

    const char *parse_codec(const struct uade_song_info *info) {
        if (strnlen(info->formatname, 256) > 0) {
            // remove "type: " included in some formats
            if (!strncmp(TYPE_PREFIX, info->formatname, strlen(TYPE_PREFIX))) {
                return info->formatname + strlen(TYPE_PREFIX);
            } else {
                return info->formatname;
            }
        } else if (strnlen(info->playername, 256) > 0) {
            return info->playername;

        } else {
            return UNKNOWN_CODEC;
        }
    }

    void update_tuple(Tuple &tuple, char *name, int subsong, struct uade_state *state) {
        const struct uade_song_info* info = uade_get_song_info(state);

        /* prefer filename as title for now, maybe make configurable in future
        bool modulename_ok = strnlen(info->modulename, 256) > 0 &&
                             !is_blacklisted_title(info);
        const char *title = modulename_ok ? info->modulename : name;
        */
        const char *title = name;

        tuple.set_str(Tuple::Title, title);

        tuple.set_str(Tuple::Codec, parse_codec(info));

        int subsongs = info->subsongs.max - info->subsongs.min + 1;

        // UADE contentdb doesn't support separate lengths for subsongs
        if (subsongs == 1 && info->duration > 0 && tuple.get_int(Tuple::Length) <= 0) {
            tuple.set_int(Tuple::Length, info->duration * 1000);
        }

        if (subsongs > 1) {
            // initial probe
            if (subsong == -1) {
                // provide mappings to uade subsong numbers
                Index<short> subtunes;
                for (int i = 0; i < subsongs; ++i) {
                    subtunes.append(info->subsongs.min + i);
                }
                tuple.set_subtunes(subtunes.len(), subtunes.begin());
            } else {
                // convert to playlist 1/x numbering
                int pl_subsong = subsong - info->subsongs.min + 1;
                tuple.set_int(Tuple::NumSubtunes, subsongs);
                tuple.set_int(Tuple::Subtune, pl_subsong);
                tuple.set_int(Tuple::Track, pl_subsong);
            }
        }

        modland_data_t *ml_data = modland_lookup(info->modulemd5);
        if (ml_data) {
            TRACE("Found modland data for %s, format:%s, author:%s, album:%s\n",
                info->modulemd5, static_cast<const char *>(ml_data->format), static_cast<const char *>(ml_data->author), static_cast<const char*>(ml_data->album));
            if (ml_data->author && strlen(ml_data->author) > 0) {
                tuple.set_str(Tuple::Artist, ml_data->author);
            }
            // prefer UADE codec names, but fall back to modland if not available
            const char *uade_codec = tuple.get_str(Tuple::Codec);
            if (!strncmp(UNKNOWN_CODEC, uade_codec, strlen(UNKNOWN_CODEC))) {
                tuple.set_str(Tuple::Codec, ml_data->format);
            }
            if (ml_data->album && strlen(ml_data->album) > 0) {
                tuple.set_str(Tuple::Album, ml_data->album);
            }
        } else {
            TRACE("No modland data for %s\n", info->modulemd5);
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
                case UADE_NOTIFICATION_SONG_END:
                    TRACE("%s: %s\n", n.song_end.happy ? "song end" : "bad song end",
                            n.song_end.reason);
                    nbytes = n.song_end.happy ? 0 : -1;
                    break;
                default:
                    WARN("Unknown notification type from libuade\n");
                    break;
            }
            uade_cleanup_notification(&n);
        }
        return nbytes;
    }
}

class UADEPlugin : public InputPlugin
{

public:

    static constexpr PluginInfo info = {
        "UADE Plugin",
        "audacious-uade",
        "Audacious UADE plugin " VERSION "\n"
        "Written by Matti Tiainen <mvtiaine@cc.hut.fi>\n"
        "\n"
        "UADE: http://zakalwe.fi/uade/",
        &plugin_prefs
    };

    constexpr UADEPlugin() : InputPlugin(info, InputInfo(FlagSubtunes)
        .with_priority(-1) // to avoid some files being recognized as MP3
        .with_exts(plugin_extensions)
        .with_mimes(plugin_mimes)) {}

    bool init();
    void cleanup();

    bool is_our_file(const char *uri, VFSFile &file);
    bool read_tag(const char *uri, VFSFile &file, Tuple &tuple, Index<char> *image);
    bool play(const char *uri, VFSFile &file);

private:
    bool playback_loop(char *buffer, struct uade_state* state);

}; // class UADEPlugin

#define EXPORT __attribute__((visibility("default")))
EXPORT UADEPlugin aud_plugin_instance;

bool UADEPlugin::init() {
    DEBUG("uade_plugin_init\n");
    aud_config_set_defaults (PLUGIN_NAME, plugin_defaults);
    probe_state = create_uade_state(NULL);
    return probe_state != NULL;
}

void UADEPlugin::cleanup() {
    DEBUG("uade_plugin_cleanup\n");
    uade_cleanup_state(probe_state);
    probe_state = NULL;
    modland_cleanup();
}

bool UADEPlugin::is_our_file(const char *uri, VFSFile &file) {
    TRACE("uade_plugin_is_our_file %s\n", uri);

    bool is_our_file = false;
    StringBuf path(4096);
    StringBuf name(256);
    StringBuf ext(256);
    int i = 0;

    parse_uri(uri, &path, &name, &ext);

    if (is_blacklisted_extension(ext) || is_blacklisted_filename(name)) {
        DEBUG("uade_plugin_is_our_file blacklisted %s\n", uri);
        return false;
    }

    pthread_mutex_lock (&probe_mutex);
    switch (uade_play(path, -1, probe_state)) {
        case 1:
            TRACE("uade_plugin_is_our_file accepted %s\n", uri);
            uade_stop(probe_state);
            is_our_file = true;
            break;
        case -1:
            WARN("uade_plugin_is_our_file fatal error on %s path %s\n", uri, static_cast<char *>(path));
            uade_cleanup_state(probe_state);
            probe_state = create_uade_state(NULL);
            break;
        default:
            DEBUG("Cannot play %s\n", uri);
            uade_stop(probe_state);
            break;
    }
    pthread_mutex_unlock(&probe_mutex);

    return is_our_file;
}

bool UADEPlugin::read_tag(const char *uri, VFSFile & file, Tuple &tuple, Index<char> *image) {
    TRACE("uade_plugin_read_tag %s\n", uri);

    StringBuf path(4096);
    StringBuf name(256);
    StringBuf ext(256);
    int subsong;
    bool success = false;

    subsong = parse_uri(uri, &path, &name, &ext);

    if (is_blacklisted_extension(ext) || is_blacklisted_filename(name)) {
        DEBUG("uade_plugin_read_tag blacklisted %s\n", uri);
        return false;
    }

    pthread_mutex_lock (&probe_mutex);
    switch (uade_play(path, subsong, probe_state)) {
        case 1:
            update_tuple(tuple, name, subsong, probe_state);
            uade_stop(probe_state);
            success = true;
            break;
        case -1:
            WARN("uade_plugin_read_tag fatal error on %s path %s\n", uri, static_cast<char *>(path));
            uade_cleanup_state(probe_state);
            probe_state = create_uade_state(NULL);
            //update_error_tuple(tuple, name);
            break;
        default:
            WARN("uade_plugin_read_tag cannot play %s\n", uri);
            uade_stop(probe_state);
            //update_error_tuple(tuple, name);
            break;
    }
    pthread_mutex_unlock (&probe_mutex);

    return success;
}

bool UADEPlugin::playback_loop(char *buffer, struct uade_state* state) {
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
        if (nbytes < 0) {
            ERROR("Playback error.\n");
            return false;
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

bool UADEPlugin::play (const char *uri, VFSFile &file) {
    TRACE("uade_plugin_play %s\n", uri);

    StringBuf path(4096);
    StringBuf name(256);
    char buffer[4096];
    int subsong, rate;
    bool ret = false;
    struct uade_state *state = NULL;

    subsong = parse_uri(uri, &path, &name, NULL);

    state = create_uade_state(NULL);
    if (!state) {
        ERROR("Could not init uade state\n");
        goto out;
    }

    rate = uade_get_sampling_rate(state);

    open_audio(FMT_S16_NE, rate, 2);

    switch (uade_play(path, subsong, state)) {
        case 1:
            ret = playback_loop(buffer, state);
            break;
        default:
            ERROR("Could not play %s\n", uri);
            ret = false;
            break;
    }

out:
    uade_cleanup_state(state);

    return ret;
}

