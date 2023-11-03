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


/* Linear interpolation resampler. */

#include "libdigibooster3.h"
#include "dsp.h"

//#include "modsynth.h"
//#include <musicmodule.h>


// Linear resampler object structure.

struct Resampler20
{
	struct DSPObject object;
	int16_t* buffer;             // vector aligned on some platforms
	uint32_t pos;                // current position on source grid * 2^16
	uint32_t step;               // current sampling step * 2^16
	int flushed;
};



//==============================================================================================
// dsp_resampler20_set()
//==============================================================================================

void dsp_resampler20_set(struct DSPObject *obj0, struct DSPTag *tags)
{
	struct Resampler20 *obj = (struct Resampler20*)obj0;

	while (tags->dspt_tag)
	{
		switch (tags->dspt_tag)
		{
			case DSPA_ResamplerRatio:
				obj->step = tags->dspt_data;
			break;
		}

		tags++;
	}
}


//==============================================================================================
// dsp_resampler20_pull()
//==============================================================================================

int dsp_resampler20_pull(struct DSPObject *obj0, int16_t *dest, int32_t samples)
{
	int leave_active = TRUE;
	struct Resampler20 *obj = (struct Resampler20*)obj0;
	struct DSPObject *prev;
	int32_t block;

	prev = (struct DSPObject*)obj->object.dsp_prev;

	while (samples)
	{
		int16_t s0, s1;
		int32_t dy;

		if (obj->flushed)  // initial buffer fill
		{
			int i;

			for (i = 0; i < 8; i++) obj->buffer[i] = 0;
			block = prev->dsp_pull(prev, &obj->buffer[8], 1016);

			// temporary zero padding

			if (block < 1016)
			{
				for (dy = 8 + block; dy < 1024; dy++) obj->buffer[dy] = 0;
				leave_active = FALSE;
			}

			obj->pos = 0;
			obj->flushed = FALSE;
		}

		if (obj->pos >= 1008 << 16)   // refill buffer
		{
			db3_memcpy(obj->buffer, &obj->buffer[1008], 32);   // 16 samples from end
			block = prev->dsp_pull(prev, &obj->buffer[16], 1008);

			// temporary zero padding

			if (block < 1008)
			{
				for (dy = 16 + block; dy < 1024; dy++) obj->buffer[dy] = 0;
				leave_active = FALSE;
			}

			obj->pos -= 1008 << 16;
		}

		s0 = obj->buffer[(obj->pos >> 16) + 8];
		s1 = obj->buffer[(obj->pos >> 16) + 9];
		dy = (s1 - s0) * (obj->pos & 0xFFFF);
		*dest++ = s0 + (dy >> 16);
		samples--;
		obj->pos += obj->step;
	}

	return leave_active;
}


//==============================================================================================
// dsp_resampler20_dispose()
//==============================================================================================

void dsp_resampler20_dispose(struct DSPObject *obj0)
{
	if (obj0)
	{
		struct Resampler20 *obj = (struct Resampler20*)obj0;

		if (obj->buffer) db3_free(obj->buffer);
		db3_free(obj0);
	}
}


//==============================================================================================
// dsp_resampler20_flush()
//==============================================================================================

void dsp_resampler20_flush(struct DSPObject *dsp)
{
	struct Resampler20 *obj = (struct Resampler20*)dsp;
	struct DSPObject *prev;

	// Call flush on the previous object first.

	prev = (struct DSPObject*)dsp->dsp_prev;
	if (prev->dsp_prev) prev->dsp_flush(prev);

	// Then invalidate data in buffer.

	obj->flushed = TRUE;
}


//==============================================================================================
// dsp_resampler20_get()
//==============================================================================================

int dsp_resampler20_get(UNUSED struct DSPObject *obj, UNUSED uint32_t attr, UNUSED int32_t *storage)
{
	return 0;
}


//==============================================================================================
// dsp_resampler20_new()
//==============================================================================================

struct DSPObject *dsp_resampler20_new(void)
{
	struct Resampler20 *obj;

	if (obj = db3_malloc(sizeof(struct Resampler20)))
	{
		if (obj->buffer = db3_malloc(2048))
		{
			int i;

			obj->object.dsp_type = DSPTYPE_RESAMPLER;
			obj->object.dsp_pull = dsp_resampler20_pull;
			obj->object.dsp_dispose = dsp_resampler20_dispose;
			obj->object.dsp_set = dsp_resampler20_set;
			obj->object.dsp_get = dsp_resampler20_get;
			obj->object.dsp_flush = dsp_resampler20_flush;
			obj->step = 65536;
			obj->flushed = TRUE;
			for (i = 0; i < 8; i++) obj->buffer[i] = 0;
			return &obj->object;
		}

		db3_free(obj);
	}
	return NULL;
}
