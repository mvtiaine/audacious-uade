// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

//#define DEBUG_TRACE 1

namespace logger {

void debug(const char *file, int line, const char *func, const char *fmt, ...) noexcept
    __attribute__((__format__(__printf__, 4, 5)));

void info(const char *file, int line, const char *func, const char *fmt, ...) noexcept
    __attribute__((__format__(__printf__, 4, 5)));

void warn(const char *file, int line, const char *func, const char *fmt, ...) noexcept
    __attribute__((__format__(__printf__, 4, 5)));

void error(const char *file, int line, const char *func, const char *fmt, ...) noexcept
    __attribute__((__format__(__printf__, 4, 5)));

} // namespace::logger

#define DEBUG(...) do { \
    logger::debug(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); \
} while (0)

#define INFO(...) do { \
    logger::info(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); \
} while (0)

#define WARN(...) do { \
    logger::warn(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); \
} while (0)

#define ERR(...) do { \
    logger::error(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); \
} while (0)

#if DEBUG_TRACE
# define TRACE DEBUG
#else
# define TRACE(...) while (0)
#endif
