// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2023-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <cassert>
#include <cstdint>
#include <mutex>
#include <utility>

#include "common/constexpr.h"
#include "common/logger.h"
#include "player/player.h"

extern "C"
{
#include "3rdparty/replay/hivelytracker/replay.h"
}

using namespace std;
using namespace common;
using namespace player;

namespace {

constexpr size_t mixBufSize(const int frequency) noexcept {
    return 4 * (frequency / 50 + (frequency % 50 != 0 ? 1 : 0));
}

constexpr_f2 ModuleInfo get_info(const string &path, struct hvl_tune *ht) noexcept  {
    const int maxsubsong = ht->ht_SubsongNr;
    const int channels = ht->ht_Channels;
    // TODO version ?
    const string format = "HivelyTracker";

    return {Player::hivelytracker, format, path, 0, maxsubsong, 0, channels};
}

bool initialized = false;
mutex init_guard;
void lazy_init() noexcept {
    if (!initialized) {
        init_guard.lock();
        if (!initialized) {
            hvl_InitReplayer();
            initialized = true;
        }
        init_guard.unlock();
    }
}

} // namespace {}

namespace player::hivelytracker {

void init() noexcept {}

void shutdown() noexcept {}

bool is_our_file(const char */*path*/, const char *buf, size_t size) noexcept {
    assert(size >= 4);
    return buf[0] == 'H' && buf[1] == 'V' && buf[2] == 'L' && buf[3] < 2;
}

optional<ModuleInfo> parse(const char *path, const char *buf, size_t size) noexcept {
    struct hvl_tune *ht = hvl_reset((uint8_t*)buf, size, 0, 0);
    if (!ht) {
        WARN("player_hivelytracker::parse parsing failed for %s\n", path);
        return {};
    }

    const auto &info = get_info(path, ht);
    hvl_FreeTune(ht);
    return info;
}

optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &config) noexcept {
    assert(subsong >= 0);
    lazy_init();
    struct hvl_tune *ht = hvl_reset((uint8_t*)buf, size, 0, config.frequency);
    if (!ht) {
        ERR("player_hivelytracker::play parsing failed for %s\n", path);
        return {};
    }

    assert(subsong <= ht->ht_SubsongNr);
    if (!hvl_InitSubsong(ht, subsong)) {
        ERR("player_hivelytracker::play init subsong failed for %s\n", path);
        hvl_FreeTune(ht);
        return {};
    }

    return PlayerState {Player::hivelytracker, subsong, config.frequency, config.endian != endian::native, ht, true, mixBufSize(config.frequency), 0};
}

bool stop(PlayerState &state) noexcept {
    assert(state.player == Player::hivelytracker);
    if (state.context) {
        auto ht = static_cast<struct hvl_tune*>(state.context);
        assert(ht);
        hvl_FreeTune(ht);
    }
    return true;
}

pair<SongEnd::Status, size_t> render(PlayerState &state, char *buf, size_t size) noexcept {
    assert(state.player == Player::hivelytracker);
    assert(size >= mixBufSize(state.frequency));
    auto ht = static_cast<struct hvl_tune*>(state.context);
    assert(ht);
    bool songend = ht->ht_SongEndReached;
    size_t totalbytes = 0;
    int8_t *mixbuf = (int8_t*)buf;
    size_t framelen = state.frequency*2*2/50;
    while (!songend && totalbytes + framelen <= mixBufSize(state.frequency)) {
        hvl_DecodeFrame(ht, mixbuf, mixbuf + 2, 4);
        totalbytes += framelen;
        songend = ht->ht_SongEndReached;
        mixbuf += framelen;
    }
    return pair<SongEnd::Status, size_t>(songend ? SongEnd::PLAYER : SongEnd::NONE, totalbytes);
}

bool restart(PlayerState &state) noexcept {
    assert(state.player == Player::hivelytracker);
    auto ht = static_cast<struct hvl_tune*>(state.context);
    assert(ht);
    return hvl_InitSubsong(ht, state.subsong);
}

} // namespace player::hivelytracker
