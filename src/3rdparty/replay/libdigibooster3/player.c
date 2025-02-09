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

/* DBM play code */

#include "libdigibooster3.h"
#include "dsp.h"
#include "player.h"

#define porta_to_note(me) ((me->Cmd1 == 3) || (me->Cmd1 == 5) || (me->Cmd2 == 3) || (me->Cmd2 == 5))


uint32_t MusicScale[96] =
{
//       0      +1      +2      +3      +4      +5      +6      +7   finetune
	 65536,  66011,  66489,  66971,  67456,  67945,  68438,  68933,  // C
	 69433,  69936,  70443,  70953,  71468,  71985,  72507,  73032,  // C#
	 73562,  74095,  74632,  75172,  75717,  76266,  76819,  77375,  // D
	 77936,  78501,  79069,  79642,  80220,  80801,  81386,  81976,  // D#
	 82570,  83169,  83771,  84378,  84990,  85606,  86226,  86851,  // E
	 87480,  88114,  88752,  89396,  90043,  90696,  91353,  92015,  // F
	 92682,  93354,  94030,  94711,  95398,  96089,  96785,  97487,  // F#
	 98193,  98905,  99621, 100340, 101070, 101800, 102540, 103280,  // G
	104032, 104786, 105545, 106310, 107080, 107856, 108638, 109425,  // G#
	110218, 111017, 111821, 112631, 113448, 114270, 115098, 115932,  // A
	116772, 117618, 118470, 119329, 120194, 121065, 121942, 122825,  // A#
	123715, 124612, 125515, 126425, 127341, 128263, 129193, 130129   // H
};


uint32_t SPorta1[1]   = { 65536 };
uint32_t SPorta2[2]   = { 65536, 65773 };
uint32_t SPorta3[3]   = { 65536, 65694, 65852 };
uint32_t SPorta4[4]   = { 65536, 65654, 65773, 65892 };
uint32_t SPorta5[5]   = { 65536, 65631, 65726, 65821, 65916 };
uint32_t SPorta6[6]   = { 65536, 65615, 65694, 65773, 65852, 65932 };
uint32_t SPorta7[7]   = { 65536, 65604, 65671, 65739, 65807, 65875, 65943 };
uint32_t SPorta8[8]   = { 65536, 65595, 65654, 65714, 65773, 65832, 65892, 65951 };
uint32_t SPorta9[9]   = { 65536, 65589, 65641, 65694, 65747, 65799, 65852, 65905, 65958 };
uint32_t SPorta10[10] = { 65536, 65583, 65631, 65678, 65726, 65773, 65821, 65868, 65916, 65963 };
uint32_t SPorta11[11] = { 65536, 65579, 65622, 65665, 65708, 65751, 65795, 65838, 65881, 65924, 65968 };
uint32_t SPorta12[12] = { 65536, 65575, 65615, 65654, 65694, 65733, 65773, 65813, 65852, 65892, 65932,
                          65971 };
uint32_t SPorta13[13] = { 65536, 65572, 65609, 65645, 65682, 65718, 65755, 65791, 65828, 65864, 65901,
                          65938, 65974 };
uint32_t SPorta14[14] = { 65536, 65570, 65604, 65637, 65671, 65705, 65739, 65773, 65807, 65841, 65875,
                          65909, 65943, 65977 };
uint32_t SPorta15[15] = { 65536, 65568, 65599, 65631, 65662, 65694, 65726, 65757, 65789, 65821, 65852,
                          65884, 65916, 65947, 65979 };
uint32_t SPorta16[16] = { 65536, 65566, 65595, 65625, 65654, 65684, 65714, 65743, 65773, 65803, 65832,
                          65862, 65892, 65922, 65951, 65981 };
uint32_t SPorta17[17] = { 65536, 65564, 65592, 65620, 65647, 65675, 65703, 65731, 65759, 65787, 65815,
                          65843, 65871, 65899, 65927, 65955, 65983 };
uint32_t SPorta18[18] = { 65536, 65562, 65589, 65615, 65641, 65668, 65694, 65720, 65747, 65773, 65799,
                          65826, 65852, 65879, 65905, 65932, 65958, 65984 };
uint32_t SPorta19[19] = { 65536, 65561, 65586, 65611, 65636, 65661, 65686, 65711, 65736, 65761, 65786,
                          65811, 65836, 65861, 65886, 65911, 65936, 65961, 65986 };
uint32_t SPorta20[20] = { 65536, 65560, 65583, 65607, 65631, 65654, 65678, 65702, 65726, 65749, 65773,
                          65797, 65821, 65844, 65868, 65892, 65916, 65939, 65963, 65987 };
uint32_t SPorta21[21] = { 65536, 65559, 65581, 65604, 65626, 65649, 65671, 65694, 65717, 65739, 65762,
                          65784, 65807, 65830, 65852, 65875, 65898, 65920, 65943, 65966, 65988 };
uint32_t SPorta22[22] = { 65536, 65558, 65579, 65601, 65622, 65644, 65665, 65687, 65708, 65730, 65751,
                          65773, 65795, 65816, 65838, 65859, 65881, 65903, 65924, 65946, 65968, 65989 };
uint32_t SPorta23[23] = { 65536, 65557, 65577, 65598, 65618, 65639, 65660, 65680, 65701, 65721, 65742,
                          65763, 65783, 65804, 65825, 65845, 65866, 65887, 65907, 65928, 65949, 65969,
                          65990 };
uint32_t SPorta24[24] = { 65536, 65556, 65575, 65595, 65615, 65635, 65654, 65674, 65694, 65714, 65733,
                          65753, 65773, 65793, 65813, 65832, 65852, 65872, 65892, 65912, 65932, 65951,
                          65971, 65991 };
uint32_t SPorta25[25] = { 65536, 65555, 65574, 65593, 65612, 65631, 65650, 65669, 65688, 65707, 65726,
                          65745, 65764, 65783, 65802, 65821, 65840, 65859, 65878, 65897, 65916, 65935,
                          65954, 65973, 65992 };
uint32_t SPorta26[26] = { 65536, 65554, 65572, 65591, 65609, 65627, 65645, 65664, 65682, 65700, 65718,
                          65737, 65755, 65773, 65791, 65810, 65828, 65846, 65864, 65883, 65901, 65919,
                          65938, 65956, 65974, 65993 };
uint32_t SPorta27[27] = { 65536, 65554, 65571, 65589, 65606, 65624, 65641, 65659, 65676, 65694, 65711,
                          65729, 65747, 65764, 65782, 65799, 65817, 65835, 65852, 65870, 65887, 65905,
                          65923, 65940, 65958, 65976, 65993 };
uint32_t SPorta28[28] = { 65536, 65553, 65570, 65587, 65604, 65621, 65637, 65654, 65671, 65688, 65705,
                          65722, 65739, 65756, 65773, 65790, 65807, 65824, 65841, 65858, 65875, 65892,
                          65909, 65926, 65943, 65960, 65977, 65994 };
uint32_t SPorta29[29] = { 65536, 65552, 65569, 65585, 65601, 65618, 65634, 65650, 65667, 65683, 65699,
                          65716, 65732, 65748, 65765, 65781, 65798, 65814, 65830, 65847, 65863, 65880,
                          65896, 65912, 65929, 65945, 65962, 65978, 65994 };
uint32_t SPorta30[30] = { 65536, 65552, 65568, 65583, 65599, 65615, 65631, 65647, 65662, 65678, 65694,
                          65710, 65726, 65741, 65757, 65773, 65789, 65805, 65821, 65836, 65852, 65868,
                          65884, 65900, 65916, 65932, 65947, 65963, 65979, 65995 };
uint32_t SPorta31[31] = { 65536, 65551, 65567, 65582, 65597, 65612, 65628, 65643, 65658, 65674, 65689,
                          65704, 65719, 65735, 65750, 65765, 65781, 65796, 65811, 65827, 65842, 65857,
                          65873, 65888, 65903, 65919, 65934, 65949, 65965, 65980, 65996 };


uint32_t* SmoothPorta[32] = { NULL, SPorta1, SPorta2, SPorta3, SPorta4, SPorta5,
 SPorta6, SPorta7, SPorta8, SPorta9, SPorta10, SPorta11, SPorta12, SPorta13,
 SPorta14, SPorta15, SPorta16, SPorta17, SPorta18, SPorta19, SPorta20, SPorta21,
 SPorta22, SPorta23, SPorta24, SPorta25, SPorta26, SPorta27, SPorta28, SPorta29,
 SPorta30, SPorta31 };


int16_t Vibrato[64] = {
    0,   25,   50,   74,   98,  121,  142,  162,
  181,  197,  213,  226,  237,  245,  251,  255,
  256,  255,  251,  245,  237,  226,  213,  197,
  181,  162,  142,  121,   98,   74,   50,   25,
    0,  -25,  -50,  -74,  -98, -121, -142, -162,
 -181, -197, -213, -226, -237, -245, -251, -255,
 -256, -255, -251, -245, -237, -226, -213, -197,
 -181, -162, -142, -121,  -98,  -74,  -50,  -25
};


uint32_t DbVals[6] = { 65536, 73562, 82570, 92682, 104032, 116772 };


#define LOOPDIR_FORWARD         0
#define LOOPDIR_BACKWARD        1


#define ITERATE_LIST(listptr, type, var) \
for (var = (type)((struct MinList*)listptr)->mlh_Head; \
 ((struct MinNode*)var)->mln_Succ; \
 var = (type)((struct MinNode*)var)->mln_Succ)

#define INIT_LIST(listptr) \
((struct MinList*)listptr)->mlh_Head = (struct MinNode*)&((struct MinList*)listptr)->mlh_Tail; \
((struct MinList*)listptr)->mlh_Tail = NULL; \
((struct MinList*)listptr)->mlh_TailPred = (struct MinNode*)&((struct MinList*)listptr)->mlh_Head;



//==============================================================================================
// bcd2bin()
//==============================================================================================

