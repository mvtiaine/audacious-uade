// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include "player/player.h"

namespace songend::precalc {

bool allow_songend_error(const player::ModuleInfo &info);
common::SongEnd precalc_song_end(const player::ModuleInfo &info, const char *buf, size_t size, int subsong, const std::string &md5hex);

} // namespace songend::precalc
