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

/* Stereo panoramizer. */

#include "libdigibooster3.h"
#include "dsp.h"


// Panoramizer implements panoraming by phase shifting. Note that it does not change amplitude. Amplitude change is merged with
// all the volume effects and applied in the mixer. IMPORTANT: panoramizer takes mono input and produces interleaved stereo
// output.


// Panoramizer object structure.

struct Panoramizer
{
	struct DSPObject object;
	int16_t DelBuf[1024 + 64];
	int DelL;                      // Delay in frames for left
	int DelR;                      // Delay in frames for right
	int16_t *PhaseTable;           // pointer to the phase table in ModSynth structure
};


// A global function for calculating panorama->delay table. This trigonometric equation is approximated by quadratic equation.
// Generated table is stored in 'msynth' structure, as it is global for all the channels and instruments. The table cannot be
// hardcoded in the code, because it depends on the mixing frequency. The function is y = a*x^2, with the assumption, that for
// +- 128 the delay is (by default) 0.333 ms, independent of mixing frequency. The function is symmetrical to Y axis, so only
// one half is stored.


void generate_panoramizer_phase_table(int16_t *phase_table, int mixfreq)
{
	int c, i;
	
	c = mixfreq * MAX_STEREO_PHASE_SHIFT_USEC / 1000000;      // maximum phase shift in frames
	
	for (i = 1; i <= 128; i++) phase_table[i - 1] = (i * i * c) >> 14;
}


//==============================================================================================
// dsp_panoramizer_set()
//==============================================================================================

void dsp_panoramizer_set(struct DSPObject *obj0, struct DSPTag *tags)
{
	struct Panoramizer *obj = (struct Panoramizer*)obj0;

	while (tags->dspt_tag)
	{
		switch (tags->dspt_tag)
		{
			case DSPA_Panning:
				obj->DelL = 0;
				obj->DelR = 0;
				if (tags->dspt_data < 0) obj->DelR = obj->PhaseTable[-tags->dspt_data - 1];
				else if (tags->dspt_data > 0) obj->DelL = obj->PhaseTable[tags->dspt_data - 1];
			break;
		}

		tags++;
	}
}



//==============================================================================================
// dsp_panoramizer_pull()
//==============================================================================================

int dsp_panoramizer_pull(struct DSPObject *obj0, int16_t *dest, int32_t frames)
{
	int leave_active = TRUE;
	struct Panoramizer *obj = (struct Panoramizer*)obj0;
	struct DSPObject *prev;

	prev = obj->object.dsp_prev;

	while (frames)
	{
		int i, chunk = frames;
		
		if (chunk > 1024) chunk = 1024;
		
		leave_active = prev->dsp_pull(prev, &obj->DelBuf[64], chunk);
		
		for (i = 0; i < chunk; i++)
		{
			*dest++ = obj->DelBuf[i + 64 - obj->DelL];
			*dest++ = obj->DelBuf[i + 64 - obj->DelR];
		}
		
		for (i = 0; i < 64; i++) obj->DelBuf[i] = obj->DelBuf[chunk + i];
		
		frames -= chunk;
	}

	return leave_active;
}


//==============================================================================================
// dsp_panoramizer_dispose()
//==============================================================================================

void dsp_panoramizer_dispose(struct DSPObject *obj0)
{
	if (obj0)
	{
		db3_free(obj0);
	}
}


//==============================================================================================
// dsp_panoramizer_flush()
//==============================================================================================

void dsp_panoramizer_flush(struct DSPObject *dsp)
{
	struct Panoramizer *obj = (struct Panoramizer*)dsp;
	struct DSPObject *prev;
	int i;
	
	// Call flush on the previous object first.

	prev = dsp->dsp_prev;
	if (prev->dsp_prev) prev->dsp_flush(prev);
	for (i = 0; i < 64; i++) obj->DelBuf[i] = 0;
}


//==============================================================================================
// dsp_panoramizer_get()
//==============================================================================================

int dsp_panoramizer_get(UNUSED struct DSPObject *obj, UNUSED uint32_t attr, UNUSED int32_t *storage)
{
	return 0;
}


//==============================================================================================
// dsp_panoramizer_new()
//==============================================================================================

struct DSPObject *dsp_panoramizer_new(int16_t *phase_table)
{
	struct Panoramizer *obj;

	if (obj = db3_malloc(sizeof(struct Panoramizer)))
	{
		int i;
		
		obj->object.dsp_type = DSPTYPE_PANORAMIZER;
		obj->object.dsp_pull = dsp_panoramizer_pull;
		obj->object.dsp_dispose = dsp_panoramizer_dispose;
		obj->object.dsp_set = dsp_panoramizer_set;
		obj->object.dsp_get = dsp_panoramizer_get;
		obj->object.dsp_flush = dsp_panoramizer_flush;
		obj->PhaseTable = phase_table;
		
		for (i = 0; i < 64; i++) obj->DelBuf[i] = 0;
		
		return &obj->object;
	}
	return NULL;
}
