// SPDX-License-Identifier: LGPL-2.1-or-later AND CC-PDM-1.0
// SPDX-AI-Disclosure: ai-assisted
// Copyright (C) 2024-2026 Matti Tiainen <mvtiaine@cc.hut.fi>

#include "common/std/optional.h"

#include <cassert>
#include <cstring>
#include <mutex>
#include <set>
#include <utility>
#include <vector>

#include "common/endian.h"
#include "common/logger.h"
#include "player/player.h"
#include "player/players/internal.h"

#include "3rdparty/replay/st3play/st3play.h"

using namespace std;
using namespace common;
using namespace player;
using namespace player::internal;
using namespace replay::st3play;

namespace {

constexpr int MAX_ORDNUM = 256;
constexpr int MAX_INSNUM = 99;
constexpr int MAX_PATNUM = 100;
constexpr int MAX_ROWS = 64;
constexpr int PATT_SEP = 254;
constexpr int PATT_END = 255;

// render block size in frames (st3play renders to its own mix buffer,
// allocated by initMusic with the same size)
constexpr size_t mixBufSize(const int frequency) noexcept {
    return 4 * (frequency / 50 + (frequency % 50 != 0 ? 1 : 0));
}

constexpr int mixBufFrames(const int frequency) noexcept {
    return frequency / 50 + (frequency % 50 != 0 ? 1 : 0);
}

mutex probe_guard;
struct st3play_context {
    const bool probe;
    int32_t audioBufferSize = 0;
    int16_t startPos = 0;
    set<pair<int16_t,int16_t>> seen; // for subsong loop detection

    st3play_context(const bool probe, const int frequency) noexcept : probe(probe) {
        if (probe) probe_guard.lock();
        audioBufferSize = mixBufFrames(frequency);
        if (probe) {
            probe::initMusic(frequency, audioBufferSize);
            probe::audio.fMixingVol = 32768.0f;
        } else {
            play::initMusic(frequency, audioBufferSize);
            play::audio.fMixingVol = 32768.0f;
        }
    }
    ~st3play_context() noexcept {
        // stop voices and free mix buffers
        if (probe) probe::closeMusic();
        else play::closeMusic();
        if (probe) probe_guard.unlock();
    }
    bool moduleLoaded() const noexcept {
        if (probe) return probe::song.moduleLoaded;
        else return play::song.moduleLoaded;
    }
    // card type is decided by internal::s3m_routing, not the loader's heuristic
    bool loadS3M(const uint8_t *dat, const uint32_t modLen, int32_t soundcardtype) noexcept {
        assert(!moduleLoaded());
        if (probe) return probe::load_st3_from_ram(dat, modLen, soundcardtype);
        else return play::load_st3_from_ram(dat, modLen, soundcardtype);
    }
    bool PlaySong(const int16_t order = 0) noexcept {
        assert(moduleLoaded());
        if (probe) return probe::zplaysong(order);
        else return play::zplaysong(order);
    }
    void musmixer(int16_t *buffer, const int32_t samples) noexcept {
        assert(moduleLoaded());
        if (probe) probe::musmixer(buffer, samples);
        else play::musmixer(buffer, samples);
    }
    void shutupsounds() noexcept {
        if (probe) probe::shutupsounds();
        else play::shutupsounds();
    }
    void resetAudioDither() noexcept {
        if (probe) probe::resetAudioDither();
        else play::resetAudioDither();
    }
    int16_t np_ord() const noexcept {
        if (probe) return probe::song.np_ord;
        else return play::song.np_ord;
    }
    int16_t np_row() const noexcept {
        if (probe) return probe::song.np_row;
        else return play::song.np_row;
    }
    uint16_t ordNum() const noexcept {
        if (probe) return probe::song.header.ordnum;
        else return play::song.header.ordnum;
    }
    uint16_t insNum() const noexcept {
        if (probe) return probe::song.header.insnum;
        else return play::song.header.insnum;
    }
    uint16_t patNum() const noexcept {
        if (probe) return probe::song.header.patnum;
        else return play::song.header.patnum;
    }
    int32_t soundcardtype() const noexcept {
        if (probe) return probe::audio.soundcardtype;
        else return play::audio.soundcardtype;
    }
    bool wavRenderFlag() const noexcept {
        if (probe) return probe::WAVRender_Flag;
        else return play::WAVRender_Flag;
    }
    uint8_t order(const int16_t ordNum) const noexcept {
        if (probe) return probe::song.order[ordNum];
        else return play::song.order[ordNum];
    }
    uint8_t *patData(const uint16_t patNum) const noexcept {
        if (probe) return probe::song.patp[patNum];
        else return play::song.patp[patNum];
    }
    uint16_t patDataLen(const uint16_t patNum) const noexcept {
        if (probe) return probe::song.patDataLens[patNum];
        else return play::song.patDataLens[patNum];
    }
    const uint8_t *channelSettings() const noexcept {
        if (probe) return probe::song.header.channel;
        else return play::song.header.channel;
    }
    void setPos(const int16_t pos) noexcept {
        assert(moduleLoaded());
        startPos = pos;
        if (probe) probe::zgotosong(pos, 0);
        else play::zgotosong(pos, 0);
    }
    // upstream's caller sets the flag when rendering starts; the replay
    // clears it when the order list wraps around (song end)
    void setWavRenderFlag(const bool flag) noexcept {
        if (probe) probe::WAVRender_Flag = flag;
        else play::WAVRender_Flag = flag;
    }
    void clearMixBuffer() noexcept {
        const auto samples = audioBufferSize;
        if (probe) {
            if (probe::audio.fMixBufferL) memset(probe::audio.fMixBufferL, 0, samples * sizeof (float));
            if (probe::audio.fMixBufferR) memset(probe::audio.fMixBufferR, 0, samples * sizeof (float));
        } else {
            if (play::audio.fMixBufferL) memset(play::audio.fMixBufferL, 0, samples * sizeof (float));
            if (play::audio.fMixBufferR) memset(play::audio.fMixBufferR, 0, samples * sizeof (float));
        }
    }

