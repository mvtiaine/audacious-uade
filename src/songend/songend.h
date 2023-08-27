// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#ifndef SONGEND_H_
#define SONGEND_H_

#include <vector>
#include <climits>

using namespace std;

namespace songend {

constexpr int PRECALC_FREQ = 8062;
constexpr int PRECALC_TIMEOUT = 3600;
constexpr int SILENCE_TIMEOUT = 30;
constexpr int MAX_SILENCE = 3000;

class SongEndDetector {
public:
    SongEndDetector(int rate) : rate(rate) {}

    void update(const char *bytes, size_t nbytes);
    int detect_loop();
    int detect_silence(int seconds);
    int detect_volume();
    int detect_repeat();
    int trim_silence(int offs_millis);
    int trim_volume(int offs_millis);

private:
    int rate;
    vector<char> buf;
    int16_t tmp[8];
    int itmp = 0;
    int ctmp = 0;
    int maxi = CHAR_MIN;
    int mini = CHAR_MAX;
};

}; // namespace songend

#endif /* SONGEND_H_ */
