// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <cassert>
#include <cstdint>
#include <mutex>
#include <set>
#include <string>

#include "common/logger.h"
#include "player/player.h"

#include "3rdparty/replay/ft2play/ft2play.h"

using namespace std;
using namespace common;
using namespace player;
using namespace replay::ft2play;

namespace {

struct xm_context {
    const bool probe;
    int16_t startPos = 0;
    set<pair<int16_t,int16_t>> seen; // for subsong loop detection

    xm_context(const bool probe) : probe(probe) {}

    bool moduleLoaded() const {
        if (probe) return probe::moduleLoaded;
        else return play::moduleLoaded;
    }
    uint16_t songLen() const {
        if (probe) return probe::song.len;
        else return play::song.len;
    }
    int16_t songPos() const {
        if (probe) return probe::song.songPos;
        else return play::song.songPos;
    }
    int16_t pattPos() const {
        if (probe) return probe::song.pattPos;
        else return play::song.pattPos;
    }
    void setModuleLoaded(bool moduleLoaded) const {
        if (probe) probe::moduleLoaded = moduleLoaded;
        else play::moduleLoaded = moduleLoaded;
    }
    void setSongTimer(uint16_t timer) const {
        if (probe) probe::song.timer = timer;
        else play::song.timer = timer;
    }
    bool loadMusicFromData(const uint8_t *data, uint32_t dataLength) const {
        if (probe) return probe::loadMusicFromData(data, dataLength);
        else return play::loadMusicFromData(data, dataLength);
    }
    void freeMusic() const {
        if (probe) probe::freeMusic();
        else play::freeMusic();
    }
    bool mix_Init(int32_t audioBufferSize) const {
        if (probe) return probe::mix_Init(audioBufferSize);
        else return play::mix_Init(audioBufferSize);
    }
    void mix_Free(void) const {
        if (probe) probe::mix_Free();
        else play::mix_Free();
    }
    void mix_ClearChannels() const {
        if (probe) probe::mix_ClearChannels();
        else play::mix_ClearChannels();
    }
    void stopVoices() const {
        if (probe) probe::stopVoices();
        else play::stopVoices();
    }
    bool dump_Init(int32_t frq, int32_t amp, int16_t songPos) const {
        if (probe) return probe::dump_Init(frq, amp, songPos);
        else return play::dump_Init(frq, amp, songPos);
    }
    void dump_Close(void) const {
        if (probe) probe::dump_Close();
        else play::dump_Close();
    }
    int32_t dump_GetFrame(int16_t *p) const {
        if (probe) return probe::dump_GetFrame(p);
        else return play::dump_GetFrame(p);
    }
    bool dump_EndOfTune(int16_t endSongPos) const {
        if (probe) return probe::dump_EndOfTune(endSongPos);
        else return play::dump_EndOfTune(endSongPos);
    }
    void setPos(int16_t pos, int16_t row) {
        startPos = pos;
        seen.clear();
        if (probe) probe::setPos(pos, row);
        else play::setPos(pos, row); 
    }
    void shutdown() const {
        dump_Close();
        mix_Free();
        freeMusic();
        setModuleLoaded(false);
    }
    uint16_t antChn() const {
        if (probe) return probe::song.antChn;
        else return play::song.antChn;
    }
    uint16_t repS() const {
        if (probe) return probe::song.repS;
        else return play::song.repS;
    }
    uint16_t pattLen() const {
        if (probe) return probe::song.pattLen;
        else return play::song.pattLen;
    }
    uint8_t pattNr(int16_t songPos) const {
        if (probe) return probe::song.songTab[songPos];
        else return play::song.songTab[songPos];
    }
    int16_t posJump(int pattNr, int16_t pattPos) const {
        if (probe) {
            auto *p = &probe::patt[pattNr][pattPos * probe::song.antChn];
        	for (uint8_t i = 0; i < probe::song.antChn; i++, p++)
                if (p->effTyp == 0xB)
                    return p->eff;
        } else {
            auto *p = &play::patt[pattNr][pattPos * play::song.antChn];
        	for (uint8_t i = 0; i < play::song.antChn; i++, p++)
                if (p->effTyp == 0xB)
                    return p->eff;
        }
        return -1;
    }
    bool jumpLoop() const {
        if (probe) return probe::song.pBreakFlag;
        else return play::song.pBreakFlag;
    }
};

constexpr size_t mixBufSize(const int frequency) {
    return 4 * 4 * (frequency / 50 + (frequency % 50 != 0 ? 1 : 0));
}

// these are assumed to use/export fully FT2-compatible XM format
const set<string> xm_prog_whitelist = {
    // FT2 authentic
    "FastTracker v2.00   ",
    "Fasttracker II clone",
    // trackers using XM format
    "MilkyTracker        ", // NOTE: some special handling in openmpt
    "MilkyTracker 1.00.00",
    "MilkyTracker 1.01.00",
    "MilkyTracker 1.02.00",
    "MilkyTracker 1.03.00",
    "MilkyTracker 1.04.00",
    "rst's SoundTracker  ",
    // trackers with XM export
    "DigiBooster Pro 2.17",
    "DigiBooster Pro 2.18",
    "DigiBooster Pro 2.19",
    "DigiBooster Pro 2.20",
    "DigiBooster Pro 2.21",
    // converters
    "DBM2XM converter    ",
    "MED2XM by J.Pynnonen", // NOTE: some special handling in xmp
    "MOD2XM 1.0",
    "Xrns2XMod 2.8.0.0",
    "Converted by MID2XM",
    "*Converted  XM-File*", // DigiTrakker?
};

mutex probe_guard;

// copied from pmplay.c
struct XMHeader {
	char sig[17], name[21], progName[20];
	uint16_t ver;
	int32_t headerSize;
	uint16_t len, repS, antChn, antPtn, antInstrs, flags, defTempo, defSpeed;
	uint8_t songTab[256];
} __attribute__ ((packed));

struct modSampleTyp {
	char name[22];
	uint16_t len;
	uint8_t fine, vol;
	uint16_t repS, repL;
} __attribute__ ((packed));

struct FSTHeader {
	char name[20];
	modSampleTyp sample[31];
	uint8_t len, repS, songTab[128];
	char Sig[4];
} __attribute__ ((packed));

bool is_fasttracker2(const char *buf, size_t size) {
    return size >= 16 && memcmp(buf, "Extended Module:", 16) == 0;
}

bool is_fasttracker1(const char *buf, size_t size) {
    if (size < sizeof(FSTHeader)) return false;
    const auto &hdr = (const FSTHeader *)buf;
    const string sig = string() + hdr->Sig[0] + hdr->Sig[1] + hdr->Sig[2] + hdr->Sig[3];
    for (const auto &cmp : MODSig) {
        if (sig == cmp)
            return true;
    }
    return false;
}

bool get_xm_header(const char *buf, size_t size, XMHeader &h) {
    assert(size > sizeof(XMHeader));
    memcpy(&h, buf, sizeof(XMHeader));
    if (h.ver < 0x0102 || h.ver > 0x104 ||
        h.antChn < 2 || h.antChn > 32 || (h.antChn & 1) != 0 ||
        h.len > 256 || h.antPtn > 256 || h.antInstrs > 128) {
        DEBUG("player_ft2play::parse failed - ver %d progName %s len %d antChn %d antPtn %d antInstrs %d\n", h.ver, h.progName, h.len, h.antChn, h.antPtn, h.antInstrs);
        return false;
    }
    const auto progName = string(h.progName).substr(0,20);
    return xm_prog_whitelist.contains(progName);
}

bool get_fst_header( const char *buf, size_t size, FSTHeader &h) {
    assert(size > sizeof(FSTHeader));
    memcpy(&h, buf, sizeof(FSTHeader));
    return true;
}

vector<int16_t> get_subsongs(const xm_context *context) {
    assert(context->moduleLoaded());
    vector<int16_t> subsongs = {0};

    set<uint8_t> seen;
    set<uint8_t> notseen;
    for (int i = 0; i < context->songLen(); ++i) {
        notseen.insert(i);
    }

    int16_t pattPos = 0;
    int16_t songPos = 0;
    bool jump = false;

    while (true) {
        if (jump && seen.contains(songPos)) {
            if (notseen.empty())
                break;
            songPos = *notseen.begin();
            pattPos = 0;
            subsongs.push_back(songPos);
        }
        jump = false;
        seen.insert(songPos);
        notseen.erase(songPos);

        int pattNr = context->pattNr(songPos);
        int16_t posJump = context->posJump(pattNr, pattPos);
        if (posJump >= 0) {
            songPos = posJump;
            pattPos = 0;
            jump = true;
        } else {
            pattPos++;
       	    if (pattPos >= context->pattLen()) {
    	    	songPos++;
                pattPos = 0;
                jump = true;
	        	if (songPos >= context->songLen()) {
			        songPos = context->repS() < context->songLen() ? context->repS() : 0;
                }
            }
        }
    }
    return subsongs;
}

ModuleInfo get_xm_info(const char *path, const XMHeader &hdr) {
    string progName = hdr.progName;
    progName.erase(progName.find_last_of(' ') + 1);
    progName.erase(progName.find_last_not_of(' ') + 1);
    return {Player::ft2play, progName, path, 1, 1, 1, hdr.antChn};
}

ModuleInfo get_fst_info(const char *path, const FSTHeader &hdr) {
    int channels = 2;
    const string sig = string() + hdr.Sig[0] + hdr.Sig[1] + hdr.Sig[2] + hdr.Sig[3];

    for (const auto &cmp : MODSig) {
        if (sig == cmp) break;
        channels += 2;
    }
    assert(channels <= 32);
    return {Player::ft2play, "Fasttracker", path, 1, 1, 1, channels};
}

} // namespace {}

