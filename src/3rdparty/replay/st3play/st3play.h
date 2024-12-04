#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
// for endianess check
#include "config.h"

#define AUDACIOUS_UADE 1
#define register // error: ISO C++17 does not allow 'register' storage class specifier [-Wregister]

#define ST3PLAY(ns) \
namespace ns { \
extern bool np_restarted, moduleLoaded; \
extern int8_t patterndelay, patloopcount; \
extern uint8_t order[256], chnsettings[32], *patdata[100]; \
extern int16_t np_ord, np_row, np_pat; \
extern uint16_t ordNum, insNum, patNum; \
extern uint16_t patDataLens[100]; \
extern int32_t *mixBufferL, *mixBufferR; \
bool loadS3M(const uint8_t *dat, uint32_t modLen); \
bool st3play_PlaySong(const uint8_t *moduleData, uint32_t dataLength, bool useInterpolationFlag, uint32_t audioFreq); \
void st3play_Close(void); \
bool st3play_FillAudioBuffer(int16_t *buffer, int32_t samples); \
void st3play_SetInterpolation(bool flag); \
void reset(); \
inline void setPos(int16_t pos) { \
    np_ord = pos; \
    np_pat = order[pos]; \
    np_row = 0; \
} \
inline void clearMixBuffer() { \
    constexpr int MIX_BUF_SAMPLES = 4096; \
    if (mixBufferL) memset(mixBufferL, 0, MIX_BUF_SAMPLES * sizeof (int32_t)); \
    if (mixBufferR) memset(mixBufferR, 0, MIX_BUF_SAMPLES * sizeof (int32_t)); \
} \
}

ST3PLAY(replay::st3play::play)
ST3PLAY(replay::st3play::probe)
