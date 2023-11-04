// SPDX-License-Identifier: BSD-3-Clause
/* These are normally defined in the Amiga's types library, but are
   defined here instead to ensure portability with minimal changes to the
   original Amiga source-code
*/

#ifndef HVL_TYPES_H
#define HVL_TYPES_H

#include <stdint.h>

typedef uint16_t		uint16;
typedef uint8_t		uint8;
typedef int16_t		int16;
typedef int8_t		int8;
typedef uint32_t		uint32;
typedef int32_t		int32;

typedef double			float64;
typedef char			TEXT;
typedef short			BOOL;
typedef int32_t			LONG;
typedef uint32_t		ULONG;
#define FALSE 0
#define TRUE 1
#define CONST const

#endif
