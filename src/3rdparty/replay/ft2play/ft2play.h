#pragma once

#include <cassert>
#include <cstdint>

#define AUDACIOUS_UADE 1

namespace replay::ft2play {
#include "tables.h"
}
namespace replay::ft2play::play {
#include "pmplay.h"
#include "pmp_mix.h"
#include "snd_masm.h"
// add noop impls to fix compile (not used for anything)
inline void lockMixer(void) {}
inline void unlockMixer(void) {}
inline bool openMixer(int32_t mixingFrequency, int32_t mixingBufferSize) { assert(false); return false; }
inline void closeMixer(void) {}
}
namespace replay::ft2play::probe {
#include "pmplay.h"
#include "pmp_mix.h"
#include "snd_masm.h"
// add noop impls to fix compile (not used for anything)
inline void lockMixer(void) {}
inline void unlockMixer(void) {}
inline bool openMixer(int32_t mixingFrequency, int32_t mixingBufferSize) { assert(false); return false; }
inline void closeMixer(void) {}
}
