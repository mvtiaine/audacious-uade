// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <cassert>

#include "../common.h"
#include "player.h"
extern "C"
{
#include "../3rdparty/hvl/hvl_replay.h"
}

using namespace std;
using namespace player;

namespace player::hvl {

void init() {
    hvl_InitReplayer();
}

optional<ModuleInfo> parse(const char *fname, const char *buf, size_t size) {
    struct hvl_tune *ht = hvl_ParseTune((const uint8_t*)buf, size, 0, 0);
    if (!ht) {
        ERR("player_hvl::parse parsing failed for %s\n", fname);
        return {};
    }

    const int maxsubsong = ht->ht_SubsongNr;
    const int channels = ht->ht_Channels;
    // TODO version ?
    const string format = "HivelyTracker";
    
    hvl_FreeTune(ht);

    const ModuleInfo info = {format, fname, maxsubsong, channels};
    return info;
}

optional<PlayerState> play(const char *fname, const char *buf, size_t size, int subsong, int frequency) {
    struct hvl_tune *ht = hvl_ParseTune((const uint8_t*)buf, size, frequency, 0);
    if (!ht) {
        ERR("player_hvl::play parsing failed for %s\n", fname);
        return {};
    }

    if (!hvl_InitSubsong(ht, subsong)) {
        ERR("player_hvl::play init subsong failed for %s\n", fname);
        hvl_FreeTune(ht);
        return {};
    }

    PlayerState state = {HIVELY, subsong, frequency, ht, 0, false};
    return state;
}

void stop(PlayerState &state) {
    assert(state.player == HIVELY);
    if (state.context) {
        hvl_FreeTune((struct hvl_tune*)state.context);
    }
}

pair<bool,size_t> render(PlayerState &state, char *buf, size_t size) {
    assert(state.player == HIVELY);
    auto ht = (struct hvl_tune*)state.context;
    bool songend = ht->ht_SongEndReached;
    size_t totalbytes = 0;
    char *mixbuf = buf;
    int framelen = state.frequency*2*2/50;
    while (!songend && totalbytes + framelen <= size) {
        hvl_DecodeFrame(ht, mixbuf, mixbuf + 2, 4);
        totalbytes += framelen;
        songend = ht->ht_SongEndReached;
        mixbuf += framelen;
    }
    return pair(songend, totalbytes);
}

bool restart(PlayerState &state) {
    assert(state.player == HIVELY);
    auto ht = (struct hvl_tune*)state.context;
    return hvl_InitSubsong(ht, state.subsong);
}

} // namespace player::hvl