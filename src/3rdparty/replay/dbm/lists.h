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

#ifndef LIBDIGIBOOSTER3_LISTS_H
#define LIBDIGIBOOSTER3_LISTS_H

#ifdef TARGET_WIN32
#define inline __inline
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

/* Amiga double linked lists implementation. */

#if (defined TARGET_MORPHOS) || (defined TARGET_AMIGAOS3) || (defined TARGET_AMIGAOS4)

#include <exec/lists.h>

#else

struct MinNode
{
	struct MinNode *mln_Succ;
	struct MinNode *mln_Pred;
};


struct MinList
{
	struct MinNode *mlh_Head;
	struct MinNode *mlh_Tail;
	struct MinNode *mlh_TailPred;
};

#endif


static inline void DB3AddTail(struct MinList *list, struct MinNode *node)
{
	struct MinNode *tailpred;

	tailpred = list->mlh_TailPred;
	node->mln_Succ = (struct MinNode*)&list->mlh_Tail;
	node->mln_Pred = tailpred;
	tailpred->mln_Succ = node;
	list->mlh_TailPred = node;
}


static inline struct MinNode* DB3RemHead(struct MinList *list)
{
	struct MinNode *node;

	node = list->mlh_Head;

	if (node->mln_Succ)
	{
		list->mlh_Head = node->mln_Succ;
		list->mlh_Head->mln_Pred = (struct MinNode*)&list->mlh_Head;
		return node;
	}
	else return NULL;
}


static inline void DB3Remove(struct MinNode *node)
{
	struct MinNode *pred, *succ;

	pred = node->mln_Pred;
	succ = node->mln_Succ;
	pred->mln_Succ = succ;
	succ->mln_Pred = pred;
}


#endif      /* LIBDIGIBOOSTER3_LISTS_H */
