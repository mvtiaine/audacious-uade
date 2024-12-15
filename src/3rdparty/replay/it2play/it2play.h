// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <cassert>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// for endianess check
#include "config.h"

#define AUDACIOUS_UADE 1

namespace replay::it2play {
#include "cpu.h"
#include "it_structs.h"
#include "it_tables.h"
#include "it2drivers/zerovol.h"
} // namespace replay::it2play

namespace replay::it2play::play {
using namespace replay::it2play;
extern hostChn_t hChn[MAX_HOST_CHANNELS];
extern slaveChn_t sChn[MAX_SLAVE_CHANNELS];
extern song_t Song;
extern driver_t Driver;
#include "it_d_rm.h"
#include "it_m_eff.h"
#include "it_music.h"
#include "loaders/it.h"
#include "loaders/s3m.h"
// add noop impls to fix compile (not used for anything)
inline void lockMixer(void) {}
inline void unlockMixer(void) {}
inline bool openMixer(int32_t mixingFrequency, int32_t mixingBufferSize) { return true; }
inline void closeMixer(void) {}
} // namespace replay::it2play::play

namespace replay::it2play::probe {
using namespace replay::it2play;
extern hostChn_t hChn[MAX_HOST_CHANNELS];
extern slaveChn_t sChn[MAX_SLAVE_CHANNELS];
extern song_t Song;
extern driver_t Driver;
#include "it_d_rm.h"
#include "it_m_eff.h"
#include "it_music.h"
#include "loaders/it.h"
#include "loaders/s3m.h"
// add noop impls to fix compile (not used for anything)
inline void lockMixer(void) {}
inline void unlockMixer(void) {}
inline bool openMixer(int32_t mixingFrequency, int32_t mixingBufferSize) { return true; }
inline void closeMixer(void) {}
} // namespace replay::it2play::probe
