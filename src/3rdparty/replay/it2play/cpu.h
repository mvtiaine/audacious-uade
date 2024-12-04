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
