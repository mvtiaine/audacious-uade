// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <cstdint>

#include "compat.h"

// TODO || !__has_include ?
#if __cplusplus <= 201703L || !__has_include(<bit>)
namespace std {
enum class endian
{
#if defined(_MSC_VER) && !defined(__clang__)
    little = 0,
    big    = 1,
    native = little
#else
    little = __ORDER_LITTLE_ENDIAN__,
    big    = __ORDER_BIG_ENDIAN__,
    native = __BYTE_ORDER__
#endif
};
}
#else
#include <bit>
#endif

namespace common {

constexpr uint32_t _byteswap(const uint32_t val) noexcept {
    const uint32_t tmp = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF ); 
    return (tmp << 16) | (tmp >> 16);
}

constexpr uint16_t _byteswap(const uint16_t val) noexcept {
    return (val << 8) | (val >> 8);
}

constexpr uint32_t _htobe32(const uint32_t &val) noexcept {
    return (std::endian::native == std::endian::big) ? val : _byteswap(val);
}

constexpr uint32_t _be32toh(const uint32_t &val) noexcept {
    return (std::endian::native == std::endian::big) ? val : _byteswap(val);
}

constexpr uint16_t _htobe16(const uint16_t &val) noexcept {
    return (std::endian::native == std::endian::big) ? val : _byteswap(val);
}

constexpr uint16_t _be16toh(const uint16_t &val) noexcept {
    return (std::endian::native == std::endian::big) ? val : _byteswap(val);
}

constexpr uint32_t _htole32(const uint32_t &val) noexcept {
    return (std::endian::native == std::endian::little) ? val : _byteswap(val);
}

constexpr uint32_t _le32toh(const uint32_t &val) noexcept {
    return (std::endian::native == std::endian::little) ? val : _byteswap(val);
}

constexpr uint16_t _htole16(const uint16_t &val) noexcept {
    return (std::endian::native == std::endian::little) ? val : _byteswap(val);
}

constexpr uint16_t _le16toh(const uint16_t &val) noexcept {
    return (std::endian::native == std::endian::little) ? val : _byteswap(val);
}

struct be_int32_t {
    constexpr be_int32_t() noexcept : be_val(0) {}
    constexpr be_int32_t(const int32_t &val) noexcept : be_val(_htobe32(val)) {}
    constexpr operator int32_t() const noexcept {
        return _be32toh(be_val);
    }
    int32_t be_val;
} __attribute__((packed));

struct be_uint32_t {
    constexpr be_uint32_t() noexcept : be_val(0) {}
    constexpr be_uint32_t(const uint32_t &val) noexcept : be_val(_htobe32(val)) {}
    constexpr operator uint32_t() const noexcept {
        return _be32toh(be_val);
    }
    uint32_t be_val;
} __attribute__((packed));


struct be_int16_t {
    constexpr be_int16_t() noexcept : be_val(0) {}
    constexpr be_int16_t(const int16_t &val) noexcept : be_val(_htobe16(val)) {}
    constexpr operator int16_t() const noexcept {
        return _be16toh(be_val);
    }
    int16_t be_val;
} __attribute__((packed));

struct be_uint16_t {
    constexpr be_uint16_t() noexcept : be_val(0) {}
    constexpr be_uint16_t(const uint16_t &val) noexcept : be_val(_htobe16(val)) {}
    constexpr operator uint16_t() const noexcept {
        return _be16toh(be_val);
    }
    uint16_t be_val;
} __attribute__((packed));

struct le_int32_t {
    constexpr le_int32_t() noexcept : le_val(0) {}
    constexpr le_int32_t(const int32_t &val) noexcept : le_val(_htole32(val)) {}
    constexpr operator int32_t() const noexcept {
        return _le32toh(le_val);
    }
    int32_t le_val;
} __attribute__((packed));

struct le_uint32_t {
    constexpr le_uint32_t() noexcept : le_val(0) {}
    constexpr le_uint32_t(const uint32_t &val) noexcept : le_val(_htole32(val)) {}
    constexpr operator uint32_t() const noexcept {
        return _le32toh(le_val);
    }
    uint32_t le_val;
} __attribute__((packed));


struct le_int16_t {
    constexpr le_int16_t() noexcept : le_val(0) {}
    constexpr le_int16_t(const int16_t &val) noexcept : le_val(_htole16(val)) {}
    constexpr operator int16_t() const noexcept {
        return _le16toh(le_val);
    }
    int16_t le_val;
} __attribute__((packed));

struct le_uint16_t {
    constexpr le_uint16_t() noexcept : le_val(0) {}
    constexpr le_uint16_t(const uint16_t &val) noexcept : le_val(_htole16(val)) {}
    constexpr operator uint16_t() const noexcept {
        return _le16toh(le_val);
    }
    uint16_t le_val;
} __attribute__((packed));

} // namespace common
