// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#ifndef AUDACIOUS_UADE
#include <stdint.h>
#endif

#ifdef _WIN32

#ifdef _WIN64
#define CPU_32BIT 0
#define CPU_64BIT 1
#else
#define CPU_32BIT 1
#define CPU_64BIT 0
#endif

#else
#ifndef AUDACIOUS_UADE
#include <limits.h>
#endif

#if __WORDSIZE == 64
#define CPU_32BIT 0
#define CPU_64BIT 1
#else
#define CPU_32BIT 1
#define CPU_64BIT 0
#endif

#endif

#if CPU_64BIT
#define CPU_BITS 64
#define uintCPUWord_t uint64_t
#define intCPUWord_t int64_t
#else
#define CPU_BITS 32
#define uintCPUWord_t uint32_t
#define intCPUWord_t int32_t
#endif

// mvtiaine: added big endian support
#define SWAP16(value) \
((uint16_t)( \
	((uint16_t)(value) << 8) | \
	((uint16_t)(value) >> 8) \
))
#define SWAP32(value) \
((uint32_t)( \
	((uint32_t)(value) << 24) | \
	(((uint32_t)(value) & 0x0000FF00U) << 8) | \
	(((uint32_t)(value) & 0x00FF0000U) >> 8) | \
	((uint32_t)(value) >> 24) \
))
#ifdef WORDS_BIGENDIAN
#define READ16LE(value) SWAP16(value)
#define READ32LE(value) SWAP32(value)
#else
#define READ16LE(value) value
#define READ32LE(value) value
#endif
