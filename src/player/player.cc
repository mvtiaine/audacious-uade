// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <cassert>
#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

#include "common/compat.h"
#include "common/foreach.h"
#include "common/logger.h"
#include "converter/converter.h"
#include "player/player.h"

#include <unistd.h>
// XXX fix morphos compile
#undef shutdown

using namespace std;
using namespace common;
using namespace player;

namespace player {
// never stop the madness
#define DEFINE_PLAYER(ns) \
namespace ns { \
void init() noexcept; \
void shutdown() noexcept; \
bool is_our_file(const char *path, const char *buf, size_t size) noexcept; \
optional<ModuleInfo> parse(const char *path, const char *buf, size_t size) noexcept; \
optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &config) noexcept; \
pair<SongEnd::Status,size_t> render(PlayerState &state, char *buf, size_t size) noexcept; \
bool stop(PlayerState &state) noexcept; \
bool restart(PlayerState &state) noexcept; \
} // namespace ns
FOREACH(DEFINE_PLAYER, PLAYERS)
}

namespace {

#define APPLY(f, p) \
    p::f;

#define CHECK(f,p) \
    if (player::p::f) return Player::p;

#define CASE(f, p) \
    case Player::p: res = p::f; break;

#define SWITCH_PLAYER(p,res,f) \
    switch (p) { \
        FOREACHA(CASE, f, PLAYERS) \
        case Player::NONE: \
        default: assert(false); \
    }

bool initialized = false;

} // namespace {}

namespace player {

void init() noexcept {
    FOREACHA(APPLY, init(), PLAYERS)
    initialized = true;
}

void shutdown() noexcept {
    assert(initialized);
    FOREACHA(APPLY, shutdown(), PLAYERS)
}

Player check(const char *path, const char *buf, size_t size) noexcept {
    assert(initialized);
    if (size < MAGIC_SIZE || size < converter::MAGIC_SIZE) return Player::NONE;
    assert(path);
    assert(buf);
    // TODO support conversion for other players
    if (converter::needs_conversion(buf, size)) return Player::uade;
    FOREACHA(CHECK, is_our_file(path, buf, size), PLAYERS)
    return Player::NONE;
}

optional<ModuleInfo> parse(const char *path, const char *buf, size_t size) noexcept {
    assert(initialized);
    if (size < MAGIC_SIZE || size < converter::MAGIC_SIZE) return {};
    assert(path);
    assert(buf);
    optional<converter::ConverterResult> conversion = {};
    if (converter::needs_conversion(buf, size)) {
        conversion = converter::convert(buf, size);
        if (!conversion->success) {
            WARN("Could not convert %s: %s\n", path, conversion->reason_failed.c_str());
            return {};
        }
        buf = conversion->data.data();
        size = conversion->data.size();
    }
    Player player = check(path, buf, size);
    if (player == Player::NONE) return {};
    optional<ModuleInfo> res;
    SWITCH_PLAYER(player, res,
        parse(path, buf, size)
    )
    if (res && conversion) {
        res->format = conversion->format;
    }
    return res;
}

optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &config) noexcept {
    assert(initialized);
    if (size < MAGIC_SIZE || size < converter::MAGIC_SIZE) return {};
    assert(path);
    assert(buf);
    assert(subsong >= 0);
    optional<converter::ConverterResult> conversion = {};
    if (converter::needs_conversion(buf, size)) {
        conversion = converter::convert(buf, size);
        if (!conversion->success) {
            WARN("Could not convert %s: %s\n", path, conversion->reason_failed.c_str());
            return {};
        }
        buf = conversion->data.data();
        size = conversion->data.size();
    }
    Player player = check(path, buf, size);
    if (player == Player::NONE) return {};
    optional<PlayerState> res;
    SWITCH_PLAYER(player, res,
        play(path, buf, size, subsong, config)
    )

    return res;
}

bool stop(PlayerState &state) noexcept {
    assert(initialized);
    assert(state.player != Player::NONE);
    bool res = false;
    SWITCH_PLAYER(state.player, res,
        stop(state)
    )
    state.player = Player::NONE;
    return res;
}

pair<SongEnd::Status,size_t> render(PlayerState &state, char *buf, size_t size) noexcept {
    assert(initialized);
    assert(state.player != Player::NONE);
    assert(buf);
    assert(size >= state.buffer_size);
    auto res = pair<SongEnd::Status,size_t>(SongEnd::ERROR, 0);
    vector<char> tmpbuf;
    char *mixbuf;
    if (!state.swap_endian) {
        mixbuf = buf;
    } else {
        tmpbuf.resize(size);
        mixbuf = tmpbuf.data();
    }
    SWITCH_PLAYER(state.player, res,
        render(state, mixbuf, size)
    )
    assert(res.second % 2 == 0);
    assert(res.second <= size);
    const int64_t bytespersec = 4 * state.frequency;
    state.pos_millis += res.second * 1000 / bytespersec;

    if (state.swap_endian && res.second > 0) {
        swab(mixbuf, buf, res.second);
    }

    return res;
}

