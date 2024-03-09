// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#ifdef __AMIGA__

#if !defined(__MORPHOS__) && !defined(__amigaos4__)
namespace std {
    struct mutex {
        inline void lock() {}
        inline void unlock() {}
    };
}
#endif

#if !defined(__amigaos4__)
inline void swab (const void *bfrom, void *bto, ssize_t n) {
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

#endif // __AMIGA__
