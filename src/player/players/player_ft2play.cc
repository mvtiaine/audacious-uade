// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#include "common/std/functional.h"
#include "common/std/optional.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <set>
#include <string>
#include <utility>

#include "common/constexpr.h"
#include "common/endian.h"
#include "common/logger.h"
#include "common/strings.h"
#include "player/player.h"

#include "3rdparty/replay/ft2play/ft2play.h"

using namespace std;
using namespace common;
using namespace player;
using namespace replay::ft2play;

namespace {

constexpr size_t mixBufSize(const int frequency) noexcept {
    return 4 * 4 * (frequency / 50 + (frequency % 50 != 0 ? 1 : 0));
}

// not in MODSig
constexpr const char *chn4 = "4CHN";

mutex probe_guard;

struct ft2play_context {
    const bool probe;
    int16_t startPos = 0;
    set<pair<int16_t,int16_t>> seen; // for subsong loop detection

    ft2play_context(const bool probe) noexcept : probe(probe) {
        if (probe) probe_guard.lock();
        reset();
    }
    ~ft2play_context() noexcept {
        stopVoices();
        mix_ClearChannels();
        mix_Free();
        freeMusic();
        if (probe) {
            memset(probe::stm, 0, sizeof (probe::stm));
        } else {
            memset(play::stm, 0, sizeof (play::stm));
        }
        setModuleLoaded(false);
        if (probe) probe_guard.unlock();
    }
    void reset() noexcept  {
        if (probe) {
            probe::song.pBreakFlag = probe::song.posJumpFlag = false;
            probe::song.pBreakPos = probe::song.pattDelTime = probe::song.pattDelTime2 = 0;
            probe::song.songPos = probe::song.pattNr = probe::song.pattPos = probe::song.pattLen = 0;
            probe::song.timer = 1;
            probe::song.globVol = 64;
        } else {
            play::song.pBreakFlag = play::song.posJumpFlag = false;
            play::song.pBreakPos = play::song.pattDelTime = play::song.pattDelTime2 = 0;
            play::song.songPos = play::song.pattNr = play::song.pattPos = play::song.pattLen = 0;
            play::song.timer = 1;
            play::song.globVol = 64;
        }
        startPos = 0;
        seen.clear();
    }
    bool moduleLoaded() const noexcept {
        if (probe) return probe::moduleLoaded;
        else return play::moduleLoaded;
    }
    uint16_t songLen() const noexcept {
        if (probe) return probe::song.len;
        else return play::song.len;
    }
    int16_t songPos() const noexcept {
        if (probe) return probe::song.songPos;
        else return play::song.songPos;
    }
    int16_t pattPos() const noexcept {
        if (probe) return probe::song.pattPos;
        else return play::song.pattPos;
    }
    void setModuleLoaded(bool moduleLoaded) const noexcept {
        if (probe) probe::moduleLoaded = moduleLoaded;
        else play::moduleLoaded = moduleLoaded;
    }
    void setSongTimer(uint16_t timer) const noexcept {
        if (probe) probe::song.timer = timer;
        else play::song.timer = timer;
    }
    void setVolumeRamping(bool volumeRamping) const noexcept {
        if (probe) probe::volumeRampingFlag = volumeRamping;
        else play::volumeRampingFlag = volumeRamping;
    }
    bool loadMusicFromData(const uint8_t *data, uint32_t dataLength) const noexcept {
        if (probe) return probe::loadMusicFromData(data, dataLength);
        else return play::loadMusicFromData(data, dataLength);
    }
    void freeMusic() const noexcept {
        if (probe) probe::freeMusic();
        else play::freeMusic();
    }
    bool mix_Init(int32_t audioBufferSize) const noexcept {
        if (probe) return probe::mix_Init(audioBufferSize);
        else return play::mix_Init(audioBufferSize);
    }
    void mix_Free(void) const noexcept {
        if (probe) probe::mix_Free();
        else play::mix_Free();
    }
    void mix_ClearChannels() const noexcept {
        if (probe) probe::mix_ClearChannels();
        else play::mix_ClearChannels();
    }
    void stopVoices() const noexcept {
        if (probe) probe::stopVoices();
        else play::stopVoices();
    }
    bool dump_Init(int32_t frq, int32_t amp, int16_t songPos) const noexcept {
        if (probe) return probe::dump_Init(frq, amp, songPos);
        else return play::dump_Init(frq, amp, songPos);
    }
    int32_t dump_GetFrame(int16_t *p) const noexcept {
        if (probe) return probe::dump_GetFrame(p);
        else return play::dump_GetFrame(p);
    }
    bool dump_EndOfTune(int16_t endSongPos) const noexcept {
        if (probe) return probe::dump_EndOfTune(endSongPos);
        else return play::dump_EndOfTune(endSongPos);
    }
    void setPos(int16_t pos) noexcept  {
        reset();
        startPos = pos;
        if (probe) probe::setPos(pos, 0);
        else play::setPos(pos, 0); 
    }
    uint16_t antChn() const noexcept  {
        if (probe) return probe::song.antChn;
        else return play::song.antChn;
    }
    uint16_t antPtn() const noexcept  {
        if (probe) return probe::song.antPtn;
        else return play::song.antPtn;
    }
    uint16_t repS() const noexcept {
        if (probe) return probe::song.repS;
        else return play::song.repS;
    }
    uint16_t pattLen(int pattNr) const noexcept {
        if (probe) return probe::pattLens[pattNr];
        else return play::pattLens[pattNr];
    }
    uint8_t pattNr(int16_t songPos) const noexcept {
        if (probe) return probe::song.songTab[songPos];
        else return play::song.songTab[songPos];
    }

