// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <charconv>
#include <cstdint>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>

namespace common {

inline std::vector<std::string> split(const std::string &str, const std::string &delimiter) noexcept {
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

inline auto split_view(const std::string_view &input, const char separator) noexcept {
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

inline auto split_view_x(const std::string_view &input, const char separator) noexcept {
    std::vector<std::string_view> results;
    size_t prevpos = 0;
    size_t pos = 0;
    while ((pos = input.find(separator, prevpos)) != std::string::npos) {
        results.push_back(input.substr(prevpos, pos-prevpos));
        prevpos = pos + 1;
    }
    results.push_back(input.substr(prevpos, input.size() - prevpos - 1));
    return results;
}

template <int N>
constexpr auto split_view(const std::string_view &input, const char separator) noexcept {
    std::array<std::string_view, N> results; 
    auto current = input.begin();
    const auto End = input.end();
    for (auto& part : results) {
        auto delim = std::find(current, End, separator);
        part = { &*current, size_t(delim-current) };
        current = delim;
        if (delim != End) ++current;
        else break;
    }
    return results;
}

template <int N>
constexpr auto split_view_x(const std::string_view &input, const char separator) noexcept {
    std::array<std::string_view, N> results; 
    auto current = input.begin();
    const auto End = input.end();
    for (auto& part : results) {
        auto delim = std::find(current, End, separator);
        if (delim == End) {
            part = { &*current, size_t(delim-current-1) };
            break;
        } else {
            part = { &*current, size_t(delim-current) };
            current = delim + 1;
        }
    }
    return results;
}

inline void mkString(const std::vector<std::string> &v, const std::string &delimiter, std::string &res) noexcept {
    size_t size = 0;
    for (size_t i = 0; i < v.size(); ++i) {
        if (v[i].size()) {
            size += v[i].size();
            if (i < v.size() - 1) {
                size += delimiter.size();
            }
        }
    }
    res.reserve(size + 1);
    for (size_t i = 0; i < v.size(); ++i) {
        if (v[i].size()) {
            res += v[i];
            if (i < v.size() - 1) {
                res += delimiter;
            }
        }
    }
}

inline std::string mkString(const std::vector<std::string> &v, const std::string &delimiter) noexcept {
    std::string res;
    mkString(v, delimiter, res);
    return res;
}

inline void mkString(const std::vector<std::string_view> &v, const std::string_view &delimiter, std::string &res) noexcept {
    size_t size = 0;
    for (size_t i = 0; i < v.size(); ++i) {
        if (v[i].size()) {
            size += v[i].size();
            if (i < v.size() - 1) {
                size += delimiter.size();
            }
        }
    }
    res.reserve(size + 1);
    for (size_t i = 0; i < v.size(); ++i) {
        if (v[i].size()) {
            res += v[i];
            if (i < v.size() - 1) {
                res += delimiter;
            }
        }
    }
}

inline std::string mkString(const std::vector<std::string_view> &v, const std::string_view &delimiter) noexcept {
    std::string res;
    mkString(v, delimiter, res);
    return res;
}

template <class T>
constexpr T from_chars(const std::string_view &s) noexcept {
    if (s.size() == 1) return s[0] - 48;
    const char *end = s.begin() + s.size();
    T number;
    auto result = std::from_chars(s.begin(), end, number);
    assert(result.ec == std::errc{});
    assert(result.ptr == end);
    return number;
}

} // namespace common
