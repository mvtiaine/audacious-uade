// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>

#define PLUGIN_NAME "uade"

//#define DEBUG_TRACE 1

#ifndef __cplusplus
# define DEBUG(fmt,...) fprintf(stderr, fmt, ## __VA_ARGS__)
#else
# include <libaudcore/runtime.h>
# define DEBUG AUDDBG
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

#endif /* COMMON_H_ */
