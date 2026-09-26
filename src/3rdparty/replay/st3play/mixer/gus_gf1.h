#pragma once

#include <stdint.h>
#include <stdbool.h>

// these are NOT thread-safe and must only be called from the thread that calls GUS_Mix()!
void GUS_VoiceSelect(int32_t voiceNum);
void GUS_SetFrequency(uint16_t freq); // 6.10fp
void GUS_SetCurrVolume(uint16_t volume); // 12.4fp
void GUS_SetStartVolume(uint8_t volume);
void GUS_SetEndVolume(uint8_t volume);
void GUS_SetVolumeRate(uint8_t rate);
void GUS_SetBalance(uint8_t balance); // 0..15
const int8_t *GUS_GetCurrAddress(void);
void GUS_SetCurrAddress(const int8_t *address);
void GUS_SetStartAddress(const int8_t *address); // loop start
void GUS_SetEndAddress(const int8_t *address);
void GUS_SetVolumeCtrl(uint8_t flags);
void GUS_SetVoiceCtrl(uint8_t flags);
uint8_t GUS_GetVoiceCtrl(void);
// --------------------------------------------

void GUS_Init(int32_t audioOutputFrequency, int32_t numVoices);
double GUS_GetOutputRate(void);
int32_t GUS_GetNumberOfVoices(void);
int32_t GUS_GetNumberOfRunningVoices(void);
void GUS_RenderSamples(float *fMixBufL, float *fMixBufR, int32_t numSamples);