uint8_t bcd2bin(uint8_t x)
{
	uint8_t r = 0;

	if ((x >> 4) < 10) r = (x >> 4) * 10;
	else return 0;

	if ((x & 0xF) < 10) r += (x & 0xF);
	else return 0;

	return r;
}


//==============================================================================================
// msynth_dsp_dispose_chain()
//==============================================================================================

void msynth_dsp_dispose_chain(struct MinList *chain)
{
	struct DSPObject *dspo;

	while (dspo = (struct DSPObject*)DB3RemHead(chain))
	{
		dspo->dsp_dispose(dspo);
	}
}


//==============================================================================================
// msynth_dsp_set_instr_attrs()
//==============================================================================================

void msynth_dsp_set_instr_attrs(struct ModTrack *mt, struct DSPTag *tags)
{
	struct DSPObject *dspo;

	ITERATE_LIST(&mt->DSPInstrChain, struct DSPObject*, dspo)
	{
		dspo->dsp_set(dspo, tags);
	}
}


//==============================================================================================
// msynth_dsp_set_track_attrs()
//==============================================================================================

void msynth_dsp_set_track_attrs(struct ModTrack *mt, struct DSPTag *tags)
{
	struct DSPObject *dspo;

	ITERATE_LIST(&mt->DSPTrackChain, struct DSPObject*, dspo)
	{
		dspo->dsp_set(dspo, tags);
	}
}


//==============================================================================================
// msynth_dsp_get_instr_attr()
//==============================================================================================

int32_t msynth_dsp_get_instr_attr(struct ModTrack *mt, uint32_t tag)
{
	struct DSPObject *dspo;
	int32_t value = 0;

	ITERATE_LIST(&mt->DSPInstrChain, struct DSPObject*, dspo)
	{
		if (dspo->dsp_get(dspo, tag, &value)) return value;
	}

	return 0;
}


//==============================================================================================
// msynth_dsp_get_track_attr()
//==============================================================================================

int32_t msynth_dsp_get_track_attr(struct ModTrack *mt, uint32_t tag)
{
	struct DSPObject *dspo;
	int32_t value = 0;

	ITERATE_LIST(&mt->DSPTrackChain, struct DSPObject*, dspo)
	{
		if (dspo->dsp_get(dspo, tag, &value)) return value;
	}

	return 0;
}


//==============================================================================================
// msynth_trigger()
//==============================================================================================

void msynth_trigger(struct DB3Module *m, struct ModTrack *mt)
{
	struct DSPObject *last;

	mt->VolEnvCurrent = 16384;
	mt->PanEnvCurrent = 0;

	// Reset envelopes interpolators.

	mt->VolEnv.Section = 0;
	mt->VolEnv.TickCtr = 0;
	
	if (mt->VolEnv.Index != 0xFFFF)
	{
		mt->VolEnv.SustainA = m->VolEnvs[mt->VolEnv.Index].SustainA;
		mt->VolEnv.SustainB = m->VolEnvs[mt->VolEnv.Index].SustainB;
		mt->VolEnv.LoopEnd = m->VolEnvs[mt->VolEnv.Index].LoopLast;
	}

	mt->PanEnv.Section = 0;
	mt->PanEnv.TickCtr = 0;

	if (mt->PanEnv.Index != 0xFFFF)
	{
		mt->PanEnv.SustainA = m->PanEnvs[mt->PanEnv.Index].SustainA;
		mt->PanEnv.SustainB = m->PanEnvs[mt->PanEnv.Index].SustainB;
		mt->PanEnv.LoopEnd = m->PanEnvs[mt->PanEnv.Index].LoopLast;
	}

	// Set sample offset and loop direction. Flush any buffers.

	if (last = (struct DSPObject*)mt->DSPInstrChain.mlh_TailPred)
	{
		last->dsp_flush(last);

		if (mt->PlayBackwards)
		{
			struct DSPTag tags[3] = {
				{ DSPA_ReversePlay, TRUE },                    // must be before DSPA_SampleOffset
				{ DSPA_SampleOffset, mt->TrigOffset },
				{ 0, 0 }
			};

			msynth_dsp_set_instr_attrs(mt, tags);
		}
		else
		{
			struct DSPTag tags[3] = {
				{ DSPA_ReversePlay, FALSE },                   // must be before DSPA_SampleOffset
				{ DSPA_SampleOffset, mt->TrigOffset },
				{ 0, 0 }
			};

			msynth_dsp_set_instr_attrs(mt, tags);
		}

		mt->VibratoCounter = 0;
		mt->IsOn = 1;
	}
}


//==============================================================================================
// msynth_instrument()
//==============================================================================================

int msynth_instrument(struct ModSynth *msyn, struct ModTrack *mt, int instr)
{
	struct DB3ModInstr *mi;

	msynth_dsp_dispose_chain(&mt->DSPInstrChain);
	mt->Instr = 0;
	mt->IsOn = 0;

	/*-----------------------------------------------------------------*/
	/* Added 20.11.2009: Some modules contain triggers of non-existing */
	/* instruments. Such instruments should be silently ignored.       */
	/*-----------------------------------------------------------------*/

	if (instr > msyn->Mod->NumInstr) return FALSE;
	mi = msyn->Mod->Instruments[instr - 1];

	/*--------------------------------*/
	/* Enable envelope interpolators. */
	/*--------------------------------*/

	mt->VolEnv.Index = mi->VolEnv;
	mt->PanEnv.Index = mi->PanEnv;

	switch (mi->Type)
	{
		case ITYPE_SAMPLE:
		{
			struct DB3ModInstrS *mis = (struct DB3ModInstrS*)mi;
			struct DB3ModSample *ms = msyn->Mod->Samples[mis->SampleNum];
			struct DSPObject *wavetable, *zeropadder, *resampler, *panoramizer;

			// There may be instruments with proper, but empty samples. If such an
			// instrument is triggered, just turn the channel off to the next trigger.

			if (!ms || !ms->Data || !ms->Frames) return FALSE;

			if ((mis->Flags & IF_LOOP_MASK) == IF_NO_LOOP)
			{
				wavetable = dsp_sampled_instr_new(ms->Data, 0, 0, 0, ms->Frames, IF_NO_LOOP);
			}
			else   // Forward or pingpong loop. I assume mis->LoopLen > 0.
			{
				wavetable = dsp_sampled_instr_new(ms->Data, mis->LoopStart, mis->LoopLen, 0x7FFFFFFF, ms->Frames, mis->Flags & IF_LOOP_MASK);
			}

			zeropadder = dsp_zeropadder_new(0);
			resampler = dsp_resampler20_new();
			panoramizer = dsp_panoramizer_new(msyn->PanPhaseTable);

			if (wavetable && zeropadder && resampler && panoramizer)
			{
				DB3AddTail(&mt->DSPInstrChain, (struct MinNode*)wavetable);
				DB3AddTail(&mt->DSPInstrChain, (struct MinNode*)zeropadder);
				DB3AddTail(&mt->DSPInstrChain, (struct MinNode*)resampler);
				DB3AddTail(&mt->DSPInstrChain, (struct MinNode*)panoramizer);
				mt->Instr = instr;
				return TRUE;
			}

			if (wavetable) wavetable->dsp_dispose(wavetable);
			if (zeropadder) zeropadder->dsp_dispose(zeropadder);
			if (resampler) resampler->dsp_dispose(resampler);
			if (panoramizer) panoramizer->dsp_dispose(panoramizer);
		}
		break;
	}
	return FALSE;
}



//==============================================================================================
// msynth_pitch()
//==============================================================================================

// Sets the pitch for the current instrument on the track. Does neither set nor
// retrigger the instrument. 'pitch' parameter is in finetune unit prescaled by
// current module speed. 0 of pitch is C-0 note.

void msynth_pitch(struct ModSynth *msyn, struct ModTrack *mt, uint16_t pitch)
{
	if (mt->Instr)
	{
		struct DB3ModInstr *mi = msyn->Mod->Instruments[mt->Instr - 1];

		if (mi)
		{
			switch (mi->Type)
			{
				case ITYPE_SAMPLE:
				{
					struct DB3ModInstrS *mis = (struct DB3ModInstrS*)mi;
					uint32_t samplestep, alpha, beta;
					uint64_t samplestep64 = 0;
					uint16_t f_tune, s_porta, octave = 0;
					struct DSPTag tags[2] = {{ DSPA_ResamplerRatio, 0 }, { 0, 0 }};

					s_porta	= pitch % msyn->Speed;
					f_tune = pitch / msyn->Speed;
					alpha = SmoothPorta[msyn->Speed][s_porta];
					f_tune = f_tune > 96 ? f_tune - 96 : 0; // mvtiaine: added sanity check

					while (f_tune >= 96)
					{
						f_tune -= 96;
						octave++;
					}

					beta = MusicScale[f_tune];
					samplestep64 = (uint64_t)mis->C3Freq * beta * alpha;
					samplestep64 >>= 19 - octave;
					samplestep = (uint32_t)(samplestep64 / msyn->MixFreq);
					tags[0].dspt_data = samplestep;
					msynth_dsp_set_instr_attrs(mt, tags);
				}
			}
		}
	}
}


//==============================================================================================
// msynth_defvolume()
//==============================================================================================

void msynth_defvolume(struct ModSynth *msyn, struct ModTrack *mt)
{
	struct DB3ModInstr *minst;

	if (mt->Instr)
	{
		minst = msyn->Mod->Instruments[mt->Instr - 1];
		mt->Volume = minst->Volume * msyn->Speed;
		mt->Panning = minst->Panning * msyn->Speed;

		// Added 05.11.2009. Volume reset should also restart panning and volume
		// envelopes for the instrument.

		// Reset envelopes interpolators.

		mt->VolEnv.Section = 0;
		mt->VolEnv.TickCtr = 0;
		mt->PanEnv.Section = 0;
		mt->PanEnv.TickCtr = 0;
	}
}

//==============================================================================================
// msynth_reset_delayed()
//==============================================================================================

void msynth_reset_delayed(struct ModSynth *msyn)
{
	msyn->DelPattBreak = -1;
	msyn->DelPattJump = -1;
	msyn->DelLoop = -1;
	msyn->DelModuleEnd = 0;
}



