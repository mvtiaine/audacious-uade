// SPDX-License-Identifier: BSD-2-Clause
#pragma once

#include <climits>
#include <cmath>
#include <cstring>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
// XXX fix morphos compile
#undef shutdown
// for endianess check
#include "config.h"

#define AUDACIOUS_UADE 1
#define __STAND_ALONE__ 1
#define __WINAMP__ 1
#define __NO_MIDI__ 1
#ifdef WORDS_BIGENDIAN
#define __BIG_ENDIAN__ 1
#endif
#define FALSE 0
#define TRUE 1
#define LOAD_READ FALSE
#define LOAD_READMEM TRUE
#define STDCALL
#define PTKEXPORT
typedef unsigned int Uint32;
typedef unsigned char Uint8;
typedef int int32;
#ifndef _AIX
typedef char int8;
#endif
typedef unsigned char uchar;
typedef int64_t int64;
typedef uint64_t Uint64;

namespace replay::protrekkr2 {

#undef FILE
struct FILE {
    const char *data;
    const size_t size;
    size_t pos;
};

int Read_Data(void *value, int size, int amount, FILE *handle) noexcept;
inline int Get_File_Size(FILE *Handle) noexcept {
    return Handle->size;
}
inline void AUDIO_ResetTimer(void) noexcept {};
inline int AUDIO_GetSamples(void) noexcept { return 0; };

#include "release/distrib/replay/lib/include/ptk_def_properties.h"
#undef PTK_GSM
#undef PTK_AT3
#undef PTK_MP3
#include "release/distrib/replay/lib/include/cubic_spline.h"
#include "release/distrib/replay/lib/include/endianness.h"
#include "release/distrib/replay/lib/include/samples_unpack.h"
#include "release/distrib/replay/lib/include/spline.h"
#include "release/distrib/replay/lib/include/wavpack.h"
#include "3rdparty/miniz/miniz.h"

#define PROTREKKR_NS \
extern int Pattern_Line; \
extern int Pattern_Line_Visual; \
extern int Song_Position_Visual; \
extern int Song_Playing; \
extern char Channels_MultiNotes[MAX_TRACKS]; \
extern char Channels_Effects[MAX_TRACKS]; \
extern char Use_Cubic; \
extern gear303 tb303engine[2]; \
extern para303 tb303[2]; \
void Initreverb(void); \
void Allocate_Wave(int n_index, int split, long lenfir, \
                   int samplechans, int clear, \
                   short *Waveform1, short *Waveform2); \
void Read_Synth_Params(int (*Read_Function)(void *, int ,int, FILE *), \
                       int (*Read_Function_Swap)(void *, int ,int, FILE *), \
                       FILE *in, \
                       int idx, \
                       int read_disto, \
                       int read_lfo_adsr, \
                       int new_version, \
                       int Env_Modulation, \
                       int New_Env, \
                       int Ntk_Beta, \
                       int Combine, \
                       int Var_Disto); \
void Load_Reverb_Data(int (*Read_Function)(void *, int ,int, FILE *), \
                      int (*Read_Function_Swap)(void *, int ,int, FILE *), \
                      FILE *in, int New); \
void Load_Old_Reverb_Presets(int Type); \
void Load_303_Data(int (*Read_Function)(void *, int ,int, FILE *), \
                   int (*Read_Function_Swap)(void *, int ,int, FILE *), \
                   FILE *in, int unit, int pattern); \
void Reset_Song_Length(void); \
int Read_Data_Swap(void *value, int size, int amount, FILE *handle); \
int Fix_Codec(int Scheme); \
void Swap_Sample(short *buffer, int sample, int bank); \
void Free_Samples(void); \
Uint8 *Depack_Data(Uint8 *Memory, int Size, int Size_Out); \
int Load_Ptk(FILE *in); \
int Calc_Length(void); \
void Mixer(Uint8 *Buffer, Uint32 Len); \
inline void Set_Default_Channels_Polyphony(void) noexcept { \
    for (int i = 0; i < MAX_TRACKS; i++) \
        Channels_Polyphony[i] = DEFAULT_POLYPHONY; \
} \
void Init_Tracker_Context_After_ModLoad(void);

namespace play {
#include "release/distrib/replay/lib/include/synth.h"
#include "release/distrib/replay/lib/include/tb_303.h"
#include "release/distrib/replay/lib/include/ptkreplay.h"
#include "release/distrib/replay/lib/include/replay.h"
#include "src/editors/include/patterns_blocks.h"
PROTREKKR_NS
} // namespace play

#ifdef PLAYER_PROBE
namespace probe {
#include "release/distrib/replay/lib/include/synth.h"
#include "release/distrib/replay/lib/include/tb_303.h"
#include "release/distrib/replay/lib/include/ptkreplay.h"
#include "release/distrib/replay/lib/include/replay.h"
#include "src/editors/include/patterns_blocks.h"
PROTREKKR_NS
} // namespace probe
#else
namespace probe = play;
#endif
} // namespace replay::protrekkr2
