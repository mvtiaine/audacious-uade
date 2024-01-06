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

#ifndef LIBDIGIBOOSTER3_DSP_H
#define LIBDIGIBOOSTER3_DSP_H

/* DSP routines of player (mixer, resampler, echo). */

#ifndef TARGET_WIN32
#include <stdint.h>
#else
#include "../stdint.h"
#endif

#include "lists.h"

#ifndef TAG_END
#define TAG_END 0
#endif


struct DSPTag
{
	uint32_t dspt_tag;
	int32_t  dspt_data;
};


struct DSPObject
{
	struct DSPObject *dsp_next;
	struct DSPObject *dsp_prev;
	int(*dsp_pull)(struct DSPObject*, int16_t*, int32_t);
	void(*dsp_dispose)(struct DSPObject*);
	void(*dsp_set)(struct DSPObject*, struct DSPTag*);
	int(*dsp_get)(struct DSPObject*, uint32_t tag, int32_t *storage);
	void(*dsp_flush)(struct DSPObject*);
	int dsp_type;
};


/*-----------------------------*/
/* Constructors of DSP objects */
/*-----------------------------*/

struct DSPObject *dsp_sampled_instr_new(int16_t *data, int32_t loop_start, int32_t loop_len, int32_t loop_count, int32_t total_frames, int loop_type);
struct DSPObject *dsp_resampler20_new(void);
struct DSPObject *dsp_panoramizer_new(int16_t *phase_table);
struct DSPObject *dsp_fetchinstr_new(struct MinList *instr_chain);
struct DSPObject *dsp_echo_new(int mixfreq, int type);
struct DSPObject *dsp_zeropadder_new(int padframes);

/*------------------------------------*/
/* Some functions global to ModSynth. */
/*------------------------------------*/

void generate_panoramizer_phase_table(int16_t *phase_table, int mixfreq);

// Types of DSP objects

#define DSPTYPE_UNROLLER             1    // wavetable with loops (source)
#define DSPTYPE_RESAMPLER            2    // a resampler (filter)
#define DSPTYPE_PANORAMIZER          3    // adds volume/panning, creates stereo signal from mono
#define DSPTYPE_ECHO                 4    // echo module
#define DSPTYPE_FETCHINSTR           5    // connector between instrument chain and track chain
#define DSPTYPE_ZEROPADDER           6    // padding for resampler

/*----------------------------*/
/* Attributes of DSP objects. */
/*----------------------------*/

#define DSPA_ResamplerRatio          1    // for resamplers
#define DSPA_SampleOffset            2    // for unroller
#define DSPA_LoopType                3    // for unroller
#define DSPA_LoopFirst               4    // for unroller
#define DSPA_LoopLast                5    // for unroller
#define DSPA_ReversePlay             6    // for unroller
#define DSPA_LimitOffsetToLoop       7    // for unroller
#define DSPA_GainLeft                8    // for panner, includes volume and amplitude panning
#define DSPA_GainRight               9    // for panner, includes volume and amplitude panning
#define DSPA_Panning                10    // for panner, used for phase panning
#define DSPA_EchoDelay              11    // for echo module
#define DSPA_EchoFeedback           12    // for echo module
#define DSPA_EchoMix                13    // for echo module
#define DSPA_EchoCross              14    // for echo module
#define DSPA_EchoType               15    // for echo module

/*------------------------------------*/
/* Special values for DSP attributes. */
/*------------------------------------*/

#define DSPV_EchoType_Old            1    // uses global echo parameters
#define DSPV_EchoType_New            2    // uses individual echo parameters

#endif   /* LIBDIGIBOOSTER3_DSP_H */
