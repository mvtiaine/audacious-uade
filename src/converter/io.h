// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2023-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <cassert>
#include <csetjmp>
#include <cstdint>
#include <vector>

#include "common/constexpr.h"
#include "common/endian.h"

using namespace common;

namespace converter {

extern thread_local std::jmp_buf error_handler;

constexpr_f2 void verify(const bool cond) noexcept {
    if (!cond) {
        std::longjmp(converter::error_handler, true);
    }
}

constexpr_f2 int8_t reads8be(const std::vector<char> &buf, size_t &offs) noexcept {
    verify(buf.size() > offs);
    return buf[offs++];
}

constexpr_f2 uint8_t readu8be(const std::vector<char> &buf, size_t &offs) noexcept {
    verify(buf.size() > offs);
    return buf[offs++];
}

constexpr_f2 be_int16_t reads16be(const std::vector<char> &buf, size_t &offs) noexcept {
    verify(buf.size() >= offs + 2);
    int8_t a;
    uint8_t b;
    a = buf[offs];
    b = buf[offs+1];
    offs += 2;
    return be_int16_t((a << 8) | b);
}

constexpr_f2 be_uint16_t readu16be(const std::vector<char> &buf, size_t &offs) noexcept {
    verify(buf.size() >= offs + 2);
    uint8_t a, b;
    a = buf[offs];
    b = buf[offs+1];
    offs += 2;
    return be_uint16_t((a << 8) | b);
}

constexpr_f2 be_int32_t reads32be(const std::vector<char> &buf, size_t &offs) noexcept {
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

constexpr_f2 be_uint32_t readu32be(const std::vector<char> &buf, size_t &offs) noexcept {
    verify(buf.size() >= offs + 4);
    uint8_t a, b, c, d;
    a = buf[offs];
    b = buf[offs+1];
    c = buf[offs+2];
    d = buf[offs+3];
    offs += 4;
    return be_uint32_t((a << 24) | (b << 16) | (c << 8) | d);
}

constexpr_f2 std::vector<char> readbytes(const std::vector<char> &buf, size_t &offs, const size_t n) noexcept {
    assert(n > 0);
    verify(buf.size() >= offs + n);
    std::vector<char> chars(n);
    copy(buf.begin() + offs, buf.begin() + offs + n, chars.begin());
    offs += n;
    return chars;
}

constexpr_f2 std::vector<uint8_t> readu8bytes(const std::vector<char> &buf, size_t &offs, const size_t n) noexcept {
    assert(n > 0);
    verify(buf.size() >= offs + n);
    std::vector<uint8_t> chars(n);
    copy(buf.begin() + offs, buf.begin() + offs + n, chars.begin());
    offs += n;
    return chars;
}

constexpr_f2 void copybytes(const std::vector<char> &buf, char *dst, size_t &offs, const size_t n) noexcept {
    if (n == 0) return;
    verify(buf.size() >= offs + n);
    copy(buf.begin() + offs, buf.begin() + offs + n, dst);
    offs += n;
}

constexpr_f2 void copyu8bytes(const std::vector<char> &buf, uint8_t *dst, size_t &offs, const size_t n) noexcept {
    if (n == 0) return;
    assert(n > 0);
    verify(buf.size() >= offs + n);
    copy(buf.begin() + offs, buf.begin() + offs + n, dst);
    offs += n;
}

} // namespace converter
