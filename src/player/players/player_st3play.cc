// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#include "common/std/optional.h"

#include <cassert>
#include <cstring>
#include <mutex>

#include "common/endian.h"
#include "common/logger.h"
#include "player/player.h"

#include "3rdparty/replay/st3play/st3play.h"

using namespace std;
using namespace common;
using namespace player;
using namespace replay::st3play;

namespace {

constexpr size_t mixBufSize(const int frequency) noexcept {
    return 4 * 4 * (frequency / 50 + (frequency % 50 != 0 ? 1 : 0));
}

mutex probe_guard;
struct st3play_context {
    const bool probe;

    st3play_context(const bool probe) noexcept : probe(probe) {}
    bool PlaySong(const uint8_t *moduleData, uint32_t dataLength, bool useInterpolationFlag, uint32_t audioFreq) noexcept {
        if (probe) return probe::st3play_PlaySong(moduleData, dataLength, useInterpolationFlag, audioFreq);
        else return play::st3play_PlaySong(moduleData, dataLength, useInterpolationFlag, audioFreq);
    }
    void Close() noexcept {
        if (probe) probe::st3play_Close();
        else play::st3play_Close();
    }
    bool FillAudioBuffer(int16_t *buffer, int32_t samples) noexcept {
        if (probe) return probe::st3play_FillAudioBuffer(buffer, samples);
        else return play::st3play_FillAudioBuffer(buffer, samples);
    }
    int16_t np_restarted() noexcept {
        if (probe) return probe::np_restarted;
        else return play::np_restarted;
    }
    void set_np_ord(int16_t np_ord) noexcept {
        if (probe) probe::np_ord = np_ord;
        else play::np_ord = np_ord;
    }
};

} // namespace {}

namespace player::st3play {

void init() noexcept {}
void shutdown() noexcept {
    probe::st3play_Close();
    play::st3play_Close();
}

bool is_our_file(const char *path, const char *buf, size_t size) noexcept {
    if (size < 0x70 || buf[0x1D] != 16 || memcmp(&buf[0x2C], "SCRM", 4) != 0)
	    return false;

    // Reject non-authentic trackers
    // TODO detect UNMO3, deMODifier, Kosmic To-S3M (see OpenMPT)
    const auto ver = *(le_uint16_t *)&buf[0x28];
    if (ver < 0x1300 || ver > 0x1321)
        return false;

    const auto ordnum = *(le_uint16_t *)&buf[0x20];
    const auto flags = *(le_uint16_t *)&buf[0x26];
    const auto uc = buf[0x34];
    const uint8_t dp = buf[0x35];
    const auto special = *(le_uint16_t *)&buf[0x3e];
    // Sound Club 2
    if (!memcmp(&buf[0x36], "SCLUB2.0", 8))
        return false;
    // ModPlug Tracker / OpenMPT
    if (special == 0 && (ordnum & 0x0f) == 0 && uc == 0 && (flags & ~0x50) == 0 && dp == 0xfc)
        return false;
    // PlayerPRO / Velvet Studio
    if (ver == 0x1320 && special == 0 && uc == 0 && flags == 0 && dp == 0)
        return false;
    // Impulse Tracker < 1.03
    if (ver == 0x1320 && special == 0 && uc == 0 && flags == 8 && dp == 0)
        return false;

    return true;
}

optional<ModuleInfo> parse(const char *path, const char *buf, size_t size) noexcept {
    assert(size >= 0x40 + 32);

    const auto ver = *(le_uint16_t *)&buf[0x28];
    assert(ver >= 0x1300 && ver <= 0x1321);
    char format[20];
    // TODO GUS/SB info
    // TODO check for OPL channels -> reject
    // TODO check for > 16 sample channels -> reject
    snprintf(format, sizeof format, "Scream Tracker 3.%02X",((uint16_t)ver) & 0xFF);
    uint8_t chnsettings[32];
	memcpy(chnsettings, &buf[0x40], sizeof chnsettings);
    int channels = 0;
    for (auto ch = 0; ch < sizeof(chnsettings); ++ch) {
        if (chnsettings[ch] != 0xFF) {
            channels++;
        }
    }
    // TODO subsongs
    return ModuleInfo{Player::st3play, format, path, 1, 1, 1, channels};
}

optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &config) noexcept {
    if (config.probe) probe_guard.lock();
    st3play_context *context = new st3play_context(config.probe);
    if (!context->PlaySong((uint8_t*)buf, size, true, config.frequency)) {
        ERR("player_st3play::play could not play %s\n", path);
        delete context;
        if (config.probe) probe_guard.unlock();
        return {};
    }
    const auto info = parse(path, buf, size);
    assert(info);
    PlayerState state = {info.value(), subsong, config.frequency, config.endian != endian::native, context, true, mixBufSize(config.frequency), 0};
    return state;
}

bool stop(PlayerState &state) noexcept {
    assert(state.info.player == Player::st3play);
    if (state.context) {
        const auto context = static_cast<st3play_context*>(state.context);
        assert(context);
        context->Close();
        if (context->probe) probe_guard.unlock();
        delete context;
    }
    return true;
}

pair<SongEnd::Status, size_t> render(PlayerState &state, char *buf, size_t size) noexcept {
    assert(state.info.player == Player::st3play);
    assert(size >= mixBufSize(state.frequency));
    const auto context = static_cast<st3play_context*>(state.context);
    assert(context);
    context->FillAudioBuffer((int16_t*)buf, mixBufSize(state.frequency) / 4);
    bool songend = context->np_restarted();
    return pair<SongEnd::Status, size_t>(songend ? SongEnd::PLAYER : SongEnd::NONE, mixBufSize(state.frequency));
}

bool restart(PlayerState &state) noexcept {
    assert(state.info.player == Player::st3play);
    const auto context = static_cast<st3play_context*>(state.context);
    assert(context);
    context->set_np_ord(0);
    return true;
}

} // namespace player::st3play
