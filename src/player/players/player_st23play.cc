// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#include "common/std/optional.h"

#include <mutex>
#include <set>
#include <string>

#include "common/endian.h"
#include "common/logger.h"
#include "player/player.h"

#include "3rdparty/replay/st23play/st23play.h"

using namespace std;
using namespace common;
using namespace player;
using namespace replay::st23play;

namespace {

constexpr int MAX_ORDERS = 128;

constexpr size_t mixBufSize(const int frequency) noexcept {
    return 4 * (frequency / 50 + (frequency % 50 != 0 ? 1 : 0));
}

mutex probe_guard;
struct st23play_context {
    const bool probe;
    set<uint8_t> seen; // for songend detection

    st23play_context(const bool probe) noexcept : probe(probe) {
        if (probe) probe_guard.lock();
        reset();
    }
    ~st23play_context() noexcept {
        clearMixBuffer();
        Close();
        if (probe) probe_guard.unlock();
    }
    void reset() noexcept  {
        if (probe) probe::reset();
        else play::reset();
        seen.clear();
    }
    bool moduleLoaded() const noexcept {
        if (probe) return probe::moduleLoaded;
        else return play::moduleLoaded;
    }
    bool loadSTM(const uint8_t *dat, const uint32_t modLen) noexcept {
        assert(!moduleLoaded());
        if (probe) return probe::loadSTM(dat, modLen);
        else return play::loadSTM(dat, modLen);
    }
    bool PlaySong(const uint8_t *moduleData, uint32_t dataLength, uint32_t audioFreq) noexcept {
        assert(!moduleLoaded());
        if (probe) return probe::st23play_PlaySong(moduleData, dataLength, audioFreq);
        else return play::st23play_PlaySong(moduleData, dataLength, audioFreq);
    }
    void Close() noexcept {
        if (probe) probe::st23play_Close();
        else play::st23play_Close();
    }
    void FillAudioBuffer(int16_t *buffer, int32_t samples) noexcept {
        assert(moduleLoaded());
        if (probe) return probe::st23play_FillAudioBuffer(buffer, samples);
        else return play::st23play_FillAudioBuffer(buffer, samples);
    }
    bool restarted() const noexcept {
        if (probe) return probe::restarted;
        else return play::restarted;
    }
    uint8_t vpnt() const noexcept {
        if (probe) return probe::vpnt;
        else return play::vpnt;
    }
    void clearMixBuffer() noexcept {
        if (probe) probe::clearMixBuffer();
        else play::clearMixBuffer();
    }
};

const set<string> stm_tracker_whitelist = {
    // based on OpenMPT and XMP
    "!Scream!",
    "BMOD2STM",
    "WUZAMOD!",
    "SWavePro",
    "PCSTV"
};

constexpr_f2 std::optional<ModuleInfo> get_stm_info(const char *path, const char *buf, size_t size) noexcept {
    const uint8_t verMajor = buf[30];
    const uint8_t verMinor = buf[31];
    const uint16_t ver = (verMajor * 100) + verMinor;
	assert(buf[29] == 2 && (ver == 200 || ver == 210 || ver == 220 || ver == 221));
    const auto tracker = string(&buf[20], 8);
    char format[20];
    // XMP assumes empty and PCSTV aqre Scream Tracker (not OpenMPT)
    if (tracker.empty() || tracker == "!Scream!" || tracker == "PCSTV") {
        // If using major/minor ver, in practice all/most STMs (at least in Modland and AMP)
        // will just show 2.21 (same with OpenMPT and XMP)
        // as there is no such Scream Tracker release(?), just use 2.x
        snprintf(format, sizeof format, "Scream Tracker 2.x");
    } else if (tracker == "SWavePro") {
        // OpenMPT ignores version, XMP does not
        snprintf(format, sizeof format, "SoundWave Pro");
    } else if (tracker == "WUZAMOD!") {
        snprintf(format, sizeof format, "Wuzamod");
    } else {
        snprintf(format, sizeof format, "%s", tracker.c_str());
    }
    return ModuleInfo{Player::st23play, format, path, 1, 1, 1, 4};
}

} // namespace {}

namespace player::st23play {

void init() noexcept {}

void shutdown() noexcept {
#ifdef PLAYER_PROBE
    probe::clearMixBuffer();
    probe::st23play_Close();
#endif
    play::clearMixBuffer();
    play::st23play_Close();
}

bool is_our_file(const char *path, const char *buf, size_t size) noexcept {
    if (size < 1040+128) return false;
    const uint16_t ver = (buf[30] * 100) + buf[31];
    if (buf[29] != 2 || (ver != 200 && ver != 210 && ver != 220 && ver != 221))
        return false; // unsupported
    const auto tracker = string(&buf[20], 8);
    if (!stm_tracker_whitelist.count(tracker)) return false;
    const uint8_t pats = buf[33];
    if (pats > MAX_ORDERS) return false;
    return true;
}

optional<ModuleInfo> parse(const char *path, const char *buf, size_t size) noexcept {
    assert(size >= 1040+128);
    st23play_context *context = new st23play_context(true);
    assert(!context->moduleLoaded());
    optional<ModuleInfo> info;
    if (context->loadSTM((const uint8_t*)buf, size)) {
        // TODO subsongs (are there any STMs with proper subsongs?)
        info = get_stm_info(path, buf, size);
    } else {
        DEBUG("player_st23play::parse parsing failed for %s\n", path);
    }
    delete context;
    return info;
}

optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &config) noexcept {
    assert(config.player == Player::st23play || config.player == Player::NONE);
    assert(config.tag == Player::st23play || config.tag == Player::NONE);
    assert(subsong == 1);
    st23play_context *context = new st23play_context(config.probe);
    assert(!context->moduleLoaded());
    if (!context->PlaySong((uint8_t*)buf, size, config.frequency)) {
        ERR("player_st23play::play could not play %s\n", path);
        delete context;
        return {};
    }
    // TODO subsongs (are there any STMs with proper subsongs?)
    return PlayerState {Player::st23play, subsong, config.frequency, config.endian != endian::native, context, true, mixBufSize(config.frequency), 0};
}

pair<SongEnd::Status,size_t> render(PlayerState &state, char *buf, size_t size) noexcept {
    assert(state.player == Player::st23play);
    assert(size >= mixBufSize(state.frequency));
    const auto context = static_cast<st23play_context*>(state.context);
    assert(context);
    assert(context->moduleLoaded());
    const auto prevVpnt = context->vpnt();
    if (context->seen.empty()) {
        context->seen.insert(prevVpnt);
    }
    context->FillAudioBuffer((int16_t*)buf, mixBufSize(state.frequency) / 4);
    bool songend = context->restarted();
    const auto vpnt = context->vpnt();
    if (vpnt != prevVpnt) {
        songend |= !context->seen.insert(vpnt).second;
    }
    return pair<SongEnd::Status, size_t>(songend ? SongEnd::PLAYER : SongEnd::NONE, mixBufSize(state.frequency));
}

bool stop(PlayerState &state) noexcept {
    assert(state.player == Player::st23play);
    if (state.context) {
        const auto context = static_cast<st23play_context*>(state.context);
        assert(context);
        delete context;
    }
    return true;
}

bool restart(PlayerState &state) noexcept {
    assert(state.player == Player::st23play);
    const auto context = static_cast<st23play_context*>(state.context);
    assert(context);
    context->clearMixBuffer();
    context->reset();
    return true;
}

} // namespace player::st23play