    pair<int16_t,int16_t> posJump(int pattNr, int16_t pattPos) const noexcept {
        int16_t effB = -1; // Position Jump
        int16_t effD = -1; // Pattern Break
        if (probe && probe::patt[pattNr]) {
            assert(pattPos < probe::pattLens[pattNr]);
            auto *p = &probe::patt[pattNr][pattPos * probe::song.antChn];
            for (uint8_t i = 0; p && i < probe::song.antChn && (effB == -1 || effD == -1); i++, p++) {
                if (p->effTyp == 0xB)
                    effB = p->eff;
                else if (p->effTyp == 0xD)
                    effD = ((p->eff >> 4) * 10) + (p->eff & 0x0F);
            }
        } else if (!probe && play::patt[pattNr]) {
            assert(pattPos < play::pattLens[pattNr]);
            auto *p = &play::patt[pattNr][pattPos * play::song.antChn];
            for (uint8_t i = 0; p && i < play::song.antChn && (effB == -1 || effD == -1); i++, p++)
                if (p->effTyp == 0xB)
                    effB = p->eff;
                else if (p->effTyp == 0xD)
                    effD = ((p->eff >> 4) * 10) + (p->eff & 0x0F);
        }
        return pair<int16_t,int16_t>(effB,effD);
    }
    bool jumpLoop() const noexcept {
        if (probe) return probe::song.pBreakFlag || probe::song.posJumpFlag || probe::song.jumpLoopFlag;
        else return play::song.pBreakFlag || play::song.posJumpFlag || play::song.jumpLoopFlag;
    }
};

// these have special playback quirks in libopenmpt, so better use that instead
const set<string> xm_prog_blacklist = {
    "FastTracker v 2.00", // modplug
    "MadTracker 2.0",
    "OpenMPT ",
    "Sk@le Tracker",
    "Skale Tracker",
};

// copied from pmplay.c
struct XMHeader {
	char sig[17], name[21], progName[20];
	le_uint16_t ver;
	le_int32_t headerSize;
	le_uint16_t len, repS, antChn, antPtn, antInstrs;
} __attribute__ ((packed));

struct modSampleTyp {
	char name[22];
	be_uint16_t len;
	uint8_t fine, vol;
	be_uint16_t repS, repL;
} __attribute__ ((packed));

struct FSTHeader {
	char name[20];
	modSampleTyp sample[31];
	uint8_t len, repS, songTab[128];
	char Sig[4];
} __attribute__ ((packed));

constexpr_f2 bool is_fasttracker2(const char *buf, size_t size) noexcept {
    if (size < sizeof(XMHeader) || memcmp(buf, "Extended Module:", 16)) return false;
    const auto &h = (const XMHeader *)buf;
    if (h->ver < 0x102 || h->ver > 0x104 ||
        h->antChn < 2 || h->antChn > 32 || (h->antChn & 1) != 0 ||
        h->len > 256 || h->antPtn > 256 || h->antInstrs > 128) {
        DEBUG("player_ft2play::parse failed - ver %d progName %s len %d antChn %d antPtn %d antInstrs %d\n", (int16_t)h->ver, h->progName, (int16_t)h->len, (int16_t)h->antChn, (int16_t)h->antPtn, (int16_t)h->antInstrs);
        return false;
    }
    const auto progName = string(h->progName).substr(0,20);
    for (const auto &name : xm_prog_blacklist) {
        if (common::starts_with(progName, name)) return false;
    }
    return true;
}

constexpr_f2 bool is_fasttracker1(const char *buf, size_t size) noexcept {
    if (size < sizeof(FSTHeader)) return false;
    const auto &hdr = (const FSTHeader *)buf;
    const string sig = string() + hdr->Sig[0] + hdr->Sig[1] + hdr->Sig[2] + hdr->Sig[3];
    if (sig == chn4) return true;
    for (const auto &cmp : MODSig)
        if (sig == cmp) return true;
    return false;
}

vector<int16_t> get_subsongs(const ft2play_context *context) noexcept {
    assert(context->moduleLoaded());
    vector<int16_t> subsongs = {0};

    set<int16_t> seen;
    set<int16_t> notseen;
    for (int i = 0; i < context->songLen(); ++i) {
        if (context->pattNr(i) <= context->antPtn())
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
            int pattNr = context->pattNr(songPos);
            if (context->pattLen(pattNr) == 0 || pattNr > context->antPtn()) {
                seen.insert(songPos);
                notseen.erase(songPos);
                if (++songPos >= context->songLen())
                    break;
                continue;
            }
            if (notseen.size() > 1 || context->pattLen(pattNr) > 0)
                subsongs.push_back(songPos);
        }
        jump = false;
        seen.insert(songPos);
        notseen.erase(songPos);
        if (notseen.empty())
            break;

        int pattNr = context->pattNr(songPos);
        const auto posJump = context->posJump(pattNr, pattPos);
        if (posJump.first >= 0 || posJump.second >= 0) {
            int16_t oldPos = songPos;
            int16_t oldPatt = pattPos;
            songPos = posJump.first == -1 ? songPos + 1 : posJump.first;
           	if (songPos >= context->songLen()) {
                break;
            } else if (posJump == prevJump && posJump.first >= 0) {
                seen.insert(songPos);
                jump = true;
            }
            pattPos = posJump.second == -1 ? 0 : posJump.second;
       	    if (pattPos >= context->pattLen(context->pattNr(songPos)))
                pattPos = 0;
            if (oldPos == songPos && oldPatt == pattPos)
                break;
            else if (oldPos != songPos || (oldPatt != pattPos && pattPos == 0))
                jump = true;
            prevJump = posJump;
        } else {
            pattPos++;
       	    if (pattPos >= context->pattLen(pattNr)) {
    	    	songPos++;
                pattPos = 0;
                jump = true;
                if (songPos >= context->songLen()) {
                    break;
                }
            }
        }
    }
    return subsongs;
}

