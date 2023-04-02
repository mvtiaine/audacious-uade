// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <stdexcept>

#include "converter.h"

using namespace std;

namespace converter::med {

bool isMED4(const char *buf, const size_t size);
ConverterResult convertMED4(const char *buf, size_t size);

} // namespace converter::med

namespace converter {

bool needs_conversion(const char *buf, const size_t size) {
    return med::isMED4(buf, size); 
}

ConverterResult convert(const char *buf, const size_t size) {
    ConverterResult res {};
    try {
        if (med::isMED4(buf, size)) {
            res = med::convertMED4(buf, size);
        } else {
            res.reason_failed = "unsupported file";
        }
    } catch (const out_of_range& e) {
        res.reason_failed = "corrupted file";
    }
    return res;
}

} // namespace converter
