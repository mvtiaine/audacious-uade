// SPDX-License-Identifier: BSD-2-Clause
/*-----------------*/
/* libdigibooster3 */
/*-----------------*/

/*
  Copyright (c) 2014, Grzegorz Kraszewski
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met: 

  1. Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer. 
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution. 

  This software is provided by the copyright holders and contributors "as is" and
  any express or implied warranties, including, but not limited to the implied
  warranties of merchantability and fitness for a particular purpose are
  disclaimed. In no event shall the copyright owner or contributors be liable for
  any direct, indirect, incidental, special, exemplary, or consequential damages
  (including, but not limited to, procurement of substitute goods or services;
  loss of use, data, or profits; or business interruption) however caused and
  on any theory of liability, whether in contract  strict liability or tort
  (including negligence or otherwise) arising in any way out of the use of this
  software, even if advised of the possibility of such damage.
*/

/* Structures describing module loaded to memory */

#ifndef LIBDIGIBOOSTER3_MUSICMODULE_H
#define LIBDIGIBOOSTER3_MUSICMODULE_H

#ifndef TARGET_WIN32
#include <stdint.h>
#else
#include "../stdint.h"
#endif

/*----------------------------------------------------------------------------*/
/* Instrument flags.                                                          */
/*----------------------------------------------------------------------------*/

#define IF_NO_LOOP          0x00000000
#define IF_FORWARD_LOOP     0x00000001
#define IF_PINGPONG_LOOP    0x00000002
#define IF_LOOP_MASK        0x00000003

/*----------------------------------------------------------------------------*/
/* Instrument types.                                                          */
/*----------------------------------------------------------------------------*/

#define ITYPE_SAMPLE        1

/*----------------------------------------------------------------------------*/
/* Maximum number of envelope points.                                         */
/*----------------------------------------------------------------------------*/

#define ENV_MAX_POINTS       32
#define ENVELOPE_DISABLED    0xFFFF
#define ENV_LOOP_DISABLED    0xFFFF
#define ENV_SUSTAIN_DISABLED 0xFFFF

/*----------------------------------------------------------------------------*/
/* Defined module creators.                                                   */
/*----------------------------------------------------------------------------*/

#define CREATOR_DIGIBOOSTER_2                2
#define CREATOR_DIGIBOOSTER_3                3

/*-----------*/
/* A sample. */
/*-----------*/

struct DB3ModSample
{
	int32_t Frames;
	int16_t *Data;
};

/*-----------------------*/
/* Single pattern entry. */
/*-----------------------*/

struct DB3ModEntry
{
	uint8_t Note;       // 0 to 11
	uint8_t Octave;     // 0 to 7
	uint8_t Instr;
	uint8_t Cmd1;
	uint8_t Param1;
	uint8_t Cmd2;
	uint8_t Param2;
	uint8_t Pad;        // pads to 8 bytes
};

/*-------------------*/
/* Complete pattern. */
/*-------------------*/

struct DB3ModPatt
{
	uint16_t NumRows;
	struct DB3ModEntry *Pattern;    // a table
};

/*----------------*/
/* An instrument. */
/*----------------*/

struct DB3ModInstr
{
	char *Name;
	uint16_t Volume;       // 0 to 64 (including)
	int16_t Panning;       // -128 full left, +128 full right
	uint16_t Type;
	uint16_t VolEnv;       // index of volume envelope, 0xFFFF if none
	uint16_t PanEnv;       // index of panning envelope, 0xFFFF if none
};

// Sample based instrument.

struct DB3ModInstrS
{
	struct DB3ModInstr Instr;
	uint32_t C3Freq;
	uint16_t SampleNum;
	int32_t LoopStart;
	int32_t LoopLen;
	uint16_t Flags;
};

/*----------------*/
/* Complete song. */
/*----------------*/

struct DB3ModSong
{
	char *Name;
	uint16_t NumOrders;        // playlist length
	uint16_t *PlayList;        // playlist table
};

/*--------------------------------------------------------*/
/* Envelope (used for both panning and volume envelopes). */
/*--------------------------------------------------------*/

struct DB3ModEnvPoint
{
	uint16_t Position;
	int16_t Value;
};

struct DB3ModEnvelope
{
	uint16_t InstrNum;     // number of instrument (from 0)
	uint16_t NumSections;
	uint16_t LoopFirst;    // point number
	uint16_t LoopLast;     // point number, loop disabled if 0xFFFF
	uint16_t SustainA;     // point number, disabled if 0xFFFF
	uint16_t SustainB;     // point number, disabled if 0xFFFF
	struct DB3ModEnvPoint Points[ENV_MAX_POINTS];
};

/*----------------------------*/
/* Global DSP echo parameters */
/*----------------------------*/

#define DSP_MASK_ECHO      0x00000001
 
struct DB3GlobalDSP
{
	uint32_t    *EffectMask;     // one longword per track, bit 0 is for global echo
	uint8_t      EchoDelay;      // 2 ms units
	uint8_t      EchoFeedback;
	uint8_t      EchoMix;
	uint8_t      EchoCross;
};


/*------------------*/
/* Complete module. */
/*------------------*/

struct DB3Module
{
	char *Name;
	uint16_t CreatorVer;
	uint16_t CreatorRev;
	uint16_t NumInstr;
	uint16_t NumSamples;
	uint16_t NumSongs;
	uint16_t NumPatterns;
	uint16_t NumTracks;
	uint16_t NumVolEnv;                 // number of volume envelopes
	uint16_t NumPanEnv;                 // number of panning envelopes
	struct DB3ModInstr **Instruments;   // table of pointers to instruments
	struct DB3ModSample **Samples;      // table of pointers to samples
	struct DB3ModSong **Songs;          // table of pointers to songs
	struct DB3ModPatt **Patterns;       // table of pointers to patterns
	struct DB3ModEnvelope *VolEnvs;     // table of volume envelopes
	struct DB3ModEnvelope *PanEnvs;     // table of panning envelopes
	struct DB3GlobalDSP DspDefaults;    // global DSP effects defaults
};

#endif  /* LIBDIGIBOOSTER3_MUSICMODULE_H */

