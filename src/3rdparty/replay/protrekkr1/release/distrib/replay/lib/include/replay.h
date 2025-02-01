// ------------------------------------------------------
// Protrekkr
// Based on Juan Antonio Arguelles Rius's NoiseTrekker.
//
// Copyright (C) 2008-2010 Franck Charlet.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL FRANCK CHARLET OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.
// ------------------------------------------------------

#ifndef _REPLAY_H_
#define _REPLAY_H_

// ------------------------------------------------------
// Includes
#include "tb_303.h"
#include "cubic_spline.h"
#include "spline.h"
#if defined(__WIN32__)
#include "../sounddriver/include/sounddriver_windows.h"
#elif defined(__LINUX__)
#include "../sounddriver/include/sounddriver_linux.h"
#elif defined(__MACOSX__)
#include "../sounddriver/include/sounddriver_macosx.h"
#elif defined(__AROS__)
#include "../sounddriver/include/sounddriver_aros.h"
#elif defined(__AMIGAOS4__)
#include "../sounddriver/include/sounddriver_aos4.h"
#elif defined(__PSP__)
#include "../sounddriver/include/sounddriver_psp.h"
#else
#error "Can't work without any sound driver !"
#endif
#include "samples_unpack.h"
#include "ptkreplay.h"
#include "synth.h"

// ------------------------------------------------------
// Constants
#define MAX_TRACKS 16
#define MAX_POLYPHONY 16
#define DEFAULT_POLYPHONY 1
#define MAX_FILTER 23
#define MAX_COMB_FILTERS 10
#define PI 3.1415926535897932384626433832795

#define MIX_RATE 44100
#define fMIX_RATE 44100.0f

#define CUBIC_INT 1
#define SPLINE_INT 2

#define DEFAULT_BASE_NOTE 48

#define SMP_PACK_GSM 0
#define SMP_PACK_MP3 1
#define SMP_PACK_TRUESPEECH 2
#define SMP_PACK_NONE 3
#define SMP_PACK_AT3 4
#define SMP_PACK_ADPCM 5
#define SMP_PACK_8BIT 6

#define MAX_ROWS 128
#define MAX_SEQUENCES 256
#define MAX_INSTRS 128
#define MAX_PATTERNS 128
#define MAX_INSTRS_SPLITS 16
#define DEFAULT_PATTERN_LEN 64

#define NOTE_MAX 119
#define NOTE_OFF 120
#define NO_NOTE 121
#define NO_INSTR 255

#define MAX_FX 2

#define PATTERN_NOTE1 0
#define PATTERN_INSTR1 1
#define PATTERN_NOTE2 2
#define PATTERN_INSTR2 3
#define PATTERN_NOTE3 4
#define PATTERN_INSTR3 5
#define PATTERN_NOTE4 6
#define PATTERN_INSTR4 7
#define PATTERN_NOTE5 8
#define PATTERN_INSTR5 9
#define PATTERN_NOTE6 10
#define PATTERN_INSTR6 11
#define PATTERN_NOTE7 12
#define PATTERN_INSTR7 13
#define PATTERN_NOTE8 14
#define PATTERN_INSTR8 15
#define PATTERN_NOTE9 16
#define PATTERN_INSTR9 17
#define PATTERN_NOTE10 18
#define PATTERN_INSTR10 19
#define PATTERN_NOTE11 20
#define PATTERN_INSTR11 21
#define PATTERN_NOTE12 22
#define PATTERN_INSTR12 23
#define PATTERN_NOTE13 24
#define PATTERN_INSTR13 25
#define PATTERN_NOTE14 26
#define PATTERN_INSTR14 27
#define PATTERN_NOTE15 28
#define PATTERN_INSTR15 29
#define PATTERN_NOTE16 30
#define PATTERN_INSTR16 31

#define PATTERN_VOLUME 32
#define PATTERN_PANNING 33
#define PATTERN_FX 34
#define PATTERN_FXDATA 35

#define PATTERN_FX2 36
#define PATTERN_FXDATA2 37

