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


/* Wavetable source. Includes loop unroller. */

#include "libdigibooster3.h"
#include "dsp.h"


// This structure defines one loop point.

struct TPoint
{
	int32_t Position;           // ordinal number of the first sample after the turnpoint
	int32_t JumpOffset;         // offset of executed jump
	int ActDir;                 // direction which triggers the turnpoint
	int ResDir;                 // direction after turnpoint execution
	int Counter;                // activation counter, trurnpoint disabled if 0
};



// Main wavetable DSP object structure.

struct SampledInstrument
{
	struct DSPObject object;
	int16_t *AudioData;
	int32_t AudioLength;
	int32_t CurPos;             // current position, the first sample to be sent in the next chunk
	int32_t LoopFirst;          // the first sample in loop
	int32_t LoopLast;           // the last sample in loop
	int CurDir;                 // current direction of unrolling
	int LoopType;               // type of loop
	int BackwardsPlay;          // true if instrument has been triggered with E3x
	struct TPoint *Tp1;
	struct TPoint *Tp2;
	struct TPoint TpA;
	struct TPoint TpB;
};


#define TPDIR_FWD           1
#define TPDIR_REV           2
#define is_in_range_fwd(pos, start, len) (((pos) >= (start)) && ((pos) < (start) + (len)))
#define is_in_range_rev(pos, start, len) (((pos) > (start) - (len)) && ((pos) <= (start)))



//==============================================================================================
// dsp_sampled_instr_regenerate()
//==============================================================================================

void dsp_sampled_instr_regenerate(struct SampledInstrument *smi)
{
	if (smi->LoopType == IF_NO_LOOP)
	{
		smi->Tp1 = NULL;
		smi->Tp2 = NULL;
	}
	else if (smi->LoopType == IF_FORWARD_LOOP)
	{
		if (smi->BackwardsPlay)
		{
			smi->TpA.Position = smi->LoopFirst;
			smi->TpA.ActDir = TPDIR_REV;
			smi->TpA.ResDir = TPDIR_REV;
			smi->TpA.JumpOffset = smi->LoopLast - smi->LoopFirst + 1;
			smi->TpA.Counter = 0x7FFFFFFF;
			smi->Tp1 = &smi->TpA;
			smi->Tp2 = NULL;
		}
		else
		{
			smi->TpA.Position = smi->LoopLast + 1;
			smi->TpA.ActDir = TPDIR_FWD;
			smi->TpA.ResDir = TPDIR_FWD;
			smi->TpA.JumpOffset = smi->LoopFirst - smi->LoopLast - 1;
			smi->TpA.Counter = 0x7FFFFFFF;
			smi->Tp1 = &smi->TpA;
			smi->Tp2 = NULL;
		}
	}
	else      // IF_PINGPONG_LOOP
	{
			smi->TpA.Position = smi->LoopFirst;
			smi->TpA.ActDir = TPDIR_REV;
			smi->TpA.ResDir = TPDIR_FWD;
			smi->TpA.JumpOffset = 0;
			smi->TpA.Counter = 0x7FFFFFFF;
			smi->Tp1 = &smi->TpA;

			smi->TpB.Position = smi->LoopLast + 1;
			smi->TpB.ActDir = TPDIR_FWD;
			smi->TpB.ResDir = TPDIR_REV;
			smi->TpB.JumpOffset = 0;
			smi->TpB.Counter = 0x7FFFFFFF;
			smi->Tp2 = &smi->TpB;
	}
}


//==============================================================================================
// dsp_sampled_instr_set()
//==============================================================================================

void dsp_sampled_instr_set(struct DSPObject *obj, struct DSPTag *tags)
{
	struct SampledInstrument *smi = (struct SampledInstrument*)obj;
	int regenerate = FALSE;

	while (tags->dspt_tag)
	{
		switch (tags->dspt_tag)
		{
			case DSPA_ReversePlay:
				smi->BackwardsPlay = tags->dspt_data;
				smi->CurDir = tags->dspt_data ? TPDIR_REV : TPDIR_FWD;
				regenerate = TRUE;
			break;

			case DSPA_LoopFirst:
				smi->LoopFirst = tags->dspt_data;
				regenerate = TRUE;
			break;

			case DSPA_LoopLast:
				smi->LoopLast = tags->dspt_data;
				regenerate = TRUE;
			break;

			case DSPA_LoopType:
				smi->LoopType = tags->dspt_data;
				regenerate = TRUE;
			break;

			case DSPA_SampleOffset:
				if (smi->BackwardsPlay) smi->CurPos = smi->AudioLength - tags->dspt_data;
				else smi->CurPos = tags->dspt_data;

				if (smi->CurPos > smi->AudioLength) smi->CurPos = smi->AudioLength;
				else if (smi->CurPos < 0) smi->CurPos = 0;
			break;

			case DSPA_LimitOffsetToLoop:
				if (tags->dspt_data)
				{
					if (smi->CurPos < smi->LoopFirst) smi->CurPos = smi->LoopFirst;
					if (smi->CurPos > smi->LoopLast) smi->CurPos = smi->LoopLast;
				}
			break;
		}

		tags++;
	}

	if (regenerate) dsp_sampled_instr_regenerate(smi);
	return;
}


//==============================================================================================
// dsp_sampled_instr_pull()
//==============================================================================================

