// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#include "common/std/optional.h"

#include <cstddef>
#include <mutex>
#include <set>
#include <utility>
#include <vector>

#include "common/endian.h"
#include "common/logger.h"
#include "player/player.h"
#include "player/players/internal.h"

#include "3rdparty/replay/it2play/it2play.h"

using namespace std;
using namespace common;
using namespace player;
using namespace player::internal;
using namespace player::it2play;
using namespace replay::it2play;

namespace {

constexpr int PATT_SEP = 254;
constexpr int PATT_END = 255;

constexpr size_t mixBufSize(const int frequency) noexcept {
    return 4 * (frequency / 50 + (frequency % 50 != 0 ? 1 : 0));
}

constexpr int limitFreq(Driver driver, int frequency) {
    switch (driver) {
        case Driver::SB16MMX:
        case Driver::SB16:
        case Driver::WAVWRITER:
            return min(frequency, 64000);
        case Driver::HQ:
#if CPU_32BIT
            return min(frequency, 48000);
#else
            return min(frequency, 768000);
#endif
        default: assert(false); return min(frequency, 48000);
    }
}

mutex probe_guard;

struct it2play_context {
    const bool probe;
    int16_t startPos = 0;
    set<pair<int16_t,int16_t>> seen; // for subsong loop detection
    it2play_context(const bool probe) noexcept : probe(probe) {
        if (probe) probe_guard.lock();
        reset();
    }
    ~it2play_context() noexcept {
        Music_FreeSong();
        Music_Close();
        if (probe) probe_guard.unlock();
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
    bool Music_Init(int32_t mixingFrequency, int32_t mixingBufferSize, Driver driver, bool useFPUCode) noexcept {
        if (probe) probe::UseFPUCode = useFPUCode;
        else play::UseFPUCode = useFPUCode;
        if (probe) return probe::Music_Init(mixingFrequency, mixingBufferSize, static_cast<int32_t>(driver));
        else return play::Music_Init(mixingFrequency, mixingBufferSize, static_cast<int32_t>(driver));
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
    int frequency() const noexcept {
        if (probe) return static_cast<int>(probe::Driver.MixSpeed);
        else return static_cast<int>(play::Driver.MixSpeed);
    }
    pair<pair<int16_t,int16_t>,uint8_t> posJump(int pattNr, int16_t pattPos) noexcept {
        int16_t effB = -1; // Position Jump
        int16_t effC = -1; // Pattern Break
        auto &song = probe ? probe::Song : play::Song;
        if (pattNr >= song.Header.PatNum || pattPos >= song.Patt[pattNr].Rows)
            return pair<pair<int16_t,int16_t>,uint8_t>(pair<int16_t,int16_t>(-1,-1),0);
      	if (pattPos == 0 || pattNr != song.DecodeExpectedPattern || ++song.DecodeExpectedRow != pattPos) {
            song.CurrentPattern = pattNr;
            song.ProcessRow = pattPos;
            UpdateGOTONote();
        }
        uint8_t *p = song.PatternOffset;
        uint8_t *p_start = song.Patt[pattNr].PackedData;
        if (p_start == NULL)
            return pair<pair<int16_t,int16_t>,uint8_t>(pair<int16_t,int16_t>(-1,-1),0);
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
        return pair<pair<int16_t,int16_t>,uint8_t>(pair<int16_t,int16_t>(effB,effC),maxChn);
    }
};

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
        const uint32_t PtrListOffset = 192 + h->OrdNum;
        const uint32_t smpOffset0 = PtrListOffset + h->InsNum * 4;
        const uint32_t patOffset0 = smpOffset0 + h->SmpNum * 4;
        if (patOffset0 + 4 >= size)
            return false;
        le_uint32_t smpVal, patVal;
        memcpy(&smpVal, &buf[smpOffset0], sizeof(le_uint32_t));
        memcpy(&patVal, &buf[patOffset0], sizeof(le_uint32_t));
        const le_uint32_t *smpPos = &smpVal;
        const le_uint32_t *patPos = &patVal;
        if (h->Cwtv == 0x0202 && h->Cmwt == 0x0200 && h->HighLightMajor == 0 && h->HighLightMinor == 0 && h->Reserved == 0 && patPos[0] != 0 && patPos[0] < smpPos[0]) {
            // ModPlug Tracker 1.0 pre-alpha / alpha
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

optional<ModuleInfo> get_it_info(const char *path, const char *buf, size_t size) noexcept {
    const auto &h = (const ITHeader*)buf;
    assert(h->Cwtv >= 0x0100 && h->Cwtv < 0x0300 && h->Cmwt >= 0x0100 && h->Cmwt < 0x0300);
    char format[25];
    // copied from OpenMPT
    if (h->Cmwt > 0x0214) {
        snprintf(format, sizeof format, "Impulse Tracker 2.15");
    } else if ((h->Cwtv & 0xFFF) >= 0x0215 && (h->Cwtv & 0xFFF) <= 0x0217) {
        snprintf(format, sizeof format, "Impulse Tracker 2.14+");
    } else {
        snprintf(format, sizeof format, "Impulse Tracker %d.%02X", (h->Cwtv & 0x0F00) >> 8, h->Cwtv & 0xFF);
    }

    return ModuleInfo{Player::it2play, format, path, 1, 1, 1, 0};
}

pair<vector<int16_t>, uint8_t> get_subsongs_and_channels(it2play_context *context) noexcept {
    assert(context->Song().Loaded);
    vector<int16_t> subsongs = {0};

    set<int16_t> seen;
    set<int16_t> notseen;
    for (int i = 0; i < context->Song().Header.OrdNum && i < MAX_ORDERS; ++i) {
        if (context->Song().Orders[i] <= context->Song().Header.PatNum)
            notseen.insert(i);
    }

    auto prevJump = pair<int16_t,int16_t>(0,0);

    int16_t pattPos = 0;
    int16_t songPos = 0;
    bool jump = false;
    uint8_t maxChn = 0;

    while (true) {
        if (jump && seen.count(songPos) && !notseen.empty()) {
            songPos = *notseen.begin();
            pattPos = 0;
            int pattNr = context->Song().Orders[songPos];
            if (context->Song().Patt[pattNr].Rows == 0 || pattNr > context->Song().Header.PatNum) {
                seen.insert(songPos);
                notseen.erase(songPos);
                if (++songPos >= context->Song().Header.OrdNum)
                    break;
                continue;
            }
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

        const auto _posJump = context->posJump(pattNr, pattPos);
        const auto posJump = _posJump.first;
        maxChn = max(maxChn, _posJump.second);
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
            int nextPattNr = context->Song().Orders[songPos];
       	    if (nextPattNr >= PATT_SEP || pattPos >= context->Song().Patt[nextPattNr].Rows)
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
    return pair<vector<int16_t>, uint8_t>(subsongs, maxChn + 1);
}

} // namespace {}

namespace player::it2play {

void init() noexcept {
}

void shutdown() noexcept {
#ifdef PLAYER_PROBE
    probe::Music_FreeSong();
    probe::Music_Close();
#endif
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

    it2play_context *context = new it2play_context(true);
    assert(!context->Song().Loaded);
    optional<ModuleInfo> info;
    if ((it && !context->LoadIT(buf, size)) || (s3m && !context->LoadS3M(buf, size))) {
        DEBUG("player_it2play::parse parsing failed for %s\n", path);
    } else {
        if (it) {
            info = get_it_info(path, buf, size);
        } else if (s3m) {
            info = get_s3m_info(path, buf, size);
        }
        if (info) {
            const auto subsongs = get_subsongs_and_channels(context);
            info->channels = subsongs.second;
            info->maxsubsong = subsongs.first.size();
        }
    }
    delete context;
    return info;
}

optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &config) noexcept {
    assert(subsong >= 1);
    it2play_context *context = new it2play_context(config.probe);
    assert(!context->Song().Loaded);
    const auto &it2play_config = static_cast<const IT2PlayConfig&>(config);
    const auto driver = it2play_config.player == Player::it2play ? it2play_config.driver : IT2PlayConfig().driver;
    int freq = limitFreq(driver, config.frequency);
    bool useFPUCode = false;
    if (isIT(buf, size)) {
        const auto info = get_it_info(path, buf, size);
        assert(info);
        useFPUCode = info->format == "Impulse Tracker 2.15";
    }
    if (!context->Music_Init(config.frequency, mixBufSize(freq), driver, useFPUCode) ||
        !context->Music_LoadFromData(buf, size)) {
        ERR("player_it2play::play could not play %s\n", path);
        delete context;
        return {};
    }

    int order = 0;
    if (subsong > 1) {
        const auto subsongs = get_subsongs_and_channels(context);
        assert(static_cast<size_t>(subsong) <= subsongs.first.size());
        order = subsongs.first[subsong - 1];
    }
    context->Music_PlaySong(order);

    return PlayerState {Player::it2play, subsong, context->frequency(), config.endian != endian::native, context, true, mixBufSize(freq), 0};
}

pair<SongEnd::Status,size_t> render(PlayerState &state, char *buf, size_t size) noexcept {
    assert(state.player == Player::it2play);
    assert(size >= mixBufSize(state.frequency));
    const auto context = static_cast<it2play_context*>(state.context);
    assert(context);
    const auto &song = context->Song();
    assert(song.Loaded);
    const auto prevPos = pair<uint16_t, uint16_t>(song.CurrentOrder, song.CurrentRow);
    bool prevJump = context->jumpLoop();
    const auto prevProcessRow = song.ProcessRow;
    context->Music_FillAudioBuffer((int16_t*)buf, mixBufSize(state.frequency) / 4);
    const auto pos = pair<uint16_t, uint16_t>(song.CurrentOrder, song.CurrentRow);
    bool jump = context->jumpLoop();
    bool songend = song.StopSong || song.CurrentOrder >= song.Header.OrdNum;
    if (prevJump && !jump && prevPos.first >= pos.first && prevPos.second >= pos.second && song.Header.OrdNum > 1) {
        for (auto i = pos.second; i <= prevPos.second; ++i) {
            context->seen.erase(pair<int16_t,int16_t>(pos.first, i));
        }
    }
    if (!songend && pos != prevPos && !prevJump && !jump) {
        songend |= !context->seen.insert(pos).second;
    }
    // XXX quick and dirty hack for Bogdan/dream.it and others (Bxx jump to same order)
    if (!prevJump && !jump && pos == prevPos && song.ProcessOrder == (song.CurrentOrder - 1) && song.ProcessRow == 0xFFFE && prevProcessRow == 0xFFFE && context->seen.count(pos))
        songend = true;
    return pair<SongEnd::Status,size_t>(songend ? SongEnd::PLAYER : SongEnd::NONE, mixBufSize(state.frequency));
}

bool stop(PlayerState &state) noexcept {
    assert(state.player == Player::it2play);
    if (state.context) {
        const auto context = static_cast<it2play_context*>(state.context);
        assert(context);
        delete context;
    }
    return true;
}

bool restart(PlayerState &state) noexcept {
    assert(state.player == Player::it2play);
    const auto context = static_cast<it2play_context*>(state.context);
    assert(context);
    context->Music_PlaySong(context->startPos);
    return true;
}

} // namespace player::it2play
