// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <cstdarg>
#include <cstdio>
#include <libaudcore/runtime.h>

#include "common/constexpr.h"

using namespace std;

namespace {

// audlog:* doesn't support va_list...
_CONSTEXPR_F2 void log(audlog::Level level, const char *file, int line, const char *func, const char *fmt, va_list args) noexcept {
    char buffer[512];
    vsnprintf(buffer, sizeof buffer, fmt, args);
    audlog::log(level, file, line, func, "%s", buffer);
}

} // namespace {}

namespace logger {

void debug(const char *file, int line, const char *func, const char *fmt, ...) noexcept {
    va_list args;
    va_start(args, fmt);
    log(audlog::Debug, file, line, func, fmt, args); 
    va_end(args);
}

void info(const char *file, int line, const char *func, const char *fmt, ...) noexcept {
    va_list args;
    va_start(args, fmt);
    log(audlog::Info, file, line, func, fmt, args); 
    va_end(args);
}

void warn(const char *file, int line, const char *func, const char *fmt, ...) noexcept {
    va_list args;
    va_start(args, fmt);
    log(audlog::Warning, file, line, func, fmt, args); 
    va_end(args);
}

void error(const char *file, int line, const char *func, const char *fmt, ...) noexcept {
    va_list args;
    va_start(args, fmt);
    log(audlog::Error, file, line, func, fmt, args); 
    va_end(args);
}

} // namespace logger
