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


/* A dummy plugin, being always the first element of the track DSP chain. Its only purpose is to fetch */
/* data from the instrument DSP chain.                                                                 */

#include "libdigibooster3.h"
#include "dsp.h"


// FetchInstr object structure.

struct FetchInstr
{
	struct DSPObject object;
	struct MinList *dsp_chain;             // Pointer to the instrument DSP chain.
};


//==============================================================================================
// dsp_fetchinstr_set()
//==============================================================================================

void dsp_fetchinstr_set(UNUSED struct DSPObject *obj0, UNUSED struct DSPTag *tags)
{
	// no attributes
}


//==============================================================================================
// dsp_fetchinstr_pull()
//==============================================================================================

int dsp_fetchinstr_pull(struct DSPObject *obj0, int16_t *dest, int32_t frames)
{
	struct FetchInstr *obj = (struct FetchInstr*)obj0;
	struct DSPObject *instr_last;

	instr_last = (struct DSPObject*)obj->dsp_chain->mlh_TailPred;        // get the last DSP object in the instrument chain
	return instr_last->dsp_pull(instr_last, dest, frames);               // then forward the pull request to it
}


//==============================================================================================
// dsp_fetchinstr_dispose()
//==============================================================================================

void dsp_fetchinstr_dispose(struct DSPObject *obj0)
{
	if (obj0)
	{
		db3_free(obj0);
	}
}


//==============================================================================================
// dsp_fetchinstr_flush()
//==============================================================================================

void dsp_fetchinstr_flush(struct DSPObject *dsp)
{
	struct FetchInstr *obj = (struct FetchInstr*)dsp;
	struct DSPObject *instr_last;

	instr_last = (struct DSPObject*)obj->dsp_chain->mlh_TailPred;    // get the last DSP object in the instrument chain
	instr_last->dsp_flush(instr_last);                               // flush it
}


//==============================================================================================
// dsp_fetchinstr_get()
//==============================================================================================

int dsp_fetchinstr_get(UNUSED struct DSPObject *obj, UNUSED uint32_t attr, UNUSED int32_t *storage)
{
	return 0;
}


//==============================================================================================
// dsp_fetchinstr_new()
//==============================================================================================

struct DSPObject *dsp_fetchinstr_new(struct MinList *instr_chain)
{
	struct FetchInstr *obj;

	if (obj = db3_malloc(sizeof(struct FetchInstr)))
	{
		obj->object.dsp_type = DSPTYPE_FETCHINSTR;
		obj->object.dsp_pull = dsp_fetchinstr_pull;
		obj->object.dsp_dispose = dsp_fetchinstr_dispose;
		obj->object.dsp_set = dsp_fetchinstr_set;
		obj->object.dsp_get = dsp_fetchinstr_get;
		obj->object.dsp_flush = dsp_fetchinstr_flush;
		obj->dsp_chain = instr_chain;
		return &obj->object;
	}

	return NULL;
}
