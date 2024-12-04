// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#include "common/std/optional.h"

#include <cstddef>
#include <mutex>
#include <set>
#include <utility>

#include "common/endian.h"
#include "common/logger.h"
#include "player/player.h"
#include "player/players/internal.h"

#include "3rdparty/replay/it2play/it2play.h"

using namespace std;
using namespace common;
using namespace player;
using namespace player::internal;
using namespace replay::it2play;

namespace {

constexpr int PATT_SEP = 254;
constexpr int PATT_END = 255;

constexpr size_t mixBufSize(const int frequency) noexcept {
    return 4 * (frequency / 50 + (frequency % 50 != 0 ? 1 : 0));
}

struct it2play_context {
    const bool probe;
    int16_t startPos = 0;
    set<pair<int16_t,int16_t>> seen; // for subsong loop detection
    it2play_context(const bool probe) noexcept : probe(probe) {
        reset();
    }
    void reset() noexcept  {
        Music_Stop();
        setSongStopped(false);
        startPos = 0;
        seen.clear();
    }
    bool LoadIT(const char *buf, size_t size) noexcept {
        if (probe) {
            probe::MEMFILE *m = probe::mopen((const uint8_t *)buf, size);
            if (!m) return false;
            bool res = probe::LoadIT(m);
            probe::mclose(&m);
            probe::Song.Loaded = true;
            return res;
        } else {
            play::MEMFILE *m = play::mopen((const uint8_t *)buf, size);
            if (!m) return false;
            bool res = play::LoadIT(m);
            play::mclose(&m);
            play::Song.Loaded = true;
            return res;
        }
    }
    bool LoadS3M(const char *buf, size_t size) noexcept {
        if (probe) {
            probe::MEMFILE *m = probe::mopen((const uint8_t *)buf, size);
            if (!m) return false;
            bool res = probe::LoadS3M(m);
            probe::mclose(&m);
            probe::Song.Loaded = true;
            return res;
        } else {
            play::MEMFILE *m = play::mopen((const uint8_t *)buf, size);
            if (!m) return false;
            bool res = play::LoadS3M(m);
            play::mclose(&m);
            play::Song.Loaded = true;
            return res;
        }
    }
    bool Music_LoadFromData(const char *buf, size_t size) noexcept {
        if (probe) return probe::Music_LoadFromData((uint8_t *)buf, size);
        else return play::Music_LoadFromData((uint8_t *)buf, size);
    }
    bool Music_Init(int32_t mixingFrequency, int32_t mixingBufferSize) noexcept {
        if (probe) return probe::Music_Init(mixingFrequency, mixingBufferSize, probe::DRIVER_HQ);
        else return play::Music_Init(mixingFrequency, mixingBufferSize, play::DRIVER_HQ);
    }
    void Music_PlaySong(uint16_t order) noexcept {
        setSongStopped(false);
        seen.clear();
        startPos = order;
        if (probe) probe::Music_PlaySong(order);
        else play::Music_PlaySong(order);
    }
    void Music_FreeSong() noexcept {
        if (probe) probe::Music_FreeSong();
        else play::Music_FreeSong();
    }
    void Music_Close() noexcept {
        if (probe) probe::Music_Close();
        else play::Music_Close();
    }
    void Music_Stop() noexcept {
        if (probe) probe::Music_Stop();
        else play::Music_Stop();
    }
    void Music_FillAudioBuffer(int16_t *buffer, int32_t numSamples) noexcept {
        if (probe) probe::Music_FillAudioBuffer(buffer, numSamples);
        else play::Music_FillAudioBuffer(buffer, numSamples);
    }
    void Music_StopChannels() noexcept {
        if (probe) probe::Music_StopChannels();
        else play::Music_StopChannels();
    }
    void setSongStopped(bool StopSong) noexcept {
        if (probe) probe::Song.StopSong = StopSong;
        else play::Song.StopSong = StopSong;
    }
    song_t& Song() const noexcept {
        if (probe) return probe::Song;
        else return play::Song;
    } 
    bool jumpLoop() const noexcept {
        if (probe) return probe::Song.PatternLooping || probe::Song.RowDelayOn;
        else return play::Song.PatternLooping || play::Song.RowDelayOn;
    }
    void UpdateGOTONote() noexcept {
        if (probe) probe::UpdateGOTONote();
        else play::UpdateGOTONote();
    }
    pair<pair<int16_t,int16_t>,uint8_t> posJump(int pattNr, int16_t pattPos) noexcept {
        int16_t effB = -1; // Position Jump
        int16_t effC = -1; // Pattern Break
        auto &song = probe ? probe::Song : play::Song;
        if (pattNr >= song.Header.PatNum || pattPos >= song.Patt[pattNr].Rows)
            return pair(pair(-1,-1),0);
      	if (pattPos == 0 || pattNr != song.DecodeExpectedPattern || ++song.DecodeExpectedRow != pattPos) {
            song.CurrentPattern = pattNr;
            song.ProcessRow = pattPos;
            UpdateGOTONote();
        }
        uint8_t *p = song.PatternOffset;
        uint8_t *p_start = song.Patt[pattNr].PackedData;
        if (p_start == NULL)
            return pair(pair(-1,-1),0);
     	uint8_t *p_max = p_start + song.Patt[song.CurrentPattern].DataLength;
        hostChn_t *hc;
        assert(p);
        uint8_t maxChn = 0;
        while (true) {
            if (effB != -1 && effC != -1)
                break;
            if (p >= p_max)
                break;
            int chnNum = *p++;
            if (chnNum == 0 || p >= p_max || ((chnNum & 0x7F) - 1) >= MAX_HOST_CHANNELS) // No more! else... go through decoding
                break;
            maxChn = (chnNum & 0x7F) - 1;

            hc = probe ? &probe::hChn[(chnNum & 0x7F) - 1] : &play::hChn[(chnNum & 0x7F) - 1];
            if (chnNum & 0x80 && p < p_max)
                hc->NotePackMask = *p++;

            if (hc->NotePackMask & 1 && p < p_max)
                p++;

            if (hc->NotePackMask & 2 && p < p_max)
                p++;

            if (hc->NotePackMask & 4 && p < p_max)
                p++;

            if (hc->NotePackMask & 8 && p + 1 < p_max) {
                uint8_t Cmd = *p++;
                uint8_t CmdVal = *p++;
                if (Cmd == 0x2)
                    effB = CmdVal;
                else if (Cmd == 0x03)
                    effC = CmdVal;
            }
        }
        song.PatternOffset = p;
        return pair(pair(effB,effC),maxChn);
    }
    void shutdown() noexcept {
        Music_FreeSong();
        Music_Close();        
        reset();
    }
};

mutex probe_guard;

struct ITHeader {
    char Sig[4];
    char SongName[26];
    uint8_t HighLightMinor;
    uint8_t HighLightMajor;
    le_uint16_t OrdNum, InsNum, SmpNum, PatNum, Cwtv, Cmwt, Flags, Special;
    uint8_t GlobalVol, MixVolume, InitialSpeed, InitialTempo, PanSep;
    uint8_t  PitchWheelDepth; 
    le_uint16_t MessageLength;
    le_uint32_t MessageOffset;
    le_uint32_t Reserved;
};

constexpr bool isIT(const char *buf, size_t size) noexcept {
    if (size > sizeof(ITHeader) && memcmp(buf, "IMPM", 4) == 0) {
        const auto &h = (const ITHeader*)buf;
        // from loaders/it.c
        if (h->OrdNum > MAX_ORDERS+1 || h->InsNum > MAX_INSTRUMENTS ||
            h->SmpNum > MAX_SAMPLES  || h->PatNum > MAX_PATTERNS) {
            return false;
        }
        // made with tracker and compatible with tracker must match Impulse Tracker 1.x - 2.x
        if (h->Cwtv < 0x0100 || h->Cwtv >= 0x0300 || h->Cmwt < 0x0100 || h->Cmwt >= 0x0300) {
            return false;
        }
        if ((h->Cwtv == 0x0214 && h->Cmwt == 0x0202 && h->Reserved == 0) ||
            (h->Cwtv == 0x0217 && h->Cmwt == 0x0200 && h->Reserved == 0)) {
            // ModPlug/OpenMPT
            return false;
        }
        if (h->Cwtv == 0x0214 && h->Cmwt == 0x0200 && h->HighLightMajor == 0 && h->HighLightMinor == 0 && h->Reserved == 0) {
            // OpenSPC conversion or ModPlug
            return false;
        }
        if (h->Cwtv == 0x0214 && h->Cmwt == 0x0214 && !memcmp(&h->Reserved, "CHBI", 4)) {
            // ChibiTracker
            return false;
        }
        return true;
    }
    return false;
}

constexpr bool isS3M(const char *buf, size_t size) noexcept {
    if (size >= 0x70 && buf[0x1D] == 16 && memcmp(&buf[0x2C], "SCRM", 4) == 0) {
        const auto ver = *(le_uint16_t *)&buf[0x28];
        if ((ver & 0xF000) == 0x3000) // Impulse Tracker >= 1.03
            return true;
        const auto flags = *(le_uint16_t *)&buf[0x26];
        const auto uc = buf[0x34];
        const uint8_t dp = buf[0x35];
        const auto special = *(le_uint16_t *)&buf[0x3e];
        if (ver == 0x3320 || (ver == 0x1320 && !special && !uc && flags == 8 && dp != 0xfc))
            return true; // Impulse Tracker 1.0x
    }
    return false;
}

optional<ModuleInfo> get_it_info(const char *path, it2play_context *context) noexcept {
    assert(context->Song().Loaded);
    assert(!context->Song().Playing);
    assert(!context->Song().StopSong);
    const auto &h = context->Song().Header;
    assert(h.Cwtv >= 0x0100 && h.Cwtv < 0x0300 && h.Cmwt >= 0x0100 && h.Cmwt < 0x0300);
    char format[25];
    // copied from OpenMPT
    if (h.Cmwt > 0x0214) {
        snprintf(format, sizeof format, "Inpulse Tracker 2.15");
    } else if ((h.Cwtv & 0xFFF) >= 0x0215 && (h.Cwtv & 0xFFF) <= 0x0217) {
        snprintf(format, sizeof format, "Impulse Tracker 2.14+");
    } else {
        snprintf(format, sizeof format, "Impulse Tracker %d.%02X", (h.Cwtv & 0x0F00) >> 8, h.Cwtv & 0xFF);
    }

    return ModuleInfo{Player::it2play, format, path, 1, 1, 1, 0};
}

pair<vector<int16_t>, uint8_t> get_subsongs_and_channels(it2play_context *context) noexcept {
    assert(context->Song().Loaded);
    vector<int16_t> subsongs = {0};

    set<int16_t> seen;
    set<int16_t> notseen;
    for (int i = 0; i < context->Song().Header.OrdNum; ++i) {
        if (context->Song().Orders[i] < PATT_SEP)
            notseen.insert(i);
    }

    auto prevJump = pair<int16_t,int16_t>(0,0);

    int16_t pattPos = 0;
    int16_t songPos = 0;
    bool jump = false;
    uint8_t maxChn = 0;

    while (true) {
        if (jump && seen.count(songPos)) {
            songPos = *notseen.begin();
            pattPos = 0;
            int pattNr = context->Song().Orders[songPos];
            if (notseen.size() > 1 || context->Song().Patt[pattNr].PackedData) {
                subsongs.push_back(songPos);
            }
        }
        jump = false;
        seen.insert(songPos);
        notseen.erase(songPos);
        if (notseen.empty())
            break;

        int pattNr = context->Song().Orders[songPos];
        if (pattNr >= PATT_SEP) {
            if (++songPos >= context->Song().Header.OrdNum)
                break;
            jump = (pattNr == PATT_END);
            seen.insert(songPos);
            continue;
        }

        const auto [posJump, maxCh] = context->posJump(pattNr, pattPos);
        maxChn = max(maxChn, maxCh);
        if (posJump.first >= 0 || posJump.second >= 0) {
            int16_t oldPos = songPos;
            int16_t oldPatt = pattPos;
            songPos = posJump.first == -1 ? songPos + 1 : posJump.first;
            if (songPos >= context->Song().Header.OrdNum) {
                break;
            } else if (posJump == prevJump && posJump.first >= 0) {
                seen.insert(songPos);
                jump = true;
            }
            pattPos = posJump.second == -1 ? 0 : posJump.second;
       	    if (pattPos >= context->Song().Patt[context->Song().Orders[songPos]].Rows)
                pattPos = 0;
            if (oldPos == songPos && oldPatt == pattPos)
                break;
            else if (oldPos != songPos || (oldPatt != pattPos && pattPos == 0))
                jump = true;
            prevJump = posJump;
        } else {
            pattPos++;
       	    if (pattPos >= context->Song().Patt[pattNr].Rows) {
    	    	songPos++;
                pattPos = 0;
                jump = true;
                if (songPos >= context->Song().Header.OrdNum) {
                    break;
                }
            }
        }
    }
    return pair(subsongs, maxChn + 1);
}

} // namespace {}

