// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <cassert>
#include <csetjmp>
#include <cstdint>
#include <vector>

#include "common/endian.h"

using namespace common;

namespace converter {

extern std::jmp_buf error_handler;

inline void verify(const bool cond) noexcept {
    if (!cond) {
        std::longjmp(converter::error_handler, true);
    }
}

inline int8_t reads8be(const std::vector<char> &buf, size_t &offs) noexcept {
    verify(buf.size() > offs);
    return buf[offs++];
}

inline uint8_t readu8be(const std::vector<char> &buf, size_t &offs) noexcept {
    verify(buf.size() > offs);
    return buf[offs++];
}

inline be_int16_t reads16be(const std::vector<char> &buf, size_t &offs) noexcept {
    verify(buf.size() >= offs + 2);
    int8_t a;
    uint8_t b;
    a = buf[offs];
    b = buf[offs+1];
    offs += 2;
    return be_int16_t((a << 8) | b);
}

inline be_uint16_t readu16be(const std::vector<char> &buf, size_t &offs) noexcept {
    verify(buf.size() >= offs + 2);
    uint8_t a, b;
    a = buf[offs];
    b = buf[offs+1];
    offs += 2;
    return be_uint16_t((a << 8) | b);
}

inline be_int32_t reads32be(const std::vector<char> &buf, size_t &offs) noexcept {
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

inline be_uint32_t readu32be(const std::vector<char> &buf, size_t &offs) noexcept {
    verify(buf.size() >= offs + 4);
    uint8_t a, b, c, d;
    a = buf[offs];
    b = buf[offs+1];
    c = buf[offs+2];
    d = buf[offs+3];
    offs += 4;
    return be_uint32_t((a << 24) | (b << 16) | (c << 8) | d);
}

inline std::vector<char> readbytes(const std::vector<char> &buf, size_t &offs, const size_t n) noexcept {
    assert(n > 0);
    verify(buf.size() >= offs + n);
    std::vector<char> chars(n);
    copy(buf.begin() + offs, buf.begin() + offs + n, chars.begin());
    offs += n;
    return chars;
}

inline std::vector<uint8_t> readu8bytes(const std::vector<char> &buf, size_t &offs, const size_t n) noexcept {
    assert(n > 0);
    verify(buf.size() >= offs + n);
    std::vector<uint8_t> chars(n);
    copy(buf.begin() + offs, buf.begin() + offs + n, chars.begin());
    offs += n;
    return chars;
}

inline void copybytes(const std::vector<char> &buf, char *dst, size_t &offs, const size_t n) noexcept {
    if (n == 0) return;
    verify(buf.size() >= offs + n);
    copy(buf.begin() + offs, buf.begin() + offs + n, dst);
    offs += n;
}

inline void copyu8bytes(const std::vector<char> &buf, uint8_t *dst, size_t &offs, const size_t n) noexcept {
    if (n == 0) return;
    assert(n > 0);
    verify(buf.size() >= offs + n);
    copy(buf.begin() + offs, buf.begin() + offs + n, dst);
    offs += n;
}

} // namespace converter
