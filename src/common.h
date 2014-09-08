#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>

#define DEBUG 1

#ifdef DEBUG
# define DBG(fmt,...) printf(fmt, ## __VA_ARGS__)
#else
# define DBG(fmt,...) while (0)
#endif

#define WRN(fmt,...) fprintf(stderr, fmt, ## __VA_ARGS__)
#define ERR(fmt,...) fprintf(stderr, fmt, ## __VA_ARGS__)

#endif /* COMMON_H_ */
