// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#ifndef AUDACIOUS_UADE
#include <stdint.h>
#include "../it_structs.h"
#endif

#define RAMPSPEED 8
#define RAMPCOMPENSATE 255

typedef void (*mixFunc)(slaveChn_t *sc, int32_t *mixBufPtr, int32_t numSamples);

extern const mixFunc WAVWriter_MixFunctionTables[4];
