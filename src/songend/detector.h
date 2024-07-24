// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <climits>
#include <cstdint>
#include <type_traits>
#include <vector>

#include "common/endian.h"

namespace songend::detector {

class SongEndDetector {

public:
    SongEndDetector(int rate, bool stereo, std::endian endian) :
        rate(rate), stereo(stereo), endian(endian) {}

    void update(const char *bytes, int nbytes);

    int detect_loop();
    
    int detect_silence(int seconds);
    int detect_volume(int seconds);
    int detect_repeat();
    
    int trim_silence(int offs_millis);
    int trim_volume(int offs_millis);

    const int rate;
    const bool stereo;
    const std::endian endian;
private:
    std::vector<int8_t> buf;
    int16_t tmp[8];
    int itmp = 0;
    int ctmp = 0;
    int maxi = INT8_MIN;
    int mini = INT8_MAX;
};

}; // namespace songend