namespace player::ft2play {

void init() {
    probe::interpolationFlag = true;
    probe::volumeRampingFlag = true;
    play::interpolationFlag = true;
    play::volumeRampingFlag = true;
}

void shutdown() {
    probe::dump_Close();
    probe::mix_Free();
    probe::freeMusic();
    probe::moduleLoaded = false;
    play::dump_Close();
    play::mix_Free();
    play::freeMusic();
    play::moduleLoaded = false;
}

bool is_our_file(const char *path, const char *buf, size_t size) {
    if (is_fasttracker2(buf, size)) {
        if (size < sizeof(XMHeader)) return false;
        XMHeader h;
        return get_xm_header(buf, size, h);
    }
    return is_fasttracker1(buf, size);
}

optional<ModuleInfo> parse(const char *path, const char *buf, size_t size) {
    XMHeader xm;
    FSTHeader fst;

    const bool isXm = is_fasttracker2(buf, size) && get_xm_header(buf, size, xm);
    const bool isFst = !isXm && is_fasttracker1(buf, size) && get_fst_header(buf, size, fst);

    if (!isXm && !isFst)
        return {};

    probe_guard.lock();
    xm_context *context = new xm_context(true);
    assert(!context->moduleLoaded());

    if (!context->loadMusicFromData((const uint8_t*)buf, size)) {
        ERR("player_ft2play::parse parsing failed for %s\n", path);
        context->shutdown();
        delete context;
        probe_guard.unlock();
        return {};
    }

    const auto subsongs = get_subsongs(context);
    context->shutdown();
    delete context;
    probe_guard.unlock();

    auto info = isXm ? get_xm_info(path, xm) : get_fst_info(path, fst);
    info.maxsubsong = subsongs.size();
    return info;
}

optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &config) {
    ModuleInfo info;
    if (is_fasttracker2(buf, size)) {
        XMHeader header;
        if (!get_xm_header(buf, size, header)) {
            ERR("player_ft2play::play parsing failed for %s\n", path);
            return {};
        }
        info = get_xm_info(path, header);

    } else if (is_fasttracker1(buf, size)) {
        FSTHeader header;
        if (!get_fst_header(buf, size, header)) {
            ERR("player_ft2play::play parsing failed for %s\n", path);
            return {};
        }
        info = get_fst_info(path, header);

    } else assert(false);

    if (config.probe) probe_guard.lock();
    xm_context *context = new xm_context(config.probe);
    assert(!context->moduleLoaded());

    if (!context->loadMusicFromData((const uint8_t*)buf, size)) {
        ERR("player_ft2play::play parsing failed for %s\n", path);
        context->shutdown();
        delete context;
        if (config.probe) probe_guard.unlock();
        return {};
    }

    if (!context->mix_Init(mixBufSize(config.frequency)) || !context->dump_Init(config.frequency, 8, 0)) {
        ERR("player_ft2play::play init failed for %s\n", path);
        context->shutdown();
        delete context;
        if (config.probe) probe_guard.unlock();
        return {};
    }

    context->setModuleLoaded(true);

    if (subsong > 1) {
        const auto subsongs = get_subsongs(context);
        assert(subsong <= subsongs.size());
        context->setPos(subsongs[subsong - 1], 0);
    }

    PlayerState state = {info, subsong, config.frequency, config.endian != endian::native, context, true, mixBufSize(config.frequency), 0};
    return state;
}

