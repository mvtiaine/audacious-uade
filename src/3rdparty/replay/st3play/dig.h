#pragma once

//#define FORCE_SOUNDCARD_TYPE SOUNDCARD_SBPRO
//#define FORCE_SOUNDCARD_TYPE SOUNDCARD_GUS

#include <stdint.h>
#include <stdbool.h>
#include "digdata.h"

// AUDIO DRIVERS
#if defined AUDIODRIVER_SDL
#include "audiodrivers/sdl/sdldriver.h"
#elif defined AUDIODRIVER_WINMM
#include "audiodrivers/winmm/winmm.h"
#else
// Read "audiodrivers/how_to_write_drivers.txt"
#endif

void setglobalvol(int8_t vol);
uint16_t roundspd(zchn_t *ch, uint16_t spd); // 8bb: for Gxx with semitones-slide enabled
uint16_t scalec2spd(zchn_t *ch, uint16_t spd);
void setspd(zchn_t *ch);
void setvol(zchn_t *ch);
uint16_t stnote2herz(uint8_t note);
void updateregs(void); // adlib/gravis
void shutupsounds(void);
void zgotosong(int16_t order, int16_t row);
bool zplaysong(int16_t order);
void musmixer(int16_t *buffer, int32_t samples);

// 8bb: my own custom routines

#ifndef PI
#define PI 3.14159265358979323846264338327950288
#endif

#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

extern bool WAVRender_Flag;
extern bool renderToWavFlag;

void closeMusic(void);
bool initMusic(int32_t audioFrequency, int32_t audioBufferSize);
void togglePause(void);
int32_t activePCMVoices(void);
int32_t activeAdLibVoices(void);
void resetAudioDither(void);
bool Dig_RenderToWAV(uint32_t audioRate, uint32_t bufferSize, const char *filenameOut);

// load.c
bool load_st3_from_ram(const uint8_t *data, uint32_t dataLength, int32_t soundCardType);
bool load_st3(const char *fileName, int32_t soundCardType);
// -------------