//==============================================================================================
// msynth_reset_loop()
//==============================================================================================

void msynth_reset_loop(struct ModSynth *msyn)
{
	msyn->LoopCounter = 0;
	msyn->LoopOrder = 0;
	msyn->LoopRow = 0;
}



//==============================================================================================
// msynth_apply_delayed()
//==============================================================================================

void msynth_apply_delayed(struct ModSynth *msyn)
{
	struct DB3ModSong *song = msyn->Mod->Songs[msyn->Song];

	// Position jump.

	if (msyn->DelPattJump != -1)
	{
		if (msyn->DelPattJump < song->NumOrders) msyn->Order = msyn->DelPattJump;
		else msyn->Order = 0;
		msyn->Pattern = song->PlayList[msyn->Order];
		msyn->Row = 0;
	}

	// Pattern break.

	if (msyn->DelPattBreak != -1)
	{
		struct DB3ModPatt *mpatt;

		if ((msyn->DelPattJump == -1) && (msyn->Row > 0))
		{
			if (++msyn->Order >= song->NumOrders) msyn->Order = 0;
		}
		
		msyn->Pattern = song->PlayList[msyn->Order];
		mpatt = msyn->Mod->Patterns[msyn->Pattern];
		if (msyn->DelPattBreak < mpatt->NumRows) msyn->Row = msyn->DelPattBreak;
		else msyn->Row = mpatt->NumRows - 1;
	}

	// Loops.

	if (msyn->DelLoop != -1)
	{
		msyn->Order = msyn->LoopOrder;
		msyn->Pattern = song->PlayList[msyn->Order];
		msyn->Row = msyn->LoopRow;
	}

	// Module end.

	if (msyn->DelModuleEnd) msyn->PatternDelay = 0x7FFFFFFF;

	// Reset all delayed things.

	msynth_reset_delayed(msyn);
}


//==============================================================================================
// msynth_echo_on_for_track()
//==============================================================================================

// Used for both standard, global echo and new per-track echo. The only difference is echo type,
// as per-track echo parameters default to standard ones. Echo is inserted as the last object in
// the track DSP chain. Echo should not be added more than one time, so if DSPTYPE_ECHO object
// already exists in the track DSP chain, this function does nothing.

void msynth_echo_on_for_track(struct ModSynth *msyn, struct ModTrack *mt, int type)
{
	struct DSPObject *echo;

	// checking echo module existence

	if (msynth_dsp_get_track_attr(mt, DSPA_EchoType) > 0) return;

	// adding echo

	echo = dsp_echo_new(msyn->MixFreq, type);

	if (echo)
	{
		DB3AddTail(&mt->DSPTrackChain, (struct MinNode*)echo);
		mt->EchoType = type;
	}
}


//==============================================================================================
// msynth_echo_off_for_track
//==============================================================================================

// Searches the track DSP chain for [the first] DSPTYPE_ECHO object and removes it. If there is
// no DSPTYPE_ECHO object in the track DSP chain, the function does nothing. The function
// removes echo only if it matches passed 'type'.

void msynth_echo_off_for_track(struct ModTrack *mt, int type)
{
	struct DSPObject *obj;
	int echo_type;

	// the check below exits the function either if there is no echo in the chain, or echo type
	// does not match

	echo_type = msynth_dsp_get_track_attr(mt, DSPA_EchoType);
	if (echo_type != type) return;

	// locate echo object and remove it

	ITERATE_LIST(&mt->DSPTrackChain, struct DSPObject*, obj)
	{
		if (obj->dsp_type == DSPTYPE_ECHO)
		{
			DB3Remove((struct MinNode*)obj);
			obj->dsp_dispose(obj);
			break;
		}
	}
}


//==============================================================================================
// msynth_echo_on_for_all_tracks()
//==============================================================================================

// Adds old style (global controlled) echo for all tracks not having any echo.

void msynth_echo_on_for_all_tracks(struct ModSynth *msyn)
{
	int track_number;

	for (track_number = 0; track_number < msyn->Mod->NumTracks; track_number++)
	{
		msynth_echo_on_for_track(msyn, &msyn->Tracks[track_number], DSPV_EchoType_Old);
	}
}


//==============================================================================================
// msynth_echo_off_for_all_tracks()
//==============================================================================================

// It only switches off the standard echo. New style echo must be switched off for each track
// with V21 command.

void msynth_echo_off_for_all_tracks(struct ModSynth *msyn)
{
	int track_number;

	for (track_number = 0; track_number < msyn->Mod->NumTracks; track_number++)
	{
		msynth_echo_off_for_track(&msyn->Tracks[track_number], DSPV_EchoType_Old);
	}
}


//==============================================================================================
// msynth_change_echo_params()
//==============================================================================================

// This function checks the echo type in the track. If there is no echo, or echo is
// DSPV_EchoType_Old, echo parameters are set for all tracks with old echo. If echo is
// DSPV_EchoType_New, parameters are changed for this track only.

void msynth_change_echo_params(struct ModSynth *msyn, struct ModTrack *mt)
{
	struct DSPTag tags[5];

	tags[0].dspt_tag = DSPA_EchoFeedback;   tags[0].dspt_data = mt->EchoFeedback;
	tags[1].dspt_tag = DSPA_EchoMix;        tags[1].dspt_data = mt->EchoMix;
	tags[2].dspt_tag = DSPA_EchoCross;      tags[2].dspt_data = mt->EchoCross;
	tags[3].dspt_tag = DSPA_EchoDelay;      tags[3].dspt_data = mt->EchoDelay;
	tags[4].dspt_tag = TAG_END;

	if (msynth_dsp_get_track_attr(mt, DSPA_EchoType) == DSPV_EchoType_New) msynth_dsp_set_track_attrs(mt, tags);
	else
	{
		int track_num;
		struct ModTrack *mt2;

		for (track_num = 0; track_num < msyn->Mod->NumTracks; track_num++)
		{
			mt2 = &msyn->Tracks[track_num];

			if (msynth_dsp_get_track_attr(mt2, DSPA_EchoType) == DSPV_EchoType_Old)
			{
				mt2->EchoFeedback = mt->EchoFeedback;
				mt2->EchoMix = mt->EchoMix;
				mt2->EchoCross = mt->EchoCross;
				mt2->EchoDelay = mt->EchoDelay;
				msynth_dsp_set_track_attrs(mt2, tags);
			}
		}
	}
}


//==============================================================================================
// msynth_effect_exx()
//==============================================================================================

void msynth_effect_exx(struct ModSynth *msyn, struct ModTrack *mt, uint8_t param)
{
	switch (param >> 4)
	{
		/* ====== E1x - FINE PORTAMENTO UP ====== */

		case 0x1:
			mt->Pitch += (param & 0xF) * msyn->Speed;
			if (mt->Pitch > msyn->MaxPitch) mt->Pitch = msyn->MaxPitch;
		break;

		/* ====== E2x - FINE PORTAMENTO DOWN ====== */

		case 0x2:
			mt->Pitch -= (param & 0xF) * msyn->Speed;
			if (mt->Pitch < msyn->MinPitch) mt->Pitch = msyn->MinPitch;
		break;

		/* ====== E3x - PLAY BACKWARDS ====== */

		case 0x3:
			if (mt->TrigCounter != 0x7FFF) mt->PlayBackwards = TRUE;
		break;

		/* ====== E4x - CHANNEL CONTROL A ====== */

		case 0x4:
			if (param == 0x40)          /* E40 - track mute */
			{
				mt->IsOn = FALSE;
			}
		break;

		/* ====== E6x - PLAY LOOP ========== */

		
		case 0x6:
			if (param & 0xF)
			{
				if (msyn->LoopCounter == 0)
				{
					msyn->LoopCounter = param & 0xF;
					msyn->DelLoop = 0;
				}
				else
				{
					if (--msyn->LoopCounter > 0) msyn->DelLoop = 0;
					else msynth_reset_loop(msyn);
				}
			}
			else    /* E60 */
			{
				if (msyn->LoopCounter == 0)
				{
					msyn->LoopOrder = msyn->Order;
					msyn->LoopRow = msyn->Row;
				}
			}
		break;

		/* ====== E7x - COARSE SAMPLE OFFSET ====== */

		case 0x7:
			mt->TrigOffset += (int32_t)(param & 0xF) << 16;
		break;

		/* ====== E8x - COARSE PANNING ====== */

		case 0x8:
			mt->Panning = (((int16_t)(param & 0xF) << 4) - 128) * msyn->Speed;
		break;

		/* ====== E9x - NOTE RETRIGGER ====== */

		case 0x9:
			mt->Retrigger = param & 0xF;
		break;

		/* ====== EAx - FINE VOLUME SLIDE UP ====== */

		case 0xA:
			mt->Volume += (param & 0xF) * msyn->Speed;
			if (mt->Volume > msyn->MaxVolume) mt->Volume = msyn->MaxVolume;
		break;

		/* ====== EBx - FINE VOLUME SLIDE DOWN ====== */

		case 0xB:
			mt->Volume -= (param & 0xF) * msyn->Speed;
			if (mt->Volume < msyn->MinVolume) mt->Volume = msyn->MinVolume;
		break;

		/* ====== ECx - NOTE CUT ====== */

		case 0xC:
			mt->CutCounter = param & 0xF;
		break;

		/* ====== EDx - NOTE DELAY ====== */

		case 0xD:
			mt->TrigCounter = param & 0xF;
		break;

		/* ====== EEx - PATTERN DELAY ====== */

		case 0xE:
			msyn->PatternDelay = param & 0x0F;
		break;
	}
}


//==============================================================================================
// msynth_effect()
//==============================================================================================

