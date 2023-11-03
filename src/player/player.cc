// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

#include "../common.h"
#include "player.h"

using namespace std;
using namespace player;

// TODO DRY

namespace player::hvl {

void init();
optional<ModuleInfo> parse(const char *fname, const char *buf, size_t size);
optional<PlayerState> play(const char *fname, const char *buf, size_t size, int subsong, int frequency);
pair<bool,size_t> render(PlayerState &state, char *buf, size_t size);
void stop(PlayerState &state);
bool restart(PlayerState &state);

} // namespace player::hvl

namespace player::dbm {

void init();
optional<ModuleInfo> parse(const char *fname, const char *buf, size_t size);
optional<PlayerState> play(const char *fname, const char *buf, size_t size, int subsong, int frequency);
pair<bool,size_t> render(PlayerState &state, char *buf, size_t size);
void stop(PlayerState &state);
bool restart(PlayerState &state);

} // namespace player::dbm

namespace {

Player parse_magic(const char *buf, size_t size) {
    if (size < MAGIC_SIZE) return NONE;
    if (buf[0] == 'H' && buf[1] == 'V' && buf[2] == 'L' && buf[3] < 2) {
        return HIVELY;
    }
    if (buf[0] == 'D' && buf[1] == 'B' && buf[2] == 'M' && buf[3] == '0') {
        return DIGIBOOSTERPRO;
    }
    return NONE;
}

} // namespace {}

namespace player {

void init() {
    hvl::init();
    dbm::init();
}

bool is_our_file(const char *buf, size_t size) {
    return parse_magic(buf, size) != NONE;
}

optional<ModuleInfo> parse(const char *fname, const char *buf, size_t size) {
    switch (parse_magic(buf, size)) {
        case HIVELY: return hvl::parse(fname, buf, size);
        case DIGIBOOSTERPRO: return dbm::parse(fname, buf, size);
        case NONE: return {};
        default: assert(false); return {};
    }
}

optional<PlayerState> play(const char *fname, const char *buf, size_t size, int subsong, int frequency) {
    switch (parse_magic(buf, size)) {
        case HIVELY: return hvl::play(fname, buf, size, subsong, frequency);
        case DIGIBOOSTERPRO: return dbm::play(fname, buf, size, subsong, frequency);
        case NONE: return {};
        default: assert(false); return {};
    }
}

void stop(PlayerState &state) {
    assert(state.player != NONE);
    switch (state.player) {
        case HIVELY: hvl::stop(state); break;
        case DIGIBOOSTERPRO: dbm::stop(state); break;
        case NONE: default: assert(false);
    }
    state.player = NONE;
}

pair<bool,size_t> render(PlayerState &state, char *buf, size_t size) {
    assert(state.player != NONE);
    assert(size == MIXBUFSIZE);
    pair<bool,size_t> res;
    switch (state.player) {
        case HIVELY: res = hvl::render(state, buf, size); break;
        case DIGIBOOSTERPRO: res = dbm::render(state, buf, size); break;
        case NONE: default: assert(false); return pair(false,0);
    }
    const size_t bytespersec = 4 * state.frequency;
    state.pos_millis += res.second * 1000 / bytespersec;
    return res;
}

bool seek(PlayerState &state, int millis) {
    assert(state.player != NONE);
    char dummybuf[MIXBUFSIZE];
    if (millis < state.pos_millis) {
        bool res = restart(state);
        if (!res) {
            // TODO better error reporting
            ERR("Could not seek to %d\n", millis);
            return false;
        }
    }
    const size_t bytespersec = 4 * state.frequency;
    const int millistoseek = millis - state.pos_millis;
    const size_t bytestoseek = bytespersec * millistoseek / 1000;
    size_t seeked = 0;
    pair<bool,size_t> res;
    while (seeked < bytestoseek && !res.first) {
        switch (state.player) {
            case HIVELY: res = hvl::render(state, dummybuf, sizeof dummybuf); break;
            case DIGIBOOSTERPRO: res = dbm::render(state, dummybuf, sizeof dummybuf); break;
            case NONE: default: assert(false); return false;
        }
        seeked += res.second;
    }
    state.pos_millis = millis;
    TRACE("POSMILLIS %d - %d\n", state.pos_millis, res.first);
    return res.first;
}

bool restart(PlayerState &state) {
    TRACE("RESTART\n");
    assert(state.player != NONE);
    bool res = false;
    switch (state.player) {
        case HIVELY: res = hvl::restart(state); break;
        case DIGIBOOSTERPRO: res = dbm::restart(state); break;
        case NONE: default: assert(false); return false;
    }
    if (res) {
        state.pos_millis = 0;
        state.songend = false;
    }
    return res;
}

}