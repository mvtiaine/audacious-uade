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


/* 
  This object is inserted before the resampler and provides leading and trailing
  sequence of zeros.
*/

#include "libdigibooster3.h"
#include "dsp.h"



struct ZeroPadder
{
	struct DSPObject Object;
	int PadSize;
	int LeadInCtr;
	int LeadOutCtr;
	int MoreData;
};



//==============================================================================================
// dsp_zeropadder_set()
//==============================================================================================

void dsp_zeropadder_set(UNUSED struct DSPObject *obj, UNUSED struct DSPTag *tags)
{
}



//==============================================================================================
// dsp_zeropadder_pull()
//==============================================================================================

int dsp_zeropadder_pull(struct DSPObject *obj, int16_t *dest, int32_t requested)
{
	struct ZeroPadder *zpd = (struct ZeroPadder*)obj;
	struct DSPObject *prev;
	int32_t delivered = 0;
	int32_t blocksize;
	int32_t i;

	/*---------*/
	/* Lead in.*/
	/*---------*/

	blocksize = zpd->LeadInCtr;

	if (blocksize > 0)
	{
		if (blocksize > requested) blocksize = requested;
		for (i = 0; i < blocksize; i++) *dest++ = 0;
		delivered += blocksize;
		requested -= blocksize;
		zpd->LeadInCtr -= blocksize;
	}

	/*------------------*/
	/* Instrument data. */
	/*------------------*/

	if (zpd->MoreData && (requested > 0))
	{
		prev = zpd->Object.dsp_prev;
		blocksize = prev->dsp_pull(prev, dest, requested);
		if (blocksize < requested) zpd->MoreData = FALSE;
		delivered += blocksize;
		requested -= blocksize;
	}

	/*-----------*/
	/* Lead out. */
	/*-----------*/

	blocksize = zpd->LeadOutCtr;
	if (blocksize > requested) blocksize = requested;

	if (blocksize > 0)
	{
		for (i = 0; i < blocksize; i++) *dest++ = 0;
		delivered += blocksize;
		zpd->LeadOutCtr -= blocksize;
	}

	return delivered;
}



//==============================================================================================================================
// dsp_zeropadder_dispose()
//==============================================================================================================================

void dsp_zeropadder_dispose(struct DSPObject *obj)
{
	if (obj) db3_free(obj);
}



//==============================================================================================================================
// dsp_zeropadder_flush()
//==============================================================================================================================

void dsp_zeropadder_flush(struct DSPObject *obj)
{
	struct ZeroPadder *zpd = (struct ZeroPadder*)obj;

	zpd->LeadInCtr = zpd->PadSize;
	zpd->LeadOutCtr = zpd->PadSize;
	zpd->MoreData = TRUE;
}



//==============================================================================================================================
// dsp_zeropadder_get()
//==============================================================================================================================

int dsp_zeropadder_get(UNUSED struct DSPObject *obj, UNUSED uint32_t attr, UNUSED int32_t *storage)
{
	return 0;
}



//==============================================================================================================================
// dsp_zeropadder_new()
//==============================================================================================================================

struct DSPObject* dsp_zeropadder_new(int padframes)
{
	struct ZeroPadder *zpd;

	if (zpd = db3_malloc(sizeof(struct ZeroPadder)))
	{
		zpd->Object.dsp_type = DSPTYPE_ZEROPADDER;
		zpd->Object.dsp_pull = dsp_zeropadder_pull;
		zpd->Object.dsp_dispose = dsp_zeropadder_dispose;
		zpd->Object.dsp_set = dsp_zeropadder_set;
		zpd->Object.dsp_get = dsp_zeropadder_get;
		zpd->Object.dsp_flush = dsp_zeropadder_flush;
		zpd->PadSize = padframes;
		zpd->LeadInCtr = padframes;
		zpd->LeadOutCtr = padframes;
		zpd->MoreData = TRUE;
		return &zpd->Object;
	}

	return NULL;
}
