#pragma once

#include <stdint.h>

void docmd1(void); // cmds done once (&0vol cutting) (8bb: tick=0 commands)
void docmd2(void); // cmds done when not next row (8bb: tick>0 commands)

void setspeed(uint8_t val);
void settempo(uint8_t bpm);
