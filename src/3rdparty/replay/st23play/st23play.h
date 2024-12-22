// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
// for endianess check
#include "config.h"

#define AUDACIOUS_UADE 1

#define ST23PLAY(ns) \
namespace ns { \
extern bool restarted, moduleLoaded; \
extern uint8_t vpnt; \
extern int32_t oversamplingFactor; \
extern double *dMixBuffer; \
bool loadSTM(const uint8_t *dat, uint32_t modLen); \
bool st23play_PlaySong(const uint8_t *moduleData, uint32_t dataLength, uint32_t audioFreq); \
void st23play_Close(void); \
void st23play_FillAudioBuffer(int16_t *buffer, int32_t samples); \
void reset(); \
inline void clearMixBuffer() { \
    constexpr int MIX_BUF_SAMPLES = 4096; \
    if (dMixBuffer) memset(dMixBuffer, 0, MIX_BUF_SAMPLES * sizeof (double) * oversamplingFactor); \
} \
}

ST23PLAY(replay::st23play::play)
ST23PLAY(replay::st23play::probe)
