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


/* Echo plugin. Works the same as in AHI. */
 
#include "libdigibooster3.h"
#include "dsp.h"


// Echo object structure.

struct Echo
{
	struct DSPObject object;
	int16_t *DelayLine;                   // echo delay line 
	int BufferSize;                       // delay line length in frames
	int WritePos;                         // current write position in the delay line
	int DelayTime;                        // in frames
	int Type;                             // one of DSPV_Echo_Type_X
	int MixFrequency;                     // received in constructor

	// echo calculation coefficients

	int PMix;
	int NMix;
	int PCrossPBack;
	int PCrossNBack;
	int NCrossPBack;
	int NCrossNBack;
}; 


//==============================================================================================
// dsp_echo_set()
//==============================================================================================

void dsp_echo_set(UNUSED struct DSPObject *obj0, UNUSED struct DSPTag *tags)
{
	struct Echo *obj = (struct Echo*)obj0;
	int mix = 0, cross = 0, fback = 0, recalc = 0;

	while (tags->dspt_tag)
	{
		switch (tags->dspt_tag)
		{
			case DSPA_EchoDelay:
				obj->DelayTime = (tags->dspt_data * obj->MixFrequency + 250) / 500;
			break;

			case DSPA_EchoMix:
				mix = tags->dspt_data;
				recalc = 1;
			break;

			case DSPA_EchoCross:
				cross = tags->dspt_data;
				recalc = 1;
			break;

			case DSPA_EchoFeedback:
				fback = tags->dspt_data;
				recalc = 1;
			break;
		}

		tags++;
	}

	if (recalc)
	{
		obj->PMix = mix;
		obj->NMix = 256 - mix;
		obj->PCrossPBack = cross * fback;
		obj->PCrossNBack = cross * (256 - fback);
		obj->NCrossPBack = (cross - 256) * fback;
		obj->NCrossNBack = (cross - 256) * (fback - 256);
	}
}


//==============================================================================================
// dsp_echo_pull()
//==============================================================================================

int dsp_echo_pull(struct DSPObject *obj0, int16_t *dest, int32_t frames)
{
	struct Echo *obj = (struct Echo*)obj0;
	struct DSPObject *prev;
	int32_t al, ar, l, r, l_del, r_del;
	int16_t b[32], *src, *del;
	int chunk, read_pos, leave_active = TRUE;

	prev = obj->object.dsp_prev;

	// Scalar code (one frame at a time).

	while (frames)
	{
		chunk = 16;
		if (chunk > frames) chunk = frames;
		
		leave_active = prev->dsp_pull(prev, b, chunk);
		src = b;
		frames -= chunk;

		while (chunk--)
		{
			read_pos = obj->WritePos - obj->DelayTime;
			if (read_pos < 0) read_pos += obj->BufferSize;
			del = &obj->DelayLine[read_pos << 1];

			// calculation of samples being stored in the delay line

			l = *src++;
			r = *src++;
			l_del = *del++;
			r_del = *del++;
			
			al = l * obj->NCrossNBack;
			al += r * obj->PCrossNBack;
			al += l_del * obj->NCrossPBack;
			al += r_del * obj->PCrossPBack;

			ar = r * obj->NCrossNBack;
			ar += l * obj->PCrossNBack;
			ar += r_del * obj->NCrossPBack;
			ar += l_del * obj->PCrossPBack;

			obj->DelayLine[obj->WritePos << 1] = al >> 16;
			obj->DelayLine[(obj->WritePos << 1) + 1] = ar >> 16;
			obj->WritePos++;
			if (obj->WritePos == obj->BufferSize) obj->WritePos = 0;

			// output samples now

			*dest++ = (l * obj->NMix + l_del * obj->PMix) >> 8;
			*dest++ = (r * obj->NMix + r_del * obj->PMix) >> 8;
		}
	}

	return TRUE;
}


//==============================================================================================
// dsp_echo_dispose()
//==============================================================================================

void dsp_echo_dispose(struct DSPObject *obj0)
{
	if (obj0)
	{
		struct Echo* obj = (struct Echo*)obj0;
		
		if (obj->DelayLine) db3_free(obj->DelayLine);
		db3_free(obj);
	}
}


//==============================================================================================
// dsp_echo_flush()
//==============================================================================================

// This module belongs to track DSP chain. This chain is never flushed.

void dsp_echo_flush(UNUSED struct DSPObject *dsp)
{
}


//==============================================================================================
// dsp_echo_get()
//==============================================================================================

int dsp_echo_get(struct DSPObject *obj0, uint32_t attr, int32_t *storage)
{
	struct Echo *obj = (struct Echo*)obj0;

	switch (attr)
	{
		case DSPA_EchoType:    *storage = obj->Type;    return 1;
	}

	return 0;
}


//==============================================================================================
// dsp_echo_new()
//==============================================================================================

struct DSPObject *dsp_echo_new(int mixfreq, int type)
{
	struct Echo *obj;

	if (obj = db3_malloc(sizeof(struct Echo)))
	{
		obj->object.dsp_type = DSPTYPE_ECHO;
		obj->object.dsp_pull = dsp_echo_pull;
		obj->object.dsp_dispose = dsp_echo_dispose;
		obj->object.dsp_set = dsp_echo_set;
		obj->object.dsp_get = dsp_echo_get;
		obj->object.dsp_flush = dsp_echo_flush;
		obj->MixFrequency = mixfreq;
		obj->PMix = 128;
		obj->NMix = 128;
		obj->PCrossPBack = 32640;
		obj->PCrossNBack = 32640;
		obj->NCrossPBack = 128;
		obj->NCrossNBack = 128;
		obj->Type = type;

		// Maximum echo delay possible is 512 ms, so it needs (0.512 * mixfreq) stereo frames. I round it up to (1/2 + 1/64) *
		// mixfreq. Buffer is rounded up (and aligned to) 16 bytes (4 stereo frames) for SIMD.

		obj->BufferSize = ((mixfreq >> 1) + (mixfreq >> 6) + 3) & ~4;

		if (obj->DelayLine = db3_malloc(obj->BufferSize << 2))
		{
			int i;
			int32_t *p = (int32_t*)obj->DelayLine;

			// let's clear the delay line

			for (i = 0; i < obj->BufferSize; i++) *p++ = 0;

			// calculate delay time in frames, reset write position

			obj->DelayTime = (64 * mixfreq + 250) / 500;   // default echo delay = 0x40;
			obj->WritePos = 0;
			return &obj->object;

		} else db3_free(obj);
	}
	return NULL;
}