void msynth_effect(struct ModSynth *msyn, struct ModTrack *mt, uint8_t cmd, uint8_t param)
{
	switch (cmd)
	{
		/* ====== 0xx - APPREGIO ====== */

		case 0x0:
			mt->ApprTable[1] = ((param >> 4) << 3) * msyn->Speed;
			mt->ApprTable[2] = ((param & 0xF) << 3) * msyn->Speed;
		break;

		/* ====== 1xx - PORTAMENTO UP (includes 1Fx) ====== */

		case 0x1:
		{
			if (!param) param = mt->Old.PortaUp;   // 100, reuse old parameter
			else mt->Old.PortaUp = param;
			if (param < 0xF0)	mt->PitchDelta += param * msyn->Speed;
			else mt->PitchDelta += param & 0x0F;                       // smooth 1Fx
		}
		break;

		/* ====== 2xx - PORTAMENTO DOWN (includes 2Fx) ====== */

		case 0x2:
		{
			if (!param) param = mt->Old.PortaDown;   // 200, reuse old parameter
			else mt->Old.PortaDown = param;
			if (param < 0xF0)	mt->PitchDelta -= param * msyn->Speed;
			else mt->PitchDelta -= param & 0x0F;                       // smooth 2Fx
		}
		break;

		/* ====== 3xx - PORTA TO NOTE ====== */

		case 0x3:
		{
			int16_t porta_target;

			if (!param) param = mt->Old.PortaSpeed;
			else mt->Old.PortaSpeed = param;

			porta_target = mt->Porta3Target * msyn->Speed;

			if (porta_target >= mt->Pitch) mt->Porta3Delta += param * msyn->Speed;
			else mt->Porta3Delta -= param * msyn->Speed;
		}
		break;


		/* ====== 4xx - VIBRATO ======= */

		case 0x4:
			if (!param) param = mt->Old.Vibrato;
			else mt->Old.Vibrato = param;

			mt->VibratoSpeed = param >> 4;
			mt->VibratoDepth = (param & 0xF) * msyn->Speed;
		break;

		/* ====== 5xx - PORTA TO NOTE + VOLUME SLIDE ====== */

		case 0x5:
		{
			int16_t porta_target, porta_speed, p0, p1;

			if (!param) param = mt->Old.VolSlide5;
			else mt->Old.VolSlide5 = param;

			porta_speed = mt->Old.PortaSpeed;
			porta_target = mt->Porta3Target * msyn->Speed;

			if (porta_target >= mt->Pitch) mt->Porta3Delta += porta_speed * msyn->Speed;
			else mt->Porta3Delta -= porta_speed * msyn->Speed;

			p0 = param >> 4;
			p1 = param & 0xF;

			if ((p0 == 0) || (p1 == 0))    // Normal 50x/5x0
			{
				if (p0) mt->VolumeDelta += p0 * msyn->Speed;
				if (p1) mt->VolumeDelta -= p1 * msyn->Speed;
			}
			else
			{
				if (p1 == 0xF) mt->VolumeDelta += p0;
				else if (p0 == 0xF) mt->VolumeDelta -= p1;
			}
		}
		break;

		/* ====== 6xx - VIBRATO + VOLUME SLIDE ======= */

		case 0x6:
		{
			int16_t p0, p1;

			if (!param) param = mt->Old.Vibrato6;
			else mt->Old.Vibrato6 = param;

			mt->VibratoSpeed = mt->Old.Vibrato >> 4;
			mt->VibratoDepth = (mt->Old.Vibrato & 0xF) * msyn->Speed;

			p0 = param >> 4;
			p1 = param & 0xF;

			if ((p0 == 0) || (p1 == 0))    // Normal 60x/6x0
			{
				if (p0) mt->VolumeDelta += p0 * msyn->Speed;
				if (p1) mt->VolumeDelta -= p1 * msyn->Speed;
			}
			else
			{
				if (p1 == 0xF) mt->VolumeDelta += p0;
				else if (p0 == 0xF) mt->VolumeDelta -= p1;
			}
		}
		break;

		/* ====== 8xx - SET PANNING ====== */

		case 0x8:
			mt->Panning = ((int16_t)param - 128) * msyn->Speed;
		break;

		/* ====== 9xx - SAMPLE OFFSET ====== */

		case 0x9:
			mt->TrigOffset += (int32_t)param << 8;
		break;

		/* ====== Axx - VOLUME SLIDE (includes AxF and AFx) ====== */

		case 0xA:
		{
			int16_t p0, p1;

			if (!param) param = mt->Old.VolSlide;   // A00, reuse old parameter
			else mt->Old.VolSlide = param;

			p0 = param >> 4;
			p1 = param & 0xF;

			if ((p0 == 0) || (p1 == 0))    // Normal A0x/Ax0
			{
				if (p0) mt->VolumeDelta += p0 * msyn->Speed;
				if (p1) mt->VolumeDelta -= p1 * msyn->Speed;
			}
			else
			{
				if (p1 == 0xF) mt->VolumeDelta += p0;
				else if (p0 == 0xF) mt->VolumeDelta -= p1;
			}
		}
		break;

		/* ====== Bxx - POSITION JUMP ====== */

		case 0xB:
			msyn->DelPattJump = param;
		break;

		/* ====== Cxx - INSTRUMENT VOLUME ====== */

		case 0xC:
			if (param <= 0x40) mt->Volume = param * msyn->Speed;
		break;

		/* ====== Dxx - PATTERN BREAK ====== */

		case 0xD:
			msyn->DelPattBreak = bcd2bin(param);
		break;

		/* ====== Exx - MISC EFFECTS ====== */

		case 0xE:
			msynth_effect_exx(msyn, mt, param);
		break;

		/* ====== Gxx - GLOBAL VOLUME ====== */

		case 0x10:
			if (param <= 0x40) msyn->GlobalVolume = param;
		break;

		/* ====== Hxx - GLOBAL VOLUME SLIDE ====== */

		case 0x11:
		{
			int16_t p0, p1;

			if (!param) param = msyn->OldGlobalVolSlide;   // H00, reuse old parameter
			else msyn->OldGlobalVolSlide = param;

			p0 = param >> 4;
			p1 = param & 0xF;

			if ((p0 == 0) || (p1 == 0))    // Normal H0x/Hx0
			{
				if (p0) msyn->GlobalVolumeSlide += p0;
				if (p1) msyn->GlobalVolumeSlide -= p1;
			}
		}
		break;

		/* ====== Pxx - PANNING SLIDE (includes future PxF and PFx) ====== */

		case 0x19:
		{
			int16_t p0, p1;

			if (!param) param = mt->Old.PanSlide;   // P00, reuse old parameter
			else mt->Old.PanSlide = param;

			p0 = param >> 4;
			p1 = param & 0xF;

			if ((p0 == 0) || (p1 == 0))    // Normal P0x/Px0
			{
				if (p0) mt->PanningDelta += p0 * msyn->Speed;
				if (p1) mt->PanningDelta -= p1 * msyn->Speed;
			}
			/* DISABLED
			else
			{
				if (p1 == 0xF)   // smooth slide up, PFF
				{
					mt->PanningDelta += p0;
				}
				else if (p0 == 0xF)  // smooth slide down
				{
					mt->PanningDelta -= p1;
				}
			}
			*/
		}
		break;

		/* ====== Vxx - ECHO SWITCH ====== */

		case 0x1F:
		{
			int p0, p1;

			p0 = param >> 4;
			p1 = param & 0xF;

			if (p1 == 0)         // echo on
			{
				switch (p0)
				{
					case 0:  msynth_echo_on_for_track(msyn, mt, DSPV_EchoType_Old);   break;
					case 1:  msynth_echo_on_for_all_tracks(msyn);                     break;
					case 2:  msynth_echo_on_for_track(msyn, mt, DSPV_EchoType_New);   break;
				}
			}
			else if (p1 == 1)    // echo off
			{
				switch (p0)
				{
					case 0:  msynth_echo_off_for_track(mt, DSPV_EchoType_Old);        break;
					case 1:  msynth_echo_off_for_all_tracks(msyn);                    break;
					case 2:  msynth_echo_off_for_track(mt, DSPV_EchoType_New);        break;
				}
			}
		}
		break;

		/* ====== Wxx - ECHO DELAY ====== */

		case 0x20:
		{
			mt->EchoDelay = param;
			msynth_change_echo_params(msyn, mt);
		}
		break;

		/* ====== Xxx - ECHO FEEDBACK ====== */

		case 0x21:
		{
			mt->EchoFeedback = param;
			msynth_change_echo_params(msyn, mt);
		}
		break;

		/* ====== Yxx - ECHO MIX ====== */

		case 0x22:
		{
			mt->EchoMix = param;
			msynth_change_echo_params(msyn, mt);
		}
		break;

		/* ====== Zxx - ECHO CROSS ====== */

		case 0x23:
		{
			mt->EchoCross = param;
			msynth_change_echo_params(msyn, mt);
		}
		break;
	}
}


//==============================================================================================
// msynth_update_callback()
//==============================================================================================

void msynth_update_callback(struct ModSynth *msyn, int sampdelay)
{
	msyn->UEvent.ue_Order = msyn->Order;
	msyn->UEvent.ue_Pattern = msyn->Pattern;
	msyn->UEvent.ue_Row = msyn->Row;
	msyn->UEvent.ue_Delay = sampdelay;
	msyn->UEvent.ue_Tempo = msyn->ChangedTempo;
	msyn->UEvent.ue_Speed = msyn->Speed;
	msyn->ChangedTempo = 0;

	if (msyn->UpdateCallback) msyn->UpdateCallback(msyn->UserData, &msyn->UEvent);
}


//==============================================================================================
// msynth_update_callback_end()
//==============================================================================================

// Called when song has ended (with MMODE_SONG_ONCE) or has been stopped with
// F00 effect. Sends -1 as row, pattern and order.

void msynth_update_callback_end(struct ModSynth *msyn, int sampdelay)
{
	msyn->UEvent.ue_Order = -1;
	msyn->UEvent.ue_Pattern = -1;
	msyn->UEvent.ue_Row = -1;
	msyn->UEvent.ue_Delay = sampdelay;
	msyn->UEvent.ue_Tempo = msyn->ChangedTempo;
	msyn->UEvent.ue_Speed = msyn->Speed;
	msyn->ChangedTempo = 0;

	if (msyn->UpdateCallback) msyn->UpdateCallback(msyn->UserData, &msyn->UEvent);
}


