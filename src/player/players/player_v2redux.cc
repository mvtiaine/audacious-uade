// SPDX-License-Identifier: LGPL-2.1-or-later AND CC-PDM-1.0
// SPDX-AI-Disclosure: ai-assisted
// Copyright (C) 2026 Matti Tiainen <mvtiaine@cc.hut.fi>

#include "common/std/optional.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <strings.h>

#include "common/logger.h"
#include "player/player.h"
#include "player/players/internal.h"

#include "3rdparty/replay/v2redux/v2redux.h"

using namespace std;
using namespace common;
using namespace player;
using namespace player::internal;

// The vendor engine lives in the global ::v2redux namespace; inside
// namespace player::v2redux it has to be referenced through these aliases.
namespace {

using V2Player = ::v2redux::Player;
using V2Result = ::v2redux::Result;

// v2redux renders stereo float32 at 44100 Hz only
constexpr int MIX_RATE = 44100;
constexpr int CHANNELS = 16;

constexpr size_t mixBufSize(const int frequency) noexcept {
    return 4 * (frequency / 50 + (frequency % 50 != 0 ? 1 : 0));
}

// v2m has no magic; full validation happens in parse() via Player::open().
// Minimal probe on the first 12 bytes (timediv, maxtime, global event count,
// all little-endian u32): all three are small in every v2m, while every
// other tracker/audio format has ASCII or binary magic at offset 0 (a huge
// timediv). Joint false-positive probability on random data ~1e-15.
bool probe_v2m(const char *buf, size_t bufsize) noexcept {
    // smallest structurally valid v2m: v0 layout, one channel with a single
    // note + program change, one patch (verified against Player::open())
    constexpr size_t MIN_V2M_SIZE = 214;
    if (bufsize < MIN_V2M_SIZE) return false;
    const auto *p = reinterpret_cast<const unsigned char *>(buf);
    const auto u32 = [p](size_t pos) noexcept {
        return unsigned(p[pos]) | (p[pos + 1] << 8) | (p[pos + 2] << 16) | (unsigned(p[pos + 3]) << 24);
    };
    const unsigned timediv = u32(0);
    const unsigned maxtime = u32(4);
    const unsigned gdnum = u32(8);
    return timediv >= 1 && timediv <= 4096 && maxtime >= 1 && maxtime <= 10000000 && gdnum <= 4096;
}

// channels carrying notes, via the container header walk (v2load.cpp
// parseContainer layout). Requires the full file; falls back to CHANNELS
// if the walk fails (parse() reports the real error via Player::open()).
int activeChannels(const char *buf, size_t bufsize) noexcept {
    const auto *p = reinterpret_cast<const unsigned char *>(buf);
    const auto u32 = [p, bufsize](size_t pos) noexcept -> optional<unsigned> {
        if (pos + 4 > bufsize) return {};
        return p[pos] | (p[pos + 1] << 8) | (p[pos + 2] << 16) | (unsigned(p[pos + 3]) << 24);
    };
    size_t pos = 12;
    const auto gdnum = u32(8);
    if (!gdnum) return CHANNELS;
    pos += size_t(*gdnum) * 10; // global events
    int active = 0;
    for (int ch = 0; ch < 16; ch++) {
        const auto noteNum = u32(pos);
        if (!noteNum) return CHANNELS;
        pos += 4;
        if (*noteNum == 0) continue; // an empty channel ends after its count
        active++;
        pos += size_t(*noteNum) * 5; // notes
        for (const int stride : {4, 5, 4, 4, 4, 4, 4, 4, 4}) { // pc, pb, 7 ctl
            const auto num = u32(pos);
            if (!num) return CHANNELS;
            pos += 4 + size_t(stride) * *num;
        }
    }
    return active;
}

// interleaved float32 [-1,1] -> interleaved int16
void toInt16(const float *src, int16_t *dst, const size_t samples) noexcept {
    for (size_t i = 0; i < samples; ++i) {
        float s = src[i];
        if (s > 1.0f) s = 1.0f;
        else if (s < -1.0f) s = -1.0f;
        const auto v = static_cast<int>(s < 0.0f ? s * 32768.0f - 0.5f : s * 32767.0f + 0.5f);
        dst[i] = static_cast<int16_t>(v);
    }
}

struct v2redux_context {
    V2Player player;
    float floatbuf[mixBufSize(MIX_RATE) / 2]; // render scratch, interleaved stereo float32
};

} // namespace {}

namespace player::v2redux {

void init() noexcept {}

void shutdown() noexcept {}

bool is_our_file(const char *path, const char *buf, size_t bufsize, size_t /*filesize*/) noexcept {
    return has_v2m_ext(path) && probe_v2m(buf, bufsize);
}

optional<ModuleInfo> parse(const char *path, const char *buf, size_t size) noexcept {
    V2Player player;
    const auto result = player.open(buf, size);
    if (result != V2Result::OK) {
        DEBUG("player_v2redux::parse open failed for %s (%d)\n", path, static_cast<int>(result));
        return {};
    }
    const string format = string("V2M v") + to_string(player.fileVersion());
    return ModuleInfo{Player::v2redux, format, path, 1, 1, 1, activeChannels(buf, size)};
}

optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &config) noexcept {
    assert(config.player == Player::v2redux || config.player == Player::NONE);
    assert(config.tag == Player::v2redux || config.tag == Player::NONE);
    assert(subsong == 1);
    auto *context = new v2redux_context;
    if (context->player.open(buf, size) != V2Result::OK) {
        ERR("player_v2redux::play open failed for %s\n", path);
        delete context;
        return {};
    }
    context->player.play(0);
    // mix rate hard coded to 44100; toInt16 emits native-endian int16,
    // the framework swabs the buffer when the host differs from the request
    return PlayerState {Player::v2redux, subsong, MIX_RATE, config.endian != endian::native, context, true, mixBufSize(MIX_RATE), 0, 0};
}

pair<SongEnd::Status, size_t> render(PlayerState &state, char *buf, size_t size) noexcept {
    assert(state.player == Player::v2redux);
    assert(size >= state.buffer_size);
    const auto context = static_cast<v2redux_context*>(state.context);
    assert(context);
    const auto frames = static_cast<uint32_t>(state.buffer_size / 4);
    context->player.render(context->floatbuf, frames);
    toInt16(context->floatbuf, (int16_t*)buf, frames * 2);
    // render() keeps producing the reverb/delay tail after the sequencer stops
    const bool songend = !context->player.isPlaying();
    return pair<SongEnd::Status, size_t>(songend ? SongEnd::PLAYER : SongEnd::NONE, state.buffer_size);
}

bool stop(PlayerState &state) noexcept {
    assert(state.player == Player::v2redux);
    if (state.context) {
        const auto context = static_cast<v2redux_context*>(state.context);
        assert(context);
        delete context;
    }
    return true;
}

bool restart(PlayerState &state) noexcept {
    assert(state.player == Player::v2redux);
    const auto context = static_cast<v2redux_context*>(state.context);
    assert(context);
    context->player.play(0);
    return true;
}

} // namespace player::v2redux
