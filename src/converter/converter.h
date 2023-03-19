// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#ifndef CONVERTER_H_
#define CONVERTER_H_

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

using namespace std;

namespace converter {

struct ConverterResult {
    bool success = false;
    string ext;
    string format;
    vector<char> data;
    string reason_failed;
};

constexpr size_t MAGIC_SIZE = 4;
bool needs_conversion(const char *buf, size_t size);
ConverterResult convert(const char *buf, size_t size);

} // namespace converter

#endif // CONVERTER_H_
