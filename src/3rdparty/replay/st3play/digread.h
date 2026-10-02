// SPDX-License-Identifier: BSD-3-Clause
#ifndef AUDACIOUS_UADE
#pragma once

#include <stdint.h>
#include <stdbool.h>
#endif

void dorow(void); // 8bb: replayer ticker
int16_t neworder(void);
void donewnote(uint8_t channel, bool fromNoteDelayEfx);
