// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#ifndef COMMON_H_
#define COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define PLUGIN_NAME "uade"

//#define DEBUG_TRACE 1

#ifdef __cplusplus
#include <libaudcore/runtime.h>
#define DEBUG AUDDBG
#else
#define DEBUG(fmt,...) fprintf(stdout, fmt, ## __VA_ARGS__)
#endif
#define WARN(fmt,...) fprintf(stderr, fmt, ## __VA_ARGS__)
#define ERROR(fmt,...) fprintf(stderr, fmt, ## __VA_ARGS__)

#ifdef DEBUG_TRACE
# define TRACE DEBUG
#else
# define TRACE(fmt,...) while (0)
#endif

// pref keys
#define MODLAND_ALLMODS_MD5_FILE "modland_allmods_md5_file"
#define PRECALC_SONGLENGTHS "precalc_songlengths"

#ifdef __cplusplus
} // extern "C"
#endif
#endif /* COMMON_H_ */
