// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#if __cplusplus < 202002L
#define _CONSTEXPR inline
#else 
#define _CONSTEXPR constexpr
#endif

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
        inline void lock() {}
        inline void unlock() {}
    };
}

#if !defined(__AROS__)
constexpr size_t strnlen(const char *s, size_t len) noexcept {
    size_t i;

    if( s == NULL )
    	return 0;

    for(i = 0; i < len && s[i]; i++)
	    ;
    return i;
}
#endif

#endif // __AMIGA__

#if defined(__AROS__) || defined(__QNX__)
#include <cstring>
namespace std {
// std::bit_cast not available, use example implementation from
// https://en.cppreference.com/w/cpp/numeric/bit_cast
template<class To, class From>
std::enable_if_t<
    sizeof(To) == sizeof(From) &&
    std::is_trivially_copyable_v<From> &&
    std::is_trivially_copyable_v<To>,
    To>
// constexpr support needs compiler magic
bit_cast(const From& src) noexcept
{
    static_assert(std::is_trivially_constructible_v<To>,
        "This implementation additionally requires "
        "destination type to be trivially constructible");
 
    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
}
} // namespace std
#endif

#ifdef __QNX__
#include <cassert>
#include <cstdlib>
#include <string>
namespace std {
inline int stoi(const std::string& str, size_t* idx = nullptr, int base = 10) {
    assert(idx == nullptr);
    assert(base == 10);
    return atoi(str.c_str());
}
}
#endif

#if defined(__MINT__)

namespace std {
    struct mutex {
        inline void lock() {}
        inline void unlock() {}
    };
}

#endif
