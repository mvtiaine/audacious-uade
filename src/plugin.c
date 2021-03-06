/*
    Copyright (C) 2014  Matti Tiainen <mvtiaine@cc.hut.fi>

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
#include <stdlib.h>

#include <audacious/input.h>
#include <audacious/misc.h>
#include <audacious/plugin.h>
#include <libaudcore/audstrings.h>

#include <uade/uade.h>

#include "config.h"
#include "common.h"
#include "extensions.h"
#include "hacks.h"
#include "modland.h"
#include "prefs.h"

static pthread_mutex_t probe_mutex = PTHREAD_MUTEX_INITIALIZER;
struct uade_state *probe_state;

struct uade_state *create_uade_state(const struct uade_config *uc) {
    struct uade_state *state = uade_new_state(NULL);
    uade_set_amiga_loader(amiga_loader_wrapper, NULL, state);
    return state;
}

bool_t plugin_init(void) {
    DEBUG("uade_plugin_init\n");
    aud_config_set_defaults (PLUGIN_NAME, plugin_defaults);
    probe_state = create_uade_state(NULL);
    return probe_state != NULL;
}

void plugin_cleanup(void) {
    DEBUG("uade_plugin_cleanup\n");
    uade_cleanup_state(probe_state);
    probe_state = NULL;
    modland_cleanup();
}

int parse_uri(const char *uri, char **path, char **name, char **ext) {
    int subsong;
    const char *tmpName, *sub, *tmpExt;
    char *tmpPath = uri_to_filename(uri);

    uri_parse(tmpPath, &tmpName, &tmpExt, &sub, &subsong);

    if (path) {
        *path = str_nget(tmpPath, strlen(tmpPath) - strlen(sub));
    }

    if (name) {
        *name = str_nget(tmpName, strlen(tmpName) - strlen(sub));
    }

    if (ext) {
        *ext = str_nget(tmpExt, strlen(tmpExt) - strlen(sub));
    }

    str_unref(tmpPath);

    return strnlen(sub, FILENAME_MAX) > 0 ? subsong : -1;
}

bool_t plugin_is_our_file_from_vfs (const char *uri, VFSFile *file) {
    TRACE("uade_plugin_is_our_file_from_vfs %s\n", uri);

    bool_t is_our_file = FALSE;
    char *path, *ext;
    int i = 0;

    parse_uri(uri, &path, NULL, &ext);

    if (is_blacklisted_extension(ext)) {
        DEBUG("Blacklisted extension for %s\n", uri);
        goto out;
    }

    pthread_mutex_lock (&probe_mutex);
    switch (uade_play(path, -1, probe_state)) {
        case 1:
            TRACE("uade_plugin_is_our_file_from_vfs accepted %s\n", uri);
            uade_stop(probe_state);
            is_our_file = TRUE;
            break;
        case -1:
            WARN("uade_plugin_is_our_file_from_vfs fatal error on %s\n", uri);
            uade_cleanup_state(probe_state);
            probe_state = create_uade_state(NULL);
            break;
        default:
            DEBUG("Cannot play %s\n", uri);
            uade_stop(probe_state);
            break;
    }
    pthread_mutex_unlock(&probe_mutex);

out:
    str_unref(path);
    str_unref(ext);

    return is_our_file;
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

void update_tuple(Tuple *tuple, char *name, int subsong, struct uade_state *state) {
    const struct uade_song_info* info = uade_get_song_info(state);

    bool_t modulename_ok = strnlen(info->modulename, 256) > 0 &&
                           !is_blacklisted_title(info);
    const char *title = modulename_ok ? info->modulename : name;

    tuple_set_str(tuple, FIELD_TITLE, title);

    tuple_set_str(tuple, FIELD_CODEC, parse_codec(info));

    int subsongs = info->subsongs.max - info->subsongs.min + 1;

    // UADE contentdb doesn't support separate lengths for subsongs
    if (subsongs == 1 && info->duration > 0 && tuple_get_int(tuple, FIELD_LENGTH) <= 0) {
        tuple_set_int(tuple, FIELD_LENGTH, info->duration * 1000);
    }
    tuple_set_str(tuple, FIELD_MIMETYPE, UADE_MIMETYPE);

    if (subsongs > 1) {
        // initial probe
        if (subsong == -1) {
            // provide mappings to uade subsong numbers
            int subtunes[subsongs], i;
            for (i = 0; i < subsongs; ++i) {
                subtunes[i] = info->subsongs.min + i;
            }
            tuple_set_subtunes(tuple, subsongs, subtunes);
        } else {
            // convert to playlist 1/x numbering
            int pl_subsong = subsong - info->subsongs.min + 1;
            tuple_set_int(tuple, FIELD_SUBSONG_NUM, subsongs);
            tuple_set_int(tuple, FIELD_SUBSONG_ID, pl_subsong);
            tuple_set_int(tuple, FIELD_TRACK_NUMBER, pl_subsong);
        }
    }

    modland_data_t *ml_data = modland_lookup(info->modulemd5);
    if (ml_data) {
        TRACE("Found modland data for %s, format:%s, author:%s, album:%s\n",
            info->modulemd5,ml_data->format, ml_data->author, ml_data->album);
        tuple_set_str(tuple, FIELD_ARTIST, ml_data->author);
        // prefer UADE codec names, but fall back to modland if not available
        char *uade_codec = tuple_get_str(tuple, FIELD_CODEC);
        if (!strncmp(UNKNOWN_CODEC, uade_codec, strlen(UNKNOWN_CODEC))) {
            tuple_set_str(tuple, FIELD_CODEC, ml_data->format);
        }
        str_unref(uade_codec);
        if (ml_data->album) {
            tuple_set_str(tuple, FIELD_ALBUM, ml_data->album);
        }
    } else {
        TRACE("No modland data for %s\n", info->modulemd5);
    }
}

void update_error_tuple(Tuple *tuple, char *name) {
    tuple_set_str(tuple, FIELD_TITLE, name);
    tuple_set_str(tuple, FIELD_CODEC, ERRORED_CODEC);
}

Tuple * plugin_probe_for_tuple (const char *uri, VFSFile *file) {
    TRACE("uade_plugin_probe_for_tuple %s\n", uri);

    char *path, *name;
    int subsong;
    Tuple *tuple = tuple_new_from_filename(uri);

    subsong = parse_uri(uri, &path, &name, NULL);

    pthread_mutex_lock (&probe_mutex);
    switch (uade_play(path, subsong, probe_state)) {
        case 1:
            update_tuple(tuple, name, subsong, probe_state);
            uade_stop(probe_state);
            break;
        case -1:
            WARN("uade_plugin_probe_for_tuple fatal error on %s\n", uri);
            uade_cleanup_state(probe_state);
            probe_state = create_uade_state(NULL);
            update_error_tuple(tuple, name);
            break;
        default:
            WARN("uade_plugin_probe_for_tuple cannot play %s\n", uri);
            uade_stop(probe_state);
            update_error_tuple(tuple, name);
            break;
    }
    pthread_mutex_unlock (&probe_mutex);

    str_unref(path);
    str_unref(name);

    return tuple;
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

int playback_loop(char *buffer, struct uade_state* state) {
    while (!aud_input_check_stop()) {
        int seek_value = aud_input_check_seek ();
        if (seek_value >= 0) {
            if (uade_seek(UADE_SEEK_SUBSONG_RELATIVE, seek_value / 1000.0, -1, state)) {
                ERROR("Could not seek to %d\n", seek_value);
            } else {
                DEBUG("Seek to %d\n", seek_value);
            };
        }
        ssize_t nbytes = render_audio(buffer, state);
        if (nbytes < 0) {
            ERROR("Playback error.\n");
            return FALSE;
        } else if (nbytes == 0) {
            TRACE("Song end.\n");
            // update length in playlist
            Tuple *tuple = aud_input_get_tuple();
            if (abs(tuple_get_int(tuple, FIELD_LENGTH) - aud_input_written_time()) > 250) {
                tuple_set_int(tuple, FIELD_LENGTH, aud_input_written_time());
                aud_input_set_tuple(tuple);
            } else {
                tuple_unref(tuple);
            }
            break;
        }
        aud_input_write_audio(buffer, nbytes);
    }
    return TRUE;
}

bool_t plugin_play (const char *uri, VFSFile *file) {
    TRACE("uade_plugin_play %s\n", uri);

    char *path, *name, buffer[4096];
    int subsong, rate;
    bool_t ret = FALSE;
    struct uade_state *state = NULL;

    subsong = parse_uri(uri, &path, &name, NULL);

    state = create_uade_state(NULL);
    if (!state) {
        ERROR("Could not init uade state\n");
        goto out;
    }

    rate = uade_get_sampling_rate(state);

    if (!aud_input_open_audio(FMT_S16_NE, rate, 2)) {
        ERROR("Could not open audio with rate %d\n", rate);
        goto out;
    }

    switch (uade_play(path, subsong, state)) {
        case 1:
            ret = playback_loop(buffer, state);
            break;
        default:
            ERROR("Could not play %s\n", uri);
            ret = FALSE;
            break;
    }

out:
    uade_cleanup_state(state);
    str_unref(path);
    str_unref(name);

    return ret;
}

static const char plugin_about[] =
    "Audacious UADE plugin " VERSION "\n"
    "Written by Matti Tiainen <mvtiaine@cc.hut.fi>\n"
    "\n"
    "UADE: http://zakalwe.fi/uade/";

AUD_INPUT_PLUGIN (
    .name = "UADE Plugin",
    .about_text = plugin_about,
    .priority = -1, // to avoid some files being recognized as MP3
    .have_subtune = TRUE,
    .mimes = plugin_mimes,
    .extensions = plugin_extensions,
    .init = plugin_init,
    .cleanup = plugin_cleanup,
    .prefs = &plugin_prefs,
    .is_our_file_from_vfs = plugin_is_our_file_from_vfs,
    .probe_for_tuple = plugin_probe_for_tuple,
    .play = plugin_play,
)
