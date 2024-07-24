// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <cstdarg>
#include <cstdio>

using namespace std;

namespace logger {

void debug(const char */*file*/, int /*line*/, const char */*func*/, const char *fmt, ...) noexcept {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);   
}

void info(const char */*file*/, int /*line*/, const char */*func*/, const char *fmt, ...) noexcept {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);  
}

void warn(const char */*file*/, int /*line*/, const char */*func*/, const char *fmt, ...) noexcept {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args); 
}

void error(const char */*file*/, int /*line*/, const char */*func*/, const char *fmt, ...) noexcept {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args); 
}

} // namespace logger
