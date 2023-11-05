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

#ifndef DIGIBOOSTER3_MODSYNTH_H
#define DIGIBOOSTER3_MODSYNTH_H

#include "libdigibooster3.h"
#include "musicmodule.h"
#include "lists.h"
#include "dsp.h"


/* Sequencer modes. */

#define MMODE_HALTED      0    // sequencer is halted, no row fetches
#define MMODE_PATTERN     1
#define MMODE_SONG        2
#define MMODE_MANUAL      3    // play single row then go to MMODE_HALTED
#define MMODE_PATTBACK    4    // play pattern with sequencer reversed
#define MMODE_MANUALBACK  5    // play single row with sequencer reversed, then go to MMODE_HALTED
#define MMODE_SONG_ONCE   6    // play song one time then wait forever


// OldValues stores last used parameters for effects supporting parameter reuse

struct OldValues
{
	uint8_t VolSlide;               // effect A00
	uint8_t PanSlide;               // effect P00
	uint8_t PortaUp;                // effect 100
	uint8_t PortaDown;              // effect 200
	uint8_t PortaSpeed;             // effect 300
	uint8_t VolSlide5;              // effect 500
	uint8_t Vibrato;                // effect 400
	uint8_t Vibrato6;               // effect 600
};


struct EnvInterp
{
	uint16_t Index;           // index to envelopes table, set in msynth_instr(), -1 if no env
	uint16_t TickCtr;         // ticks left in section
	uint16_t Section;         // current section
	int16_t XDelta;           // current section length in ticks
	int16_t YDelta;           // current section value delta
	int16_t YStart;           // value at section start
	int16_t PrevVal;          // previous returned value (used for sustains)
	uint16_t SustainA;        // set by trigger, cleared to 0xFFFF with keyoff
	uint16_t SustainB;        // set by trigger, cleared to 0xFFFF with keyoff
	uint16_t LoopEnd;         // set by trigger, cleaerd to 0xFFFF with keyoff
};


struct ModTrack
{
	int Instr;                      // a currently set instrument number (from 1!)
	int IsOn;                       // TRUE if channel active
	int Muted;                      // TRUE if to be ignored at mixer

	struct MinList DSPInstrChain;   // a chain of DSP objects for instrument
	struct MinList DSPTrackChain;   // a chain of DSP objects for track

	int16_t GainL;                  // final tick gain (after all effects), left
	int16_t GainR;                  // final tick gain (after all effects), right
	int32_t Volume;                 // speed prescaled, <0, 64>
	int32_t Panning;                // speed prescaled, <-128, +128>
	int32_t Pitch;                  // speed prescaled, <96, 768>
	int16_t VolumeDelta;            // speed prescaled, <-30, +30>
	int16_t PanningDelta;           // speed prescaled, <-30, +30>
	int16_t PitchDelta;             // speed prescaled, <-30, +30>
	int16_t Porta3Delta;            // speed prescaled
	int16_t ApprTable[3];           // appregio, speed prescaled
	int16_t Porta3Target;           // this is *not* speed prescaled, <96, 767>
	int16_t VibratoSpeed;
	int16_t VibratoDepth;
	int16_t VibratoCounter;

	struct EnvInterp VolEnv;        // volume envelope interpolator data
	struct EnvInterp PanEnv;        // panning envelope interpolator data
	int16_t VolEnvCurrent;          // current volume envelope value
	int16_t PanEnvCurrent;          // current panning envelope value

	int32_t TrigCounter;            // triggers instrument when counts down to 0
	int32_t CutCounter;             // switches track off at 0, inhibit triggers then
	int32_t Retrigger;              // retrigger period in ticks (0 for no retrigger)
	int32_t TrigOffset;             // apply at next trigger
	int PlayBackwards;              // E3x command handling
	struct OldValues Old;           // old values for parameter reuse

	int EchoType;                   // Type of echo for this track (off/standard/variable)
	int EchoDelay;                  // 0 to 255, 2 ms (tracker units)
	int EchoFeedback;               // 0 to 255
	int EchoMix;                    // 0 dry, 255 wet
	int EchoCross;                  // 0 to 255
};


struct ModSynth
{
	uint32_t MixFreq;               // mixdown frequency
	struct DB3Module *Mod;          // the module played
	struct ModTrack *Tracks;        // table of tracks
	int Mode;                       // sequencer mode (row/pattern/song/song_once)
	int Pattern;                    // current pattern
	int Song;                       // current song
	int Row;                        // current row (position) in a pattern
	int Order;                      // current order number in a song
	int Tick;                       // current tick of a position
	int Speed;                      // ticks per position
	int Tempo;                      // beats per minute
	int NewTempo;                   // non zero when tempo changed with ModSynth_Tempo() and not yet applied.
	int ChangedTempo;               // non zero when tempo changed by effect and not yet reported with update callback.
	int TickSamplesLo;              // current tick in samples (integer part)
	int TickSamplesHi;              // current tick in samples (fractional part)
	int PatternDelay;               // (EEx) pattern delay in positions, also used for sequencer stopping.
	int DelModuleEnd;               // Module ending (because of end of playlist or F00 command) is also delayed.
	int DelPattBreak;               // (Dxx) delayed pattern break position (position num)
	int DelPattJump;                // (Bxx) delayed pattern jump (pattern num)
	int DelLoop;                    // (E6x) 0 if there is a loop to start, -1 otherwise
	int LoopCounter;                // (E6x) loop counter
	int LoopOrder;                  // (E6x) loop order number in a song
	int LoopRow;                    // (E6x) loop row
	int16_t GlobalVolume;           // (Gxx, Hxx)
	int16_t GlobalVolumeSlide;      // (Hxx)
	uint8_t OldGlobalVolSlide;      // (Hxx)
	int16_t *PreMixBuf;             // buffer for single track data before mixing
	int32_t *Accumulator;           // mixdown accumulator (32-bit, stereo)

	void(*UpdateCallback)(void*, struct UpdateEvent*);  // update callback pointer
	void *UserData;                 // user data pointer passed to UpdateCallback
	struct UpdateEvent UEvent;      // passed to callback

	int16_t MinVolume;              // current sliding limits prescaled with speed
	int16_t MaxVolume;
	int16_t MinPanning;
	int16_t MaxPanning;
	int16_t MinPitch;
	int16_t MaxPitch;

	uint8_t ApprCounter;            // appregio counter

	int32_t BoostMultiplier;        // master volume
	int32_t BoostLimit;             // saturation threshold

	int ManualUpdate;               // send (one) tracker position update being in HALTED mode

	int16_t PanPhaseTable[128];     // panning phase table
};


/* Internal functions used in optional modules. */

int msynth_instrument(struct ModSynth *msyn, struct ModTrack *mt, int instr);
void msynth_pitch(struct ModSynth *msyn, struct ModTrack *mt, uint16_t pitch);
void msynth_defvolume(struct ModSynth *msyn, struct ModTrack *mt);
void msynth_trigger(struct DB3Module *m, struct ModTrack *mt);
void msynth_reset(struct ModSynth *msyn, int unmute);
void msynth_reset_track(struct ModTrack *mt);
void msynth_dsp_set_instr_attrs(struct ModTrack *mt, struct DSPTag *tags);

#endif      /* DIGIBOOSTER3_MODSYNTH_H */
