// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#ifndef UADE_COMMON_H_
#define UADE_COMMON_H_

#include "config.h"
extern "C" {
#include "../uade/src/frontends/include/uade/options.h"
#include "../uade/src/frontends/include/uade/uadeconfstructure.h"
#include "../uade/src/frontends/include/uade/uade.h"
}
#include <cassert>
#include <pthread.h>

#include "player/player.h"
#include "songend/songend.h"

constexpr string_view TYPE_PREFIX = "type: ";
constexpr string_view UNKNOWN_CODEC = "UADE";

struct song_end {
    enum Status {
        NONE = 0,
        PLAYER = -1,
        ERROR = -2,
        STOP = -3,
        TIMEOUT = -4,
        UADE_SILENCE = -5,
        DETECT_LOOP = -6,
        DETECT_SILENCE = -7,
        DETECT_VOLUME = -8,
        DETECT_REPEAT = -9,
        PLAYER_PLUS_SILENCE = -10,
        PLAYER_PLUS_VOLUME = -11,
        LOOP_PLUS_SILENCE = -12,
        LOOP_PLUS_VOLUME = -13,
        NOSOUND = -14,
    };
    Status status;
    int length = 0;

    string status_string() const {
        switch (status) {
            case song_end::PLAYER: return "player";
            case song_end::ERROR: return "error";
            case song_end::TIMEOUT: return "timeout";
            case song_end::UADE_SILENCE:
            case song_end::DETECT_SILENCE: return "silence";
            case song_end::DETECT_LOOP: return "loop";
            case song_end::DETECT_VOLUME: return "volume";
            case song_end::DETECT_REPEAT: return "repeat";
            case song_end::PLAYER_PLUS_SILENCE: return "player+silence";
            case song_end::PLAYER_PLUS_VOLUME: return "player+volume";
            case song_end::LOOP_PLUS_SILENCE: return "loop+silence";
            case song_end::LOOP_PLUS_VOLUME: return "loop+volume";
            case song_end::NOSOUND: return "nosound";
            case song_end::NONE: return "none";
            case song_end::STOP: return "stop";
            default: assert(false); return "error";
        }
    }
};

uade_state *create_uade_probe_state(int freq = songend::PRECALC_FREQ_UADE);
pair<song_end::Status, ssize_t> render_audio(char *buffer, const int bufsize, uade_state *state);
pair<song_end::Status, ssize_t> render_audio_player(char *buffer, const int bufsize, player::PlayerState &state);
song_end precalc_song_length(uade_state *state, const struct uade_song_info *info);
song_end precalc_song_length_player(player::PlayerState &state, const char *fname);
string parse_codec(const struct uade_song_info *info);

#endif // UADE_COMMON_H_
