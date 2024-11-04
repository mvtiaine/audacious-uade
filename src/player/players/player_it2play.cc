// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#include "common/std/optional.h"

#include <cstddef>
#include <utility>

#include "player/player.h"

namespace {

constexpr size_t mixBufSize(const int frequency) noexcept {
    return 4 * 4 * (frequency / 50 + (frequency % 50 != 0 ? 1 : 0));
}

} // namespace {}

namespace player::it2play {

void init() noexcept {
}

void shutdown() noexcept {
}

bool is_our_file(const char *path, const char *buf, size_t size) noexcept {

}
optional<ModuleInfo> parse(const char *path, const char *buf, size_t size) noexcept  {

}
optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &config) noexcept {

}
pair<SongEnd::Status,size_t> render(PlayerState &state, char *buf, size_t size) noexcept {

}
bool stop(PlayerState &state) noexcept {

}
bool restart(PlayerState &state) noexcept {

}