namespace player::it2play {

void init() noexcept {
}

void shutdown() noexcept {
    probe::Music_FreeSong();
    probe::Music_Close();
    play::Music_FreeSong();
    play::Music_Close();
}

bool is_our_file(const char *path, const char *buf, size_t size) noexcept {
    // accepts also some S3Ms (when made with Impulse Tracker)
    return isIT(buf, size) || isS3M(buf, size);
}

optional<ModuleInfo> parse(const char *path, const char *buf, size_t size) noexcept {
    bool it = isIT(buf, size);
    bool s3m = !it && isS3M(buf, size);
    if (!it && !s3m) return {};

    probe_guard.lock();
    it2play_context *context = new it2play_context(true);
    assert(!context->Song().Loaded);
    if ((it && !context->LoadIT(buf, size)) || (s3m && !context->LoadS3M(buf, size))) {
        WARN("player_it2play::parse parsing failed for %s\n", path);
        context->shutdown();
        delete context;
        probe_guard.unlock();
        return {};
    }

    optional<ModuleInfo> info;
    if (it) {
        info = get_it_info(path, context);
    } else if (s3m) {
        info = get_s3m_info(path, buf, size);
    }
    if (info) {
       const auto [subsongs, maxChn] = get_subsongs_and_channels(context);
       info->channels = maxChn;
       info->maxsubsong = subsongs.size();
    }

    context->shutdown();
    delete context;
    probe_guard.unlock();

    return info;
}

optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &config) noexcept {
    if (config.probe) probe_guard.lock();
    it2play_context *context = new it2play_context(config.probe);
    assert(!context->Song().Loaded);
    if (!context->Music_Init(config.frequency, mixBufSize(config.frequency)) ||
        !context->Music_LoadFromData(buf, size)) {
        ERR("player_it2play::play could not play %s\n", path);
        context->shutdown();
        delete context;
        if (config.probe) probe_guard.unlock();
        return {};
    }

    auto info = isIT(buf, size) ? get_it_info(path, context)
        : isS3M(buf, size) ? get_s3m_info(path, buf, size)
        : optional<ModuleInfo>{};
    if (!info) return {};

    const auto [subsongs, maxChn] = get_subsongs_and_channels(context);
    info->channels = maxChn;
    info->maxsubsong = subsongs.size();
    int order = 0;
    if (subsong > 1) {
        assert(static_cast<size_t>(subsong) <= subsongs.size());
        order = subsongs[subsong - 1];
    }
    context->Music_PlaySong(order);
    return PlayerState {info.value(), subsong, config.frequency, config.endian != endian::native, context, true, mixBufSize(config.frequency), 0};
}

