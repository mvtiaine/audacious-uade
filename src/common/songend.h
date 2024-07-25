// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <cassert>
#include <cstdint>
#include <string>

#include "constexpr.h"

namespace common {

struct SongEnd {
    enum Status : int8_t {
        ERROR = -1,
        NONE = 0,
        PLAYER = 1,
        TIMEOUT = 2,
        DETECT_SILENCE = 3,
        // these only with precalcing
        DETECT_LOOP = 4,
        DETECT_VOLUME = 5,
        DETECT_REPEAT = 6,
        PLAYER_PLUS_SILENCE = 7,
        PLAYER_PLUS_VOLUME = 8,
        LOOP_PLUS_SILENCE = 9,
        LOOP_PLUS_VOLUME = 10,
        NOSOUND = 11,
    };
    Status status;
    uint32_t length = 0;
    static std::string status_string(const Status status) noexcept {
        switch (status) {
            case ERROR: return "error";
            case NONE: return "none";
            case PLAYER: return "player";
            case TIMEOUT: return "timeout";
            case DETECT_SILENCE: return "silence";
            case DETECT_LOOP: return "loop";
            case DETECT_VOLUME: return "volume";
            case DETECT_REPEAT: return "repeat";
            case PLAYER_PLUS_SILENCE: return "player+silence";
            case PLAYER_PLUS_VOLUME: return "player+volume";
            case LOOP_PLUS_SILENCE: return "loop+silence";
            case LOOP_PLUS_VOLUME: return "loop+volume";
            case NOSOUND: return "nosound";
            default: assert(false); return "error";
        }
    }
    _CONSTEXPR_F2 std::string status_string() const noexcept {
        return status_string(status);
    }
};

} // namespace common