//==============================================================================================
// msynth_next_pattern()
//==============================================================================================

void msynth_next_pattern(struct ModSynth *msyn)
{
	struct DB3ModSong *song;

	song = msyn->Mod->Songs[msyn->Song];

	if (++msyn->Order >= song->NumOrders)    // end of song
	{
		msyn->Order = 0;
		if (msyn->Mode == MMODE_SONG_ONCE) msyn->DelModuleEnd = 1;
	}
	
	msyn->Pattern = song->PlayList[msyn->Order];
	msyn->Row = 0;
}


//==============================================================================================
// msynth_scan_for_speed()
//==============================================================================================

void msynth_scan_for_speed(struct ModSynth *msyn)
{
	struct DB3ModPatt *mptt;
	struct DB3ModEntry *me;
	int track;

	mptt = msyn->Mod->Patterns[msyn->Pattern];
	me = &mptt->Pattern[msyn->Row * msyn->Mod->NumTracks];

	/* Speed/tempo effect scan. */

	for (track = 0; track < msyn->Mod->NumTracks; track++)
	{
		if (me->Cmd1 == 0x0F)
		{
			if (me->Param1 == 0x00) msyn->PatternDelay = 0x7FFFFFFF;  // F00
			else if (me->Param1 < 0x20) msyn->Speed = me->Param1;
			else
			{	
				msyn->Tempo = me->Param1;
				msyn->ChangedTempo = me->Param1;
			}
		}

		if (me->Cmd2 == 0x0F)
		{
			if (me->Param2 == 0x00) msyn->PatternDelay = 0x7FFFFFFF;  // F00
			else if (me->Param2 < 0x20) msyn->Speed = me->Param2;
			else
			{	
				msyn->Tempo = me->Param2;
				msyn->ChangedTempo = me->Param2;
			}
		}

		me++;
	}
}


//==============================================================================================
// msynth_next_row()
//==============================================================================================

void msynth_next_row(struct ModSynth *msyn)
{
	struct DB3ModPatt *mptt;
	struct DB3ModEntry *me;
	int track;

	mptt = msyn->Mod->Patterns[msyn->Pattern];
	me = &mptt->Pattern[msyn->Row * msyn->Mod->NumTracks];

	for (track = 0; track < msyn->Mod->NumTracks; track++)
	{
		struct ModTrack *mt = &msyn->Tracks[track];

		// Let's set trigger counter to $7FFF. As no position can have so many
		// ticks, triggering is for now disabled for this row and track. Do the
		// same for CutCounter, so no cut is set. Disable retriggers as well.

		mt->TrigCounter = 0x7FFFFFFF;
		mt->CutCounter = 0x7FFFFFFF;
		mt->Retrigger = 0;
		mt->PlayBackwards = FALSE;

		// Important! Different combinations of presence of note and instrument
		// number give different results:
		//
		// note instr    instr. retrigger   pitch change   vol. to default
		//
		//  no   no            no              no              no
		//  no  yes       change + no          no             yes
		// yes   no           yes             yes*             no
		// yes  yes       change + yes        yes*            yes
		//
		// (*) Pitch is not set, if any of the two commands is porta to note (3xx)
		// or porta to note + volume slide (5xx).

		// So well, if we have note and instrument, check if instrument is the same
		// as the previosly played one on the track. If not, change instrument.

		if (me->Instr && (me->Instr != mt->Instr)) msynth_instrument(msyn, mt, me->Instr);

		// If we have note, we should change the pitch and set trigger, but not so
		// fast! There are porta to note effects! We have to watch out keyoffs too
		// Do not trigger & pitch if note is > 11. Handle envelope control point
		// instead.

		if (me->Octave)
		{
			if (me->Note < 12)
			{
				int16_t ft_note = ((me->Octave << 3) + (me->Octave << 2) + me->Note) << 3;

				if (!porta_to_note(me))
				{
					mt->Pitch = ft_note;
					mt->Pitch *= msyn->Speed;
					mt->TrigCounter = 0;             // trigger at tick 0, effects can change it later
				}
				else
				{
					mt->Porta3Target = ft_note;
				}
			}
			else
			{
				// Key off. Apply for both volume and panning envelopes, in "loop,
				// sustain A, sustain B" order. If an instruments has neither volume
				// nor panning envelope, cut the channel off.

				if ((mt->VolEnv.Index == 0xFFFF) && (mt->PanEnv.Index = 0xFFFF)) mt->IsOn = FALSE;

				if (mt->VolEnv.Index != 0xFFFF)
				{
					if ((mt->VolEnv.LoopEnd <= mt->VolEnv.SustainA) &&
							(mt->VolEnv.LoopEnd <= mt->VolEnv.SustainB))
							 mt->VolEnv.LoopEnd = 0xFFFF;

					else if ((mt->VolEnv.SustainA <= mt->VolEnv.SustainB))
					 mt->VolEnv.SustainA = 0xFFFF;

					else mt->VolEnv.SustainB = 0xFFFF;
				}

				if (mt->PanEnv.Index != 0xFFFF)
				{
					if ((mt->PanEnv.LoopEnd <= mt->PanEnv.SustainA) &&
							(mt->PanEnv.LoopEnd <= mt->PanEnv.SustainB))
							 mt->PanEnv.LoopEnd = 0xFFFF;

					else if ((mt->PanEnv.SustainA <= mt->PanEnv.SustainB))
					 mt->PanEnv.SustainA = 0xFFFF;

					else mt->PanEnv.SustainB = 0xFFFF;
				}
			}
		}

		// If we have instrument, let's set the default volume and reset trigger offset.

		if (me->Instr)
		{
			msynth_defvolume(msyn, mt);
			mt->TrigOffset = 0;
		}

		// Now is the time for effects.

		if (me->Cmd1 || me->Param1) msynth_effect(msyn, mt, me->Cmd1, me->Param1);
		if (me->Cmd2 || me->Param2) msynth_effect(msyn, mt, me->Cmd2, me->Param2);
		me++;
	}

	// Moving to the next row (or not, depending on playback mode).
	// Nothing happens for MMODE_ROW.

	switch (msyn->Mode)
	{
		case MMODE_MANUAL:
			if (++msyn->Row >= mptt->NumRows) msyn->Row = 0;
			msyn->Mode = MMODE_HALTED;
			msyn->ManualUpdate = TRUE;
		break;

		case MMODE_MANUALBACK:
			if (--msyn->Row < 0) msyn->Row = mptt->NumRows - 1;
			msyn->Mode = MMODE_HALTED;
			msyn->ManualUpdate = TRUE;
		break;

		case MMODE_PATTERN:
			if (++msyn->Row >= mptt->NumRows) msyn->Row = 0;
		break;

		case MMODE_PATTBACK:
			if (--msyn->Row < 0) msyn->Row = mptt->NumRows - 1;
		break;

		case MMODE_SONG:
		case MMODE_SONG_ONCE:
			if (++msyn->Row >= mptt->NumRows) msynth_next_pattern(msyn);
		break;
	}
}


//==============================================================================================
// msynth_post_tick()
//==============================================================================================

void msynth_post_tick(struct ModSynth *msyn)
{
	int track;

	for (track = 0; track < msyn->Mod->NumTracks; track++)
	{
		struct ModTrack *mt = &msyn->Tracks[track];
		int32_t porta_target;

		// Volume slides (accumulated).

		mt->Volume += mt->VolumeDelta;
		if (mt->Volume > msyn->MaxVolume) mt->Volume = msyn->MaxVolume;
		if (mt->Volume < msyn->MinVolume) mt->Volume = msyn->MinVolume;

		// Panning slides (accumulated).

		mt->Panning += mt->PanningDelta;
		if (mt->Panning > msyn->MaxPanning) mt->Panning = msyn->MaxPanning;
		if (mt->Panning < msyn->MinPanning) mt->Panning = msyn->MinPanning;

		// Portamento to note

		if (mt->Porta3Delta)
		{
			porta_target = mt->Porta3Target * msyn->Speed;
			mt->Pitch += mt->Porta3Delta;

			if (mt->Porta3Delta > 0)   // porta to note goes up
			{
				if (mt->Pitch > porta_target) mt->Pitch = porta_target;
			}
			else                       // porta to note goes down
			{
				if (mt->Pitch < porta_target) mt->Pitch = porta_target;
			}
		}

		// Other portamentos.

		mt->Pitch += mt->PitchDelta;

		// Pitch clipping.

		if (mt->Pitch > msyn->MaxPitch) mt->Pitch = msyn->MaxPitch;
		if (mt->Pitch < msyn->MinPitch) mt->Pitch = msyn->MinPitch;
	}

	// Hxx, global volume slide

	msyn->GlobalVolume += msyn->GlobalVolumeSlide;
	if (msyn->GlobalVolume > 64) msyn->GlobalVolume = 64;
	else if (msyn->GlobalVolume < 0) msyn->GlobalVolume = 0;
}


//==============================================================================================
// msynth_do_triggers()
//==============================================================================================

void msynth_do_triggers(struct ModSynth *msyn)
{
	int track;

	for (track = 0; track < msyn->Mod->NumTracks; track++)
	{
		struct ModTrack *mt = &msyn->Tracks[track];

		if (mt->Instr)
		{
			if (mt->TrigCounter == 0)
			{
				msynth_trigger(msyn->Mod, mt);
				mt->TrigCounter = mt->Retrigger;
			}
			mt->TrigCounter--;

			// Note that note cut does not switch the channel off, the note is being
			// continued with 0 volume.

			if (mt->CutCounter-- <= 0) mt->Volume = 0;
		}
	}
}


//==============================================================================================
// msynth_clear_slides()
//==============================================================================================

