#include <pthread.h>

#include <audacious/input.h>
#include <audacious/plugin.h>
#include <libaudcore/audstrings.h>

#include <uade/uade.h>

#include "common.h"
#include "extensions.h"
#include "modland.h"

static pthread_mutex_t probe_mutex = PTHREAD_MUTEX_INITIALIZER;
struct uade_state *probe_state;

bool_t plugin_init(void) {
    DBG("uade_plugin_init\n");
    probe_state = uade_new_state(NULL);
    return probe_state != NULL && !modland_init();
}

void plugin_cleanup(void) {
    DBG("uade_plugin_cleanup\n");
    uade_cleanup_state(probe_state);
    probe_state = NULL;
    modland_cleanup();
}

int parse_uri(const char *uri, char **path, char **name) {
    int subsong;
    const char *tmpName, *sub;
    char *tmpPath = uri_to_filename(uri);

    uri_parse(uri, &tmpName, NULL, &sub, &subsong);

    if (path) {
        *path = str_nget(tmpPath, strlen(tmpPath) - strlen(sub));
    }

    if (name) {
        *name = str_nget(tmpName, strlen(tmpName) - strlen(sub));
    }

    str_unref(tmpPath);

    return subsong <= 0 ? 1 : subsong;
}

bool_t plugin_is_our_file_from_vfs (const char *uri, VFSFile *file) {
    DBG("uade_plugin_is_our_file_from_vfs %s\n", uri);

    bool_t is_our_file = FALSE;
    char *path;

    parse_uri(uri, &path, NULL);

    if (path) {
        pthread_mutex_lock (&probe_mutex);
        is_our_file = uade_is_our_file(path, probe_state);
        pthread_mutex_unlock(&probe_mutex);
    }

    str_unref(path);

    return is_our_file;
}

#define TYPE_PREFIX "type: "
#define UNKNOWN_CODEC "UADE"
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

    tuple_set_str(tuple, FIELD_TITLE,
            strnlen(info->modulename, 256) > 0 ? info->modulename : name);

    tuple_set_str(tuple, FIELD_CODEC, parse_codec(info));

    // UADE contentdb doesn't support separate lengths for subsongs
    if (info->subsongs.max == 1 && info->duration > 0 && tuple_get_int(tuple, FIELD_LENGTH) <= 0) {
        tuple_set_int(tuple, FIELD_LENGTH, info->duration * 1000);
    }
    tuple_set_str(tuple, FIELD_MIMETYPE, UADE_MIMETYPE);

    if (info->subsongs.max > 1) {
        tuple_set_int(tuple, FIELD_SUBSONG_NUM, info->subsongs.max);
        tuple_set_int(tuple, FIELD_SUBSONG_ID, subsong);
        tuple_set_int(tuple, FIELD_TRACK_NUMBER, subsong);
        tuple_set_subtunes(tuple, info->subsongs.max, NULL);
    }

    modland_data_t *ml_data = modland_lookup(info->modulemd5);
    if (ml_data) {
        DBG("Found modland data for %s, format:%s, author:%s, album:%s\n",info->modulemd5,ml_data->format, ml_data->author, ml_data->album);
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
        DBG("No modland data for %s\n", info->modulemd5);
    }
}

Tuple * plugin_probe_for_tuple (const char *uri, VFSFile *file) {
    DBG("uade_plugin_probe_for_tuple %s\n", uri);

    char *path, *name;
    int subsong;
    Tuple *tuple = tuple_new_from_filename(uri);

    subsong = parse_uri(uri, &path, &name);

    if (path && tuple) {
        pthread_mutex_lock (&probe_mutex);
        switch (uade_play(path, subsong, probe_state)) {
            case 1:
                update_tuple(tuple, name, subsong, probe_state);
                uade_stop(probe_state);
                break;
            case -1:
                uade_cleanup_state(probe_state);
                probe_state = uade_new_state(NULL);
                break;
            default:
                uade_stop(probe_state);
                break;
        }
        pthread_mutex_unlock (&probe_mutex);
    }

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
                DBG("Amiga message: %s\n", n.msg);
                break;
            case UADE_NOTIFICATION_SONG_END:
                DBG("%s: %s\n", n.song_end.happy ? "song end" : "bad song end",
                        n.song_end.reason);
                nbytes = n.song_end.happy ? 0 : -1;
                break;
            default:
                DBG("Unknown notification type from libuade\n");
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
                ERR("Could not seek to %d\n", seek_value);
            } else {
                DBG("Seek to %d\n", seek_value);
            };
        }
        ssize_t nbytes = render_audio(buffer, state);
        if (nbytes < 0) {
            ERR("Playback error.\n");
            return FALSE;
        } else if (nbytes == 0) {
            DBG("Song end.\n");
            // update length in playlist
            Tuple *tuple = aud_input_get_tuple();
            if (tuple_get_int(tuple, FIELD_LENGTH) != aud_input_written_time()) {
                tuple_set_int(tuple, FIELD_LENGTH, aud_input_written_time());
                aud_input_set_tuple(tuple);
            }
            tuple_unref(tuple);
            break;
        }
        aud_input_write_audio(buffer, nbytes);
    }
    return TRUE;
}

bool_t plugin_play (const char *uri, VFSFile *file) {
    DBG("uade_plugin_play %s\n", uri);

    char *path, *name, buffer[4096];
    int subsong, rate;
    bool_t ret = FALSE;
    struct uade_state *state = NULL;

    subsong = parse_uri(uri, &path, &name);

    state = uade_new_state(NULL);
    if (!state) {
        ERR("Could not init uade state\n");
        goto out;
    }

    rate = uade_get_sampling_rate(state);

    if (!aud_input_open_audio(FMT_S16_NE, rate, 2)) {
        ERR("Could not open audio with rate %d\n", rate);
        goto out;
    }

    switch (uade_play(path, subsong, state)) {
        case 1:
            ret = playback_loop(buffer, state);
            break;
        default:
            ERR("Could not play %s\n", uri);
            ret = FALSE;
            break;
    }

out:
    uade_cleanup_state(state);
    str_unref(path);
    str_unref(name);

    return ret;
}

const char *plugin_mimes[] = {
    UADE_MIMETYPE,
    NULL
};


AUD_INPUT_PLUGIN (
    .name = "UADE",
    .about_text = "Plugin for UADE",
    .priority = -1, // to avoid some files being recognized as MP3
    .have_subtune = TRUE,
    .mimes = plugin_mimes,
    .extensions = plugin_extensions,
    .init = plugin_init,
    .cleanup = plugin_cleanup,
    .is_our_file_from_vfs = plugin_is_our_file_from_vfs,
    .probe_for_tuple = plugin_probe_for_tuple,
    .play = plugin_play,
)
