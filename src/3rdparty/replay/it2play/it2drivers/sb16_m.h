// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#ifndef AUDACIOUS_UADE
#include <stdint.h>
#include "../it_structs.h"
#endif

typedef void (*mixFunc)(slaveChn_t *sc, int32_t *mixBufPtr, int32_t numSamples);

extern const mixFunc SB16_MixFunctionTables[8];
