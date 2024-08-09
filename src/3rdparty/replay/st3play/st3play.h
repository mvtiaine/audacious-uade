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
extern int16_t np_ord; \
extern bool np_restarted; \
bool st3play_PlaySong(const uint8_t *moduleData, uint32_t dataLength, bool useInterpolationFlag, uint32_t audioFreq); \
void st3play_Close(void); \
bool st3play_FillAudioBuffer(int16_t *buffer, int32_t samples); \
}

ST3PLAY(replay::st3play::play)
ST3PLAY(replay::st3play::probe)
