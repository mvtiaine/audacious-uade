// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <cstdint>

#include "common/portable_endian.h"

namespace common {

struct be_int32_t {
    constexpr be_int32_t() : be_val(0) {}
    constexpr be_int32_t(const int32_t &val) : be_val(htobe32(val)) {}
    constexpr operator int32_t() const noexcept {
        return be32toh(be_val);
    }
    int32_t be_val;
} __attribute__((packed));

struct be_uint32_t {
    constexpr be_uint32_t() : be_val(0) {}
    constexpr be_uint32_t(const uint32_t &val) : be_val(htobe32(val)) {}
    constexpr operator uint32_t() const noexcept {
        return be32toh(be_val);
    }
    uint32_t be_val;
} __attribute__((packed));


struct be_int16_t {
    constexpr be_int16_t() : be_val(0) {}
    constexpr be_int16_t(const int16_t &val) : be_val(htobe16(val)) {}
    constexpr operator int16_t() const noexcept {
        return be16toh(be_val);
    }
    int16_t be_val;
} __attribute__((packed));

struct be_uint16_t {
    constexpr be_uint16_t() : be_val(0) {}
    constexpr be_uint16_t(const uint16_t &val) : be_val(htobe16(val)) {}
    constexpr operator uint16_t() const noexcept {
        return be16toh(be_val);
    }
    uint16_t be_val;
} __attribute__((packed));

struct le_int32_t {
    constexpr le_int32_t() : le_val(0) {}
    constexpr le_int32_t(const int32_t &val) : le_val(htole32(val)) {}
    constexpr operator int32_t() const noexcept {
        return le32toh(le_val);
    }
    int32_t le_val;
} __attribute__((packed));

struct le_uint32_t {
    constexpr le_uint32_t() : le_val(0) {}
    constexpr le_uint32_t(const uint32_t &val) : le_val(htole32(val)) {}
    constexpr operator uint32_t() const noexcept {
        return le32toh(le_val);
    }
    uint32_t le_val;
} __attribute__((packed));


struct le_int16_t {
    constexpr le_int16_t() : le_val(0) {}
    constexpr le_int16_t(const int16_t &val) : le_val(htole16(val)) {}
    constexpr operator int16_t() const noexcept {
        return le16toh(le_val);
    }
    int16_t le_val;
} __attribute__((packed));

struct le_uint16_t {
    constexpr le_uint16_t() : le_val(0) {}
    constexpr le_uint16_t(const uint16_t &val) : le_val(htole16(val)) {}
    constexpr operator uint16_t() const noexcept {
        return le16toh(le_val);
    }
    uint16_t le_val;
} __attribute__((packed));

} // namespace common
