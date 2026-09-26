#pragma once

#include <stdint.h>
#include <stdbool.h>

void dorow(void); // 8bb: replayer ticker
int16_t neworder(void);
void donewnote(uint8_t channel, bool fromNoteDelayEfx);
