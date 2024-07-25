// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include "std/string_view.h"

#include <algorithm>
#include <array>
#include <cassert>
#if __has_include(<charconv>)
#include <charconv>
#endif
#include <cstdint>
#include <numeric>
#include <string>
#include <vector>

#include "constexpr.h"

namespace common {

_CONSTEXPR_F2 std::vector<std::string> split(const std::string &str, const std::string &delimiter) noexcept {
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

_CONSTEXPR_F2 auto split_view(const std::string_view &input, const char separator) noexcept {
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

_CONSTEXPR_F2 auto split_view_x(const std::string_view &input, const char separator) noexcept {
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

_CONSTEXPR_F2 void mkString(const std::vector<std::string> &v, const std::string &delimiter, std::string &res) noexcept {
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

_CONSTEXPR_F2 std::string mkString(const std::vector<std::string> &v, const std::string &delimiter) noexcept {
    std::string res;
    mkString(v, delimiter, res);
    return res;
}

_CONSTEXPR_F2 void mkString(const std::vector<std::string_view> &v, const std::string_view &delimiter, std::string &res) noexcept {
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
            res += std::string(v[i]);
            if (i < v.size() - 1) {
                res += std::string(delimiter);
            }
        }
    }
}

_CONSTEXPR_F2 std::string mkString(const std::vector<std::string_view> &v, const std::string_view &delimiter) noexcept {
    std::string res;
    mkString(v, delimiter, res);
    return res;
}

template <class T>
_CONSTEXPR_F T from_chars(const std::string_view &s) noexcept {
    if (s.size() == 1) return s[0] - 48;
#if !__has_include(<charconv>)
    const std::string ss = {s.begin(), s.end()};
    return std::stoi(ss);
#else
    T number = 0;
#if defined(__Fuchsia__)
    // XXX Fuchsia toolchain bug? error: no viable conversion from '__wrap_iter<const char *>' to 'const char *'
    const char *end = __unwrap_iter(s.begin() + s.size());
    auto result = std::from_chars(__unwrap_iter(s.begin()), end, number);
#else
    const char *end = s.begin() + s.size();
    auto result = std::from_chars(s.begin(), end, number);
#endif
    assert(result.ec == std::errc{});
    assert(result.ptr == end);
    return number;
#endif
}

_CONSTEXPR_F bool starts_with(const std::string_view &s, const std::string_view &prefix) noexcept {
#if !defined(__cpp_lib_starts_ends_with)
    if (prefix.length() > s.length()) return false;
    return s.rfind(prefix, 0) == 0;
#else 
    return s.starts_with(prefix);
#endif
}

_CONSTEXPR_F bool ends_with(const std::string_view &s, const std::string_view &suffix) noexcept {
#if !defined(__cpp_lib_starts_ends_with)
    if (suffix.length() > s.length()) return false;
    return s.rfind(suffix, s.length() - suffix.length()) == 0;
#else
    return s.ends_with(suffix);
#endif
}

} // namespace common
