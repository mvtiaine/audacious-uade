// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#include "common/std/optional.h"

#include "common/endian.h"
#include "common/logger.h"
#include "player/player.h"

extern "C" {
#include <xmp.h>
}

using namespace std;
using namespace common;
using namespace player;

namespace {

constexpr size_t mixBufSize(const int frequency) noexcept {
    return 4 * (frequency / 50 + (frequency % 50 != 0 ? 1 : 0));
}

} // namespace {}

namespace player::libxmp {

void init() noexcept {}

void shutdown() noexcept {}

bool is_our_file(const char *path, const char *buf, size_t size) noexcept {
	return xmp_test_module_from_memory(buf, size, nullptr) == 0;
}

optional<ModuleInfo> parse(const char *path, const char *buf, size_t size) noexcept {
    xmp_context c = xmp_create_context();
    if (!c) {
        ERR("player_libxmp::parse xmp_create_context failed for %s\n", path);
        return {};
    }
    int ret = xmp_load_module_from_memory(c, buf, size);
    if (ret) {
        DEBUG("player_libxmp::parse xmp_load_module_from_memory failed for %s (%d)\n", path, ret);
        xmp_free_context(c);
        return {};
    }
    xmp_module_info info;
    xmp_get_module_info(c, &info);
    const auto channels = info.mod->chn;
    const auto subsongs = info.num_sequences;
    const auto format = string(info.mod->type);
    xmp_release_module(c);
    xmp_free_context(c);
    return ModuleInfo {Player::libxmp, format, path, 0, subsongs - 1, 0, channels};
}

optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &_config) noexcept {
    assert(subsong >= 0);
    const auto &__config = static_cast<const LibXMPConfig&>(_config);
    const auto config = __config.player == Player::libxmp ? __config : LibXMPConfig(_config);
    xmp_context c = xmp_create_context();
    int frequency = min(_config.frequency, 48000);
    if (!c) {
        ERR("player_libxmp::play xmp_create_context failed for %s\n", path);
        return {};
    }
    int ret = xmp_load_module_from_memory(c, buf, size);
    if (ret) {
        ERR("player_libxmp::play xmp_load_module_from_memory failed for %s (%d)\n", path, ret);
        goto error;
    }
    xmp_module_info info;
    xmp_get_module_info(c, &info);
    if (subsong >= info.num_sequences) {
        ERR("player_libxmp::play invalid subsong %d for %s\n", subsong, path);
        goto error;
    }
    ret = xmp_start_player(c, frequency, 0);
    if (ret) {
        ERR("player_libxmp::play xmp_start_player failed for %s (%d)\n", path, ret);
        goto error;
    }
    if (subsong > 0) {
        int pos = info.seq_data[subsong].entry_point;
        ret = xmp_set_position(c, pos);
        if (ret != pos) {
            ERR("player_libxmp::play xmp_set_position failed for %s (%d)\n", path, ret);
            goto error;
        }
    }
    if (config.probe) {
        xmp_set_player(c, XMP_PLAYER_MIX, 0);
        xmp_set_player(c, XMP_PLAYER_INTERP, 0);
        xmp_set_player(c, XMP_PLAYER_DSP, 0);
    } else {
        xmp_set_player(c, XMP_PLAYER_INTERP, XMP_INTERP_SPLINE);
        xmp_set_player(c, XMP_PLAYER_DSP, XMP_DSP_ALL);
        xmp_set_player(c, XMP_PLAYER_MIX, (1.0f - config.panning) * 100);
        if (config.filter == Filter::A500) {
            xmp_set_player(c, XMP_PLAYER_FLAGS, XMP_FLAGS_A500);
        }
    }
    return PlayerState {Player::libxmp, subsong, frequency, config.endian != endian::native, c, !config.probe, mixBufSize(frequency), 0};
error:
    xmp_release_module(c);
    xmp_free_context(c);
    return {};
}

pair<SongEnd::Status,size_t> render(PlayerState &state, char *buf, size_t size) noexcept {
    assert(state.player == Player::libxmp);
    assert(size >= mixBufSize(state.frequency));
    xmp_context c = static_cast<xmp_context>(state.context);
    assert(c);
    int ret = xmp_play_buffer(c, buf, size, 1);
    bool songend = ret == -XMP_END;
    if (ret && !songend) {
        ERR("player_libxmp::render xmp_play_buffer failed (%d)\n", ret);
        return {SongEnd::ERROR, 0};
    }
    return {songend ? SongEnd::PLAYER : SongEnd::NONE, mixBufSize(state.frequency)};
}

bool stop(PlayerState &state) noexcept {
    assert(state.player == Player::libxmp);
    xmp_context c = static_cast<xmp_context>(state.context);
    assert(c);
    xmp_end_player(c);
    xmp_release_module(c);
    xmp_free_context(c);
    return true;
}

bool restart(PlayerState &state) noexcept {
    assert(state.player == Player::libxmp);
    xmp_context c = static_cast<xmp_context>(state.context);
    assert(c);
    xmp_restart_module(c);
    return true;
}

} // namespace player::libxmp
