// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <deadbeef/deadbeef.h>
#include <cstdarg>
#include <cstdio>

#include "common/constexpr.h"
#include "common/logger.h"

namespace plugin::deadbeef {
extern DB_decoder_t *uade_plugin;
extern DB_functions_t *deadbeef;
}

using namespace std;
using namespace plugin::deadbeef;

// file, line, func ignored
namespace logger {

void debug(const char *, int, const char *, const char *fmt, ...) noexcept {
    (void)fmt;
#if DEBUG_TRACE
    va_list args;
    va_start(args, fmt);
    deadbeef->vlog_detailed(&uade_plugin->plugin, DDB_LOG_LAYER_INFO, fmt, args);
    va_end(args);
#endif
}

void info(const char *, int, const char *, const char *fmt, ...) noexcept {
    va_list args;
    va_start(args, fmt);
    deadbeef->vlog_detailed(&uade_plugin->plugin, DDB_LOG_LAYER_INFO, fmt, args);
    va_end(args);
}

void warn(const char *, int, const char *, const char *fmt, ...) noexcept {
    va_list args;
    va_start(args, fmt);
    deadbeef->vlog_detailed(&uade_plugin->plugin, DDB_LOG_LAYER_INFO, fmt, args);
    va_end(args);
}

void error(const char *, int, const char *, const char *fmt, ...) noexcept {
    va_list args;
    va_start(args, fmt);
    deadbeef->vlog_detailed(&uade_plugin->plugin, DDB_LOG_LAYER_DEFAULT, fmt, args);
    va_end(args);
}

} // namespace logger
