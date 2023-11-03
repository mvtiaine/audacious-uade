// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <string>

#include "uade_common.h"
#include "common.h"
#include "hacks.h"

using namespace std;

namespace {

void apply_detector(songend::SongEndDetector &detector, song_end &songend) {
    int silence = detector.detect_silence(songend::SILENCE_TIMEOUT);
    if (silence > 0) {
        if (songend.length == silence) {
            songend.length = 0;
            songend.status = song_end::NOSOUND;
        } else {
            songend.length = silence + songend::MAX_SILENCE;
            songend.status = song_end::DETECT_SILENCE;
        }
    } else {
        int volume = detector.detect_volume();
        if (volume > 0) {
            songend.length = volume + songend::MAX_SILENCE;
            songend.status = song_end::DETECT_VOLUME;
        } else {
            int repeat = detector.detect_repeat();
            if (repeat > 0) {
                songend.length = repeat;
                songend.status = song_end::DETECT_REPEAT;
            } else {
                int loop = detector.detect_loop();
                if (loop > 0) {
                    songend.length = loop;
                    songend.status = song_end::DETECT_LOOP;
                    int silence = detector.trim_silence(songend.length);
                    if (silence > songend::MAX_SILENCE) {
                        songend.length = songend.length - silence + songend::MAX_SILENCE;
                        songend.status = song_end::LOOP_PLUS_SILENCE;
                    } else {
                        int volume = detector.trim_volume(songend.length);
                        if (volume > songend::MAX_SILENCE && volume < songend.length) {
                            songend.length = songend.length - volume + songend::MAX_SILENCE;
                            songend.status = song_end::LOOP_PLUS_VOLUME;
                        }
                    }
                } else {
                    songend.length = songend::PRECALC_TIMEOUT * 1000;
                    songend.status = song_end::TIMEOUT;
                }
            }
        }
    }
}

} // namespace {}

struct uade_state *create_uade_probe_state(int freq) {
    struct uade_config *uc = uade_new_config();
#if DEBUG_TRACE
    uade_config_set_option(uc, UC_VERBOSE, nullptr);
#endif
    // use our uade.conf, song.conf and contentdb even with system libuade
    uade_config_set_option(uc, UC_BASE_DIR, UADEDATADIR);
#if SYSTEM_LIBUADE
    // make sure to use system version for these
    uade_config_set_option(uc, UC_UAE_CONFIG_FILE, UADE_CONFIG_BASE_DIR "/uaerc");
    uade_config_set_option(uc, UC_SCORE_FILE, UADE_CONFIG_BASE_DIR "/score");
    uade_config_set_option(uc, UC_UADECORE_FILE, UADE_CONFIG_UADE_CORE);
#else
    uade_config_set_option(uc, UC_NO_CONTENT_DB, nullptr);
#endif
    uade_config_set_option(uc, UC_ONE_SUBSONG, nullptr);
    uade_config_set_option(uc, UC_TIMEOUT_VALUE, "-1");
    uade_config_set_option(uc, UC_DISABLE_TIMEOUTS, nullptr);
    uade_config_set_option(uc, UC_SUBSONG_TIMEOUT_VALUE, to_string(songend::PRECALC_TIMEOUT).c_str());
    uade_config_set_option(uc, UC_SILENCE_TIMEOUT_VALUE,to_string(songend::SILENCE_TIMEOUT).c_str());
    uade_config_set_option(uc, UC_FREQUENCY, to_string(freq).c_str());
    uade_config_set_option(uc, UC_FILTER_TYPE, "none");
    uade_config_set_option(uc, UC_RESAMPLER, "none");
    uade_config_set_option(uc, UC_PANNING_VALUE, "1");
    uade_config_set_option(uc, UC_NO_FILTER, nullptr);
    uade_config_set_option(uc, UC_NO_HEADPHONES, nullptr);
 
    struct uade_state *state = uade_new_state(uc);
    uade_set_amiga_loader(amiga_loader_wrapper, nullptr, state);
    return state;
}

pair<song_end::Status, ssize_t> render_audio(char *buffer, const int bufsize, uade_state *state) {
    uade_notification n;
    song_end::Status status = song_end::NONE;
    ssize_t nbytes = uade_read(buffer, bufsize, state);
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
                    if (reason == reason_timeout1 || reason == reason_timeout2)
                        status = song_end::TIMEOUT;
                    else if (reason == reason_silence)
                        status = song_end::UADE_SILENCE;
                    else
                        status = song_end::PLAYER;
                } else {
                    status = song_end::ERROR;
                }
                break;
            }
            default:
                WARN("Unknown notification type from libuade\n");
                break;
        }
        uade_cleanup_notification(&n);
    }
    if (nbytes < 0) {
        status = song_end::ERROR;
    }
    return pair(status,nbytes);
}

pair<song_end::Status, ssize_t> render_audio_player(char *buffer, const int bufsize, player::PlayerState &state) {
    song_end::Status status = song_end::NONE;
    const auto [songend, nbytes] = player::render(state, buffer, bufsize);
    if (songend || nbytes == 0) {
        status = song_end::PLAYER;
    }
    if (nbytes < 0) {
        status = song_end::ERROR;
    }
    return pair(status,nbytes);
}

