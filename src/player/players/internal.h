// SPDX-License-Identifier: LGPL-2.1-or-later AND CC-PDM-1.0
// SPDX-AI-Disclosure: ai-assisted
// Copyright (C) 2024-2026 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include "common/std/optional.h"

#include <cassert>
#include <cstring>

#include "common/endian.h"
#include "common/strings.h"
#include "player/player.h"

using namespace std;
using namespace player;
using namespace common;

namespace player::internal {

// .sid extension conflict with SIDMon vs C64 SID files
constexpr_f2 bool is_sid(const char *path, const char *buf, size_t size) noexcept {
    return size >= 4 && (buf[0] == 'P' || buf[0] == 'R') && buf[1] == 'S' && buf[2] == 'I' && buf[3] == 'D';
};

constexpr_f2 bool is_prt(const char *path, const char *buf, size_t size) noexcept {
    return size >= 3 && (buf[0] == 'P' && buf[1] == 'R' && buf[2] == 'T');
}

constexpr_f2 bool is_ml(const char *path, const char *buf, size_t size) noexcept {
    return size >= 8 && (buf[0] == 'M' && buf[1] == 'L' && buf[2] == 'E' && buf[3] == 'D' &&
                         buf[4] == 'M' && buf[5] == 'O' && buf[6] == 'D' && buf[7] == 'L');
}

constexpr_f2 bool is_dm1(const char *path, const char *buf, size_t size) noexcept {
    return size >= 4 && (buf[0] == 'A' && buf[1] == 'L' && buf[2] == 'L' && buf[3] == ' ');
}

constexpr_f2 bool check_dm1(const char *path, const char *buf, size_t size) noexcept {
    // Based on HippoPlayer 68k assembly detection: read declared sample lengths and verify total size
    if (size >= 104 && is_dm1(path, buf, size)) {
        uint64_t total = 104;
        auto be32 = [&](size_t off) noexcept -> uint32_t {
            return (uint32_t)(((uint8_t)buf[off] << 24) | ((uint8_t)buf[off + 1] << 16) |
                              ((uint8_t)buf[off + 2] << 8) | (uint8_t)buf[off + 3]);
        };

        // read 25 big-endian ints starting at offset 4
        for (int i = 0; i < 25; ++i) {
            const size_t off = 4 + (size_t)i * 4;
            if (off + 4 > size)
                return false; // buffer too small to read declared lengths
            total += be32(off);
            if (total > (uint64_t)size)
                return false;
        }

        return total <= (uint64_t)size;
    }
    return false;
}

constexpr_f2 bool is_cm(const char *path, const char *buf, size_t size) noexcept {
    // Based on HippoPlayer 68k assembly detection: check jump/jsr/bra patterns then
    // scan a 0x400-byte window starting at offset 8 for three consecutive
    // big-endian longs 0x42280030, 0x42280031, 0x42280032.

    auto be16 = [&](size_t off) noexcept -> uint16_t {
        return (uint16_t)(((uint8_t)buf[off] << 8) | (uint8_t)buf[off + 1]);
    };

    auto be32 = [&](size_t off) noexcept -> uint32_t {
        return (uint32_t)(((uint8_t)buf[off] << 24) | ((uint8_t)buf[off + 1] << 16) |
                          ((uint8_t)buf[off + 2] << 8) | (uint8_t)buf[off + 3]);
    };

    size_t offs = 0;
    if (buf[0] == 0)
        offs = 0x40; // assume ...RON_KLAREN_SOUNDMODULE!... header in beginning

    if (size < offs + 8 + 0x400)
        return false;

    uint16_t w0 = be16(offs);

    // If first word is JMP (4EF9) or JSR (4EB9) we expect a JMP at offset 6.
    if (w0 == 0x4EF9 || w0 == 0x4EB9) {
        if (be16(offs + 6) != 0x4EF9)
            return false;
    } else if (w0 == 0x6000) {
        if (be16(offs + 4) != 0x6000)
            return false;
    } else {
        return false;
    }

    // Prepare scanning window: A1 = 8, A2 = A1 + 0x400
    const size_t start = offs + 8;
    const size_t window = 0x400;
    const size_t end = start + window;

    const uint32_t c1 = 0x42280030u;
    const uint32_t c2 = 0x42280031u;
    const uint32_t c3 = 0x42280032u;

    for (size_t off = start; off != end; off += 2) {
        if (be32(off) == c1 && be32(off + 4) == c2 && be32(off + 8) == c3)
            return true;
    }

    return false;
}

constexpr_f2 bool is_fc13(const char *path, const char *buf, size_t size) noexcept {
    return size >= 4 && buf[0] == 'S' && buf[1] == 'M' && buf[2] == 'O' && buf[3] == 'D';
}

constexpr_f2 bool check_fc13(const char *path, const char *buf, size_t size) noexcept {
    return size >= 5 && buf[0] == 'S' && buf[1] == 'M' && buf[2] == 'O' && buf[3] == 'D' &&
                        buf[4] == 0;
}

constexpr_f2 bool is_digibooster(const char *path, const char *buf, size_t size) noexcept {
    return size >= 13 && memcmp(buf, "DIGI Booster ", 13) == 0;
}

constexpr_f2 bool check_digibooster(const char *path, const char *buf, size_t size) noexcept {
    // "DIGI Booster module\0"
    return size >= 20 && memcmp(buf, "DIGI Booster module\0", 20) == 0;
}

constexpr_f2 bool is_ftm(const char *path, const char *buf, size_t size) noexcept {
    return size >= 3 && (buf[0] == 'F' && buf[1] == 'T' && buf[2] == 'M');
}

constexpr_f2 bool check_ftm(const char *path, const char *buf, size_t size) noexcept {
    return size >= 4 && (buf[0] == 'F' && buf[1] == 'T' && buf[2] == 'M' && buf[3] == 'N');
}

// detect xm early to avoid running uadecore and reduce log spam
constexpr_f2 bool is_xm(const char *path, const char *buf, size_t size) noexcept {
    return size >= 16 && memcmp(buf, "Extended Module:", 16) == 0;
}
    
// detect fst early to avoid running uadecore and reduce log spam
constexpr_f2 bool is_fst(const char *path,  const char *buf, size_t size) noexcept {
    // copied from uade amifilemagic.c (MOD_PC)
    return (size > 0x43b && (
      ((buf[0x438] >= '0' && buf[0x438] <= '9') && (buf[0x439] >= '0' && buf[0x439] <= '9') && buf[0x43a] == 'C' && buf[0x43b] == 'H')
        || ((buf[0x438] >= '0' && buf[0x438] <= '9') && buf[0x439] == 'C' && buf[0x43a] == 'H' && buf[0x43b] == 'N')
        || ( buf[0x438] == 'T' && buf[0x439] == 'D' && buf[0x43a] == 'Z')
        || ( buf[0x438] == 'O' && buf[0x439] == 'C' && buf[0x43a] == 'T' && buf[0x43b] == 'A')
        || ( buf[0x438] == 'C' && buf[0x439] == 'D' && buf[0x43a] == '8' && buf[0x43b] == '1'))
    );
}
    
constexpr_f2 bool is_s3m(const char *path,  const char *buf, size_t size) noexcept {
    return size >= 0x30 && memcmp(&buf[0x2C], "SCRM", 4) == 0;
}
    
constexpr_f2 bool is_it(const char *path,  const char *buf, size_t size) noexcept {
    return size >= 4 && buf[0] == 'I' && buf[1] == 'M' && buf[2] == 'P' && buf[3] == 'M';
}

// routing for S3M files: Impulse Tracker made files to it2play, the rest to st3play
struct S3mRouting {
    bool impulse;
    bool soundblaster; // SB/AdLib hardware, else GUS
    int channels;
};

// Impulse Tracker made S3M (based on OpenMPT)
constexpr_f2 bool s3m_impulse(const char *buf) noexcept {
    const auto ver = *(le_uint16_t *)&buf[0x28];
    const auto flags = *(le_uint16_t *)&buf[0x26];
    const auto uc = buf[0x34];
    const uint8_t dp = buf[0x35];
    const auto special = *(le_uint16_t *)&buf[0x3e];
    return ver == 0x3320 || (ver == 0x1320 && !special && !uc && flags == 8 && dp != 0xfc) ||
           ((ver & 0xFF00) == 0x3200 && (ver & 0xFF) >= 0x15 && (ver & 0xFF) <= 0x17) ||
           (ver & 0xF000) == 0x3000;
}

// validate the module against the capabilities of the given player (no fallback to
// another player); player NONE selects the player by the routing heuristics
// st3play's loader cannot pick the hardware for modules without PCM samples
// (load.c requires numSamples >= 2), and OPL is only present on SB/AdLib cards
inline optional<S3mRouting> s3m_routing(const char *buf, size_t size, Player player = Player::NONE) noexcept {
    const auto ver = *(le_uint16_t *)&buf[0x28];
    assert((ver >= 0x1300 && ver <= 0x1321) || (ver & 0xF000) == 0x3000);

    const bool impulse = s3m_impulse(buf);

    // a player disabled in configure cannot be selected; the sample format checks
    // below decide whether the forced player can play the module
    switch (player) {
    case Player::it2play:
        if (!PLAYER_it2play)
            return {};
        break;
    case Player::st3play:
        if (!PLAYER_st3play)
            return {};
        break;
    case Player::st3playold:
        if (!PLAYER_st3playold)
            return {};
        break;
    case Player::NONE:
        if (impulse ? !PLAYER_it2play : (!PLAYER_st3play && !PLAYER_st3playold))
            return {};
        break;
    default:
        break; // the general purpose players handle any S3M
    }

    const bool it = player == Player::NONE ? impulse : player == Player::it2play;
    const bool old = player == Player::st3playold ||
                     (player == Player::NONE && !PLAYER_st3play && PLAYER_st3playold);
    // it2play has no OPL emulation, st3play synthesizes it; st3playold's loader
    // rejects OPL instruments
    const bool hasOPL = !it && !old;

    uint8_t chnsettings[32];
	memcpy(chnsettings, &buf[0x40], sizeof chnsettings);
    bool hwseen[128] = {};
    int entries = 0;
    int distinct = 0;
    bool opl = false;
    bool hiPcm = false; // pattern columns 16-31 mapped to PCM channels
    for (size_t ch = 0; ch < sizeof(chnsettings); ++ch) {
        if (chnsettings[ch] & 0x80)
            continue; // channel muted
        const uint8_t type = chnsettings[ch] & 0x7F;
        // 16-31 are the AdLib channels
        if (type >= 16 && type <= 31) {
            if (!hasOPL)
                return {};
            // OPL percussion (25-31) is not supported by st3play
            if (type > 24)
                return {};
            opl = true;
        } else if (ch >= 16) {
            hiPcm = true;
        }
        // it2play: up to 64 host channels (S3M channel table has 32 entries)
        // st3play: 0-15 sample channels + 16-24 OPL melodic
        entries++;
        if (!hwseen[type]) {
            hwseen[type] = true;
            distinct++;
        }
    }
    // Authentic ST3 tops out at 16 PCM channels plus the 9 melodic AdLib ones,
    // so unmuted PCM in columns 16-31 duplicates a channel of columns 0-15 and
    // the table was written by a converter or a tracker masquerading as Scream
    // Tracker. st3play indexes voices by hardware channel and collapses the
    // duplicates, dropping their notes; only it2play plays them as written.
    if (hiPcm && !it)
        return {};
    // something wrong
    const int channels = it ? entries : distinct;
    if (!channels)
        return {};

    int16_t ordNum = *(le_uint16_t *)&buf[0x20];
    int16_t insnum = *(le_uint16_t *)&buf[0x22];
    uint16_t gusAddresses = 0;
    bool anySamples = false;
    for (auto i = 0; i < insnum; ++i) {
        // avoid UB read
        uint16_t offs = ((unsigned char)buf[0x60 + ordNum + (i * 2) + 1] << 8 |
                         (unsigned char)buf[0x60 + ordNum + (i * 2)]) << 4;
        if (offs == 0)
            continue; // empty
        assert((size_t)offs + 0x28 < size);
        uint8_t *ptr8 = (uint8_t *)&buf[offs];
        uint8_t type = ptr8[0x00];
        uint32_t length = *(le_uint32_t *)&ptr8[0x10];
        uint8_t flags = ptr8[0x1F];
        // ADPCM samples are not supported by any player here
        if (length && ptr8[0x1E] != 0)
            return {};
        // st3play supports 8-bit mono PCM only; Scream Tracker 3 did not support
        // stereo or 16-bit samples either. it2play does both (loaders/s3m.c),
        // st3playold 16-bit only
        if (length && !it && (flags & 2))
            return {};
        if (length && !it && !old && (flags & 4))
            return {};
        // OPL instruments (type 2/3) are only synthesized by st3play
        if (length && !hasOPL && type > 1)
            return {};
        // GUS memory address, stamped by ST3 for each PCM sample (openmpt
        // fingerprints it the same way)
        if (type <= 1) {
            if (length)
                anySamples = true;
            gusAddresses |= *(le_uint16_t *)&ptr8[0x28];
        }
    }
    // ST3 (except the early 3.00 revisions) writes a GUS address per sample; a
    // module claiming a later ST3 version with none was written by other software
    // (converters, trackers masquerading as ST3) and is not authentic
    if (!it && anySamples && !gusAddresses && ver != 0x1300)
        return {};
    return S3mRouting{impulse, opl || gusAddresses <= 1, channels};
}

// soundcardtype ("GUS"/"SB") can be passed by the caller after loading the module;
// the header based routing is used when omitted
// the caller's player must support the module (no fallback to another player);
// st3playold is a legacy alternative for the modules of st3play
inline std::optional<ModuleInfo> get_s3m_info(const char *path, const char *buf, size_t size,
                                              const char *soundcardtype = nullptr,
                                              Player player = Player::NONE) noexcept {
    const auto routing = s3m_routing(buf, size, player);
    if (!routing)
        return {};
    const auto ver = *(le_uint16_t *)&buf[0x28];
    const auto flags = *(le_uint16_t *)&buf[0x26];
    const auto uc = buf[0x34];
    const uint8_t dp = buf[0x35];
    const auto special = *(le_uint16_t *)&buf[0x3e];
    const bool impulse = routing->impulse;
    if (!soundcardtype)
        soundcardtype = routing->soundblaster ? "SB" : "GUS";

    if (player == Player::NONE)
        player = impulse ? Player::it2play : (PLAYER_st3play ? Player::st3play : Player::st3playold);

    char format[27];
    if (impulse) {
        if (ver == 0x3320 || (ver == 0x1320 && !special && !uc && flags == 8 && dp != 0xfc)) {
            snprintf(format, sizeof format, "Impulse Tracker 1.0x");
        } else if ((ver & 0xFF00) == 0x3200 && (ver & 0xFF) >= 0x15 && (ver & 0xFF) <= 0x17) {
            snprintf(format, sizeof format, "Impulse Tracker 2.14+");
        } else {
            snprintf(format, sizeof format, "Impulse Tracker %d.%02X", (ver & 0x0F00) >> 8, ver & 0xFF);
        }
    } else {
        if (ver == 0x1320) {
            // 3.21 writes the version number as 3.20
            snprintf(format, sizeof format, "Scream Tracker 3.2x (%s)", soundcardtype);
        } else {
            snprintf(format, sizeof format, "Scream Tracker 3.%02X (%s)", ver & 0xFF, soundcardtype);
        }
    }
    assert(player == Player::st3play || player == Player::st3playold || player == Player::it2play);
    return ModuleInfo{player, format, path, 1, 1, 1, routing->channels};
    //return ModuleInfo{player, format, path, 1, 1, 1, routing->channels}; // , songname};
}

} // namespace player::internal
