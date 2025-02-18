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
