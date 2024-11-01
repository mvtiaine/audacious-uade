// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#include "common/std/optional.h"

#include <cassert>
#include <cstring>
#include <mutex>
#include <set>

#include "common/endian.h"
#include "common/logger.h"
#include "player/player.h"

#include "3rdparty/replay/st3play/st3play.h"

using namespace std;
using namespace common;
using namespace player;
using namespace replay::st3play;

namespace {

constexpr int MAX_ORDNUM = 256;
constexpr int MAX_INSNUM = 100;
constexpr int MAX_PATNUM = 100;
constexpr int MAX_ROWS = 64;
constexpr int PATT_SEP = 254;
constexpr int PATT_END = 255;

constexpr size_t mixBufSize(const int frequency) noexcept {
    return 4 * 4 * (frequency / 50 + (frequency % 50 != 0 ? 1 : 0));
}

mutex probe_guard;
struct st3play_context {
    const bool probe;
    int16_t startPos = 0;
    set<pair<int16_t,int16_t>> seen; // for subsong loop detection

    st3play_context(const bool probe) noexcept : probe(probe) {
        reset();
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
    void shutdown() noexcept {
        // stop voices
        SetInterpolation(false);
        clearMixBuffer();
        Close();
        reset();
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
        if (context->order(i) < PATT_SEP)
            notseen.insert(i);
    }

    auto prevJump = pair<int16_t,int16_t>(0,0);

    int16_t pattPos = 0;
    int16_t songPos = 0;
    bool jump = false;

    while (true) {
        if (jump && seen.count(songPos)) {
            songPos = *notseen.begin();
            pattPos = 0;
            int pattNr = context->order(songPos);
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

        pair<int16_t,int16_t> posJump = context->posJump(pattNr, pattPos);
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

optional<ModuleInfo> get_s3m_info(const char *path, const char *buf, size_t size) noexcept {
    const auto ver = *(le_uint16_t *)&buf[0x28];
    assert(ver >= 0x1300 && ver <= 0x1321);
    uint8_t chnsettings[32];
	memcpy(chnsettings, &buf[0x40], sizeof chnsettings);
    int channels = 0;
    for (size_t ch = 0; ch < sizeof(chnsettings); ++ch) {
        if (chnsettings[ch] > 0xF && chnsettings[ch] <= 0x7F) {
            // reject mods with OPL channels as they are not supported
            return {};
        } else if ((chnsettings[ch] & 0x7F) < 16) {
            channels++;
        }
    }
    int16_t ordNum = *(le_uint16_t *)&buf[0x20];
    int16_t insnum = *(le_uint16_t *)&buf[0x22];
    uint16_t gusAddresses = 0;
    assert(ordNum <= MAX_ORDNUM);
    assert(insnum <= MAX_INSNUM);
    for (auto i = 0; i < insnum; ++i) {
        uint16_t offs = *(le_uint16_t *)&buf[0x60 + ordNum + (i * 2)] << 4;
        if (offs == 0)
            continue; // empty
        assert((size_t)offs + 0x28 < size);
        uint8_t *ptr8 = (uint8_t *)&buf[offs];
        uint8_t type = ptr8[0x00];
        uint32_t length = *(le_uint32_t *)&ptr8[0x10];
        uint8_t flags = ptr8[0x1F];
        // reject mods with OPL, ADPCM or stereo samples
        if (length && (type > 1 || ptr8[0x1E] != 0 || flags & 2))
            return {};
        gusAddresses |= *(uint16_t *)&ptr8[0x28];
    }

    // Reject non-authentic trackers (based on OpenMPT)
    if(!gusAddresses && ver != 0x1300)
        return {};

    uint8_t soundcardtype = gusAddresses > 1 ? 0 : 1;

    char format[26];
    // // 3.21 writes the version number as 3.20
    if (ver == 0x1320)
        snprintf(format, sizeof format, "Scream Tracker 3.2x (%s)", soundcardtype == 0 ? "GUS" : "SB");
    else
        snprintf(format, sizeof format, "Scream Tracker 3.%02X (%s)", ((uint16_t)ver) & 0xFF, soundcardtype == 0 ? "GUS" : "SB");

    return channels > 0 && channels <= 16
        ? ModuleInfo{Player::st3play, format, path, 1, 1, 1, channels}
        : optional<ModuleInfo>{};
}

} // namespace {}

namespace player::st3play {

void init() noexcept {}
void shutdown() noexcept {
    probe::st3play_SetInterpolation(false);
    probe::clearMixBuffer();
    probe::st3play_Close();
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

    return get_s3m_info(path, buf, size).has_value();
}

optional<ModuleInfo> parse(const char *path, const char *buf, size_t size) noexcept {
    assert(size >= 0x40 + 32);

    probe_guard.lock();
    st3play_context *context = new st3play_context(true);
    assert(!context->moduleLoaded());

    if (!context->loadS3M((const uint8_t*)buf, size)) {
        ERR("player_st3play::parse parsing failed for %s\n", path);
        context->shutdown();
        delete context;
        probe_guard.unlock();
        return {};
    }

    auto info = get_s3m_info(path, buf, size);
    if (info) {
       const auto subsongs = get_subsongs(context);
       info->maxsubsong = subsongs.size();
    }

    context->shutdown();
    delete context;
    probe_guard.unlock();

    return info;
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

    auto info = get_s3m_info(path, buf, size);
    if (!info) return {};

    const auto subsongs = get_subsongs(context);
    info->maxsubsong = subsongs.size();
    if (subsong > 1) {
        assert(static_cast<size_t>(subsong) <= subsongs.size());
        context->setPos(subsongs[subsong - 1]);
    }
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
    assert(context->moduleLoaded());
    const auto prevPos = pair<int16_t,int16_t>(context->np_ord(), context->np_row());
    bool prevJump = context->jumpLoop();
    bool filled = context->FillAudioBuffer((int16_t*)buf, mixBufSize(state.frequency) / 4);
    assert(filled);
    const auto pos = pair<int16_t,int16_t>(context->np_ord(), context->np_row());
    bool jump = context->jumpLoop();
    bool songend = context->np_restarted();
    if (prevJump && !jump && prevPos.first >= pos.first && prevPos.second >= pos.second) {
        for (auto i = pos.second; i <= prevPos.second; ++i) {
            context->seen.erase(pair<int16_t,int16_t>(pos.first, i));
        }
    }
    if (!songend && pos != prevPos && !jump) {
        songend |= !context->seen.insert(pos).second;
    }
    return pair<SongEnd::Status, size_t>(songend ? SongEnd::PLAYER : SongEnd::NONE, mixBufSize(state.frequency));
}

bool restart(PlayerState &state) noexcept {
    assert(state.info.player == Player::st3play);
    const auto context = static_cast<st3play_context*>(state.context);
    assert(context);
    context->clearMixBuffer();
    // stops voices
    context->SetInterpolation(true);
    context->setPos(context->startPos);
    return true;
}

} // namespace player::st3play
