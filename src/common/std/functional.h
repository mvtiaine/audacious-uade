// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#if __has_include(<version>)
#include <version>
#elif __has_include(<bits/c++config.h>)
#include <bits/c++config.h>
#endif

// std::not_fn is available in GCC/libstdc++-7+, Clang/libc++-7+, but __cpp_lib_not_fn not defined
#if defined(__cpp_lib_not_fn) \
|| (defined(_GLIBCXX_RELEASE) && _GLIBCXX_RELEASE >= 7) \
|| (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 7000)
#include <functional>
#else
#include <experimental/functional>
#ifndef __cpp_lib_experimental_not_fn
#error "std::not_fn or std::experimental::not_fn is required!"
#endif
namespace std {
template <typename Fn>
const auto not_fn = experimental::not_fn<Fn>;
}
#endif
