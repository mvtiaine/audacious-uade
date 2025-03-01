// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include "constexpr.h"

// XXX need trailing comma depending if __VA_OPT__ is supported or not (gcc < 8.1, clang < 6.0)
#define PP_THIRD_ARG(a,b,c,...) c
#define VA_OPT_SUPPORTED_I(...) PP_THIRD_ARG(__VA_OPT__(,),true,false,)
#define VA_OPT_SUPPORTED VA_OPT_SUPPORTED_I(?)
#if VA_OPT_SUPPORTED
#define TRAILING_COMMA
#else
#define TRAILING_COMMA ,
#endif
#define VA_LIST(...) __VA_ARGS__ TRAILING_COMMA

#if defined(__QNX__)
#include <sys/neutrino.h>
#if _NTO_VERSION <= 650
// XXX error: call of overloaded 'to_string(const float&)' is ambiguous
#define _GLIBCXX_USE_C99_STDIO 1
#endif
#endif

#if (defined(__AMIGA__) || defined(__COSMOCC__) || defined(__serenity__)) && !defined(__amigaos4__) && !defined(WARPUP)
#include <cstddef>
#include <sys/types.h>
extern "C" {
#if defined(__MORPHOS__)
inline void swab(const void *bfrom, void *bto, size_t n) noexcept {
#else
inline void swab(const void *bfrom, void *bto, ssize_t n) noexcept {
#endif
  const char *from = (const char *) bfrom;
  char *to = (char *) bto;
  n &= ~((ssize_t) 1);
  while (n > 1)
    {
      const char b0 = from[--n], b1 = from[--n];
      to[n] = b0;
      to[n + 1] = b1;
    }
}
}
#endif

#if (defined(__AMIGA__) || defined(__MINT__)) && !defined(__amigaos4__) && !defined(__MORPHOS__)

namespace std {
    struct mutex {
        constexpr_f void lock() {}
        constexpr_f void unlock() {}
    };
}

#endif // __AMIGA__

#if (defined(__QNX__) && _NTO_VERSION <= 650) || defined(__BLACKBERRY__)
#include <cassert>
#include <cstdlib>
#include <string>
namespace std {
constexpr_f2 int stoi(const std::string& str, size_t* idx = nullptr, int base = 10) {
    assert(idx == nullptr);
    assert(base == 10);
    return atoi(str.c_str());
}
}
#endif

#if defined(__libnix__) || defined(__CLIB2__) || defined(__CLIB4__) || defined (__AROS__) || defined (__ixemul__)
// XXX __NEWLIB__ may get defined via sys includes even with other runtimes
#  ifdef __NEWLIB__
#    undef __NEWLIB__
#  endif
#endif

#if defined(__CLIB2__)
#warning "clib2 runtime not supported"
#endif

#if defined(__ixemul__)
#warning "ixemul runtime not supported"
#endif

#if defined(__AMIGA__) && defined(__NEWLIB__) && !defined(__amigaos4__)
#warning "newlib runtime not supported on AmigaOS3"
#endif