void msynth_clear_slides(struct ModSynth *msyn)
{
	int track;

	// Scale back current volume/pan to PT units and pitch to finetunes.
	// Clear deltas.
	// Clear appregio table entries 1 and 2 (entry 0 is always cleared).
	// Clear appregio (global) counter
	// Clear vibrato

	for (track = 0; track < msyn->Mod->NumTracks; track++)
	{
		struct ModTrack *mt = &msyn->Tracks[track];

		mt->VolumeDelta = 0;
		mt->PanningDelta = 0;
		mt->PitchDelta = 0;
		mt->Porta3Delta = 0;
		mt->Volume /= msyn->Speed;
		mt->Panning /= msyn->Speed;
		mt->Pitch /= msyn->Speed;

		/* Do not clear appregio and vibrato when sequencer is halted (single step mode). */

		if (msyn->Mode != MMODE_HALTED)
		{
			mt->ApprTable[1] = 0;
			mt->ApprTable[2] = 0;
			mt->VibratoSpeed = 0;
			mt->VibratoDepth = 0;
		}
	}

	// Hxx

	msyn->GlobalVolumeSlide = 0;

	/* Do not clear appregio counter when sequencer is halted (single step mode). */

	if (msyn->Mode != MMODE_HALTED) msyn->ApprCounter = 0;
}


//==============================================================================================
// msynth_setup_slides()
//==============================================================================================

void msynth_setup_slides(struct ModSynth *msyn)
{
	int track;

	// Scale current volume, panning, pitch with current speed.

	for (track = 0; track < msyn->Mod->NumTracks; track++)
	{
		struct ModTrack *mt = &msyn->Tracks[track];

		mt->Volume *= msyn->Speed;
		mt->Panning *= msyn->Speed;
		mt->Pitch *= msyn->Speed;
	}

	// Calculate limits.

	msyn->MinVolume = 0;
	msyn->MinPanning = -(msyn->Speed << 7);
	msyn->MinPitch = msyn->Speed * 96;

	msyn->MaxVolume = msyn->Speed << 6;
	msyn->MaxPanning = msyn->Speed << 7;
	msyn->MaxPitch = msyn->Speed * 864;
}


//==============================================================================================
// msynth_tick_gains_and_pitch()
//==============================================================================================

void msynth_tick_gains_and_pitch(struct ModSynth *msyn)
{
	int track;

	for (track = 0; track < msyn->Mod->NumTracks; track++)
	{
		struct ModTrack *mt = &msyn->Tracks[track];
		int32_t vol, volc, pan, pitch, p2;
		int16_t p1;
		struct DSPTag tags[2] = { 
			{ DSPA_Panning, 0 },
			{ TAG_END, 0},
		};

		pitch = mt->Pitch;
		pitch += mt->ApprTable[msyn->ApprCounter];
		pitch += Vibrato[mt->VibratoCounter] * mt->VibratoDepth >> 8;
		mt->VibratoCounter += mt->VibratoSpeed;
		mt->VibratoCounter &= 0x3F;
		msynth_pitch(msyn, mt, pitch);


		// Convert [PT * speed] units of accumulated volume slide to 14-bit gain.
		// Apply panning then.
		// mt->Volume               <0, +64 * speed>    |  volc      <0, +16384>
		// mt->Panning   <-128 * speed, +128 * speed>   |  pan  <-16384, +16384>

		volc = ((int32_t)mt->Volume << 8) / msyn->Speed;
		pan = (int32_t)((uint32_t)mt->Panning << 7) / msyn->Speed;

		// Apply envelopes. Volume envelope is just multiplied with the current
		// gain and renormalized (gain: <0, +16384>, vol envelope <0, +16384>).
		// Panning envelope is a bit more complicated. Panning envelope center is
		// moved to the current panning, then pan envelope range is shrinked to
		// fit the final panning range (if the current panning is 0, there is no
		// shrinking, when panning is moving to the left or right channel,
		// shrinking increases, for full L/R panning, envelope range is reduced
		// to 0. The exact formula is:
		//
		// p = p0 + (1.0 - abs(p0)) * ev
		//
		// where:
		//  p0 - panning before applying envelope (result of default panning and
		//       panning effects.
		//  ev - current value of panning envelope in full range
		//  p  - final panning value
		//
		// The value above is for panning normalized to <-1.0, +1.0>. We use fixed
		// point math here, so some scaling is used.
		//  p0       <-16384, +16384>
		//  ev       <-16384, +16384>
		//  p        <-16384, +16384>

		volc *= mt->VolEnvCurrent;
		volc >>= 14;
		volc *= msyn->GlobalVolume;
		volc >>= 6;

		p1 = (pan > 0) ? pan : -pan;     // abs
		p1 = 16384 - p1;
		p2 = p1 * mt->PanEnvCurrent >> 14;
		pan += p2;

		vol = volc;

		if (pan > 0)
		{
			vol *= 16384 - pan;
			vol >>= 14;
		}

		mt->GainL = vol;

		vol = volc;

		if (pan < 0)
		{
			vol *= 16384 + pan;
			vol >>= 14;
		}

		mt->GainR = vol;

		// Send panning to the panoramizer for phase augmented panning.

		tags[0].dspt_data = mt->Panning / msyn->Speed;
		msynth_dsp_set_instr_attrs(mt, tags);
	}
}


//==============================================================================================
// msynth_volenv_interpolator()
//==============================================================================================

int16_t msynth_volenv_interpolator(struct EnvInterp *evi, struct DB3ModEnvelope *mde)
{
	if (evi->TickCtr == 0)   // end of section, fetch next one
	{
		if (evi->Section == evi->LoopEnd) evi->Section = mde->LoopFirst;
		evi->YStart = mde->Points[evi->Section].Value << 8;

		if (evi->Section == evi->SustainA) return evi->YStart;
		else if (evi->Section == evi->SustainB) return evi->YStart;
		else if (evi->Section >= mde->NumSections) return evi->YStart;
		else
		{
			evi->XDelta = mde->Points[evi->Section + 1].Position - mde->Points[evi->Section].Position;
			evi->TickCtr = evi->XDelta;
			evi->YDelta = (mde->Points[evi->Section + 1].Value << 8) - evi->YStart;
			evi->Section++;
		}
	}

	evi->PrevVal = evi->YStart + (evi->YDelta * (evi->XDelta - evi->TickCtr--)) / evi->XDelta;

	return evi->PrevVal;
}


//==============================================================================================
// msynth_panenv_interpolator()
//==============================================================================================

int16_t msynth_panenv_interpolator(struct EnvInterp *evi, struct DB3ModEnvelope *mde)
{
	if (evi->TickCtr == 0)   // end of section, fetch next one
	{
		if (evi->Section == evi->LoopEnd) evi->Section = mde->LoopFirst;
		evi->YStart = (int16_t)(((uint16_t)mde->Points[evi->Section].Value) << 7); // mvtiaine: fixed UB
		if (evi->Section == evi->SustainA) return evi->YStart;
		else if (evi->Section == evi->SustainB) return evi->YStart;
		else if (evi->Section >= mde->NumSections) return evi->YStart;
		else
		{
			evi->XDelta = mde->Points[evi->Section + 1].Position - mde->Points[evi->Section].Position;
			evi->TickCtr = evi->XDelta;
			evi->YDelta = (int16_t)((uint16_t)mde->Points[evi->Section + 1].Value << 7) - evi->YStart; // mvtiaine: fixed UB
			evi->Section++;
		}
	}

	evi->PrevVal = evi->YStart + (evi->YDelta * (evi->XDelta - evi->TickCtr--)) / evi->XDelta;

	return evi->PrevVal;
}


//==============================================================================================
// msynth_do_envelopes()
//==============================================================================================

void msynth_do_envelopes(struct ModSynth *msyn)
{
	int16_t track;

	for (track = 0; track < msyn->Mod->NumTracks; track++)
	{
		struct ModTrack *mt = &msyn->Tracks[track];

		if (mt->IsOn && (mt->VolEnv.Index != 0xFFFF))
		{
			mt->VolEnvCurrent = msynth_volenv_interpolator(&mt->VolEnv, &msyn->Mod->VolEnvs[mt->VolEnv.Index]);
		}

		if (mt->IsOn && (mt->PanEnv.Index != 0xFFFF))
		{
			mt->PanEnvCurrent = msynth_panenv_interpolator(&mt->PanEnv, &msyn->Mod->PanEnvs[mt->PanEnv.Index]);
		}
	}
}


//==============================================================================================
// msynth_next_tick()
//==============================================================================================

int msynth_next_tick(struct ModSynth *msyn, unsigned long bufdelay)
{
	int bpm2, samples, stop = 0;

	msynth_post_tick(msyn);

	if (msyn->Tick == 0)
	{
		if (msyn->PatternDelay > 0)
		{
			// EEx can delay up to 15 ticks and is not additive. Any higher
			// delay must be caused by either song end in MMODE_SONG_ONCE mode or
			// F00 effect.

			if (msyn->PatternDelay-- > 15)
			{
				msynth_update_callback_end(msyn, bufdelay);
				stop = 1;
			}
		}
		else
		{
			msynth_apply_delayed(msyn);

			// When sequencer is halted, no position updates are sent, with exception of the
			// first tick zero after going from MANUAL to HALTED state. The ManualUpdate flag
			// is set in msynth_next_row() when MANUAL state is turned into HALTED.

			if (msyn->Mode == MMODE_HALTED)
			{
				if (msyn->ManualUpdate)
				{
					msyn->ManualUpdate = FALSE;
					msynth_update_callback(msyn, bufdelay);
				}
			}
			else msynth_update_callback(msyn, bufdelay);

			msynth_clear_slides(msyn);
			
			if (msyn->NewTempo)
			{
				msyn->Tempo = msyn->NewTempo;
				msyn->NewTempo = 0;
			}

			if (msyn->Mode != MMODE_HALTED) msynth_scan_for_speed(msyn);
			msynth_setup_slides(msyn);
			if (msyn->Mode != MMODE_HALTED) msynth_next_row(msyn);
		}
	}

	msynth_do_triggers(msyn);
	msynth_do_envelopes(msyn);
	msynth_tick_gains_and_pitch(msyn);

	if (++msyn->ApprCounter > 2) msyn->ApprCounter = 0;

	// Calculate this tick length in samples. It can be done by multiplying tick
	// length in seconds by mix frequency. Tick length in seconds is derived from
	// BPM. In case when tick length in samples is not integer, TickSamplesLo
	// accumulates the fractional part, then tick is extended with one sample
	// when accumulated fraction is higher than one. Of course TickSamplesLo only
	// holds fraction numerator, denominator is pd->BeatsPerMinute * 2.

	bpm2 = msyn->Tempo << 1;
	samples =	5 * msyn->MixFreq;
	msyn->TickSamplesHi = samples / bpm2;
	msyn->TickSamplesLo += samples % bpm2;

	if (msyn->TickSamplesLo > bpm2)
	{
		msyn->TickSamplesLo -= bpm2;
		msyn->TickSamplesHi++;
	}

	if (++msyn->Tick == msyn->Speed) msyn->Tick = 0;

	return stop;
}


