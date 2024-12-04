// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include "common/std/optional.h"

#include <cassert>

#include "common/endian.h"
#include "player/player.h"

using namespace std;
using namespace player;
using namespace common;

namespace player::internal {

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

    const auto flags = *(le_uint16_t *)&buf[0x26];
    const auto uc = buf[0x34];
    const uint8_t dp = buf[0x35];
    const auto special = *(le_uint16_t *)&buf[0x3e];
    Player player;

    char format[26];
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
            snprintf(format, sizeof format, "Scream Tracker 3.%02X (%s)", ((uint16_t)ver) & 0xFF, soundcardtype);
        }
    }
    assert(player == Player::it2play || player == Player::st3play);
    return ModuleInfo{player, format, path, 1, 1, 1, channels};
}

} // namespace player::internal
