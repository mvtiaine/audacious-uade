// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#if __has_include(<optional>)
#include <optional>
#elif __has_include(<experimental/optional>)
#include <experimental/optional>
namespace std {
template <typename T>
using optional = typename experimental::optional<T>;
}
#else
#error "<optional> or <experimental/optional> is required!"
#endif