//#define PATTERN_NIBBLES 11
#define PATTERN_BYTES (PATTERN_FXDATA2 + 1)
#define PATTERN_ROW_LEN MAX_TRACKS * PATTERN_BYTES
#define PATTERN_TRACK_LEN MAX_ROWS * PATTERN_BYTES
#define PATTERN_LEN PATTERN_ROW_LEN * MAX_ROWS
#define PATTERN_POOL_SIZE PATTERN_LEN * MAX_PATTERNS

#define FLANGER_LOPASS_CUTOFF 0.1f
#define FLANGER_LOPASS_RESONANCE 0.4f

#define SMP_LOOP_NONE 0
#define SMP_LOOP_FORWARD 1
#define SMP_LOOP_PINGPONG 2

#define SMP_LOOPING_FORWARD 0
#define SMP_LOOPING_BACKWARD 1

#define SYNTH_WAVE_OFF 0
#define SYNTH_WAVE_CURRENT 1

#define PLAYING_NOSAMPLE 0
#define PLAYING_SAMPLE 1
#define PLAYING_SAMPLE_NOTEOFF 2
#define PLAYING_STOCK 3

#define SYNTH_ATTACK 1
#define SYNTH_DECAY 2
#define SYNTH_SUSTAIN 3
#define SYNTH_RELEASE 4

// ------------------------------------------------------
// Structures

// SAMPLE COUNTER
struct smpos
{
#if defined(__BIG_ENDIAN__)
    Uint32 first;
    Uint32 last;
#else
    Uint32 last;
    Uint32 first;
#endif
};

union s_access
{
    smpos half;
    int64 absolu;
};

// ------------------------------------------------------
// Variables
extern float decays[MAX_COMB_FILTERS];
extern int delays[MAX_COMB_FILTERS];       // delays for the comb filters
extern int counters[MAX_COMB_FILTERS];

extern char Mp3_BitRate[MAX_INSTRS];
extern int Type_Mp3_BitRate[];

extern char At3_BitRate[MAX_INSTRS];
extern int Type_At3_BitRate[];

#if defined(PTK_COMPRESSOR)
extern char num_echoes;
#endif

#if defined(PTK_303)
extern unsigned char track3031;
extern unsigned char track3032;
#endif

extern unsigned char *RawPatterns;
extern int cPosition;
extern unsigned int lchorus_counter;
extern unsigned int rchorus_counter;
extern unsigned int lchorus_counter2;
extern unsigned int rchorus_counter2;
extern int lchorus_delay;
extern int rchorus_delay;
extern float mas_comp_threshold;
extern float mas_comp_ratio;
extern unsigned char nPatterns;
extern char Songtracks;
extern unsigned char sLength;
extern unsigned char pSequence[256];
extern short patternLines[MAX_ROWS];
extern char nameins[MAX_INSTRS][20];
extern char Midiprg[MAX_INSTRS];

#if defined(PTK_SYNTH)
extern unsigned char Synthprg[MAX_INSTRS];
#endif

#if !defined(__NO_CODEC__)
#if !defined(__STAND_ALONE__)
extern char SamplesSwap[MAX_INSTRS];
extern short *RawSamples_Swap[MAX_INSTRS][2][16];
#endif
#endif

extern char SampleType[MAX_INSTRS][16];
extern char SampleCompression[MAX_INSTRS];
extern char SampleName[MAX_INSTRS][16][64];
extern char Basenote[MAX_INSTRS][16];
extern Uint32 LoopStart[MAX_INSTRS][16];
extern Uint32 LoopEnd[MAX_INSTRS][16];
extern char LoopType[MAX_INSTRS][16];
extern Uint32 SampleNumSamples[MAX_INSTRS][16];
extern char Finetune[MAX_INSTRS][16];
extern float SampleVol[MAX_INSTRS][16];
extern float FDecay[MAX_INSTRS][16];
extern short *RawSamples[MAX_INSTRS][2][16];
extern char SampleChannels[MAX_INSTRS][16];         // Mono / Stereo
extern float TCut[MAX_TRACKS];
extern float ICut[MAX_TRACKS];
extern float TPan[MAX_TRACKS];
extern int FType[MAX_TRACKS];
extern int FRez[MAX_TRACKS];
extern float DThreshold[MAX_TRACKS];
extern float DClamp[MAX_TRACKS];
extern float DSend[MAX_TRACKS]; 
extern int CSend[MAX_TRACKS];
extern char Channels_Polyphony[MAX_TRACKS];
extern char compressor; // 0-->Off 1-->On
extern int c_threshold;
extern int BeatsPerMin;
extern int TicksPerBeat;
extern float mas_vol;
extern float local_mas_vol;
extern volatile float local_ramp_vol;

