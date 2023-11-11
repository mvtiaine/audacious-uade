// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <cassert>
#include <cstdint>

#include "common/logger.h"
#include "player/player.h"
extern "C"
{
#include "3rdparty/replay/hvl/replay.h"
}

using namespace std;
using namespace player;

namespace {

ModuleInfo get_info(const string &path, struct hvl_tune *ht) {
    const int maxsubsong = ht->ht_SubsongNr;
    const int channels = ht->ht_Channels;
    // TODO version ?
    const string format = "HivelyTracker";

    return {Player::hvl, format, path, 0, maxsubsong, 0, channels};
}

} // namespace {}

namespace player::hvl {

void init() {
    hvl_InitReplayer();
}

void shutdown() {}

bool is_our_file(const char */*path*/, const char *buf, size_t size) {
    assert(size >= 4);
    return buf[0] == 'H' && buf[1] == 'V' && buf[2] == 'L' && buf[3] < 2;
}

optional<ModuleInfo> parse(const char *path, const char *buf, size_t size) {
    struct hvl_tune *ht = hvl_reset((uint8_t*)buf, size, 0, 0);
    if (!ht) {
        ERR("player_hvl::parse parsing failed for %s\n", path);
        return {};
    }

    const auto &info = get_info(path, ht);
    hvl_FreeTune(ht);
    return info;
}

optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &config) {
    struct hvl_tune *ht = hvl_reset((uint8_t*)buf, size, 0, config.frequency);
    if (!ht) {
        ERR("player_hvl::play parsing failed for %s\n", path);
        return {};
    }

    if (subsong == -1) {
        subsong = ht->ht_SongNum;
    }

    assert(subsong >= 0 && subsong <= ht->ht_SubsongNr);
    if (!hvl_InitSubsong(ht, subsong)) {
        ERR("player_hvl::play init subsong failed for %s\n", path);
        hvl_FreeTune(ht);
        return {};
    }

    const auto &info = get_info(path, ht);
    PlayerState state = {info, subsong, config.frequency, config.endian != endian::native, ht, true, 0};
    return state;
}

bool stop(PlayerState &state) {
    assert(state.info.player == Player::hvl);
    if (state.context) {
        auto ht = static_cast<struct hvl_tune*>(state.context);
        assert(ht);
        hvl_FreeTune(ht);
    }
    return true;
}

pair<SongEnd::Status, size_t> render(PlayerState &state, char *buf, size_t size) {
    assert(state.info.player == Player::hvl);
    auto ht = static_cast<struct hvl_tune*>(state.context);
    assert(ht);
    bool songend = ht->ht_SongEndReached;
    size_t totalbytes = 0;
    int8_t *mixbuf = (int8_t*)buf;
    size_t framelen = state.frequency*2*2/50;
    while (!songend && totalbytes + framelen <= size) {
        hvl_DecodeFrame(ht, mixbuf, mixbuf + 2, 4);
        totalbytes += framelen;
        songend = ht->ht_SongEndReached;
        mixbuf += framelen;
    }
    return pair(songend ? SongEnd::PLAYER : SongEnd::NONE, totalbytes);
}

bool restart(PlayerState &state) {
    assert(state.info.player == Player::hvl);
    auto ht = static_cast<struct hvl_tune*>(state.context);
    assert(ht);
    return hvl_InitSubsong(ht, state.subsong);
}

} // namespace player::hvl
