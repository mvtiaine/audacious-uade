#pragma once

#include <stdint.h>
#include "digdata.h"

void gcmd_inittables(void);
void gcmd_setvoices(uint8_t numVoices);
void gcmd_setstereo(void);
void gcmd_update(zchn_t *ch);
