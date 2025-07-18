// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <mutex>

#include "common/logger.h"
#include "player/player.h"

#include "3rdparty/replay/protrekkr2/protrekkr2.h"

using namespace std;
using namespace common;
using namespace player;
using namespace replay::protrekkr2;

namespace {

constexpr size_t mixBufSize(const int frequency) noexcept {
    return 4 * (frequency / 50 + (frequency % 50 != 0 ? 1 : 0));
}

mutex probe_guard;
struct protrekkr2_context {
    const bool probe;
    unsigned int length = 0; // song length in millis

    protrekkr2_context(const bool probe) noexcept : probe(probe) {
        if (probe) probe_guard.lock();
    }
    ~protrekkr2_context() noexcept {
        Ptk_Stop();
        Free_Samples();
        if (probe) probe_guard.unlock();
    }

    int Load_Ptk(const char *buf, size_t size) noexcept {
        replay::protrekkr2::FILE file = {buf, size, 0};
        if (probe) return probe::Load_Ptk(&file);
        else return play::Load_Ptk(&file);
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
        if (probe) return probe::Song_Tracks;
        else return play::Song_Tracks;
    }
    string artist() const noexcept {
        if (probe) return probe::artist;
        else return play::artist;
    }
    string FileName() const noexcept {
        if (probe) return probe::FileName;
        else return play::FileName;
    }
    void reset() noexcept {
        if (probe) probe::Init_Tracker_Context_After_ModLoad();
        else play::Init_Tracker_Context_After_ModLoad();
    }
};

string get_tracker(const char *buf) noexcept {
    switch (buf[7]) {
        case '1':
            return "NoiseTrekker 1.x";
        case '2':
            return "NoiseTrekker 2.x";
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
            return "ProTrekkr 1.x";
        default:
            return "ProTrekkr 2.x";
    }
}
} // namespace {}

namespace player::protrekkr2 {

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

bool is_our_file(const char *path, const char *buf, size_t bufsize, size_t filesize) noexcept {
    return bufsize >= 8 && (
        // NoiseTrekker?
        (buf[0] == 'T' && buf[1] == 'W' && buf[2] == 'N' && buf[3] == 'N' &&
         buf[4] == 'S' && buf[5] == 'N' && buf[6] == 'G') ||
        // ProTrekker?
        (buf[0] == 'P' && buf[1] == 'R' && buf[2] == 'O' && buf[3] == 'T' &&
         buf[4] == 'R' && buf[5] == 'E' && buf[6] == 'K')
        ) && buf[7] >= '1' && buf[7] <= 'T';
}

optional<ModuleInfo> parse(const char *path, const char *buf, size_t size) noexcept {
    protrekkr2_context *context = new protrekkr2_context(true);
    // TODO assert(!context->moduleLoaded());
    optional<ModuleInfo> info;
    if (context->Load_Ptk(buf, size)) {
        const string tracker = get_tracker(buf);
        info = ModuleInfo{Player::protrekkr2, tracker, path, 1, 1, 1, context->Channels()};
    } else {
        DEBUG("player_protrekkr2::parse parsing failed for %s\n", path);
    }
    delete context;
    return info;
}

optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &config) noexcept {
    assert(config.player == Player::protrekkr2 || config.player == Player::NONE);
    assert(config.tag == Player::protrekkr2 || config.tag == Player::NONE);
    assert(subsong == 1);
    protrekkr2_context *context = new protrekkr2_context(config.probe);
    if (!context->Load_Ptk(buf, size)) {
        ERR("player_protrekkr2::play could not play %s\n", path);
        delete context;
        return {};
    }
    context->Calc_Length();
    context->Ptk_Play();
    // mix rate hard coded to 44100
    return PlayerState {Player::protrekkr2, subsong, MIX_RATE, config.endian != endian::native, context, true, mixBufSize(MIX_RATE), 0};
}

pair<SongEnd::Status,size_t> render(PlayerState &state, char *buf, size_t size) noexcept {
    assert(state.player == Player::protrekkr2);
    assert(size >= state.buffer_size);
    const auto context = static_cast<protrekkr2_context*>(state.context);
    assert(context);
    context->Mixer((Uint8 *)buf, state.buffer_size);
    const int64_t bytespersec = 4 * state.frequency;
    bool songend = state.pos_millis + state.buffer_size * 1000 / bytespersec >= context->length;
    return pair<SongEnd::Status, size_t>(songend ? SongEnd::PLAYER : SongEnd::NONE, state.buffer_size);
}

bool stop(PlayerState &state) noexcept {
    assert(state.player == Player::protrekkr2);
    if (state.context) {
        const auto context = static_cast<protrekkr2_context*>(state.context);
        assert(context);
        delete context;
    }
    return true;
}

bool restart(PlayerState &state) noexcept {
    assert(state.player == Player::protrekkr2);
    const auto context = static_cast<protrekkr2_context*>(state.context);
    assert(context);
    context->reset();
    context->Ptk_Play();
    return true;
}

} // namespace player::protrekkr2
