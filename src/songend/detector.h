// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <bit>
#include <climits>
#include <cstdint>
#include <type_traits>
#include <vector>

namespace songend::detector {

class SongEndDetector {

public:
    SongEndDetector(int32_t rate, bool stereo, std::endian endian) :
        rate(rate), stereo(stereo), endian(endian) {}

    void update(const char *bytes, int32_t nbytes);

    uint32_t detect_loop();
    
    uint32_t detect_silence(int32_t seconds);
    uint32_t detect_volume(int32_t seconds);
    uint32_t detect_repeat();
    
    uint32_t trim_silence(int32_t offs_millis);
    uint32_t trim_volume(int32_t offs_millis);

    const uint32_t rate;
    const bool stereo;
    const std::endian endian;
private:
    std::vector<int8_t> buf;
    int16_t tmp[8];
    int32_t itmp = 0;
    int32_t ctmp = 0;
    int32_t maxi = INT8_MIN;
    int32_t mini = INT8_MAX;
};

}; // namespace songend
