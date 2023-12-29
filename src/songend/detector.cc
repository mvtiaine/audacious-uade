// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

// This code tries to detect song end in the input audio, similar to Deliplayer "deep songend scan" feature.
// It first tries to detect loop length and then find the start of first loop based on that.
// Max length of loop detected is around 10-20mins from 1h of audio data.

// The algorithm used can be described as "improvised autocorrelation" and is very ad hoc.
// I'd estimate it works quite nicely (within couple of seconds of correct length)
// at least 90% of the time, and works also for some cases Deliplayer fails with.
// It doesn't work as well with very basic/raw (non-amiga) synth sounds/players though (like YMST).
// There are also some edge cases handled separately, to detect for example some low volume "stuck" sound
// in the end which uade silence detection won't catch.

// Needs quite a lot of memory currently, something like 150M-200M for processing 1h of audio.
// Input should be S16LE but in mono, as only the other stereo channel is used (uade panning=1).
// Audio frequency developed/tested with is 8062Hz.

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <ios>
#include <map>
#include <numeric>
#include <set>
#include <utility>
#include <unistd.h>

#include "songend/detector.h"

using namespace std;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat" // FIXME Xcode wants %lld and GCC wants %ld for int64_t :P

//#define TRACE1(fmt,...) fprintf(stderr, fmt, ## __VA_ARGS__)
//#define TRACE2(fmt,...) fprintf(stderr, fmt, ## __VA_ARGS__)
#define TRACE1(fmt,...) while (0)
#define TRACE2(fmt,...) while (0)

