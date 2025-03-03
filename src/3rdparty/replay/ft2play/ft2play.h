// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <type_traits>
#include <utility>

// for endianess check
#include "config.h"

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

#ifdef PLAYER_PROBE
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
#else
namespace replay::ft2play { namespace probe = replay::ft2play::play; }
#endif