    // scan packed pattern data at pattPos for position jump (Bxx) and
    // pattern break (Cxx) effects
    pair<int16_t,int16_t> posJump(const int pattNr, const int16_t pattPos) const noexcept {
        int16_t effB = -1; // Position Jump
        int16_t effC = -1; // Pattern Break
        uint8_t *patseg = patData(pattNr);
        const uint8_t *chnsettings = channelSettings();
        const uint16_t len = patDataLen(pattNr);
        if (!patseg || !chnsettings) {
            return pair<int16_t,int16_t>(-1,-1);
        }
        // find pattPos offset in packed pattern data
        int i = pattPos, offs = 0;
        uint8_t dat = 0;
        while (i > 0) {
            if (offs >= len) {
                return pair<int16_t,int16_t>(-1,-1);
            }
            dat = patseg[offs++];
            if (dat == 0) {
                i--;
            } else {
                if (dat & 0x20) offs += 2;
                if (dat & 0x40) offs += 1;
                if (dat & 0x80) offs += 2;
            }
        }
        while (true) {
            if (effB != -1 && effC != -1) {
                break;
            }
            while (true) {
                if (offs >= len) {
                    return pair<int16_t,int16_t>(-1,-1);
                }
                dat = patseg[offs++];
                if (dat == 0)
                    return pair<int16_t,int16_t>(effB,effC);
                if ((chnsettings[dat & 0x1F] & 0x80) == 0)
                    break;
                // channel off, skip
                if (dat & 0x20) offs += 2;
                if (dat & 0x40) offs += 1;
                if (dat & 0x80) offs += 2;
            }
            if (dat & 0x20) offs += 2;
            if (dat & 0x40) offs += 1;
            if (dat & 0x80) {
                if (offs + 1 >= len) {
                    return pair<int16_t,int16_t>(-1,-1);
                }
                uint8_t cmd = patseg[offs++];
                uint8_t info = patseg[offs++];
                if (cmd == 0x2)
                    effB = info;
                else if (cmd == 0x3)
                    effC = info;
            }
        }
        return pair<int16_t,int16_t>(effB,effC);
    }
    bool jumpLoop() const noexcept {
        if (probe) return probe::song.patloopcount > 0 || probe::song.patterndelay > 0;
        else return play::song.patloopcount > 0 || play::song.patterndelay > 0;
    }
};

vector<int16_t> get_subsongs(const st3play_context *context) noexcept {
    assert(context->moduleLoaded());
    vector<int16_t> subsongs = {0};

    set<int16_t> seen;
    set<int16_t> notseen;
    for (int i = 0; i < context->ordNum(); ++i) {
        if (context->order(i) <= context->patNum())
            notseen.insert(i);
    }

    auto prevJump = pair<int16_t,int16_t>(0,0);

    int16_t pattPos = 0;
    int16_t songPos = 0;
    bool jump = false;

    while (true) {
        if (jump && seen.count(songPos) && !notseen.empty()) {
            songPos = *notseen.begin();
            pattPos = 0;
            int pattNr = context->order(songPos);
            if (context->patDataLen(pattNr) == 0 || pattNr > context->patNum()) {
                seen.insert(songPos);
                notseen.erase(songPos);
                if (++songPos >= context->ordNum())
                    break;
                continue;
            }
            if (notseen.size() > 1 || context->patDataLen(pattNr) > 0) {
                subsongs.push_back(songPos);
            }
        }
        jump = false;
        seen.insert(songPos);
        notseen.erase(songPos);
        if (notseen.empty())
            break;

        int pattNr = context->order(songPos);
        if (pattNr >= PATT_SEP) {
            if (++songPos >= context->ordNum())
                break;
            jump = (pattNr == PATT_END);
            seen.insert(songPos);
            continue;
        }

        const auto posJump = context->posJump(pattNr, pattPos);
        if (posJump.first >= 0 || posJump.second >= 0) {
            int16_t oldPos = songPos;
            int16_t oldPatt = pattPos;
            songPos = posJump.first == -1 ? songPos + 1 : posJump.first;
            if (songPos >= context->ordNum())
                break;
            if (posJump == prevJump && posJump.first >= 0) {
                seen.insert(songPos);
                jump = true;
            }
            pattPos = posJump.second == -1 ? 0 : posJump.second;
            if (pattPos >= MAX_ROWS)
                pattPos = 0;
            if (oldPos == songPos && oldPatt == pattPos)
                break;
            else if (oldPos != songPos || (oldPatt != pattPos && pattPos == 0))
                jump = true;
            prevJump = posJump;
        } else {
            pattPos++;
            if (pattPos >= MAX_ROWS) {
                songPos++;
                pattPos = 0;
                jump = true;
                if (songPos >= context->ordNum())
                    break;
            }
        }
    }
    return subsongs;
}

} // namespace {}