song_end precalc_song_length(uade_state *state, const struct uade_song_info *info) {
    song_end songend;
    char buffer[4096];
    pair<song_end::Status, ssize_t> render;
    size_t totalbytes = 0;
    const size_t bytespersec = UADE_BYTES_PER_FRAME * uade_get_sampling_rate(state);
    // UADE plays some mods for hours or possibly forever (with always_ends default)
    size_t maxbytes = songend::PRECALC_TIMEOUT * bytespersec;
    songend::SongEndDetector detector(songend::PRECALC_FREQ_UADE, false);
    while ((render = render_audio(buffer, sizeof buffer, state)).second > 0) {
        // ignore "tail bytes" to avoid pop in end of audio if song restarts
        // messing up with silence/volume trimming etc.
        if (render.first != song_end::NONE && totalbytes > 0) {
            //TRACE("IGNORED TAILBYTES %zd\n", render.first);
            break;
        }
        totalbytes += render.second;
        detector.update(buffer, render.second);
        if (totalbytes >= maxbytes || render.first != song_end::NONE) {
            break;
        }
    }
    if (totalbytes >= maxbytes || render.first == song_end::TIMEOUT) {
        apply_detector(detector, songend);
        TRACE("precalc_song_length %s - status: %d length: %d\n", info->modulefname, songend.status, songend.length);
        return songend;
    }
    if (render.first != song_end::ERROR || allow_songend_error(info)) {
        // UADE does not update info->duration, use totalbytes instead
        //TRACE("TOTALBYTES %zu UADE %lld/%lld - %s\n", totalbytes, info->subsongbytes, info->songbytes, info->modulefname);
        songend.length = totalbytes * 1000 / bytespersec;
        songend.status = render.first;
        if (songend.status == song_end::PLAYER) {
            int silence = detector.trim_silence(songend.length);
            if (silence == songend.length) {
                songend.status = song_end::NOSOUND;
                songend.length = 0;
            } else if (silence > songend::MAX_SILENCE) {
                songend.length = songend.length - silence + songend::MAX_SILENCE;
                songend.status = song_end::PLAYER_PLUS_SILENCE;
            } else {
                int volume = detector.trim_volume(songend.length);
                if (volume > songend::MAX_SILENCE && volume < songend.length) {
                    songend.length = songend.length - volume + songend::MAX_SILENCE;
                    songend.status = song_end::PLAYER_PLUS_VOLUME;
                }
            }
        } else if (songend.status == song_end::UADE_SILENCE) {
            int silence = detector.trim_silence(songend.length);
            if (silence == songend.length) {
                songend.status = song_end::NOSOUND;
                songend.length = 0;
            } else {
                songend.length = songend.length - songend::SILENCE_TIMEOUT * 1000 + songend::MAX_SILENCE;
            }
        } else if (songend.status == song_end::TIMEOUT) {
            songend.length = songend::PRECALC_TIMEOUT * 1000;
        }
        TRACE("precalc_song_length %s - status: %d length: %d\n", info->modulefname, songend.status, songend.length);
    } else {
        songend.status = song_end::ERROR;
        ERR("Error precalcing %s\n", info->modulefname);
    }
    return songend;
}

song_end precalc_song_length_player(player::PlayerState &state, const char *fname) {
    song_end songend;
    char buffer[player::MIXBUFSIZE];
    pair<bool, size_t> render;
    size_t totalbytes = 0;
    const size_t bytespersec = 4 * songend::PRECALC_FREQ_PLAYER;
    size_t maxbytes = songend::PRECALC_TIMEOUT * bytespersec;
    songend::SongEndDetector detector(songend::PRECALC_FREQ_PLAYER, true);
    while ((render = player::render(state, buffer, sizeof buffer)).second > 0) {
        state.pos_millis = totalbytes * 1000 / bytespersec;
        totalbytes += render.second;
        detector.update(buffer, render.second);
        if (totalbytes >= maxbytes || render.first) {
            break;
        }
    }
    if (totalbytes >= maxbytes || !render.first) {
        apply_detector(detector, songend);
        TRACE("precalc_song_length_player %s - status: %d length: %d\n", fname, songend.status, songend.length);
        return songend;
    }

    if (render.first) {
        songend.length = totalbytes * 1000 / bytespersec;
        songend.status = song_end::PLAYER;
        int silence = detector.trim_silence(songend.length);
        if (silence == songend.length) {
            songend.status = song_end::NOSOUND;
            songend.length = 0;
        } else if (silence > songend::MAX_SILENCE) {
            songend.length = songend.length - silence + songend::MAX_SILENCE;
            songend.status = song_end::PLAYER_PLUS_SILENCE;
        } else {
            int volume = detector.trim_volume(songend.length);
            if (volume > songend::MAX_SILENCE && volume < songend.length) {
                songend.length = songend.length - volume + songend::MAX_SILENCE;
                songend.status = song_end::PLAYER_PLUS_VOLUME;
            }
        }
        TRACE("precalc_song_length_player %s - status: %d length: %d\n", fname, songend.status, songend.length);
    } else {
        songend.status = song_end::ERROR;
        ERR("Error precalcing %s\n", fname);
    }
    return songend;
}

string parse_codec(const struct uade_song_info *info) {
    const string_view formatname(info->formatname);
    const string_view playername(info->playername);
    if (!formatname.empty()) {
        // remove "type: " included in some formats
        if (formatname.find(TYPE_PREFIX) == 0) {
            string name (formatname.substr(TYPE_PREFIX.length()));
            if (is_octamed(info)) {
                return "OctaMED (" + name + ")";
            }
            return name;
        } else {
            return info->formatname;
        }
    } else if (!playername.empty()) {
        return info->playername;

    } else {
        return UNKNOWN_CODEC.begin();
    }
}
