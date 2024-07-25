// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <string>

#include "common/constexpr.h"
#include "player/player.h"

namespace songend::precalc {

_CONSTEXPR_F2 bool allow_songend_error(const std::string &format) noexcept {
    return format == "VSS";
}

common::SongEnd precalc_song_end(const player::ModuleInfo &info, const char *buf, size_t size, int subsong, const std::string &md5hex) noexcept;

} // namespace songend::precalc
