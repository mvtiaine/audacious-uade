// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <cassert>
#include <charconv>
#include <cstdint>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>

#include "common/compat.h"

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
    static std::string status_string(const Status status) {
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
    std::string status_string() const {
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

inline auto split_view(const std::string_view &input, const char separator) {
    std::vector<std::string_view> results;
    size_t prevpos = 0;
    size_t pos = 0;
    while ((pos = input.find(separator, prevpos)) != std::string::npos) {
        results.push_back(input.substr(prevpos, pos-prevpos));
        prevpos = pos + 1;
    }
    results.push_back(input.substr(prevpos));
    return results;
}

template <int N>
inline auto split_view(const std::string_view &input, const char separator) {
    std::array<std::string_view, N> results; 
    auto current = input.begin();
    const auto End = input.end();
    for (auto& part : results) {
        if (current == End) {
            const bool is_last_part = &part == &(results.back());
            assert(is_last_part);
        }
        auto delim = std::find(current, End, separator);
        part = { &*current, size_t(delim-current) };
        current = delim;
        if (delim != End) ++current;
    }
    return results;
}

inline std::string mkString(const std::vector<std::string> &v, const std::string &delimiter) {
    return std::accumulate(v.begin(), v.end(), std::string(),[&delimiter](const std::string &ss, const std::string &s) {
      return ss.empty() ? s : ss + delimiter + s;
    });
}

inline std::string mkString(const std::vector<std::string_view> &v, const std::string_view &delimiter) {
    return std::accumulate(v.begin(), v.end(), std::string(),[delimiter](const std::string_view &ss, const std::string_view &s) {
        return ss.empty() ? std::string(s) : std::string(ss) + std::string(delimiter) + std::string(s);
    });
}

template <class T>
inline T from_chars(const std::string_view &s) {
    if (s.size() == 1) return s[0] - 48;
    const char *end = s.begin() + s.size();
    T number;
    auto result = std::from_chars(s.begin(), end, number);
    assert(result.ec == std::errc{});
    assert(result.ptr == end);
    return number;
}

} // namespace common