namespace player::st3play {

void init() noexcept {}
void shutdown() noexcept {
#ifdef PLAYER_PROBE
    probe::closeMusic();
#endif
    play::closeMusic();
}

bool is_our_file(const char *path, const char *buf, size_t bufsize, size_t filesize) noexcept {
    if (bufsize < 0x70 || buf[0x1D] != 16 || memcmp(&buf[0x2C], "SCRM", 4) != 0)
        return false;

    // Reject non-authentic trackers (based on OpenMPT)
    const auto ver = *(le_uint16_t *)&buf[0x28];
    if (ver < 0x1300 || ver > 0x1321)
        return false;
    const auto ordnum = *(le_uint16_t *)&buf[0x20];
    if (ordnum > MAX_ORDNUM)
        return false;
    const auto insnum = *(le_uint16_t *)&buf[0x22];
    if (insnum > MAX_INSNUM)
        return false;
    const auto patNum = *(le_uint16_t *)&buf[0x24];
    if (patNum > MAX_PATNUM)
        return false;
    const auto flags = *(le_uint16_t *)&buf[0x26];
    const auto uc = buf[0x34];
    const uint8_t dp = buf[0x35];
    const auto special = *(le_uint16_t *)&buf[0x3e];
    // Sound Club 2
    if (!memcmp(&buf[0x36], "SCLUB2.0", 8))
        return false;
    // ModPlug Tracker / OpenMPT or Schism Tracker
    // NOTE: OpenMPT also has check for offsetsAreCanonical
    if (ver == 0x1320 && !special && !(ordnum & 0x01) && !uc && !(flags & ~0x50) && dp == 0xfc)
        return false;
    // PlayerPRO / Velvet Studio
    if (ver == 0x1320 && !special && !uc && !flags && dp != 0xfc)
        return false;
    return get_s3m_info(path, buf, bufsize) ? true : false;
}

optional<ModuleInfo> parse(const char *path, const char *buf, size_t size) noexcept {
    assert(size >= 0x40 + 32);

    st3play_context *context = new st3play_context(true, PRECALC_FREQ);
    assert(!context->moduleLoaded());

    const auto routing = s3m_routing(buf, size);
    assert(routing);
    const char *soundcardtype = routing->soundblaster ? "SB" : "GUS";

    optional<ModuleInfo> info;
    if (context->loadS3M((const uint8_t*)buf, size,
                         routing->soundblaster ? SOUNDCARD_SBPRO : SOUNDCARD_GUS)) {
        assert(context->soundcardtype() == (routing->soundblaster ? SOUNDCARD_SBPRO : SOUNDCARD_GUS));
        info = get_s3m_info(path, buf, size, soundcardtype);
        if (info) {
            info->maxsubsong = static_cast<int>(get_subsongs(context).size());
        }
    } else {
        DEBUG("player_st3play::parse parsing failed for %s\n", path);
    }

    delete context;
    return info;
}

optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &config) noexcept {
    assert(config.player == Player::st3play || config.player == Player::NONE);
    assert(config.tag == Player::st3play || config.tag == Player::NONE);
    assert(subsong >= 1);
    const auto routing = s3m_routing(buf, size);
    assert(routing);

    st3play_context *context = new st3play_context(config.probe, config.frequency);
    assert(!context->moduleLoaded());
    if (!context->loadS3M((const uint8_t*)buf, size,
                          routing->soundblaster ? SOUNDCARD_SBPRO : SOUNDCARD_GUS) ||
        !context->PlaySong()) {
        ERR("player_st3play::play could not play %s\n", path);
        delete context;
        return {};
    }

    if (subsong > 1) {
        const auto subsongs = get_subsongs(context);
        assert(static_cast<size_t>(subsong) <= subsongs.size());
        context->setPos(subsongs[subsong - 1]);
    }
    context->setWavRenderFlag(true);

    return PlayerState {Player::st3play, subsong, config.frequency, config.endian != endian::native, context, true, mixBufSize(config.frequency), 0, 0};
}

