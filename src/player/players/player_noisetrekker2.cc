// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2025 Matti Tiainen <mvtiaine@cc.hut.fi>

// NOTE: while the replay code is based on the NoiseTrekker 2 final source
// the included sources don't actually completely match the final binary (verified with Ghidra).
// For example negative resonance values in synth filter do not behave the same way
// and there is audible crackling in some mods not present with the original binary.

#include <mutex>

#include "common/logger.h"
#include "player/player.h"

#include "3rdparty/replay/noisetrekker2/noisetrekker2.h"

using namespace std;
using namespace common;
using namespace player;
using namespace replay::noisetrekker2;

namespace {

constexpr size_t mixBufSize(const int frequency) noexcept {
    return 4 * (frequency / 50 + (frequency % 50 != 0 ? 1 : 0));
}

mutex probe_guard;
struct noisetrekker2_context {
    const bool probe;

    noisetrekker2_context(const bool probe) noexcept : probe(probe) {
        if (probe) probe_guard.lock();
        reset();
    }
    ~noisetrekker2_context() noexcept {
        SongStop();
        FreeAll();
        if (probe) probe_guard.unlock();
    }

    int LoadMod(const char *buf, size_t size) noexcept {
        replay::noisetrekker2::FILE file = {buf, size, 0};
        if (probe) return probe::LoadMod(&file);
        else return play::LoadMod(&file);
    }
    void SongPlay() noexcept {
        if (probe) probe::SongPlay();
        else play::SongPlay();
    }
    void GetPlayerValues() noexcept {
        if (probe) probe::GetPlayerValues(probe::mas_vol);
        else play::GetPlayerValues(play::mas_vol);
    }
    void SongStop() noexcept {
        if (probe) probe::SongStop();
        else play::SongStop();
    }
    void FreeAll() noexcept {
        if (probe) probe::FreeAll();
        else play::FreeAll();
    }
    int left_value() const noexcept {
        if (probe) return probe::left_value;
        else return play::left_value;
    }
    int right_value() const noexcept {
        if (probe) return probe::right_value;
        else return play::right_value;
    }
    unsigned char cPosition() const noexcept {
        if (probe) return probe::cPosition;
        else return play::cPosition;
    }
    int ped_line() const noexcept {
        if (probe) return probe::ped_line;
        else return play::ped_line;
    }
    char Songtracks() const noexcept {
        if (probe) return probe::Songtracks;
        else return play::Songtracks;
    }
    int SamplesPerSec() const noexcept {
        if (probe) return probe::SamplesPerSec;
        else return play::SamplesPerSec;
    }
    void reset() noexcept {
        if (probe) probe::reset();
        else play::reset();
    }
};

} // namespace {}

namespace player::noisetrekker2 {

void init() noexcept {
    if (!play::AllocPattern()) assert(false);
#ifdef PLAYER_PROBE
    if (!probe::AllocPattern()) assert(false);
#endif
}

void shutdown() noexcept {
    play::SongStop();
    play::FreeAll();
    if (play::RawPatterns) free(play::RawPatterns);
#ifdef PLAYER_PROBE
    probe::SongStop();
    probe::FreeAll();
    if (probe::RawPatterns) free(probe::RawPatterns);
#endif
}

bool is_our_file(const char *path, const char *buf, size_t size) noexcept {
    return size >= 8 &&
        buf[0] == 'T' && buf[1] == 'W' && buf[2] == 'N' && buf[3] == 'N' &&
        buf[4] == 'S' && buf[5] == 'N' && buf[6] == 'G' && buf[7] == '2';
}

optional<ModuleInfo> parse(const char *path, const char *buf, size_t size) noexcept {
    noisetrekker2_context *context = new noisetrekker2_context(true);
    optional<ModuleInfo> info;
    if (context->LoadMod(buf, size)) {
        info = ModuleInfo{Player::noisetrekker2, "NoiseTrekker 2.x", path, 1, 1, 1, context->Songtracks()};
    } else {
        DEBUG("player_noisetrekker2::parse parsing failed for %s\n", path);
    }
    delete context;
    return info;
}

optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &config) noexcept {
    assert(config.player == Player::noisetrekker2 || config.player == Player::NONE);
    assert(config.tag == Player::noisetrekker2 || config.tag == Player::NONE);
    assert(subsong == 1);
    noisetrekker2_context *context = new noisetrekker2_context(config.probe);
    if (!context->LoadMod(buf, size)) {
        WARN("player_noisetrekker2::play loading failed for %s\n", path);
        delete context;
        return {};
    }
    context->SongPlay();
    const int frequency = context->SamplesPerSec(); // hardcoded to 44100
    return PlayerState {Player::noisetrekker2, subsong, frequency, config.endian != endian::native, context, true, mixBufSize(frequency), 0};
}

pair<SongEnd::Status, size_t> render(PlayerState &state, char *buf, size_t size) noexcept {
    assert(state.player == Player::noisetrekker2);
    assert(size >= state.buffer_size);
    const auto context = static_cast<noisetrekker2_context*>(state.context);
    assert(context);
    size_t samples = 0;
    bool songend = false;
    bool bru = false;
    while (samples <= size/2 - 4 && !songend) {
        context->GetPlayerValues();
        ((int16_t*)buf)[samples++] = context->left_value();
        ((int16_t*)buf)[samples++] = context->right_value();
        if (context->ped_line()) bru = true;
        songend |= bru && !context->cPosition() && !context->ped_line();
    }
    return pair<SongEnd::Status, size_t>(songend ? SongEnd::PLAYER : SongEnd::NONE, state.buffer_size);
}

bool stop(PlayerState &state) noexcept {
    assert(state.player == Player::noisetrekker2);
    if (state.context) {
        const auto context = static_cast<noisetrekker2_context*>(state.context);
        assert(context);
        delete context;
    }
    return true;
}

bool restart(PlayerState &state) noexcept {
    assert(state.player == Player::noisetrekker2);
    const auto context = static_cast<noisetrekker2_context*>(state.context);
    assert(context);
    context->reset();
    context->SongPlay();
    return true;
}

} // namespace player::noisetrekker2