int dsp_sampled_instr_pull(struct DSPObject *obj, int16_t *dest, int32_t requested)
{
	struct SampledInstrument *smi = (struct SampledInstrument*)obj;
	int32_t delivered = 0;
	int32_t block;
	int end_of_instrument = FALSE;

	// The main loop is driven by number of samples requested.

	while (!end_of_instrument && ((block = requested - delivered) > 0))
	{
		struct TPoint *execute = NULL;         // turnpoint to execute
		int16_t *s;
		int32_t i;

		// Steps taken are different depending on the current unroller direction. The general
		// rule is to search for nearest active turnpoint in reqested range of samples, cut
		// the request to the turnpoint and copy samples, then execute the turnpoint. The first
		// step is to search the turnpoint.

		if (smi->CurDir == TPDIR_FWD)
		{
			// For forward direction the turnpoint must be inside the range and be activated
			// with forward direction. Both turnpoints are checked. If the second one is
			// nearer, or at the same position as the first, it takes precedence.

			if (smi->Tp1)
			{
				if (is_in_range_fwd(smi->Tp1->Position, smi->CurPos, block) && (smi->Tp1->ActDir == TPDIR_FWD))
				{
					execute = smi->Tp1;
					block = smi->Tp1->Position - smi->CurPos;
				}
			}

			if (smi->Tp2)
			{
				if (is_in_range_fwd(smi->Tp2->Position, smi->CurPos, block) && (smi->Tp2->ActDir == TPDIR_FWD))
				{
					execute = smi->Tp2;
					block = smi->Tp2->Position - smi->CurPos;
				}
			}

			// Check now if the end of instrument data is on the way. If so, the block should
			// be limited to the end, and main loop should be left after current block.

			if (smi->CurPos + block > smi->AudioLength)
			{
				block = smi->AudioLength - smi->CurPos;
				end_of_instrument = TRUE;
			}

			// Trimmed request execution. Audio samples first, zero padding then if needed.

			s = &smi->AudioData[smi->CurPos];
			for (i = 0; i < block; i++) { *dest++ = *s++; }
			smi->CurPos += block;
		}
		else
		{
			// Backward direction. Again the turnpoint must be located inside the range, but
			// tre range calculation is slightly different. Both the turnpoints are checked.
			// The same as for forward, if the second one is neared the current position, or
			// placed at the same position as the first, the second one takes precedence.

			if (smi->Tp1)
			{
				if (is_in_range_rev(smi->Tp1->Position, smi->CurPos, block) && (smi->Tp1->ActDir == TPDIR_REV))
				{
					execute = smi->Tp1;
					block = smi->CurPos - smi->Tp1->Position;
				}
			}

			if (smi->Tp2)
			{
				if (is_in_range_rev(smi->Tp2->Position, smi->CurPos, block) && (smi->Tp2->ActDir == TPDIR_REV))
				{
					execute = smi->Tp2;
					block = smi->CurPos - smi->Tp2->Position;
				}
			}

			// Check now if the start of instrument data is on the way. If so, the block should
			// be limited to the end, and main loop should be left after current block.

			if (block > smi->CurPos)
			{
				block = smi->CurPos;
				end_of_instrument = TRUE;
			}

			// Trimmed request execution. Audio samples first, zero padding then if needed.

			s = &smi->AudioData[smi->CurPos];
			for (i = 0; i < block; i++) { *dest++ = *--s; }
			smi->CurPos -= block;
		}

		delivered += block;

		// Turnpoint execution. It is already checked that activation direction is matched.

		if (execute && (execute->Counter > 0))
		{
			smi->CurPos += execute->JumpOffset;
			smi->CurDir = execute->ResDir;
			execute->Counter--;
		}
	}

	return delivered;
}


//==============================================================================================
// dsp_sampled_instr_dispose()
//==============================================================================================

void dsp_sampled_instr_dispose(struct DSPObject *obj)
{
	if (obj) db3_free(obj);
}


//==============================================================================================
// dsp_sampled_instr_flush()
//==============================================================================================

void dsp_sampled_instr_flush(UNUSED struct DSPObject *obj0)
{
//    struct SampledInstrument *obj = (struct SampledInstrument*)obj0;

	return;
}


//==============================================================================================
// dsp_sampled_instr_get()
//==============================================================================================

int dsp_sampled_instr_get(struct DSPObject *obj, uint32_t attr, int32_t *storage)
{
	struct SampledInstrument *dsp = (struct SampledInstrument*)obj;

	switch (attr)
	{
		case DSPA_SampleOffset:    *storage = dsp->CurPos;   return 1;
	}

	return 0;
}


//==============================================================================================
// dsp_sampled_instr_new()
//==============================================================================================

struct DSPObject *dsp_sampled_instr_new(int16_t *data, int32_t loop_start,
 int32_t loop_len, UNUSED int32_t loop_count, int32_t total_frames, int loop_type)
{
	struct SampledInstrument *smi;

	if (smi = db3_malloc(sizeof(struct SampledInstrument)))
	{
		smi->object.dsp_type = DSPTYPE_UNROLLER;
		smi->object.dsp_pull = dsp_sampled_instr_pull;
		smi->object.dsp_dispose = dsp_sampled_instr_dispose;
		smi->object.dsp_set = dsp_sampled_instr_set;
		smi->object.dsp_get = dsp_sampled_instr_get;
		smi->object.dsp_flush = dsp_sampled_instr_flush;

		smi->AudioData = data;
		smi->AudioLength = total_frames;
		smi->BackwardsPlay = FALSE;
		smi->LoopFirst = loop_start;
		smi->LoopLast = loop_start + loop_len - 1;
		smi->LoopType = loop_type;
		smi->CurPos = 0;
		smi->CurDir = TPDIR_FWD;

		dsp_sampled_instr_regenerate(smi);
		return &smi->object;
	}

	return NULL;
}