bool restart(PlayerState &state) noexcept {
    assert(state.player != Player::NONE);
    bool res = false;
    SWITCH_PLAYER(state.player, res,
        restart(state)
    )
    if (res) {
        state.pos_millis = 0;
    }
    return res;
}

bool seek(PlayerState &state, int millis) noexcept {
    assert(initialized);
    assert(state.player != Player::NONE);

    TRACE("Seeking to %d current pos %d player %d\n", millis, state.pos_millis, static_cast<int>(state.player));

    // use UADEs own seek as it doesn't support "restart"
    if (state.player == Player::uade) return uade::seek(state, millis);

    vector<char> dummybuf(state.buffer_size);
    if (millis < state.pos_millis) {
        bool res = restart(state);
        if (!res) {
            // TODO better error reporting
            ERR("Could not seek to %d\n", millis);
            return false;
        }
    }
    const int64_t bytespersec = 4 * state.frequency;
    const int64_t millistoseek = millis - state.pos_millis;
    const int64_t bytestoseek = bytespersec * millistoseek / 1000;
    int64_t seeked = 0;
    auto res = pair<SongEnd::Status,uint64_t>(SongEnd::ERROR, 0);
    while (seeked < bytestoseek) {
        SWITCH_PLAYER(state.player, res,
            render(state, dummybuf.data(), dummybuf.size())
        )
        if (res.first != SongEnd::NONE) return false;
        seeked += res.second;
    }
    state.pos_millis = millis;
    return true;
}

} // namespace player

namespace player::support {
 
PlaybackResult playback_loop(
    PlayerState &state,
    const PlayerConfig &config,
    const function<bool(void)> check_stop,
    const function<int(void)> check_seek,
    const function<void(char *, int)> write_audio) noexcept {

    vector<char> buffer(state.buffer_size);
    SongEnd songend;
    songend.status = SongEnd::TIMEOUT;
    songend.length = PRECALC_TIMEOUT * 1000;
    bool stopped = false;
    bool seeked = false;
    const int64_t bytespersec = 4 * state.frequency;
    // UADE plays some mods for hours or possibly forever (with always_ends default)
    int64_t maxbytes = config.known_timeout > 0 ?
        config.known_timeout * bytespersec / 1000 : PRECALC_TIMEOUT * bytespersec;
    int64_t totalbytes = 0;

    while (!(stopped = check_stop()) && totalbytes < maxbytes) {
        int seek_millis = check_seek();
        if (seek_millis >= 0) {
            seeked = true;
            if (!seek(state, seek_millis)) {
                ERR("Could not seek to %d\n", seek_millis);
                songend.status = SongEnd::ERROR;
                break;
            } else {
                DEBUG("Seek to %d\n", seek_millis);
                totalbytes = seek_millis * bytespersec / 1000;
            };
        }
        const auto res = render(state, buffer.data(), buffer.size());
        if (res.second > 0 && res.first != SongEnd::ERROR) {
            // ignore "tail bytes" to avoid pop in end of audio if song restarts
            // messing up with silence/volume trimming etc.
            if (res.first == SongEnd::NONE || totalbytes == 0) {
                write_audio(buffer.data(), res.second);
                totalbytes += res.second;
            }
        }

        if (res.first == SongEnd::ERROR) {
            ERR("Playback error.\n");
            songend.status = SongEnd::ERROR;
            break;
        } else if (res.first == SongEnd::TIMEOUT) {
            TRACE("Song end (timeout).\n");
            songend.status = SongEnd::TIMEOUT;
            break;
        } else if (res.first == SongEnd::DETECT_SILENCE) {
            TRACE("Song end (silence).\n");
            songend.status = SongEnd::DETECT_SILENCE;
            break;
        } else if (res.first == SongEnd::PLAYER) {
            TRACE("Song end.\n");
            songend.status = SongEnd::PLAYER;
            break;
        }
    }
    
    if (!seeked && !stopped && songend.status != SongEnd::TIMEOUT) {
        songend.length = totalbytes * 1000 / bytespersec;
    }

    // TODO silence detection for other players during playback
    if (state.player == Player::uade && songend.status == SongEnd::DETECT_SILENCE) {
        const auto &uade_config = static_cast<const uade::UADEConfig&>(config);
        assert(uade_config.player == Player::uade);
        songend.length -= uade_config.silence_timeout;
        if (uade_config.silence_timeout > MAX_SILENCE) {
            songend.length += MAX_SILENCE;
        }
    }
    return {songend, seeked, stopped};
}

} // namespace player::support