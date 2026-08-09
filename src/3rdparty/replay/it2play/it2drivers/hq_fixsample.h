// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#ifndef AUDACIOUS_UADE
#include <stdint.h>
#include "../it_structs.h"
#endif

void fixSamplesPingpong(sample_t *s, slaveChn_t *sc);
void unfixSamplesPingpong(sample_t *s, slaveChn_t *sc);
void fixSamplesFwdLoop(sample_t *s, slaveChn_t *sc);
void unfixSamplesFwdLoop(sample_t *s, slaveChn_t *sc);
void fixSamplesNoLoop(sample_t *s, slaveChn_t *sc);