extern int delay_time;
extern float Feedback;

extern float lchorus_feedback;
extern float rchorus_feedback;
extern int shuffle;

extern char CHAN_ACTIVE_STATE[256][16];
extern char CHAN_HISTORY_STATE[256][16];
extern float CCoef[MAX_TRACKS];
extern int CHAN_MIDI_PRG[MAX_TRACKS];

extern char LFO_ON[MAX_TRACKS];
extern float LFO_RATE[MAX_TRACKS];
extern float LFO_AMPL[MAX_TRACKS];

extern char FLANGER_ON[MAX_TRACKS];
extern float FLANGER_AMOUNT[MAX_TRACKS];
extern float FLANGER_DEPHASE[MAX_TRACKS];
extern float FLANGER_RATE[MAX_TRACKS];
extern float FLANGER_AMPL[MAX_TRACKS];
extern float FLANGER_GR[MAX_TRACKS];
extern float FLANGER_FEEDBACK[MAX_TRACKS];
extern int FLANGER_DELAY[MAX_TRACKS];
extern int FLANGER_OFFSET[MAX_TRACKS];

extern float foff2[MAX_TRACKS];
extern float foff1[MAX_TRACKS];

extern int CHAN_MUTE_STATE[MAX_TRACKS]; // 0->Normal 1->Muted
extern char Disclap[MAX_TRACKS];

extern char artist[20];
extern char style[20];

extern char beatsync[128];
extern short beatlines[128];

extern float REVERBFILTER;

extern float CustomVol[128];

#if !defined(__STAND_ALONE__)
extern unsigned int SubCounter;
extern int PosInTick;
extern int plx;
extern int Reserved_Sub_Channels[MAX_TRACKS][MAX_POLYPHONY];
extern int Locked_Sub_Channels[MAX_TRACKS][MAX_POLYPHONY];
extern int sp_Stage[MAX_TRACKS][MAX_POLYPHONY];

#if defined(PTK_SYNTH)
extern int sp_Stage2[MAX_TRACKS][MAX_POLYPHONY];
extern int sp_Stage3[MAX_TRACKS][MAX_POLYPHONY];
#endif

extern int L_MaxLevel;
extern int R_MaxLevel;

#if defined(PTK_SYNTH)
extern CSynth Synthesizer[MAX_TRACKS][MAX_POLYPHONY];
#endif

extern float Player_FD[MAX_TRACKS];
extern char sp_channelsample[MAX_TRACKS][MAX_POLYPHONY];
extern char sp_split[MAX_TRACKS][MAX_POLYPHONY];
#if defined(__PSP__)
extern volatile int Songplaying;
#else
extern int Songplaying;
#endif
extern int left_value;
extern int right_value;

#if defined(PTK_SYNTH)
#if !defined(__STAND_ALONE__) || defined(__WINAMP__)
extern SynthParameters PARASynth[128];
#else
extern SYNTH_DATA PARASynth[128];
#endif
#endif

extern float ramper[MAX_TRACKS];
extern unsigned char nPatterns;
extern int delay_time;
extern int DelayType;
#endif

// ------------------------------------------------------
// Functions
void Pre_Song_Init(void);
void Sp_Player(void);
void Play_Instrument(int channel, int sub_channel,
                     int note, int sample,
                     float vol, unsigned int offset,
                     int glide, int Play_Selection, int midi_sub_channel);
void ResetFilters(int tr);
void ComputeStereo(int channel);
void GetPlayerValues(void);
void noteoff303(char strack);
void init_sample_bank(void);
void KillInst(int inst_nbr);
void Post_Song_Init(void);

#if !defined(__STAND_ALONE__) || defined(__WINAMP__)
void ResetSynthParameters(SynthParameters *TSP);
#endif

void Free_Samples(void);
void Mas_Compressor_Set_Variables(float treshold, float ratio);
int Get_Free_Sub_Channel(int channel, int polyphony);
int Get_Pattern_Offset(int pattern, int track, int row);
void InitRevervbFilter(void);

#endif