namespace {

constexpr int THRESHOLD_SILENCE = 1;
constexpr int THRESHOLD_VOLUME = 4;
constexpr auto REPEAT_THRESHOLD = 6000u;

void flatten_diffsums(vector<int64_t> &diffsums) {
    const auto shift = [&](const vector<pair<size_t,int64_t>> &shifts, vector<int64_t> &diffsums) {
        ssize_t baseidx = shifts.back().first;
        auto baseval = shifts.back().second;
        ssize_t previdx = baseidx;
        for (ssize_t i = shifts.size() - 2; i >= 0; --i) {
            const ssize_t newidx = shifts[i].first;
            const auto val = shifts[i].second;
            const auto shift = baseval - val;

            for (auto j = previdx - 1; j >= newidx; --j) {
                const auto mirrorj = diffsums.size() / 2 + (diffsums.size() / 2 - j - 1);
                auto newval = diffsums[j] + shift;
                diffsums[j] = newval;
                newval = diffsums[mirrorj] + shift;
                diffsums[mirrorj] = newval;
            }
            previdx = newidx;
        }
    };

    vector<pair<size_t,int64_t>> downshifts;
    vector<pair<size_t,int64_t>> upshifts;
    int64_t smallestdiff = INT64_MAX;
    int64_t biggestdiff = INT64_MIN;

    for (size_t i = 0; i < diffsums.size(); ++i) {
        const auto diffsum = diffsums[i];
        int64_t newsmallestdiff = min(diffsum, smallestdiff);
        if (smallestdiff != newsmallestdiff) {
            if (i < diffsums.size() / 2) {
                downshifts.push_back(pair(i, newsmallestdiff));
            }
            smallestdiff = newsmallestdiff;
        }
        int64_t newbiggestdiff = max(diffsum, biggestdiff);
        if (biggestdiff != newbiggestdiff) {
            if (i < diffsums.size() / 2) {
                upshifts.push_back(pair(i, newbiggestdiff));
            }
            biggestdiff = newbiggestdiff;
        }
    }

    TRACE1("DIFFSUMS %lu DOWNSHIFTS %lu UPSHIFTS %lu\n", diffsums.size(), downshifts.size(), upshifts.size());

    shift(downshifts, diffsums);
    shift(upshifts, diffsums);
};

vector<int64_t> calc_diffsums(const vector<int8_t> &buf, const size_t begin, const unsigned int SAMPLES_PER_SEC, const int64_t basesum, const bool flatten) {
    vector<int64_t> diffsums(buf.size() - begin);

    int64_t smallestdiff = INT64_MAX;
    int64_t biggestdiff = INT64_MIN;
    int64_t bottom = basesum;
    int64_t top = basesum;

    for (auto i = begin; i < buf.size(); ++i) {
        auto val = buf[i];
        bottom -= val;
        val = buf[buf.size() - (i + 1) + begin];
        top -= val;
        int64_t diffsum = top - bottom;
        diffsums[i - begin] = diffsum;
        
        if (diffsum == 0) {
            TRACE2("(%lu) diffsum %ld smallestdiff %ld, biggestdiff %ld\n", i/SAMPLES_PER_SEC, diffsum, smallestdiff, biggestdiff);
        }
        if (i % SAMPLES_PER_SEC == 0) {
            TRACE2("(%lu) diffsum %ld smallestdiff %ld, biggestdiff %ld\n", i/SAMPLES_PER_SEC, diffsum, smallestdiff, biggestdiff);
        }
        
        smallestdiff = min(diffsum, smallestdiff);
        biggestdiff = max(diffsum, biggestdiff);
    }

    if (flatten) {
        flatten_diffsums(diffsums);
        flatten_diffsums(diffsums);
        flatten_diffsums(diffsums);
        flatten_diffsums(diffsums);
        for (size_t i = 0; i < diffsums.size(); ++i) {
            const auto diffsum = diffsums[i];
            if (diffsum == 0) {
                TRACE2("FLATTENED (%zu) diffsum %ld smallestdiff %ld, biggestdiff %ld\n", i/SAMPLES_PER_SEC + 1200, diffsum, smallestdiff, biggestdiff);
            }
            if (i % SAMPLES_PER_SEC == 0) {
                TRACE2("FLATTENED (%zu) diffsum %ld smallestdiff %ld, biggestdiff %ld\n", i/SAMPLES_PER_SEC + 1200, diffsum, smallestdiff, biggestdiff);
            }
        }
    }
    
    return diffsums;
}

pair<size_t, int> get_looplen(const vector<int8_t> &buf, const size_t begin, const unsigned int SAMPLES_PER_SEC, const bool flatten, const bool strict) {
    assert(buf.size() < INT32_MAX);

    constexpr unsigned int ACR_PER_SEC = 50;
    TRACE1("SAMPLES_PER_SEC %d ACR_PER_SEC %d\n", SAMPLES_PER_SEC, ACR_PER_SEC);

    int64_t basesum = 0;
    for (auto i = begin; i < buf.size(); ++i) {
        basesum += buf[i];
    }
    TRACE1("basesum %ld buf size %lu avg %ld\n", basesum, buf.size(), basesum / (int64_t)(buf.size() - begin));

    const auto diffsums = calc_diffsums(buf, begin, SAMPLES_PER_SEC, basesum, flatten);

    int64_t smallestdiff = INT64_MAX;
    int64_t biggestdiff = INT64_MIN;

    vector<int64_t> tmpbuf;
    vector<int64_t> tmpbuf2;
    int64_t diffsumavg = 0;

    for (size_t i = 0; i < diffsums.size(); ++i) {
        int64_t diffsum = diffsums[i];
        diffsumavg += diffsum;
        tmpbuf.push_back(diffsum);
        
        if (diffsum == 0) {
            TRACE2("(%zu) diffsum %ld smallestdiff %ld, biggestdiff %ld\n", i/SAMPLES_PER_SEC, diffsum, smallestdiff, biggestdiff);
        }
        
        smallestdiff = min(diffsum, smallestdiff);
        biggestdiff = max(diffsum, biggestdiff);
        
        if (i % SAMPLES_PER_SEC == 0) {
            TRACE2("(%zu) diffsum %ld smallestdiff %ld, biggestdiff %ld\n", i/SAMPLES_PER_SEC, diffsum, smallestdiff, biggestdiff);
        }
        
        // smooth / sample 50Hz
        if (tmpbuf.size() == (SAMPLES_PER_SEC / ACR_PER_SEC)) {
            int64_t avg = 0;
            for (size_t j = 0; j < SAMPLES_PER_SEC / ACR_PER_SEC; ++j) {
                avg += tmpbuf[j];
            }
            avg = avg * ACR_PER_SEC / SAMPLES_PER_SEC;
            tmpbuf2.push_back(avg);
            tmpbuf.clear();
        }
    }
    int64_t avg1 = diffsumavg / (int64_t)(buf.size() - begin);
    int64_t avg2 = (biggestdiff + smallestdiff) / 2;
    diffsumavg = (avg1 + avg2) / 2;
    TRACE1("AVG %ld MAX %ld MIN %ld AVG1 %ld AVG2 %ld buf %lu\n", diffsumavg, biggestdiff, smallestdiff, avg1, avg2, tmpbuf2.size());
    int64_t prevdiffsum = INT64_MAX;
    vector<int> acrplus;
    vector<int> acrminus;
    bool minus = false;
    bool plus = false;
    for (size_t i = 0; i < tmpbuf2.size(); ++i) {
        auto diffsum = tmpbuf2[i];
        if (prevdiffsum != INT64_MAX) {
            if (diffsum >= diffsumavg && prevdiffsum < diffsumavg && minus) {
                TRACE2("(%lu) ACR + diffsum %ld\n", begin / SAMPLES_PER_SEC + i/ACR_PER_SEC, diffsum);
                acrplus.push_back(i);
                minus = false;
            } else if (diffsum < diffsumavg && prevdiffsum >= diffsumavg && plus) {
                TRACE2("(%lu) ACR - diffsum %ld\n", begin / SAMPLES_PER_SEC + i/ACR_PER_SEC, diffsum); 
                acrminus.push_back(i);
                plus = false;
            }
        }
        if (diffsum < (smallestdiff * 3 + diffsumavg)/4) {
            minus = true;
        }
        if (diffsum > (biggestdiff * 3 + diffsumavg)/4) {
            plus = true;
        }
        prevdiffsum = diffsum;
    }

    vector<int> pvals;
    for (size_t i = 1; i < acrplus.size(); ++i) {
        const auto val = acrplus[i] - acrplus[i - 1];
        pvals.push_back(val);
    }
    sort(pvals.begin(), pvals.end());

    int64_t pacrsum = 0;
    int pacrcnt = 0;
    for (size_t i = pvals.size() / 3; i < pvals.size() * 2 / 3; ++i) {
        pacrsum += pvals[i];
        pacrcnt++;
    }
    double pacravg = pacrcnt > 0 ? pacrsum / pacrcnt : 0;

    TRACE2("PACRCNT %d PACRSUM %ld PACRAVG %f PACRSIZE %lu\n", pacrcnt, pacrsum, pacravg/ACR_PER_SEC, acrplus.size());

    vector<int> nvals;
    for (size_t i = 1; i < acrminus.size(); ++i) {
        const auto val = acrminus[i] - acrminus[i - 1];
        nvals.push_back(val);
    }
    sort(nvals.begin(), nvals.end());

    int64_t nacrsum = 0;
    int nacrcnt = 0;
    for (size_t i = nvals.size() / 3; i < nvals.size() * 2 / 3; ++i) {
        nacrsum += nvals[i];
        nacrcnt++;
    }
    double nacravg = nacrcnt > 0 ? nacrsum / nacrcnt : 0;

    TRACE2("NACRCNT %d NACRSUM %ld NACRAVG %f NACRSIZE %lu\n", nacrcnt, nacrsum, nacravg/ACR_PER_SEC, acrminus.size());

    int mincnt = strict ? 2 : 1;
    int pok = nacrcnt >= mincnt && (pacrcnt > mincnt || pacrcnt * 2 == nacrcnt);
    int nok = pacrcnt >= mincnt && (nacrcnt > mincnt || nacrcnt * 2 == pacrcnt);
    double looplen = 0;
    
    if (pok == nok) {
        if (pok && pacravg / ACR_PER_SEC >= 5 && nacravg >= pacravg * 3 && pacrcnt >= nacrcnt * 3) {
            looplen = pacravg;
            nok = false;
        } else if (nok && nacravg / ACR_PER_SEC >= 5 && pacravg >= nacravg * 3 && nacrcnt >= pacrcnt * 3) {
            looplen = nacravg;
            pok = false;
        } else {
            looplen = max(pacravg, nacravg);
        }
    } else if (pok) {
        looplen = pacravg;
    } else if (nok) {
        looplen = nacravg;
    }

    if (looplen * SAMPLES_PER_SEC / ACR_PER_SEC >= buf.size() / 3) {
        TRACE1("INVALID LOOPLEN %f\n", looplen);
        looplen = 0;
    }

    if ((biggestdiff < UCHAR_MAX && smallestdiff > -UCHAR_MAX) || (biggestdiff - smallestdiff) < UCHAR_MAX) {
        TRACE1("INVALID SMALLEST/BIGGESTDIFF smallestdiff %ld biggestdiff %ld\n", smallestdiff, biggestdiff);
        looplen = 0;
    }

    int ok = pok || nok;
    ok += pok && nok && pacrcnt > 0 && nacrcnt > 0;
    ok += pacrcnt > 0 && nacrcnt > 0 && abs(pacravg / ACR_PER_SEC - nacravg / ACR_PER_SEC) <= 1;
    ok += pacrcnt > 0 && nacrcnt > 0 && abs(2 * pacravg / ACR_PER_SEC - nacravg / ACR_PER_SEC) <= 1;
    ok += pacrcnt > 0 && nacrcnt > 0 && abs(pacravg / ACR_PER_SEC - 2 * nacravg / ACR_PER_SEC) <= 1;
    ok += pacrcnt > 0 && nacrcnt > 0 && abs(3 * pacravg / ACR_PER_SEC - nacravg / ACR_PER_SEC) <= 1;
    ok += pacrcnt > 0 && nacrcnt > 0 && abs(pacravg / ACR_PER_SEC - 3 * nacravg / ACR_PER_SEC) <= 1;
    ok += pacrcnt > 0 && nacrcnt > 0 && abs(4 * pacravg / ACR_PER_SEC - nacravg / ACR_PER_SEC) <= 1;
    ok += pacrcnt > 0 && nacrcnt > 0 && abs(pacravg / ACR_PER_SEC - 4 * nacravg / ACR_PER_SEC) <= 1;

    ok = looplen > 0 ? ok : 0;

    if (strict && ok > 0) {
        ok = looplen > ACR_PER_SEC ? ok : 0;
        ok = looplen > ACR_PER_SEC * REPEAT_THRESHOLD / 1000 ? ok : ok - 1;
    }

    ok = max(ok, 0);

    TRACE1("LOOP LEN %f POS %f NEG %f pok %d nok %d ok %d\n", looplen / ACR_PER_SEC, pacravg / ACR_PER_SEC, nacravg / ACR_PER_SEC, pok, nok, ok);
    
    return pair(round(looplen * SAMPLES_PER_SEC / ACR_PER_SEC), ok);
}

size_t get_loopstart(const vector<int8_t> &buf, const size_t SAMPLES_PER_SEC, const size_t looplen, const size_t offs) {
    assert(buf.size() < INT32_MAX);

    const auto _loopstart = [&](const size_t offs, const size_t window, const bool strict) -> size_t {
        int64_t basesum0 = 0;
        int64_t basesum1 = 0;
        for (auto i = offs; i < offs + window; ++i) {
            basesum0 += buf[i];
            basesum1 += buf[i + looplen];
        }

        TRACE1("offs %lu window %lu looplen %lu basesum0 %ld basesum1 %ld\n", offs, window, looplen, basesum0, basesum1);

        int64_t smallestdiff = INT64_MAX;
        int64_t biggestdiff = INT64_MIN;
        int64_t bottom = basesum0;
        int64_t top = basesum1;
        int64_t prevdiffsum = INT64_MAX;

        int mcnt = 0;
        int pcnt = 0;

        size_t loopstart = SIZE_MAX;
        size_t earliest = SIZE_MAX;

        size_t exactcnt = 0;
        size_t incstop = SIZE_MAX;
        size_t decstop = SIZE_MAX;
        int64_t smoothed = 0;
        int64_t prevsmoothed = INT64_MAX;

        for (size_t i = offs; i < offs + window; ++i) {
            const auto val0 = buf[i];
            const auto val1 = buf[i + window];
            const auto val2 = buf[i + looplen];
            const auto val3 = buf[i + window + looplen];

            if (val0 == val1 && val0 == val2 && val0 == val3) {
                exactcnt++;
            }

            bottom -= val0;
            bottom += val1;
            top -= val2;
            top += val3;
            const auto diffsum = top - bottom;
            
            if (diffsum == 0 && smallestdiff != 0 && biggestdiff != 0) {
                TRACE2("(%lu) diffsum %ld smallestdiff %ld, biggestdiff %ld\n", i/SAMPLES_PER_SEC, diffsum, smallestdiff, biggestdiff);
            }
            
            smallestdiff = min(diffsum, smallestdiff);
            biggestdiff = max(diffsum, biggestdiff);
            
            if (i % SAMPLES_PER_SEC == 0) {
                TRACE2("(%lu) diffsum %ld smallestdiff %ld, biggestdiff %ld\n", i/SAMPLES_PER_SEC, diffsum, smallestdiff, biggestdiff);
            }
            
            if (prevdiffsum != INT64_MAX && prevdiffsum < 0 && diffsum >= 0) {
                TRACE2("(%lu) diffsum+ %ld smallestdiff %ld, biggestdiff %ld\n", i/SAMPLES_PER_SEC, diffsum, smallestdiff, biggestdiff);
                pcnt++;
                earliest = min(earliest, i);
            } else if (prevdiffsum != INT64_MAX && prevdiffsum >= 0 && diffsum < 0) {
                TRACE2("(%lu) diffsum- %ld smallestdiff %ld, biggestdiff %ld\n", i/SAMPLES_PER_SEC, diffsum, smallestdiff, biggestdiff);
                mcnt++;
                earliest = min(earliest, i);
            }
            if (pcnt >= 2 || mcnt >= 2) {
                loopstart = min(i, loopstart);
            }
            
            smoothed += diffsum;
            prevdiffsum = diffsum;

            if (i > 0 && i % (SAMPLES_PER_SEC * 2) == 0) {
                const auto diffsum = smoothed / (SAMPLES_PER_SEC * 2);
                if (prevsmoothed != INT64_MAX && diffsum > prevsmoothed) {
                    //TRACE2("INC %zu diffsum %ld prev %ld\n", i / SAMPLES_PER_SEC, diffsum, prevsmoothed);
                    decstop = min(i, decstop);
                }
                if (prevsmoothed != INT64_MAX && diffsum < prevsmoothed) {
                    //TRACE2("DEC %zu diffsum %ld prev %ld\n", i / SAMPLES_PER_SEC, diffsum, prevsmoothed);
                    incstop = min(i, incstop);
                }
                prevsmoothed = diffsum;
                smoothed = 0;
            }
        }

        TRACE1("loopstart (%lu/%lu) %lu earliest %lu incstop %zu decstop %zu exactcnt %zu\n", offs / SAMPLES_PER_SEC, window / SAMPLES_PER_SEC, loopstart / SAMPLES_PER_SEC, earliest / SAMPLES_PER_SEC, incstop / SAMPLES_PER_SEC, decstop / SAMPLES_PER_SEC, exactcnt);
        if (loopstart < SIZE_MAX) {
            return loopstart;
        }
        if (earliest < SIZE_MAX) {
            return earliest;
        }
        if (!strict && incstop < SIZE_MAX && decstop < SIZE_MAX) {
            TRACE2("INCSTOP %zu DECSTOP %zu\n", incstop/SAMPLES_PER_SEC, decstop/SAMPLES_PER_SEC);
            return max(incstop, decstop);
        }
        // sometimes sample data and loop len match is almost "bit perfect", assume looping from start
        if (exactcnt > window * 99 / 100) {
            TRACE2("EXACTCNT %zu WINDOW %zu\n", exactcnt, window);
            return 0;
        }
        return SIZE_MAX;
    };

    auto len = max(looplen, 60 * SAMPLES_PER_SEC);
    auto maxlen = len;
    auto loopstart = _loopstart(offs, min(buf.size() / 4 - 1, maxlen), true);
    while (loopstart == SIZE_MAX && maxlen < buf.size() / 4 - 1) {
        maxlen += len;
        loopstart = _loopstart(offs, min(buf.size() / 4 - 1, maxlen), true);
    }
    if (loopstart == SIZE_MAX) {
        loopstart = _loopstart(offs, min(buf.size() / 4 - 1, len), false);
    }
    TRACE1("LOOP START %lu\n", loopstart / SAMPLES_PER_SEC);

    return loopstart;
}

size_t volume_detect(const vector<int8_t> &buf, size_t maxcnt, int threshold) {
    bool seenhigher = false;
    size_t volumecnt = 0;
    for (size_t i = 0; i < buf.size(); ++i) {
        const int val = buf[i];
        if (abs(val) <= threshold) {
            volumecnt++;
            if (volumecnt > maxcnt && seenhigher) {
                return i - volumecnt + 1;
            }
        } else {
            volumecnt = 0;
            seenhigher = true;
        }
    }
    return 0;
}

size_t volume_trim(const vector<int8_t> &buf, int threshold, size_t offs) {
    ssize_t i = offs - 1;
    while(i >= 0 && abs(buf[i]) <= threshold) i--;
    return i >= 0 ? offs - (i+1) : offs;
}

} // namespace {}

