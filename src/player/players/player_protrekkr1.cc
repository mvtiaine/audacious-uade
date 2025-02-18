// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <mutex>

#include "common/logger.h"
#include "player/player.h"

#include "3rdparty/replay/protrekkr1/protrekkr1.h"

using namespace std;
using namespace common;
using namespace player;
using namespace replay::protrekkr1;

namespace {

constexpr size_t mixBufSize(const int frequency) noexcept {
    return 4 * (frequency / 50 + (frequency % 50 != 0 ? 1 : 0));
}

mutex probe_guard;
struct protrekkr1_context {
    const bool probe;
    unsigned int length = 0; // song length in millis

    protrekkr1_context(const bool probe) noexcept : probe(probe) {
        if (probe) probe_guard.lock();
    }
    ~protrekkr1_context() noexcept {
        Ptk_Stop();
        Free_Samples();
        if (probe) probe_guard.unlock();
    }

    int LoadMod(const char *buf, size_t size) noexcept {
        replay::protrekkr1::FILE file = {buf, size, 0};
        if (probe) return probe::LoadMod(&file);
        else return play::LoadMod(&file);
    }
    void Calc_Length(void) noexcept {
        if (probe) length = probe::Calc_Length();
        else length = play::Calc_Length();
    }
    void Ptk_Play() noexcept {
        if (probe) probe::Ptk_Play();
        else play::Ptk_Play();
    }
    void Mixer(Uint8 *Buffer, Uint32 Len) noexcept {
        if (probe) probe::Mixer(Buffer, Len);
        else play::Mixer(Buffer, Len);
    }
    void Ptk_Stop() noexcept {
        if (probe) probe::Ptk_Stop();
        else play::Ptk_Stop();
    }
    void Free_Samples() noexcept {
        if (probe) probe::Free_Samples();
        else play::Free_Samples();
    }
    int Channels() const noexcept {
        if (probe) return probe::Songtracks;
        else return play::Songtracks;
    }
    void reset() noexcept {
        if (probe) probe::Init_Tracker_Context_After_ModLoad();
        else play::Init_Tracker_Context_After_ModLoad();
    }
};

} // namespace {}

namespace player::protrekkr1 {

void init() noexcept {
    if (!play::Ptk_InitDriver()) assert(false);
    if (!play::Alloc_Patterns_Pool()) assert(false);
#ifdef PLAYER_PROBE
    if (!probe::Ptk_InitDriver()) assert(false);
    if (!probe::Alloc_Patterns_Pool()) assert(false);
#endif
}

void shutdown() noexcept {
    play::Ptk_Stop();
    play::Free_Samples();
    if (play::RawPatterns) free(play::RawPatterns);
#ifdef PLAYER_PROBE
    probe::Ptk_Stop();
    probe::Free_Samples();
    if (probe::RawPatterns) free(probe::RawPatterns);
#endif
}

bool is_our_file(const char *path, const char *buf, size_t size) noexcept {
    return size >= 8 &&
        (buf[0] == 'T' && buf[1] == 'W' && buf[2] == 'N' && buf[3] == 'N' &&
         buf[4] == 'S' && buf[5] == 'N' && buf[6] == 'G') &&
        buf[7] >= '2' && buf[7] <= 'I';
}

optional<ModuleInfo> parse(const char *path, const char *buf, size_t size) noexcept {
    protrekkr1_context *context = new protrekkr1_context(true);
    optional<ModuleInfo> info;
    if (context->LoadMod(buf, size)) {
        info = ModuleInfo{Player::protrekkr1, "ProTrekkr 1.x", path, 1, 1, 1, context->Channels()};
    } else {
        DEBUG("player_protrekkr1::parse parsing failed for %s\n", path);
    }
    delete context;
    return info;
}

optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &config) noexcept {
    assert(config.player == Player::protrekkr1 || config.player == Player::NONE);
    assert(config.tag == Player::protrekkr1 || config.tag == Player::NONE);
    assert(subsong == 1);
    protrekkr1_context *context = new protrekkr1_context(config.probe);
    if (!context->LoadMod(buf, size)) {
        ERR("player_protrekkr1::play could not play %s\n", path);
        delete context;
        return {};
    }
    context->Calc_Length();
    context->Ptk_Play();
    // mix rate hard coded to 44100
    return PlayerState {Player::protrekkr1, subsong, MIX_RATE, config.endian != endian::native, context, true, mixBufSize(MIX_RATE), 0};
}

pair<SongEnd::Status,size_t> render(PlayerState &state, char *buf, size_t size) noexcept {
    assert(state.player == Player::protrekkr1);
    assert(size >= mixBufSize(state.frequency));
    const auto context = static_cast<protrekkr1_context*>(state.context);
    assert(context);
    context->Mixer((Uint8 *)buf, mixBufSize(state.frequency));
    const int64_t bytespersec = 4 * state.frequency;
    bool songend = state.pos_millis + mixBufSize(state.frequency) * 1000 / bytespersec >= context->length;
    return pair<SongEnd::Status, size_t>(songend ? SongEnd::PLAYER : SongEnd::NONE, mixBufSize(state.frequency));
}

bool stop(PlayerState &state) noexcept {
    assert(state.player == Player::protrekkr1);
    if (state.context) {
        const auto context = static_cast<protrekkr1_context*>(state.context);
        assert(context);
        delete context;
    }
    return true;
}

bool restart(PlayerState &state) noexcept {
    assert(state.player == Player::protrekkr1);
    const auto context = static_cast<protrekkr1_context*>(state.context);
    assert(context);
    context->reset();
    context->Ptk_Play();
    return true;
}

} // namespace player::protrekkr1