constexpr_f2 ModuleInfo get_xm_info(const char *path, const char *buf) noexcept {
    const auto &hdr = (const XMHeader *)buf;
    string progName = string(hdr->progName).substr(0,20);
    if (common::ends_with(progName, " ")) {
        progName.erase(progName.find_last_of(' ') + 1);
        progName.erase(progName.find_last_not_of(' ') + 1);
    }
    replace_if(progName.begin(), progName.end(), ::not_fn<int (*)(int _c)>(::isprint), '?');
    if (progName.empty()) progName = "<Unknown>";
    return {Player::ft2play, progName, path, 1, 1, 1, hdr->antChn};
}

constexpr_f2 int get_xm_version(const char *buf) noexcept {
    const auto &hdr = (const XMHeader *)buf;
    return hdr->ver;
}

constexpr_f2 ModuleInfo get_fst_info(const char *path, const char *buf) noexcept {
    const auto &hdr = (const FSTHeader *)buf;
    int channels = 2;
    const string sig = string() + hdr->Sig[0] + hdr->Sig[1] + hdr->Sig[2] + hdr->Sig[3];
    if (sig == chn4) {
        channels = 4;
    } else {
        for (const auto &cmp : MODSig) {
            if (sig == cmp) break;
            channels += 2;
        }
    }
    assert(channels <= 32);
    const auto format = (sig == "M.K.") ? "Fasttracker (M.K.)" : "Fasttracker";
    return {Player::ft2play, format, path, 1, 1, 1, channels};
}

} // namespace {}

