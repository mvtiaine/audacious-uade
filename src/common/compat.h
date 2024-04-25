// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#if defined(__AMIGA__) && !defined(__amigaos4__)

#if !defined(__MORPHOS__)
namespace std {
    struct mutex {
        inline void lock() {}
        inline void unlock() {}
    };
}
#endif

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

#if !defined(__AROS__) && !defined(__MORPHOS__)
inline size_t strnlen(const char *s, size_t len) noexcept {
    size_t i;

    if( s == NULL )
    	return 0;

    for(i = 0; i < len && s[i]; i++)
	    ;
    return i;
}
#endif

#endif // __AMIGA__
