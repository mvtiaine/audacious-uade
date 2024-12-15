// SPDX-License-Identifier: BSD-3-Clause
#ifndef AUDACIOUS_UADE
#pragma once

#include <stdint.h>
#include "../it_structs.h"
#endif

void UpdateNoLoop(slaveChn_t *sc, uint32_t numSamples);
void UpdateForwardsLoop(slaveChn_t *sc, uint32_t numSamples);
void UpdatePingPongLoop(slaveChn_t *sc, uint32_t numSamples);
void UpdateNoLoopHQ(slaveChn_t *sc, uint32_t numSamples);
void UpdateForwardsLoopHQ(slaveChn_t *sc, uint32_t numSamples);
void UpdatePingPongLoopHQ(slaveChn_t *sc, uint32_t numSamples);
