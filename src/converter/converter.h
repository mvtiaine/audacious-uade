// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <string>
#include <vector>

//#define DEBUG_TRACE 1

#ifndef TRACE
#ifdef DEBUG_TRACE
# define TRACE(fmt,...) fprintf(stderr, fmt, ## __VA_ARGS__)
#else
# define TRACE(fmt,...) while (0)
#endif
#endif

namespace converter {

struct ConverterResult {
    bool success = false;
    std::string ext;
    std::string format;
    std::vector<char> data;
    std::string reason_failed;
};

constexpr int MAGIC_SIZE = 4;
bool needs_conversion(const char *buf, size_t size) noexcept;
ConverterResult convert(const char *buf, size_t size) noexcept;

} // namespace converter
