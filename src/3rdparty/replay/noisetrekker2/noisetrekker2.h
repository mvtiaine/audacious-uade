// SPDX-License-Identifier: LicenseRef-NoiseTrekker2
#pragma once

#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "common/logger.h"

#define AUDACIOUS_UADE 1
#define Screen void
#define _MAX_PATH 1024
#define PI M_PI
typedef int32_t __int32;
typedef uint32_t __uint32;
typedef int64_t __int64;
typedef uint64_t __uint64;

namespace replay::noisetrekker2 {
#undef FILE
struct FILE {
    const char *data;
    const size_t size;
    size_t pos;
};
#undef fread
size_t fread(void *ptr, size_t size, size_t n, FILE *stream);
#undef fclose
#define fclose(x) do {} while (0)

#define guiDial(x, y, sx, sy, str, brill) do {} while (0)
//#define mess_box(str) TRACE("noisetrekker2: " str "\n")
#define mess_box(str) do {} while (0)
#define draw_mained() do {} while (0)
#define Actualize_Main_Ed() do {} while (0)
#define Actualize_Master(gode) do {} while (0)
#define Actualize_Patterned() do {} while (0)
#define Actualize_Sequencer() do {} while (0)
#define MidiAllNotesOff() do {} while (0)
#define MidiReset() do {} while (0)

#define NOISETREKKER2_NS \
extern long SamplesPerTick; \
extern char Songtracks; \
extern int SamplesPerSec; \
extern unsigned char *RawPatterns; \
extern int left_value; \
extern int right_value; \
extern unsigned char cPosition; \
extern int ped_line; \
extern float mas_vol; \
int AllocPattern(void); \
bool LoadMod(FILE *in); \
void SongPlay(void); \
void SongStop(void); \
void FreeAll(void); \
void GetPlayerValues(float master_coef); \
void ResetFilters(char tr); \
void DoEffects(void); \
void Initreverb(); \
void Go303(void); \
void Fire303(unsigned char number,char unit); \
void noteoff303(char strack); \
void Sp_Playwave(int channel, float note, int sample,float vol, unsigned int offset, int glide); \
void Compressor_work(void); \
void ComputeCoefs(int freq, int r, int t); \
void live303(int pltr_eff_row,int pltr_dat_row); \
float Filter( float x, char i ); \
float Kutoff( int v); \
float Reonance( float v); \
float Bandwidth( int v); \
float ApplyLfo(float cy,char trcy); \
inline int f2i(double d); \
float int_filter2p(char ch,float input,float f,float q, float q2); \
float filter2p(char ch,float input,float f,float q); \
float filterhp(char ch,float input,float f,float q); \
float filterhp2(char ch,float input,float f,float q); \
float filter2px(char ch,float input,float f,float q); \
float filter2p24d(char ch,float input,float f,float q); \
float filterRingMod(char ch,float input,float f,float q); \
float filterRingModStereo(char ch,float input); \
float filterWater(char ch,float input,float f,float q); \
float filterWaterStereo(char ch,float input,float f,float q); \
float filterBellShaped(char ch,float input,float f,float q, float g); \
float filterDelta(char ch,float input,float f,float q); \
void allPassInit(float miliSecs); \
void ComputeStereo(char channel); \
void reset() noexcept; \

namespace play { NOISETREKKER2_NS }
namespace probe { NOISETREKKER2_NS }

} // namespace replay::noisetrekker2