pair<SongEnd::Status,size_t> render(PlayerState &state, char *buf, size_t size) noexcept {
    assert(state.info.player == Player::it2play);
    assert(size >= mixBufSize(state.frequency));
    const auto context = static_cast<it2play_context*>(state.context);
    assert(context);
    assert(context->Song().Loaded);
    const auto prevPos = pair(context->Song().CurrentOrder, context->Song().CurrentRow);
    bool prevJump = context->jumpLoop();
    context->Music_FillAudioBuffer((int16_t*)buf, mixBufSize(state.frequency) / 4);
    const auto pos = pair(context->Song().CurrentOrder, context->Song().CurrentRow);
    bool jump = context->jumpLoop();
    bool songend = context->Song().StopSong;
    if (prevJump && !jump && prevPos.first >= pos.first && prevPos.second >= pos.second) {
        for (auto i = pos.second; i <= prevPos.second; ++i) {
            context->seen.erase(pair(pos.first, i));
        }
    }
    if (!songend && pos != prevPos && !jump) {
        songend |= !context->seen.insert(pos).second;
    }
    return pair<SongEnd::Status, size_t>(songend ? SongEnd::PLAYER : SongEnd::NONE, mixBufSize(state.frequency));
}

bool stop(PlayerState &state) noexcept {
    assert(state.info.player == Player::it2play);
    if (state.context) {
        const auto context = static_cast<it2play_context*>(state.context);
        assert(context);
        context->shutdown();
        if (context->probe) probe_guard.unlock();
        delete context;
    }
    return true;
}

bool restart(PlayerState &state) noexcept {
    assert(state.info.player == Player::it2play);
    const auto context = static_cast<it2play_context*>(state.context);
    assert(context);
    context->Music_PlaySong(context->startPos);
    return true;
}

} // namespace player::it2play