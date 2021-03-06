/*
    Copyright (C) 2014  Matti Tiainen <mvtiaine@cc.hut.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>

#include <audacious/debug.h>

#define PLUGIN_NAME "uade"

//#define DEBUG_TRACE 1

#ifdef DEBUG_TRACE
# define TRACE AUDDBG
#else
# define TRACE(fmt,...) while (0)
#endif

#define DEBUG AUDDBG
#define WARN(fmt,...) fprintf(stderr, fmt, ## __VA_ARGS__)
#define ERROR(fmt,...) fprintf(stderr, fmt, ## __VA_ARGS__)

// pref keys
#define MODLAND_ALLMODS_MD5_FILE "modland_allmods_md5_file"

#endif /* COMMON_H_ */
