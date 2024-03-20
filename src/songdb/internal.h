// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <climits>
#include <cstdint>
#include <functional>
#include <type_traits>

#include "common/common.h"

#if 0
// XXX too slow
namespace songdb::internal {
struct uint24_t {
    // 16 million md5s should be enough for everyone
    uint8_t data[3];
    constexpr uint24_t() {}
    constexpr uint24_t(const uint24_t &other) {
        data[0] = other.data[0];
        data[1] = other.data[1];
        data[2] = other.data[2];
    }
    constexpr uint24_t(const uint32_t val) {
        assert(val <= (1u << 24) - 1);
        data[0] = (val >> 16) & 0xff;
        data[1] = (val >> 8) & 0xff;
        data[2] = val & 0xff;
    }
    constexpr uint32_t value() const {
        return (data[0] << 16) | (data[1] << 8) | data[2];
    }
    constexpr operator uint32_t() const {
        return value();
    }
    constexpr bool operator==(const uint24_t &other) const {
        return value() == other.value();
    }
    constexpr bool operator<(const uint24_t &other) const {
        return value() < other.value();
    }
    constexpr bool operator>(const uint24_t &other) const {
        return value() > other.value();
    }
    constexpr uint24_t operator++(int) {
        const uint24_t val = value();
        const uint24_t next = val + 1;
        data[0] = (&next)->data[0];
        data[1] = (&next)->data[1];
        data[2] = (&next)->data[2];
        return val;
    }
} __attribute__((packed));

struct uint48_t {
    uint16_t data[3];
    constexpr uint48_t() {}
    constexpr uint48_t(const uint48_t &other) {
        data[0] = other.data[0];
        data[1] = other.data[1];
        data[2] = other.data[2];
    }
    constexpr uint48_t(const uint64_t val) {
        assert(val <= (1ull << 48) - 1);
        data[0] = (val >> 32) & 0xffff;
        data[1] = (val >> 16) & 0xffff;
        data[2] = val & 0xffff;
    }
    constexpr uint64_t value() const {
        return
            (static_cast<uint64_t>(data[0]) << 32) |
            (static_cast<uint64_t>(data[1]) << 16) |
            data[2];
    }
    constexpr operator uint64_t() const {
        return value();
    }
    constexpr bool operator==(const uint48_t &other) const {
        return value() == other.value();
    }
    constexpr bool operator<(const uint48_t &other) const {
        return value() < other.value();
    }
    constexpr bool operator>(const uint48_t &other) const {
        return value() > other.value();
    }
} __attribute__((packed));

} // namespace songdb::internal
template<>
struct std::hash<songdb::internal::uint48_t>
{
    constexpr std::size_t operator()(const songdb::internal::uint48_t& s) const noexcept
    {
        return s.value();
    }
};
namespace common {
template<>
inline songdb::internal::uint24_t from_chars<songdb::internal::uint24_t>(const std::string_view &s) {
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
typedef uint24_t md5_idx_t;
constexpr md5_idx_t MD5_NOT_FOUND = UINT24_T_MAX;

typedef common::SongEnd::Status songend_t;

struct _Data {
    md5_idx_t md5;
} __attribute__((packed));
template <typename T>
concept _Data_ = std::is_base_of<_Data, T>::value;

struct _ModInfo : _Data {
    string_t format;
    channels_t channels;
} __attribute__((packed));

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

struct _SongInfo {
    subsong_t subsong;
    songlength_t songlength;
    songend_t songend;
} __attribute__((packed));

} // namespace songdb::internal