//==============================================================================================
// msynth_init_tracks()
//==============================================================================================

// Initializes all tracks. Performs following operations:
// - Disables and unmutes track.
// - Sets instrument to 0.
// - Sets pitch to 0.
// - Resets appregiator and vibrato counters.
// - Resets porta-to-note target to C-4.
// - Resets volume/panning slide value storage (for x00) to 0.
// - Resets playback direction to forward.
// - Initializes list of DSP blocks.

void msynth_init_tracks(struct ModSynth *msyn, int unmute)
{
	int16_t track;

	for (track = 0; track < msyn->Mod->NumTracks; track++)
	{
		struct ModTrack *mt = &msyn->Tracks[track];
		struct DSPObject *fetchinstr;

		mt->Instr = 0;
		mt->IsOn = 0;
		mt->Muted = 1;
		if (unmute) mt->Muted = 0;
		mt->Pitch = 0;
		mt->ApprTable[0] = 0;     // entries 1 and 2 are cleared before every position
		mt->Porta3Target = 576;   // C-4
		mt->Old.VolSlide = 0;
		mt->Old.PanSlide = 0;
		mt->PlayBackwards = FALSE;
		mt->VibratoCounter = 0;
		mt->TrigCounter = 0x7FFFFFFF;
		mt->CutCounter = 0x7FFFFFFF;
		mt->Retrigger = 0;
		mt->VolumeDelta = 0;
		mt->PanningDelta = 0;
		mt->PitchDelta = 0;
		mt->Porta3Delta = 0;
		mt->ApprTable[1] = 0;
		mt->ApprTable[2] = 0;
		mt->VibratoSpeed = 0;
		mt->VibratoDepth = 0;
		mt->Volume = 0;
		mt->Panning = 0;

		INIT_LIST(&mt->DSPInstrChain);
		INIT_LIST(&mt->DSPTrackChain);

		/* A FetchInstr DSP object should be inserted always as the first element of the track DSP chain. It    */
		/* connects instrument DSP chain (variable, changed after every trigger) with track DSP chain (static). */

		fetchinstr = dsp_fetchinstr_new(&mt->DSPInstrChain);
		DB3AddTail(&mt->DSPTrackChain, (struct MinNode*)fetchinstr);

		/* Echo should be enabled or disabled depending on effect mask in the module. Default echo parametrs    */
		/* are initialized for all tracks however in case echo is turned on later with 'V' command.             */

		mt->EchoDelay = msyn->Mod->DspDefaults.EchoDelay;
		mt->EchoFeedback = msyn->Mod->DspDefaults.EchoFeedback;
		mt->EchoMix = msyn->Mod->DspDefaults.EchoMix;
		mt->EchoCross = msyn->Mod->DspDefaults.EchoCross;

		if (msyn->Mod->DspDefaults.EffectMask[track] & DSP_MASK_ECHO) msynth_echo_on_for_track(msyn, mt, DSPV_EchoType_Old);
		else mt->EchoType = 0;
	}
}


//==============================================================================================
// msynth_reset()
//==============================================================================================

void msynth_reset(struct ModSynth* msyn, int unmute)
{
	msyn->Tick = 0;
	msyn->Speed = 6;
	msyn->Tempo = 125;
	msyn->NewTempo = 0;
	msyn->ChangedTempo = 0;
	msyn->TickSamplesHi = 0;
	msyn->TickSamplesLo = 0;
	msyn->PatternDelay = 0;
	msyn->Pattern = 0;
	msyn->Row = 0;
	msyn->Order = 0;
	msyn->Song = 0;
	//msyn->Mode = MMODE_HALTED;
	msyn->Mode = MMODE_SONG_ONCE;
	msyn->GlobalVolume = 64;
	msyn->GlobalVolumeSlide = 0;
	msyn->ApprCounter = 0;
	msyn->MinVolume = 0;
	msyn->MinPanning = -768;
	msyn->MinPitch = 576;
	msyn->MaxVolume = 384;
	msyn->MaxPanning = 768;
	msyn->MaxPitch = 4608;             // up to the start of 8-th octave
	msyn->ManualUpdate = FALSE;
	msynth_reset_delayed(msyn);
	msynth_reset_loop(msyn);
	msynth_init_tracks(msyn, unmute);
}


//==============================================================================================
// msynth_mix_track_in()
//==============================================================================================

void msynth_mix_track_in(struct ModSynth *msyn, unsigned int track, int32_t *accu, unsigned long frames)
{
	struct ModTrack *mt = &msyn->Tracks[track];
	struct DSPObject *dspo;

	// Skip inactive track immediately.

	if (!mt->IsOn) return;

	// Just pull needed frames from the last DSP object on the track to PreMixBuf.
	// Then PreMixBuf gets mixed into Accumulator. If Pull() returns 0, it means
	// instrument has finished playing, so the track is turned off.

	dspo = (struct DSPObject*)mt->DSPTrackChain.mlh_TailPred;

	if (dspo->dsp_next)    // The chain is not empty?
	{
		uint32_t i;
		int16_t *premix = msyn->PreMixBuf;

		mt->IsOn = dspo->dsp_pull(dspo, premix, frames);

		// Mixing. Volume effects, panning, envelopes are applied and result in
		// left and right gains (signed 14-bit values) for both channels. Sample
		// is multiplied by gains, shifted 6 bits right (as I can have up to 256
		// channels), then added to left and right channel.

		if (!mt->Muted)
		{
			for (i = 0; i < frames; i++)
			{
				int32_t left, right;

				// PreMix buffer contains stereo samples, because panoramizer splits channels.

				left = *premix++ * mt->GainL;
				right = *premix++ * mt->GainR;
				*accu++ += left >> 14;
				*accu++ += right >> 14;
			}
		}
	}
}


//==============================================================================================
// msynth_accumulator_clear()
//==============================================================================================

void msynth_accumulator_clear(struct ModSynth *msyn, uint32_t frames)
{
	int32_t *p = msyn->Accumulator;

	while (frames--)
	{
		*p++ = 0;
		*p++ = 0;
	}
}


//==============================================================================================
// msynth_accumulator_flush()
//==============================================================================================

void msynth_accumulator_flush(struct ModSynth *msyn, uint32_t frames, int16_t *out)
{
	int32_t *p = msyn->Accumulator;
	uint32_t ctr = frames;

	while (ctr--)
	{
		int32_t s;

		s = *p++;                          // left
		if (s > msyn->BoostLimit) *out = 0x7FFF;
		else if (s < -msyn->BoostLimit) *out = 0x8001;
		else *out = s * msyn->BoostMultiplier >> 16;
		out++;

		s = *p++;                          // right
		if (s > msyn->BoostLimit) *out = 0x7FFF;
		else if (s < -msyn->BoostLimit) *out = 0x8001;
		else *out = s * msyn->BoostMultiplier >> 16;
		out++;
	}
}



//==============================================================================================
// msynth_boost_multiplier()
//==============================================================================================

int32_t msynth_boost_multiplier(int16_t db_boost, int16_t chans)
{
	int64_t b = 0x00010000;

	while (db_boost >= 6) { db_boost -= 6; b <<= 1; }
	while (db_boost < 0) { db_boost += 6; b >>= 1; }
	b	*= DbVals[db_boost];
	b	/= chans;
	b	>>= 16;
	return (int32_t)b;
}


/* === API === */

/****** libdigibooster3/DB3_NewEngine ***************************************
*
* NAME
*   DB3_NewEngine() -- creates and setups a new module synthesizer.
*
* SYNOPSIS
*   void* DB3_NewEngine(struct DB3Module *mod, uint32_t mixfreq, uint32_t
*   maxbuf);
*
* FUNCTION
*   Creates and setups a new module synthesizer for passed module, mixing
*   frequency and declared maximum mising buffer size. Allocates all buffers
*   needed.
*
* INPUTS
*   mod - complete music module as defined in "musicmodule.h". NULL is safe,
*     function just returns NULL.
*   mixfreq - downmix sampling frequency in Hz. Allowed frequencies are 8 kHz
*     to 192 kHz (including).
*   maxbuf - maximum number of frames that will be requested in DB3_Mix()
*     calls. Note that some internal buffers are allocated based on this
*     value, so setting it high may result in unexpected memory consumption.
*     Values up to mixing frequency (which yields one second buffer) are
*     reasonable. Very short buffers are allowed (yes, even one single
*     frame), but highly ineffective. Zero passed here causes the function to
*     quit immediately with NULL result.
*
* RESULT
*   An opaque pointer to the new module synthesizer, or NULL in case of wrong
*   arguments or memory shortage.
*
* SEE ALSO
*   DB3_Mix()
*
*****************************************************************************
*
*/

void* DB3_NewEngine(struct DB3Module *m, uint32_t mixfreq, uint32_t bufsize)
{
	struct ModSynth *msyn = NULL;

	if (m && bufsize && (mixfreq >= 8000) && (mixfreq <= 192000))
	{
		if (msyn = db3_malloc(sizeof(struct ModSynth)))
		{
			if (msyn->Accumulator = db3_malloc(bufsize << 3))
			{
				if (msyn->PreMixBuf = db3_malloc(bufsize << 2))     
				{
					if (msyn->Tracks = db3_malloc(m->NumTracks * sizeof(struct ModTrack)))
					{
						msyn->Mod = m;
						msyn->MixFreq = mixfreq;
						msyn->UpdateCallback = NULL;
						msynth_reset(msyn, TRUE);
						generate_panoramizer_phase_table(msyn->PanPhaseTable, mixfreq);
						DB3_SetVolume(msyn, 0);
						DB3_SetPos(msyn, 0, 0, 0);
						return (void*)msyn;
					}

					db3_free(msyn->PreMixBuf);
				}

				db3_free(msyn->Accumulator);
			}

			db3_free(msyn);
		}
	}

	return NULL;
}


