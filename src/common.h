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
