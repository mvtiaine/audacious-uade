// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <csetjmp>

#include "converter/converter.h"

using namespace std;

namespace converter::med {

bool isMED4(const char *buf, const size_t size) noexcept;
ConverterResult convertMED4(const char *buf, size_t size) noexcept;

} // namespace converter::med

namespace converter {

jmp_buf error_handler;

bool needs_conversion(const char *buf, const size_t size) noexcept {
    return med::isMED4(buf, size); 
}

ConverterResult convert(const char *buf, const size_t size) noexcept {
    ConverterResult res {};

    if (!med::isMED4(buf, size)) {
        res.reason_failed = "unsupported file";
        return res;
    }

    if (setjmp(error_handler)) {
        res.reason_failed = "corrupted file";
        return res;
    }

    res = med::convertMED4(buf, size);

    return res;
}

} // namespace converter
