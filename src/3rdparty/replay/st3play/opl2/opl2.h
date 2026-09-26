#pragma once

#include <stdint.h>

void OPL2_Init(int32_t audioOutputFrequency);
void OPL2_WritePort(uint16_t reg_num, uint8_t val);
void OPL2_RenderSamples(float *fMixBufL, float *fMixBufR, int32_t numSamples);