namespace songend::detector {

// does some smoothing for input audio and converts to 8-bit/single channel
void SongEndDetector::update(const char *bytes, const size_t nbytes) {
    const auto idx = [&](const int i) constexpr {
        const int newi = (itmp + i) % 8;
        return newi < 0 ? 8 + newi : newi;
    };
    const auto b = [&](const int i) constexpr -> int {
        return tmp[idx(i-8)];
    };

    int8_t buftmp[nbytes / 8 + 1];
    int n = 0;

    for (size_t i = 0; i < nbytes; i+=4) {
        const int b0 = (endian == endian::little) ? (int8_t)bytes[i] : (int8_t)bytes[i+1];
        const int b1 = (endian == endian::little) ? (int8_t)bytes[i+1] : (int8_t)bytes[i];
        int val = b1 * 256 + b0;

        if (stereo) {
            const int b2 = (endian == endian::little) ? (int8_t)bytes[i+2] : (int8_t)bytes[i+3];
            const int b3 = (endian == endian::little) ? (int8_t)bytes[i+3] : (int8_t)bytes[i+2];
            val = (val + (b3 * 256 + b2)) / 2;
        }

        tmp[itmp] = val;
        itmp = idx(+1);
        ctmp++;
        if (ctmp == 8) {
            const int tmp = b(2) + b(3) + b(4) + b(5);
            const int val0 = (b(0) + b(1) + tmp) / (6*256);
            buftmp[n++] = val0;
            itmp = idx(+2);
            const int val1 = (tmp + b(4) + b(5)) / (6*256);
            itmp = idx(-2);
            buftmp[n++] = val1;
            ctmp -= 4;

            maxi = max(maxi, val0);
            maxi = max(maxi, val1);
            mini = min(mini, val0);
            mini = min(mini, val1);
        }
    }
    buf.insert(buf.end(), buftmp, buftmp+n);
}

int SongEndDetector::detect_loop() {
    const auto SAMPLES_PER_SEC = rate / 2;
    vector<int8_t> buf0(buf.size());

    TRACE2("MAXI %d MINI %d\n", maxi, mini);
    const int maximini = maxi - mini;
    for (size_t i = 0; i < buf.size(); ++i) {
        const int val = buf[i];
        const auto val0 = -16 + (val - mini) * 32 / maximini;
        if (val0 < 0) {
            buf0[i] = -(val0 * val0) / 2;
        } else {
            buf0[i] = (val0 * val0) / 2 - 1;
        }
    }

    vector<int8_t>& bufz = buf;

    const auto _looplen = [&](const size_t begin, const bool strict) -> size_t {
        pair<size_t, int> looplen0 = pair(0,0);
        pair<size_t, int> looplen1 = pair(0,0);
        pair<size_t, int> looplen2 = pair(0,0);
        pair<size_t, int> looplen3 = pair(0,0);
        pair<size_t, int> looplen = pair(0,0);
        const auto check = [&](pair<size_t, int> len) -> bool {
            if (len.second > looplen.second) {
                looplen = len;
                return true;
            } else if (len.second > 1 && len.second == looplen.second && len.first > looplen.first) {
                looplen = len;
                return true;
            } else if (strict && len.second > 0 && len.second == looplen.second && looplen.first > 0 && looplen.first < REPEAT_THRESHOLD * SAMPLES_PER_SEC / 500 && len.first > 3 * looplen.first) {
                looplen = len;
                return true;
            } else if (!strict && len.second > 0 && len.second == looplen.second && len.first < looplen.first) {
                looplen = len;
                return true;
            }
            return false;
        };
        if (check(looplen0 = get_looplen(buf, begin, SAMPLES_PER_SEC, false, strict))) bufz = buf;
        if (check(looplen1 = get_looplen(buf, begin, SAMPLES_PER_SEC, true, strict))) bufz = buf;
        if (looplen0.second < 3 || looplen.second < 3 || looplen.first < REPEAT_THRESHOLD * SAMPLES_PER_SEC / 1000) {
            if (check(looplen2 = get_looplen(buf0, begin, SAMPLES_PER_SEC, false, strict))) bufz = buf0;
            if (check(looplen3 = get_looplen(buf0, begin, SAMPLES_PER_SEC, true, strict))) bufz = buf0;
        }
        if (looplen0.second > 0 && abs((ssize_t)looplen.first * 2 - (ssize_t)looplen0.first) < SAMPLES_PER_SEC) {
            looplen = looplen0;
            bufz = buf;
        }
        return looplen.first;
    };

    auto looplen = _looplen(buf.size() / 3, true);
    if (looplen > 0 && looplen < (size_t)REPEAT_THRESHOLD * SAMPLES_PER_SEC / 1000) {
        looplen = max((size_t)REPEAT_THRESHOLD * SAMPLES_PER_SEC / 1000, looplen);
    }

    const auto _songlen0 = [&]() -> size_t {
        size_t songlen = 0;
        if (looplen < SIZE_MAX && looplen > 0) {
            size_t loopstart = get_loopstart(bufz, SAMPLES_PER_SEC, looplen, 0);
            if (loopstart >= 0 && loopstart < SIZE_MAX) {
                songlen = (loopstart + looplen) * 1000 / SAMPLES_PER_SEC + 1000;
                TRACE1("SONGLEN0 %zu loopstart %zu looplen %zu\n", songlen, loopstart / SAMPLES_PER_SEC, looplen / SAMPLES_PER_SEC);
            }
        }
        return songlen;
    };

    size_t songlen = _songlen0();

    const auto _songlen1 = [&](const size_t minoffs, const size_t maxoffs) {
        int8_t mini = CHAR_MAX;
        int8_t maxi = CHAR_MIN;
        for (auto i = maxoffs; i >= minoffs; --i) {
            mini = min(bufz[i], mini);
            maxi = max(bufz[i], maxi);
        }
        int newmin = 0;
        int newmax = 0;
        size_t lastmin = SIZE_MAX;
        size_t lastmax = SIZE_MAX;
        ssize_t i = 0;
        for (i = minoffs; i >= 0; --i) {
            const auto val = bufz[i];
            if (val < mini) {
                newmin++;
            } else if (val == mini) {
                lastmin = i;
            }
            if (val > maxi) {
                newmax++;
            } else if (val == maxi) {
                lastmax = i;
            }
            if (newmin > 12 && newmax > 12 && lastmin < SIZE_MAX && lastmax < SIZE_MAX) {
                const auto offs = min(lastmax, lastmin);
                if (offs < SIZE_MAX) {
                    const auto newlooplen = looplen < SIZE_MAX && looplen > 0 ? looplen : (size_t)REPEAT_THRESHOLD * SAMPLES_PER_SEC / 1000;
                    auto newloopstart = get_loopstart(bufz, SAMPLES_PER_SEC, newlooplen, offs);
                    if (newloopstart < SIZE_MAX) {
                        const size_t newsonglen = (newloopstart + newlooplen) * 1000ul / SAMPLES_PER_SEC + 1000;
                        TRACE1("SONGLEN1 %zu OLDSONGLEN %zu NEWLOOPLEN %zu NEWLOOPSTART %zu offs %zu mini %d maxi %d lastmin %zu lastmax %zu newmin %d newmax %d buf %d i %zu\n", newsonglen, songlen, newlooplen / SAMPLES_PER_SEC, newloopstart / SAMPLES_PER_SEC, offs / SAMPLES_PER_SEC, mini, maxi, lastmin / SAMPLES_PER_SEC, lastmax / SAMPLES_PER_SEC, newmin, newmax, val, i / SAMPLES_PER_SEC);
                        songlen = max(songlen, newsonglen);
                    } else {
                        TRACE1("SKIPLOOPSTART SONGLEN %zu offs %zu newlooplen %zu newmin %d lastmin %zu newmax %d lastmax %zu\n", songlen, offs / SAMPLES_PER_SEC, newlooplen / SAMPLES_PER_SEC, newmin, lastmin, newmax, lastmax);
                    }
                } else {
                    TRACE1("SKIPOFFS SONGLEN %zu newmin %d lastmin %zu newmax %d lastmax %zu\n", songlen, newmin, lastmin, newmax, lastmax);
                }
                break;
            }
        }
        if (i <= 0) {
            TRACE1("SKIPSONGLEN1\n");
        }
    };

    if (songlen && looplen <= REPEAT_THRESHOLD * SAMPLES_PER_SEC / 500) {
        _songlen1(bufz.size() / 3, bufz.size() - 1);
    }

    if (!songlen) {
        looplen = _looplen(60ul * SAMPLES_PER_SEC, false);
        songlen = _songlen0();
        TRACE1("SONGLEN2 %zu looplen %zu\n", songlen, looplen / SAMPLES_PER_SEC);
    }

    return songlen;
}

int SongEndDetector::detect_silence(int seconds) {
    TRACE2("MAXI %d MINI %d\n", maxi, mini);
    const ssize_t SAMPLES_PER_SEC = rate / 2;
    int songlen = volume_detect(buf, SAMPLES_PER_SEC * seconds, THRESHOLD_SILENCE) * 1000 / SAMPLES_PER_SEC;
    if (songlen) {
        TRACE1("SILENCE SONGLEN %d\n", songlen);
    }
    return songlen;
}

int SongEndDetector::detect_volume(int seconds) {
    TRACE2("MAXI %d MINI %d\n", maxi, mini);
    const ssize_t SAMPLES_PER_SEC = rate / 2;
    int songlen = volume_detect(buf, SAMPLES_PER_SEC * seconds, THRESHOLD_VOLUME) * 1000 / SAMPLES_PER_SEC;
    if (songlen) {
        TRACE1("VOLUME SONGLEN %d\n", songlen);
    }
    return songlen;
}

int SongEndDetector::detect_repeat() {
    TRACE2("MAXI %d MINI %d\n", maxi, mini);
    const auto SAMPLES_PER_SEC = rate / 2;
    const size_t WINDOW = REPEAT_THRESHOLD * SAMPLES_PER_SEC / 1000;
    size_t lastmin = SIZE_MAX;
    size_t lastmax = SIZE_MAX;
    size_t maxmin = 0;
    size_t maxmax = 0;
    int maxi = INT_MIN;
    int mini = INT_MAX;
    int64_t windowsum = 0;
    int64_t minwindowsum = INT64_MAX;
    int64_t maxwindowsum = INT64_MIN;
    for (ssize_t i = buf.size() - 1; i >= (ssize_t)buf.size() / 2; --i) {
        const int val0 = buf[i];
        const int val = val0 >= 0 ? sqrt(val0) : -sqrt(-val0);
        if (val > maxi) {
            lastmax = i;
            maxi = val;
        } else if (val == maxi) {
            maxmax = max(maxmax, lastmax - i);
            lastmax = i;
        }
        if (val < mini) {
            lastmin = i;
            mini = val;
        } else if (val == mini) {
            maxmin = max(maxmin, lastmin - i);
            lastmin = i;
        }
        if (i % WINDOW == 0) {
            minwindowsum = min(minwindowsum, windowsum);
            maxwindowsum = max(maxwindowsum, windowsum);
            windowsum = 0;
        } else {
            windowsum += val;
        }
    }

    TRACE2("MAXI %d MINI %d MINWINDOWSUM %ld MAXWINDOWSUM %ld\n", maxi, mini, minwindowsum, maxwindowsum);

    if (abs(maxwindowsum - minwindowsum) > abs(minwindowsum)/5) {
        return 0;
    }

    const auto threshold = REPEAT_THRESHOLD * SAMPLES_PER_SEC / 1000;

    TRACE2("REPEAT1 MAXMIN %zu MAXMAX %zu LASTMIN %zu LASTMAX %zu threshold %d\n", maxmin, maxmax, lastmin - buf.size() / 2, lastmax - buf.size() / 2, threshold);
 
    if (maxmin > 0 && maxmax > 0 && lastmin <= buf.size() / 2 + threshold && lastmax <= buf.size() / 2 + threshold && maxmin <= threshold && maxmax < threshold) {
        ssize_t i = 0;
        for (i = buf.size()/2 - 1; i >= 0; --i) {
            const int val0 = buf[i];
            const int val = val0 >= 0 ? sqrt(val0) : -sqrt(-val0);
            if (val > maxi) {
                break;
            } else if (val == maxi) {
                maxmax = max(maxmax, lastmax - i);
                lastmax = i;
            }
            if (val < mini) {
                break;
            } else if (val == mini) {
                maxmin = max(maxmin, lastmin - i);
                lastmin = i;
            }
        }

        TRACE2("REPEAT2 MAXMIN %zu MAXMAX %zu LASTMIN %zu LASTMAX %zu threshold %d\n", maxmin, maxmax, lastmin, lastmax, threshold);
 
        if (maxmin > 0 && maxmax > 0 && lastmin <= threshold && lastmax <= threshold && maxmin <= threshold && maxmax < threshold) {
            const auto looplen = max((size_t)REPEAT_THRESHOLD * SAMPLES_PER_SEC / 2000, max(maxmin,maxmax) * 2);
            const auto loopstart = max((size_t)REPEAT_THRESHOLD * SAMPLES_PER_SEC / 2000, get_loopstart(buf, SAMPLES_PER_SEC, looplen, 0));
            const auto repeat = (looplen + loopstart) * 1500 / SAMPLES_PER_SEC;
            TRACE1("REPEATa %lu looplen %zu loopstart %zu\n", repeat, looplen * 1000 / SAMPLES_PER_SEC, loopstart * 1000 / SAMPLES_PER_SEC);
            // XXX avoid some suspicious results / false positives
            if (repeat > 0) {
                if (repeat > 60000) return 0;
                if (loopstart * 1000 / SAMPLES_PER_SEC <= 3000) return repeat > 9000 ? 0 : repeat;
            }
            return repeat;
        } else if (maxmin > 0 && maxmax > 0 && maxmin <= threshold && maxmax < threshold) {
            const auto repeat = max(lastmin, lastmax) * 1000 / SAMPLES_PER_SEC + max((size_t)REPEAT_THRESHOLD, (max(maxmin, maxmax)) * 1500 / SAMPLES_PER_SEC);
            TRACE1("REPEATb %lu\n", repeat);
            return repeat;
        }
    }

    return 0;
}

int SongEndDetector::trim_silence(int offs_millis) {
    TRACE2("MAXI %d MINI %d\n", maxi, mini);
    const auto SAMPLES_PER_SEC = rate / 2;
    const auto MARGIN = SAMPLES_PER_SEC / 1000 - 1;
    const auto offs = (size_t)offs_millis * SAMPLES_PER_SEC / 1000;
    assert(offs <= buf.size() + MARGIN);
    const auto offs_fixed = offs >= buf.size() ? buf.size() : offs;
    int trimmed = volume_trim(buf, THRESHOLD_SILENCE, offs_fixed) * 1000 / SAMPLES_PER_SEC;
    if (trimmed)
        TRACE1("TRIMSILENCE %d offs_millis %d offs %zu offs_fixed %zu bufsize %zu \n", trimmed, offs_millis, offs, offs_fixed, buf.size());
    return abs(offs_millis - trimmed) <= MARGIN ? offs_millis : trimmed;
}

int SongEndDetector::trim_volume(int offs_millis) {
    TRACE2("MAXI %d MINI %d\n", maxi, mini);
    const auto SAMPLES_PER_SEC = rate / 2;
    const auto MARGIN = SAMPLES_PER_SEC / 1000 - 1;
    const auto offs = (size_t)offs_millis * SAMPLES_PER_SEC / 1000;
    assert(offs <= buf.size() + MARGIN);
    const auto offs_fixed = offs >= buf.size() ? buf.size() : offs;
    int trimmed = volume_trim(buf, THRESHOLD_VOLUME, offs_fixed) * 1000 / SAMPLES_PER_SEC;
    if (trimmed)
        TRACE1("TRIMVOLUME %d offs_millis %d offs %zu offs_fixed %zu bufsize %zu \n", trimmed, offs_millis, offs, offs_fixed, buf.size());
    return abs(offs_millis - trimmed) <= MARGIN ? offs_millis : trimmed;
}

} // namespace songend

#pragma GCC diagnostic pop
