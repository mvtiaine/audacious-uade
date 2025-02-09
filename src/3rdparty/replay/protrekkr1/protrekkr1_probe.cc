// SPDX-License-Identifier: BSD-2-Clause
#include <algorithm>
#include <cassert>
#include <cstddef>

#include "protrekkr1.h"

using namespace std;

namespace replay::protrekkr1::probe {
char artist[20];
char style[20];
char SampleName[128][16][64];
char Midiprg[128];
char nameins[128][20];
int CHAN_MIDI_PRG[MAX_TRACKS];
char CHAN_HISTORY_STATE[256][16];
// sound driver
int AUDIO_Latency = 0;
int AUDIO_Play_Flag = 0;
int done = 0;
#include "src/editors/patterns_blocks.cpp"
#include "src/files/files.cpp"
#include "release/distrib/replay/lib/replay.cpp"
#include "release/distrib/replay/lib/synth.cpp"
#include "release/distrib/replay/lib/tb_303.cpp"
}
