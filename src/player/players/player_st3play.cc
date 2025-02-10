// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

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
constexpr int MAX_INSNUM = 100;
constexpr int MAX_PATNUM = 100;
constexpr int MAX_ROWS = 64;
constexpr int PATT_SEP = 254;
constexpr int PATT_END = 255;

constexpr size_t mixBufSize(const int frequency) noexcept {
    return 4 * (frequency / 50 + (frequency % 50 != 0 ? 1 : 0));
}

mutex probe_guard;
struct st3play_context {
    const bool probe;
    int16_t startPos = 0;
    set<pair<int16_t,int16_t>> seen; // for subsong loop detection

    st3play_context(const bool probe) noexcept : probe(probe) {
        if (probe) probe_guard.lock();
        reset();
    }
    ~st3play_context() noexcept {
        // stop voices
        SetInterpolation(false);
        clearMixBuffer();
        Close();
        if (probe) probe_guard.unlock();
    }
    void reset() noexcept  {
        if (probe) probe::reset();
        else play::reset();
        startPos = 0;
        seen.clear();
    }
    bool moduleLoaded() const noexcept {
        if (probe) return probe::moduleLoaded;
        else return play::moduleLoaded;
    }
    bool loadS3M(const uint8_t *dat, const uint32_t modLen) noexcept {
        assert(!moduleLoaded());
        if (probe) return probe::loadS3M(dat, modLen);
        else return play::loadS3M(dat, modLen);
    }
    bool PlaySong(const uint8_t *moduleData, uint32_t dataLength, bool useInterpolationFlag, uint32_t audioFreq) noexcept {
        assert(!moduleLoaded());
        if (probe) return probe::st3play_PlaySong(moduleData, dataLength, useInterpolationFlag, audioFreq);
        else return play::st3play_PlaySong(moduleData, dataLength, useInterpolationFlag, audioFreq);
    }
    void Close() noexcept {
        if (probe) probe::st3play_Close();
        else play::st3play_Close();
    }
    bool FillAudioBuffer(int16_t *buffer, int32_t samples) noexcept {
        assert(moduleLoaded());
        if (probe) return probe::st3play_FillAudioBuffer(buffer, samples);
        else return play::st3play_FillAudioBuffer(buffer, samples);
    }
    void SetInterpolation(bool flag) noexcept {
        if (probe) probe::st3play_SetInterpolation(flag);
        else play::st3play_SetInterpolation(flag);
    }
    bool np_restarted() const noexcept {
        if (probe) return probe::np_restarted;
        else return play::np_restarted;
    }
    int16_t np_ord() const noexcept {
        if (probe) return probe::np_ord;
        else return play::np_ord;
    }
    int16_t np_row() const noexcept {
        if (probe) return probe::np_row;
        else return play::np_row;
    }
    uint16_t ordNum() const noexcept {
        if (probe) return probe::ordNum;
        else return play::ordNum;
    }
    uint16_t insNum() const noexcept {
        if (probe) return probe::insNum;
        else return play::insNum;
    }
    uint16_t patNum() const noexcept {
        if (probe) return probe::patNum;
        else return play::patNum;
    }
    uint8_t order(int16_t ordNum) const noexcept {
        if (probe) return probe::order[ordNum];
        else return play::order[ordNum];
    }
    uint16_t patDataLen(uint16_t patNum) const noexcept {
        if (probe) return probe::patDataLens[patNum];
        else return play::patDataLens[patNum];
    }
    void setPos(int16_t pos) noexcept  {
        assert(moduleLoaded());
        reset();
        startPos = pos;
        if (probe) probe::setPos(pos);
        else play::setPos(pos);
    }
    void clearMixBuffer() noexcept {
        if (probe) probe::clearMixBuffer();
        else play::clearMixBuffer();
    }

