// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

#include "common.h"
#include "player/player.h"
#include "player/foreach.h"

using namespace std;
using namespace player;

// never stop the madness
#define DEFINE_PLAYER(ns) \
namespace ns { \
void init(); \
bool is_our_file(const char *buf, size_t size); \
optional<ModuleInfo> parse(const char *fname, const char *buf, size_t size); \
optional<PlayerState> play(const char *fname, const char *buf, size_t size, int subsong, int frequency); \
pair<bool,size_t> render(PlayerState &state, char *buf, size_t size); \
bool stop(PlayerState &state); \
bool restart(PlayerState &state); \
} // namespace ns

#define INIT_PLAYER(p) \
    p::init();

#define CHECK(f,p) \
    if (player::p::f) return Player::p;

#define CASE(f, p) \
    case Player::p: res = p::f; break;

#define SWITCH_MAGIC(buf,size,res,f) \
    switch (parse_magic(buf, size)) { \
        FOREACHA(CASE, f, PLAYERS) \
        case Player::NONE: res = {}; break; \
        default: assert(false); res = {}; \
    }

#define SWITCH_PLAYER(p,res,f) \
    switch (p) { \
        FOREACHA(CASE, f, PLAYERS) \
        case Player::NONE: \
        default: assert(false); \
    }

namespace player {

FOREACH(DEFINE_PLAYER, PLAYERS)

} // namespace player

namespace {

Player parse_magic(const char *buf, size_t size) {
    if (size < MAGIC_SIZE) return Player::NONE;
    FOREACHA(CHECK, is_our_file(buf, size), PLAYERS)
    return Player::NONE;
}

} // namespace {}

namespace player {

void init() {
    FOREACH(INIT_PLAYER, PLAYERS)
}

bool is_our_file(const char *buf, size_t size) {
    return parse_magic(buf, size) != Player::NONE;
}

optional<ModuleInfo> parse(const char *fname, const char *buf, size_t size) {
    optional<ModuleInfo> res;
    SWITCH_MAGIC(buf, size, res,
        parse(fname, buf, size)
    )
    return res;
}

optional<PlayerState> play(const char *fname, const char *buf, size_t size, int subsong, int frequency) {
    optional<PlayerState> res;
    SWITCH_MAGIC(buf, size, res,
        play(fname, buf, size, subsong, frequency)
    )
    return res;
}

bool stop(PlayerState &state) {
    assert(state.player != Player::NONE);
    bool res = false;
    SWITCH_PLAYER(state.player, res,
        stop(state)
    )
    state.player = Player::NONE;
    return res;
}

pair<bool,size_t> render(PlayerState &state, char *buf, size_t size) {
    assert(state.player != Player::NONE);
    assert(size == MIXBUFSIZE);
    pair<bool,size_t> res = pair(false, 0);
    SWITCH_PLAYER(state.player, res,
        render(state, buf, size)
    )
    const size_t bytespersec = 4 * state.frequency;
    state.pos_millis += res.second * 1000 / bytespersec;
    return res;
}

bool seek(PlayerState &state, int millis) {
    assert(state.player != Player::NONE);
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
    pair<bool,size_t> res = pair(false, 0);
    while (seeked < bytestoseek && !res.first) {
        SWITCH_PLAYER(state.player, res,
            render(state, dummybuf, sizeof dummybuf)
        )
        if (!res.first) return false;
        seeked += res.second;
    }
    state.pos_millis = millis;
    return res.first;
}

bool restart(PlayerState &state) {
    assert(state.player != Player::NONE);
    bool res = false;
    SWITCH_PLAYER(state.player, res,
        restart(state)
    )
    if (res) {
        state.pos_millis = 0;
        state.songend = false;
    }
    return res;
}

} // namespace player
