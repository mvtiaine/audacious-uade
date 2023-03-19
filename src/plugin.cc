// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <libgen.h>
#include <pthread.h>
#include <cstdlib>
#include <cstring>
#include <cmath>
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

struct uade_state *create_uade_state(struct uade_config *uc) {
#ifdef DEBUG_TRACE
    if (!uc) {
        uc = uade_new_config();
        uade_config_set_option(uc, UC_VERBOSE, NULL);
    }
#endif
    struct uade_state *state = uade_new_state(uc);
    // should not be needed anymore (modland now uses prefix for TFMX)
    // uade_set_amiga_loader(amiga_loader_wrapper, NULL, state);
    return state;
}

struct uade_state *create_uade_probe_state() {
    struct uade_config *uc = uade_new_config();
#ifdef DEBUG_TRACE
    uade_config_set_option(uc, UC_VERBOSE, NULL);
#endif
    uade_config_set_option(uc, UC_FREQUENCY, "3000");
    uade_config_set_option(uc, UC_FILTER_TYPE, "none");
    uade_config_set_option(uc, UC_RESAMPLER, "none");
    uade_config_set_option(uc, UC_PANNING_VALUE, "0");
    uade_config_set_option(uc, UC_ONE_SUBSONG, NULL);
    uade_config_set_option(uc, UC_NO_FILTER, NULL);
    uade_config_set_option(uc, UC_NO_HEADPHONES, NULL);
    uade_config_set_option(uc, UC_NO_PANNING, NULL);
    uade_config_set_option(uc, UC_NO_POSTPROCESSING, NULL);
    struct uade_state *state = uade_new_state(uc);
    // should not be needed anymore (modland now uses prefix for TFMX)
    // uade_set_amiga_loader(amiga_loader_wrapper, NULL, state);
    return state;
}

struct probe_state *get_probe_state() {
    pthread_mutex_lock (&probe_mutex);
    struct probe_state *state;
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

void update_tuple(Tuple &tuple, char *name, int subsong, const struct uade_song_info* info, const char modulemd5[33], const char formatname[256]) {
    /* prefer filename as title for now, maybe make configurable in future
    bool modulename_ok = strnlen(info->modulename, 256) > 0 &&
                            !is_blacklisted_title(info);
    const char *title = modulename_ok ? info->modulename : name;
    */
    const char *title = name;

    tuple.set_str(Tuple::Title, title);

    tuple.set_str(Tuple::Codec, formatname);

    int subsongs = info->subsongs.max - info->subsongs.min + 1;

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

    modland_data_t *ml_data = modland_lookup(modulemd5);
    if (ml_data) {
        TRACE("Found modland data for %s, format:%s, author:%s, album:%s\n",
            modulemd5, static_cast<const char *>(ml_data->format), static_cast<const char *>(ml_data->author), static_cast<const char*>(ml_data->album));
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
        TRACE("No modland data for %s\n", modulemd5);
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
                const char *reason_timeout1 = "song timeout";
                const char *reason_timeout2 = "subsong timeout";
                const char *reason_silence = "silence";
                if (n.song_end.happy) {
                    bool timeout = !strncmp(reason_timeout1, n.song_end.reason, strlen(reason_timeout1)) ||
                        !strncmp(reason_timeout2, n.song_end.reason, strlen(reason_timeout2));
                    bool silence = !strncmp(reason_silence, n.song_end.reason, strlen(reason_silence));
                    nbytes = 0;
                    if (timeout) nbytes = -2;
                    if (silence) nbytes = -3;
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
    int bytespersec = UADE_BYTES_PER_FRAME * uade_get_sampling_rate(state);
    // cut short after 1h as UADE plays some mods for hours or possibly forever (with always_ends default)
    uint64_t maxbytes = 3600 * bytespersec;
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
    StringBuf &path,
    StringBuf &name,
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
            return uade_play_from_buffer(name, res.data.data(), res.data.size(), subsong, state);
        } else {
            WARN("uade_plugin conversion failed for %s reason: %s\n", uri, res.reason_failed.c_str());
            return 0;
        }
    } else {
        return uade_play(path, subsong, state);
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
    for (int i = 0; i < MAX_PROBES; ++i) {
        probes[i] = {};
    }
    return true;
}

void UADEPlugin::cleanup() {
    DEBUG("uade_plugin_cleanup\n");
    for (int i = 0; i < MAX_PROBES; ++i) {
        if (probes[i].state != NULL) {
            cleanup_uade_state(probes[i].state);
        }
    }
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
        // don't try uade_play yet
        return true;
    }

    struct probe_state *probe_state = get_probe_state();

    switch (uade_play(path, -1, probe_state->state)) {
        case 1:
            TRACE("uade_plugin_is_our_file accepted %s\n", uri);
            uade_stop(probe_state->state);
            is_our_file = true;
            break;
        case -1:
            WARN("uade_plugin_is_our_file fatal error on %s path %s\n", uri, static_cast<char *>(path));
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

    StringBuf path(4096);
    StringBuf name(256);
    StringBuf ext(256);
    int subsong;
    bool success = false;

    subsong = parse_uri(uri, &path, &name, &ext);

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

    optional<string> formatname;
    optional<string> modulemd5;

    switch (play_uade(uri, file, path, name, subsong, probe_state->state, formatname, modulemd5)) {
        case 1: {
            const struct uade_song_info* info = uade_get_song_info(probe_state->state);
            update_tuple(tuple, name, subsong, info,
                modulemd5.has_value() ? modulemd5.value().c_str() : info->modulemd5,
                formatname.has_value() ? formatname.value().c_str() : parse_codec(info));
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
            WARN("uade_plugin_read_tag fatal error on %s path %s\n", uri, static_cast<char *>(path));
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
        return false;
    }

    rate = uade_get_sampling_rate(state);

    open_audio(FMT_S16_NE, rate, 2);

    optional<string> formatname;
    optional<string> modulemd5;

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
            ret = playback_loop(buffer, state);
            break;
        default:
            ERROR("Could not play %s\n", uri);
            ret = false;
            break;
    }

    cleanup_uade_state(state);

    return ret;
}