    pair<int16_t,int16_t> posJump(int pattNr, int16_t pattPos) const noexcept {
        int16_t effB = -1; // Position Jump 
        int16_t effC = -1; // Pattern Break
        uint8_t *patseg = nullptr;
        uint8_t *chnsettings = nullptr;
        uint16_t patDataLen = 0;
        if (probe && probe::patdata[pattNr]) {
            patseg = probe::patdata[pattNr];
            chnsettings = probe::chnsettings;
            patDataLen = probe::patDataLens[pattNr];
        } else if (!probe && play::patdata[pattNr]) {
            patseg = play::patdata[pattNr];
            chnsettings = play::chnsettings;
            patDataLen = play::patDataLens[pattNr];
        } else {
            return pair<int16_t,int16_t>(-1,-1);
        }
        assert(patseg);
        assert(chnsettings);
        // find pattPos offset in packed pattern data
        int i = pattPos, offs = 0;
       	uint8_t dat = 0;
        while (i > 0) {
            if (offs >= patDataLen) {
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
                if (offs >= patDataLen) {
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
                if (offs + 1 >= patDataLen) {
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
        if (probe) return probe::patloopcount > 0 || probe::patterndelay > 0;
        else return play::patloopcount > 0 || play::patterndelay > 0;
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
    probe::st3play_SetInterpolation(false);
    probe::clearMixBuffer();
    probe::st3play_Close();
#endif
    play::st3play_SetInterpolation(false);
    play::clearMixBuffer();
    play::st3play_Close();
}

bool is_our_file(const char *path, const char *buf, size_t size) noexcept {
    if (size < 0x70 || buf[0x1D] != 16 || memcmp(&buf[0x2C], "SCRM", 4) != 0)
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
    // Impulse Tracker < 1.03
    if (ver == 0x1320 && !special && !uc && flags == 8 && dp != 0xfc)
        return false;

    return get_s3m_info(path, buf, size) ? true : false;
}

optional<ModuleInfo> parse(const char *path, const char *buf, size_t size) noexcept {
    assert(size >= 0x40 + 32);

    st3play_context *context = new st3play_context(true);
    assert(!context->moduleLoaded());

    optional<ModuleInfo> info;
    if (context->loadS3M((const uint8_t*)buf, size)) {
        info = get_s3m_info(path, buf, size);
        if (info) {
            const auto subsongs = get_subsongs(context);
            info->maxsubsong = subsongs.size();
        }
    } else {
        WARN("player_st3play::parse parsing failed for %s\n", path);
    }

    delete context;
    return info;
}

optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &config) noexcept {
    assert(subsong >= 1);
    st3play_context *context = new st3play_context(config.probe);
    assert(!context->moduleLoaded());
    if (!context->PlaySong((uint8_t*)buf, size, true, config.frequency)) {
        ERR("player_st3play::play could not play %s\n", path);
        delete context;
        return {};
    }

    if (subsong > 1) {
        const auto subsongs = get_subsongs(context);
        assert(static_cast<size_t>(subsong) <= subsongs.size());
        context->setPos(subsongs[subsong - 1]);
    }

    return PlayerState {Player::st3play, subsong, config.frequency, config.endian != endian::native, context, true, mixBufSize(config.frequency), 0};
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
    assert(size >= mixBufSize(state.frequency));
    const auto context = static_cast<st3play_context*>(state.context);
    assert(context);
    assert(context->moduleLoaded());
    const auto prevPos = pair<int16_t,int16_t>(context->np_ord(), context->np_row());
    bool prevJump = context->jumpLoop();
    bool filled = context->FillAudioBuffer((int16_t*)buf, mixBufSize(state.frequency) / 4);
    assert(filled);
    const auto pos = pair<int16_t,int16_t>(context->np_ord(), context->np_row());
    bool jump = context->jumpLoop();
    bool songend = context->np_restarted() || context->np_ord() >= context->ordNum();
    if (prevJump && !jump && prevPos.first >= pos.first && prevPos.second >= pos.second && context->ordNum() > 1) {
        for (auto i = pos.second; i <= prevPos.second; ++i) {
            context->seen.erase(pair<int16_t,int16_t>(pos.first, i));
        }
    }
    if (!songend && pos != prevPos && !prevJump && !jump) {
        songend |= !context->seen.insert(pos).second;
    }
    return pair<SongEnd::Status, size_t>(songend ? SongEnd::PLAYER : SongEnd::NONE, mixBufSize(state.frequency));
}

bool restart(PlayerState &state) noexcept {
    assert(state.player == Player::st3play);
    const auto context = static_cast<st3play_context*>(state.context);
    assert(context);
    context->clearMixBuffer();
    // stops voices
    context->SetInterpolation(true);
    context->setPos(context->startPos);
    return true;
}

} // namespace player::st3play
