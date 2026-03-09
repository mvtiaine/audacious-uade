// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

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

    auto offs = 0;
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
    return size > 0x2C && memcmp(&buf[0x2C], "SCRM", 4) == 0;
}
    
constexpr_f2 bool is_it(const char *path,  const char *buf, size_t size) noexcept {
    return size >= 4 && buf[0] == 'I' && buf[1] == 'M' && buf[2] == 'P' && buf[3] == 'M';
}

inline std::optional<ModuleInfo> get_s3m_info(const char *path, const char *buf, size_t size) noexcept {
    const auto ver = *(le_uint16_t *)&buf[0x28];
    assert((ver >= 0x1300 && ver <= 0x1321) || (ver & 0xF000) == 0x3000);
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
    // something wrong
    if (!channels)
        return {};

    int16_t ordNum = *(le_uint16_t *)&buf[0x20];
    int16_t insnum = *(le_uint16_t *)&buf[0x22];
    uint16_t gusAddresses = 0;
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
        // reject mods with OPL, ADPCM or stereo samples
        if (length && (type > 1 || ptr8[0x1E] != 0 || flags & 2))
            return {};
        gusAddresses |= *(le_uint16_t *)&ptr8[0x28];
    }

    const auto flags = *(le_uint16_t *)&buf[0x26];
    const auto uc = buf[0x34];
    const uint8_t dp = buf[0x35];
    const auto special = *(le_uint16_t *)&buf[0x3e];
    Player player;

    char format[27];
    if (ver == 0x3320 || (ver == 0x1320 && !special && !uc && flags == 8 && dp != 0xfc)) {
        player = Player::it2play;
        snprintf(format, sizeof format, "Impulse Tracker 1.0x");
    } else if ((ver & 0xFFF) >= 0x0215 && (ver & 0xFFF) <= 0x0217) {
        player = Player::it2play;
        snprintf(format, sizeof format, "Impulse Tracker 2.14+");
    } else if ((ver & 0xF000) == 0x3000) {
        player = Player::it2play;
        snprintf(format, sizeof format, "Impulse Tracker %d.%02X", (ver & 0x0F00) >> 8, ver & 0xFF);
    } else {
        player = Player::st3play;
        // Reject non-authentic trackers (based on OpenMPT)
        if(!gusAddresses && ver != 0x1300)
            return {};

        // max 16 channels for authentic Scream Tracker 3
        if (channels > 16)
            return {}; 

        const char *soundcardtype = gusAddresses > 1 ? "GUS" : "SB";
        if (ver == 0x1320) {
            // 3.21 writes the version number as 3.20
            snprintf(format, sizeof format, "Scream Tracker 3.2x (%s)", soundcardtype);
        } else {
            snprintf(format, sizeof format, "Scream Tracker 3.%02X (%s)", ver & 0xFF, soundcardtype);
        }
    }
    assert(player == Player::it2play || player == Player::st3play);
    return ModuleInfo{player, format, path, 1, 1, 1, channels};
}

} // namespace player::internal
