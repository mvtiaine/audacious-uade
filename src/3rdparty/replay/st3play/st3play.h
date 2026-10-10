// SPDX-License-Identifier: BSD-3-Clause AND CC-PDM-1.0
// SPDX-AI-Disclosure: ai-assisted
#pragma once

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// for endianess check
#include "config.h"

#define AUDACIOUS_UADE 1

// Types and tables shared by the play and probe instances.
namespace replay::st3play {
#include "digdata.h"
#include "mixer/sinc.h"
}

namespace replay::st3play::play {
using namespace replay::st3play;
extern song_t song;
extern audio_t audio;
extern bool WAVRender_Flag;
extern bool renderToWavFlag; // defined by upstream's example main
#include "dig.h"
#include "digcmd.h"
#include "digread.h"
#include "dig_gus.h"
#include "digadl.h"
#include "digamg.h"
// the mixers have identically named file scope symbols, kept apart by sub namespaces
namespace sbpro {
#include "mixer/sbpro.h"
}
namespace gus {
#include "mixer/gus_gf1.h"
}
namespace opl2 {
#include "opl2/opl2.h"
}
using namespace sbpro;
using namespace gus;
using namespace opl2;
// noop audio output device impls
inline void lockMixer(void) {}
inline void unlockMixer(void) {}
inline bool openMixer(int32_t mixingFrequency, int32_t mixingBufferSize) { return true; }
inline void closeMixer(void) {}
} // namespace replay::st3play::play

#ifdef PLAYER_PROBE
namespace replay::st3play::probe {
using namespace replay::st3play;
extern song_t song;
extern audio_t audio;
extern bool WAVRender_Flag;
extern bool renderToWavFlag; // defined by upstream's example main
#include "dig.h"
#include "digcmd.h"
#include "digread.h"
#include "dig_gus.h"
#include "digadl.h"
#include "digamg.h"
namespace sbpro {
#include "mixer/sbpro.h"
}
namespace gus {
#include "mixer/gus_gf1.h"
}
namespace opl2 {
#include "opl2/opl2.h"
}
using namespace sbpro;
using namespace gus;
using namespace opl2;
// noop audio output device impls
inline void lockMixer(void) {}
inline void unlockMixer(void) {}
inline bool openMixer(int32_t mixingFrequency, int32_t mixingBufferSize) { return true; }
inline void closeMixer(void) {}
} // namespace replay::st3play::probe
#else
namespace replay::st3play { namespace probe = replay::st3play::play; }
#endif