bool stop(PlayerState &state) {
    assert(state.info.player == Player::ft2play);
    if (state.context) {
        const auto context = static_cast<xm_context*>(state.context);
        assert(context);
        context->shutdown();
        delete context;
        if (context->probe) probe_guard.unlock();
    }
    return true;
}

pair<SongEnd::Status, size_t> render(PlayerState &state, char *buf, size_t size) {
    assert(state.info.player == Player::ft2play);
    assert(size >= mixBufSize(state.frequency));
    const auto context = static_cast<xm_context*>(state.context);
    assert(context);
    assert(context->moduleLoaded());
    const auto prevPos = pair(context->songPos(), context->pattPos());
    bool prevJumpLoop = context->jumpLoop();
    ssize_t totalbytes = context->dump_GetFrame((int16_t*)buf);
    const auto pos = pair(context->songPos(), context->pattPos());
    bool jumpLoop = context->jumpLoop();
    bool songend = context->dump_EndOfTune(context->songLen()-1);
    if (prevJumpLoop && !jumpLoop) {
        assert(prevPos.first == pos.first);
        assert(prevPos.second >= pos.second);
        for (auto i = pos.second; i <= prevPos.second; ++i) {
            context->seen.erase(pair(pos.first, i));
        }
    }
    if (!songend && pos != prevPos) {
        songend |= !context->seen.insert(pos).second;
    }
    return pair(songend ? SongEnd::PLAYER : SongEnd::NONE, totalbytes);
}

bool restart(PlayerState &state) {
    assert(state.info.player == Player::ft2play);
    const auto context = static_cast<xm_context*>(state.context);
    assert(context);
    context->setSongTimer(1);
    context->mix_ClearChannels();
    context->stopVoices();
    context->setPos(context->startPos, 0);
    return true;
}

} // namespace player::ft2play
