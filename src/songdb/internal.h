// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <climits>
#include <cstdint>
#include <functional>
#include <type_traits>

#include "common/songend.h"
#include "common/strings.h"

#if 1

namespace songdb::internal {

struct uint24_t {
    uint16_t hi;
    uint8_t lo;
    constexpr uint24_t() {}
    constexpr uint24_t(const uint24_t &other) noexcept {
        hi = other.hi;
        lo = other.lo;
    }
    constexpr uint24_t(const uint32_t val) noexcept {
        assert(val <= (1u << 24) - 1);
        hi = (val >> 8) & 0xffff;
        lo = val & 0xff;
    }
    constexpr uint32_t value() const noexcept {
        return (static_cast<uint32_t>(hi) << 8) | lo;
    }
    constexpr operator uint32_t() const noexcept {
        return value();
    }
    constexpr bool operator==(const uint24_t &other) const noexcept {
        return hi == other.hi && lo == other.lo;
    }
    constexpr bool operator<(const uint24_t &other) const noexcept {
        return hi < other.hi || (hi == other.hi && lo < other.lo);
    }
    constexpr bool operator>(const uint24_t &other) const noexcept {
        return hi > other.hi || (hi == other.hi && lo > other.lo);
    }
    constexpr uint24_t operator++(int) noexcept {
        const uint32_t val = value();
        const uint24_t next = val + 1;
        hi = (&next)->hi;
        lo = (&next)->lo;
        return val;
    }
} __attribute__((packed));

struct uint48_t {
    uint32_t hi;
    uint16_t lo;
    constexpr uint48_t() {}
    constexpr uint48_t(const uint48_t &other) noexcept {
        hi = other.hi;
        lo = other.lo;
    }
    constexpr uint48_t(const uint64_t val) noexcept {
        assert(val <= (1ull << 48) - 1);
        hi = (val >> 16) & 0xffffffff;
        lo = val & 0xffff;
    }
    constexpr uint64_t value() const noexcept {
        return (static_cast<uint64_t>(hi) << 16) | lo;
    }
    constexpr operator uint64_t() const noexcept {
        return value();
    }
    constexpr bool operator==(const uint48_t &other) const noexcept {
        return hi == other.hi && lo == other.lo;
    }
    constexpr bool operator<(const uint48_t &other) const noexcept {
        return hi < other.hi || (hi == other.hi && lo < other.lo);
    }
    constexpr bool operator>(const uint48_t &other) const noexcept {
        return hi > other.hi || (hi == other.hi && lo > other.lo);
    }
} __attribute__((packed));

} // namespace songdb::internal

template<>
struct std::hash<songdb::internal::uint24_t>
{
    constexpr std::size_t operator()(const songdb::internal::uint24_t& s) const noexcept {
        return s.value();
    }
};
template<>
struct std::hash<songdb::internal::uint48_t>
{
    constexpr std::size_t operator()(const songdb::internal::uint48_t& s) const noexcept {
        return s.value();
    }
};

namespace common {
template<>
constexpr songdb::internal::uint24_t from_chars<songdb::internal::uint24_t>(const std::string_view &s) noexcept {
    return common::from_chars<uint32_t>(s);
}
}

namespace songdb::internal {

#else

namespace songdb::internal {

typedef uint32_t uint24_t;
typedef uint64_t uint48_t;

#endif

constexpr int32_t UINT24_T_MAX = (1u << 24) - 1;
constexpr int64_t UINT48_T_MAX = (1ull << 48) - 1;

typedef uint8_t channels_t;
typedef uint8_t subsong_t;
typedef uint8_t year_t;
typedef uint24_t songlength_t;
typedef uint48_t md5_t;
constexpr md5_t MD5_T_MAX = UINT48_T_MAX;
// indexed types
typedef uint16_t string_t;
constexpr string_t UNKNOWN_AUTHOR_T = 0;
constexpr string_t STRING_NOT_FOUND = UINT16_MAX;
// 16 million md5s should be enough for everyone
typedef uint24_t md5_idx_t;
constexpr md5_idx_t MD5_NOT_FOUND = UINT24_T_MAX;

typedef common::SongEnd::Status songend_t;

struct _Data {
    md5_idx_t md5;
} __attribute__((packed));
template <typename T>
concept _Data_ = std::is_base_of<_Data, T>::value;

struct _ModlandData : _Data {
    string_t author;
    string_t album;
} __attribute__((packed));

struct _UnExoticaData : _Data {
    string_t author;
    string_t album;
    string_t publisher;
    year_t year;
} __attribute__((packed));

struct _AMPData : _Data {
    string_t author;
} __attribute__((packed));;

struct _DemozooData : _Data {
    string_t author;
    string_t album;
    string_t publisher;
    year_t year;
} __attribute__((packed));

struct _ModInfo {
    string_t format;
    channels_t channels;
} __attribute__((packed));

struct _SubSongInfo {
    unsigned int _songlength : 18 __attribute__((packed)); // (20ms accuracy)
    int _songend : 6 __attribute__((packed));

    constexpr _SubSongInfo() noexcept {
    }
    constexpr _SubSongInfo(const songlength_t songlength, const songend_t songend) noexcept {
        assert(songlength <= (1 << 18)); // 20ms accuracy
        _songlength = songlength;
        _songend = songend;
    }
    constexpr songlength_t songlength() const noexcept {
        return _songlength;
    }
    constexpr songlength_t songlength_ms() const noexcept {
        return _songlength * 20;
    }
    constexpr songend_t songend() const noexcept {
       return static_cast<common::SongEnd::Status>(_songend);
    }
} __attribute__((packed));

struct _SongInfo {
    unsigned int _has_subsongs : 1 __attribute__((packed));
    unsigned int _min_subsong : 7 __attribute__((packed));
    _SubSongInfo _info;

    constexpr _SongInfo() noexcept {}
    constexpr _SongInfo(const bool has_subsongs, const uint8_t min_subsong, const _SubSongInfo info) noexcept {
        assert(min_subsong <= 127);
        _has_subsongs = has_subsongs;
        _min_subsong = min_subsong;
        _info = info;
    }
    constexpr bool has_subsongs() const noexcept {
        return _has_subsongs;
    }
    constexpr uint8_t min_subsong() const noexcept {
        return _min_subsong;
    }
    constexpr _SubSongInfo info() const noexcept {
        return _info;
    }
} __attribute__((packed));

} // namespace songdb::internal