bool stop(PlayerState &state) noexcept {
    assert(state.player == Player::st3play);
    if (state.context) {
        const auto context = static_cast<st3play_context*>(state.context);
        assert(context);
        delete context;
    }
    return true;
}

pair<SongEnd::Status, size_t> render(PlayerState &state, char *buf, size_t size) noexcept {
    assert(state.player == Player::st3play);
    assert(size >= state.buffer_size);
    const auto context = static_cast<st3play_context*>(state.context);
    assert(context);
    assert(context->moduleLoaded());
    const auto prevPos = pair<int16_t,int16_t>(context->np_ord(), context->np_row());
    bool prevJump = context->jumpLoop();
    context->musmixer((int16_t*)buf, state.buffer_size / 4);
    const auto pos = pair<int16_t,int16_t>(context->np_ord(), context->np_row());
    bool jump = context->jumpLoop();
    // st3play clears WAVRender_Flag when the order list wraps around
    bool songend = !context->wavRenderFlag() || context->np_ord() >= context->ordNum();
    if (prevJump && !jump && prevPos.first >= pos.first && prevPos.second >= pos.second && context->ordNum() > 1) {
        for (auto i = pos.second; i <= prevPos.second; ++i) {
            context->seen.erase(pair<int16_t,int16_t>(pos.first, i));
        }
    }
    if (!songend && pos != prevPos && !prevJump && !jump) {
        songend |= !context->seen.insert(pos).second;
    }
    return pair<SongEnd::Status, size_t>(songend ? SongEnd::PLAYER : SongEnd::NONE, state.buffer_size);
}

bool restart(PlayerState &state) noexcept {
    assert(state.player == Player::st3play);
    const auto context = static_cast<st3play_context*>(state.context);
    assert(context);
    context->clearMixBuffer();
    // stops voices
    context->shutupsounds();
    context->resetAudioDither();
    context->setPos(context->startPos);
    context->setWavRenderFlag(true);
    return true;
}

} // namespace player::st3play
