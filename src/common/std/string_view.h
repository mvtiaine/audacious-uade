// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2024-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#if __has_include(<string_view>)
#include <string_view>
#elif __has_include(<experimental/string_view>)
#include <experimental/string_view>
namespace std {
using string_view = experimental::string_view;
}
#else
#error "<string_view> or <experimental/string_view> is required!"
#endif
