// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <cassert>
#include <cstdint>
#include <stdexcept>
#ifdef __APPLE__
#include "3rdparty/macos_endian.h"
#define CONSTEXPR constexpr
#else
#include <endian.h>
#define CONSTEXPR
#endif

namespace converter {

struct be_int32_t {
    constexpr be_int32_t() : be_val(0) {}
    CONSTEXPR be_int32_t(const int32_t &val) : be_val(htobe32(val)) {}
    CONSTEXPR operator int32_t() const {
        return be32toh(be_val);
    }
    int32_t be_val;
} __attribute__((packed));

struct be_uint32_t {
    constexpr be_uint32_t() : be_val(0) {}
    CONSTEXPR be_uint32_t(const uint32_t &val) : be_val(htobe32(val)) {}
    CONSTEXPR operator uint32_t() const {
        return be32toh(be_val);
    }
    uint32_t be_val;
} __attribute__((packed));


struct be_int16_t {
    constexpr be_int16_t() : be_val(0) {}
    CONSTEXPR be_int16_t(const int16_t &val) : be_val(htobe16(val)) {}
    CONSTEXPR operator int16_t() const {
        return be16toh(be_val);
    }
    int16_t be_val;
} __attribute__((packed));

struct be_uint16_t {
    constexpr be_uint16_t() : be_val(0) {}
    CONSTEXPR be_uint16_t(const uint16_t &val) : be_val(htobe16(val)) {}
    CONSTEXPR operator uint16_t() const {
        return be16toh(be_val);
    }
    uint16_t be_val;
} __attribute__((packed));

inline void verify(const bool cond) {
    if (!cond) {
        throw std::out_of_range("read past EOF");
    }
}

inline int8_t reads8be(const std::vector<char> &buf, size_t &offs) {
    verify(buf.size() > offs);
    return buf[offs++];
}

inline uint8_t readu8be(const std::vector<char> &buf, size_t &offs) {
    verify(buf.size() > offs);
    return buf[offs++];
}

inline be_int16_t reads16be(const std::vector<char> &buf, size_t &offs) {
    verify(buf.size() >= offs + 2);
    int8_t a;
    uint8_t b;
    a = buf[offs];
    b = buf[offs+1];
    offs += 2;
    return be_int16_t((a << 8) | b);
}

inline be_uint16_t readu16be(const std::vector<char> &buf, size_t &offs) {
    verify(buf.size() >= offs + 2);
    uint8_t a, b;
    a = buf[offs];
    b = buf[offs+1];
    offs += 2;
    return be_uint16_t((a << 8) | b);
}

inline be_int32_t reads32be(const std::vector<char> &buf, size_t &offs) {
    verify(buf.size() >= offs + 4);
    int8_t a;
    uint8_t b, c, d;
    a = buf[offs];
    b = buf[offs+1];
    c = buf[offs+2];
    d = buf[offs+3];
    offs += 4;
    return be_int32_t((a << 24) | (b << 16) | (c << 8) | d);
}

inline be_uint32_t readu32be(const std::vector<char> &buf, size_t &offs) {
    verify(buf.size() >= offs + 4);
    uint8_t a, b, c, d;
    a = buf[offs];
    b = buf[offs+1];
    c = buf[offs+2];
    d = buf[offs+3];
    offs += 4;
    return be_uint32_t((a << 24) | (b << 16) | (c << 8) | d);
}

inline std::vector<char> readbytes(const std::vector<char> &buf, size_t &offs, const size_t n) {
    assert(n > 0);
    verify(buf.size() >= offs + n);
    std::vector<char> chars(n);
    copy(buf.begin() + offs, buf.begin() + offs + n, chars.begin());
    offs += n;
    return chars;
}

inline std::vector<uint8_t> readu8bytes(const std::vector<char> &buf, size_t &offs, const size_t n) {
    assert(n > 0);
    verify(buf.size() >= offs + n);
    std::vector<uint8_t> chars(n);
    copy(buf.begin() + offs, buf.begin() + offs + n, chars.begin());
    offs += n;
    return chars;
}

inline void copybytes(const std::vector<char> &buf, char *dst, size_t &offs, const size_t n) {
    if (n == 0) return;
    verify(buf.size() >= offs + n);
    copy(buf.begin() + offs, buf.begin() + offs + n, dst);
    offs += n;
}

inline void copyu8bytes(const std::vector<char> &buf, uint8_t *dst, size_t &offs, const size_t n) {
    if (n == 0) return;
    assert(n > 0);
    verify(buf.size() >= offs + n);
    copy(buf.begin() + offs, buf.begin() + offs + n, dst);
    offs += n;
}

} // namespace converter