namespace player::ft2play {

void init() noexcept {
    probe::interpolationFlag = true;
    play::interpolationFlag = true;
}

void shutdown() noexcept {
#ifdef PLAYER_PROBE
    probe::stopVoices();
    probe::mix_ClearChannels();
    probe::mix_Free();
    probe::freeMusic();
    probe::moduleLoaded = false;
#endif
    play::stopVoices();
    play::mix_ClearChannels();
    play::mix_Free();
    play::freeMusic();
    play::moduleLoaded = false;
}

bool is_our_file(const char *path, const char *buf, size_t bufsize, size_t filesize) noexcept {
    return is_fasttracker2(buf, bufsize) || is_fasttracker1(buf, bufsize);
}

optional<ModuleInfo> parse(const char *path, const char *buf, size_t size) noexcept {
    XMHeader xm;
    FSTHeader fst;

    const bool isXm = is_fasttracker2(buf, size);
    const bool isFst = !isXm && is_fasttracker1(buf, size);

    if (!isXm && !isFst)
        return {};

    ft2play_context *context = new ft2play_context(true);
    assert(!context->moduleLoaded());

    optional<ModuleInfo> info;
    if (context->loadMusicFromData((const uint8_t*)buf, size)) {
        info = isXm ? get_xm_info(path, buf) : get_fst_info(path, buf);
        const auto subsongs = get_subsongs(context);
        info->maxsubsong = subsongs.size();
    } else {
        DEBUG("player_ft2play::parse parsing failed for %s\n", path);
    }
    delete context;
    return info;
}

optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &config) noexcept {
    assert(config.player == Player::ft2play || config.player == Player::NONE);
    assert(config.tag == Player::ft2play || config.tag == Player::NONE);
    assert(subsong >= 1);
    bool volumeRamping = false;
    if (is_fasttracker2(buf, size))
        volumeRamping = get_xm_version(buf) >= 0x104;
    else
        assert(is_fasttracker1(buf, size));

    ft2play_context *context = new ft2play_context(config.probe);
    assert(!context->moduleLoaded());

    if (!context->loadMusicFromData((const uint8_t*)buf, size)) {
        ERR("player_ft2play::play parsing failed for %s\n", path);
        delete context;
        return {};
    }

    context->setVolumeRamping(volumeRamping);
    
    if (!context->mix_Init(mixBufSize(config.frequency)) || !context->dump_Init(config.frequency, 8, 0)) {
        ERR("player_ft2play::play init failed for %s\n", path);
        delete context;
        return {};
    }

    context->setModuleLoaded(true);

    if (subsong > 1) {
        const auto subsongs = get_subsongs(context);
        assert(static_cast<size_t>(subsong) <= subsongs.size());
        context->setPos(subsongs[subsong - 1]);
    }

    return PlayerState {Player::ft2play, subsong, config.frequency, config.endian != endian::native, context, true, mixBufSize(config.frequency), 0};
}

bool stop(PlayerState &state) noexcept {
    assert(state.player == Player::ft2play);
    if (state.context) {
        const auto context = static_cast<ft2play_context*>(state.context);
        assert(context);
        delete context;
    }
    return true;
}

pair<SongEnd::Status, size_t> render(PlayerState &state, char *buf, size_t size) noexcept {
    assert(state.player == Player::ft2play);
    assert(size >= state.buffer_size);
    const auto context = static_cast<ft2play_context*>(state.context);
    assert(context);
    assert(context->moduleLoaded());
    const auto prevPos = pair<int16_t, int16_t>(context->songPos(), context->pattPos());
    bool prevJump = context->jumpLoop();
    ssize_t totalbytes = context->dump_GetFrame((int16_t*)buf);
    const auto pos = pair<int16_t, int16_t>(context->songPos(), context->pattPos());
    bool jump = context->jumpLoop();
    bool songend = (context->dump_EndOfTune(context->songLen()-1) || context->songPos() >= context->songLen()) && !context->seen.empty();
    if (prevJump && !jump && prevPos.first >= pos.first && prevPos.second >= pos.second && context->songLen() > 1) {
        for (auto i = pos.second; i <= prevPos.second; ++i) {
            context->seen.erase(pair<int16_t,int16_t>(pos.first, i));
        }
    }
    if (!songend && pos != prevPos && !prevJump && !jump) {
        songend |= !context->seen.insert(pos).second;
    }
    return pair<SongEnd::Status, size_t>(songend ? SongEnd::PLAYER : SongEnd::NONE, totalbytes);
}

bool restart(PlayerState &state) noexcept {
    assert(state.player == Player::ft2play);
    const auto context = static_cast<ft2play_context*>(state.context);
    assert(context);
    context->setSongTimer(1);
    context->mix_ClearChannels();
    context->stopVoices();
    context->setPos(context->startPos);
    return true;
}

} // namespace player::ft2play
