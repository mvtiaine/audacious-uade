// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2024 Matti Tiainen <mvtiaine@cc.hut.fi>

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

#if (defined(__AMIGA__) || defined(__COSMOCC__)) && !defined(__amigaos4__) && !defined(WARPUP)
#include <cstddef>
inline void swab(const void *bfrom, void *bto, ssize_t n) noexcept {
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
#endif

#if defined(__AMIGA__) && !defined(__amigaos4__) && !defined(__MORPHOS__)

namespace std {
    struct mutex {
        constexpr_f void lock() {}
        constexpr_f void unlock() {}
    };
}

#if !defined(__AROS__)
constexpr size_t strnlen(const char *s, size_t len) noexcept {
    size_t i = 0;

    if( s == NULL )
    	return 0;

    for(i = 0; i < len && s[i]; i++)
	    ;
    return i;
}
#endif

#endif // __AMIGA__

#ifdef __QNX__
#include <cassert>
#include <cstdlib>
#include <string>
namespace std {
constexpr_f int stoi(const std::string& str, size_t* idx = nullptr, int base = 10) {
    assert(idx == nullptr);
    assert(base == 10);
    return atoi(str.c_str());
}
}
#endif

#if defined(__MINT__)

namespace std {
    struct mutex {
        constexpr_f void lock() {}
        constexpr_f void unlock() {}
    };
}

#endif
