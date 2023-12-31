// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <cassert>
#include <cstdint>
#include <numeric>
#include <string>
#include <vector>

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
    int length = 0;
    constexpr static std::string status_string(const Status status) {
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
    constexpr std::string status_string() const {
        return status_string(status);
    }
};

inline std::vector<std::string> split(const std::string &str, const std::string &delimiter) {
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string s = str;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        if (pos != 0) {
            tokens.push_back(s.substr(0, pos));
        } else {
            tokens.push_back("");
        }
        s.erase(0, pos + delimiter.length());
    }
    tokens.push_back(s); // add last part

    return tokens;
}

inline std::string mkString(const std::vector<std::string> &v, const std::string &delimiter) {
    return std::accumulate(v.begin(), v.end(), std::string(),[&delimiter](const std::string &ss, const std::string &s) {
      return ss.empty() ? s : ss + delimiter + s;
    });
}

} // namespace common