/****** libdigibooster3/DB3_SetCallback() ***********************************
*
* NAME
*   DB3_SetCallback() -- Sets a position update callback function for a
*   synthesizer.
*
* SYNOPSIS
*   void DB3_SetCallback(void *engine, void(*callback)(void*, struct
*   UpdateEvent*), void *userdata);
*
* FUNCTION
*   This function is provided for players that want to know exact time of
*   position changes in a module. Useful for example for displaying current
*   pattern/row number, scrolling pattern display or synchronizing something
*   to the music. Using this callback it is possible to get position change
*   timing with single downmix frame accuracy. The callback gets number of
*   row, pattern, song order and delay, measured in downmix frames relative
*   to the start of last module fragment rendered with ModSynth_Mix(). To get
*   realtime trigger, audio interrupts or timers may be used, depending on
*   features available on host. To convert the delay to seconds one just
*   divides delay in frames by mixing frequency.
*
* INPUTS
*   engine - an opaque pointer to the module synthesizer created with
*     DB3_NewEngine(). NULL is safe, the function does nothing in the case.
*
*   callback - a callback function defined as:
*
*     void callback(void *udata, struct UpdateEvent *uevent);
*
*     where:
*       udata - may be anything. This is just a pointer passed as userdata to
*         DB3_SetCallback(), untouched.
*       uevent - pointer to a read-only UpdateEvent structure:
*         From "libdigibooster3.h":
*
*         struct UpdateEvent
*         {
*           int32_t ue_Order;     // number of order in the playlist
*           int32_t ue_Pattern;   // ordinal number of pattern
*           int32_t ue_Row;       // number of row (position) in the pattern
*           int32_t ue_Delay;     // event delay in sample frames
*           int32_t ue_Tempo;     // current module tempo ("CIA tempo"), BPM
*           int32_t ue_Speed;     // current module speed (ticks per row)
*           int32_t ue_SeqHalted; // TRUE if sequencer is halted
*         };
*
*     NULL callback is safe and is considered as turning the callback feature
*     off. Callback may be turned on and off (or changed) as needed.
*
*   userdata - any value, just passed to the callback as described above.
*
* RESULT
*   None.
*
* EXAMPLE
*   A step by step instruction how to get realtime position change triggers
*   using Amiga timers:
*   - Be prepared that one call to ModSynth_Mix() will call your callback
*     more than one time.
*   - In the callback convert delay from samples to timer units (usually
*     EClocks or CPU clocks). Prepare a timer request for every callback.
*   - Get current system clock.
*   - Push the rendered buffer to audio hardware.
*   - Add current clock to requests and sent them all to timer (WAITECLOCK or
*     WAITCPUCLOCK unit).
*   - while buffer is playing, wait for timer requests to be replied and take
*     desired actions.
*   There may be some additional delay if an event occurs just a few frames
*   after buffer start (adding time and sending requests takes some CPU
*   cycles). It may be compensated by using triple buffering and sending time
*   requests one buffer earlier (a buffer length in frames should be added to the
*   delay before conversion to timer units).
*
* SEE ALSO
*   DB3_Mix()
*
*****************************************************************************
*
*/

void DB3_SetCallback(void *msyn0, void(*callback)(void*, struct UpdateEvent*), void *userdata)
{
	struct ModSynth *msyn = (struct ModSynth*)msyn0;

	msyn->UserData = userdata;
	msyn->UpdateCallback = callback;
}


/****** libdigibooster3/DB3_SetVolume() *************************************
*
* NAME
*   DB3_SetVolume() -- sets the current master volume level.
*
* SYNOPSIS
*   void DB3_SetVolume(void *engine, int16_t level);
*
* FUNCTION
*   The function sets the master volume level in decibels. 0 dB level is
*   defined in the following way: if one assumes full amplitude on all the
*   tracks in the module, the final downmix will have full amplitude too.
*   Then 0 dB and below is overdrive-safe. Master volume above 0 dB may cause
*   overdrive, which is then saturated. The allowed range is from -84 dB to
*   +24 dB including.
*
*  Master volume level operates inside the internal 32-bit mixer. Then
*  adjusting volume with this function gives less quantization noise than
*  scaling the output 16-bit stream.
*
* INPUTS
*   engine - an opaque pointer to the module synthesizer created with
*     DB3_NewEngine(). Must not be NULL.
*   level - master volume level in decibels. Must be inside <-84, +24> range.
*
* RESULT
*   None.
*
*****************************************************************************
*
*/

void DB3_SetVolume(void *msyn0, int16_t level)
{
	struct ModSynth *msyn = (struct ModSynth*)msyn0;

	if (msyn->BoostMultiplier = msynth_boost_multiplier(level, msyn->Mod->NumTracks))
	{
		msyn->BoostLimit = 0x7FFFFFFF / msyn->BoostMultiplier;
	}
	else msyn->BoostLimit = 0x7FFFFFFF;
}


/****** libdigibooster3/DB3_SetPos() ****************************************
*
* NAME
*   DB3_SetPos() -- Sets the current sequencer position.
*
* SYNOPSIS
*   void DB3_SetPos(void *modsynth, uint32_t song, uint32_t order, uint32_t
*   row);
*
* FUNCTION
*
* INPUTS
*   engine - an opaque pointer to the module synthesizer created with
*     DB3_NewEngine(). Must not be NULL.
*   song - number of song in the module. Starting from 0.
*   order - number of playlist order in the song. Starting from 0.
*   row - number of row in the pattern selected with order. Starting from 0.
*
* RESULT
*   None.
*
* NOTES
*   All three parameters *must* specify a valid position inside the module.
*   If not, the sequencer will read random data and probably crash.
*
*****************************************************************************
*
*/

void DB3_SetPos(void *msyn0, uint32_t song, uint32_t order, uint32_t row)
{
	struct ModSynth *msyn = (struct ModSynth*)msyn0;

	msyn->Song = song;
	msyn->Order = order;
	msyn->Pattern = msyn->Mod->Songs[song]->PlayList[order];
	msyn->Row = row;
	msyn->Tick = 0;
}


/****** libdigibooster3/DB3_Mix() *******************************************
*
* NAME
*   DB3_Mix() -- Mixes down a next chunk of module.
*
* SYNOPSIS
*   uint32_t DB3_Mix(void *engine, uint32_t frames, int16_t *output);
*
* FUNCTION
*   Synthetises and mixes frame chunk of module sound. Mixed frames are
*   stored in 'output'. If a position callback is defined, it is called for
*   every module position change within rendered audio frames range.
*   Following calls to this function need not to use the same number of
*   audio frames.
*
* INPUTS
*   engine - a blackbox pointer to the synthesizer engine.
*   frames - number of audio frames to render. Must not be higher than buffer
*     size declared in DB3_NewEngine(), will be clipped if higher. Passing 0
*     is OK (function returns immediately).
*   output - buffer for rendered frames. Note that frames are stereo, so the
*     buffer must have at least (4 * frames) bytes. It must point to a valid
*     buffer.
*
* RESULT
*   Number of valid frames in the buffer. It may be less than 'frames' in
*   case the sequencer has been stopped.
*
* SEE ALSO
*   DB3_NewEngine(), DB3_SetCallback()
*
*****************************************************************************
*
*/

uint32_t DB3_Mix(void *msyn0, uint32_t frames, int16_t *out)
{
	struct ModSynth *msyn = (struct ModSynth*)msyn0;
	uint32_t frame_counter = 0;
	unsigned long frames_left = frames;
	int32_t *accu = msyn->Accumulator;
	int stop = 0;

	msynth_accumulator_clear(msyn, frames);

	while (!stop && frames_left)
	{
		uint32_t frame_chunk;
		int16_t track;

		if (msyn->TickSamplesHi == 0) stop = msynth_next_tick(msyn, frame_counter);
		frame_chunk = msyn->TickSamplesHi;
		if (frame_chunk > frames_left) frame_chunk = frames_left;

		// Resampling and mixing.

		for (track = 0; track < msyn->Mod->NumTracks; track++)
		{
			msynth_mix_track_in(msyn, track, accu, frame_chunk);
		}

		accu += frame_chunk << 1;              // shift beacuse 'accu' is stereo.
		frames_left -= frame_chunk;
		msyn->TickSamplesHi -= frame_chunk;
		frame_counter += frame_chunk;
	}

	msynth_accumulator_flush(msyn, frames, out);
	return frame_counter;
}


/****** libdigibooster3/DB3_DisposeEngine() *********************************
*
* NAME
*   DB3_DisposeEngine() -- disposes a module synthesizer.
*
* SYNOPSIS
*   void DB3_DisposeEngine(void *engine);
*
* FUNCTION
*   Disposes a module synthesizer, frees all resources.
*
* INPUTS
*   engine - a blackbox pointer to the synthesizer engine. Passing NULL is
*     OK (function returns immediately).
*
* RESULT
*   None.
*
* SEE ALSO
*   DB3_NewEngine()
*
*****************************************************************************
*
*/

void DB3_DisposeEngine(void *msyn0)
{
	struct ModSynth *msyn = (struct ModSynth*)msyn0;

	if (msyn)
	{
		int16_t track;

		// Dispose all DSP chains.

		for (track = 0; track < msyn->Mod->NumTracks; track++)
		{
			struct ModTrack *mt = &msyn->Tracks[track];
			msynth_dsp_dispose_chain(&mt->DSPTrackChain);
			msynth_dsp_dispose_chain(&mt->DSPInstrChain);
		}

		// Free tables.

		db3_free(msyn->Tracks);
		db3_free(msyn->PreMixBuf);
		db3_free(msyn->Accumulator);
		db3_free(msyn);
	}
}
