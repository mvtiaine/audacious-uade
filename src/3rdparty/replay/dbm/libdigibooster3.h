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

#ifndef LIBDIGIBOOSTER3_LIBDIGIBOOSTER3_H
#define LIBDIGIBOOSTER3_LIBDIGIBOOSTER3_H

#ifndef TARGET_WIN32
#include <stdint.h>
#else
#define _CRT_SECURE_NO_DEPRECATE
#define UNUSED __pragma(warning(suppress:4100))
#include "../stdint.h"
#endif

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#endif

#undef TRUE
#define TRUE 1
#undef FALSE
#define FALSE 0

#include "musicmodule.h"

/* API */

struct UpdateEvent
{
	int32_t ue_Order;        // number of order in the playlist, starts from 0
	int32_t ue_Pattern;      // ordinal number of pattern, starts from 0
	int32_t ue_Row;          // number of row (position) in the pattern, starts from 0
	int32_t ue_Delay;        // event delay in sample frames from current buffer start
	int32_t ue_Tempo;        // current module tempo ("CIA tempo"), BPM.
	int32_t ue_Speed;        // current module speed (ticks per row).
	int32_t ue_SeqHalted;    // TRUE if sequencer is halted.
};


struct AbstractHandle;

typedef int ReadCallback(struct AbstractHandle*, void*, int);

struct AbstractHandle
{
	void *ah_Handle;
	ReadCallback *ah_Read;
};


struct DB3Module *DB3_Load(char *filename, int *errptr);
void DB3_Unload(struct DB3Module* module);
void* DB3_NewEngine(struct DB3Module *module, uint32_t mixfreq, uint32_t bufsize);
void DB3_SetCallback(void *engine, void(*callback)(void*, struct UpdateEvent*), void *userdata);
void DB3_SetVolume(void *engine, int16_t level);
void DB3_SetPos(void *engine, uint32_t song, uint32_t order, uint32_t row);
uint32_t DB3_Mix(void *engine, uint32_t frames, int16_t *out);
void DB3_DisposeEngine(void *engine);




#define MAX_STEREO_PHASE_SHIFT_USEC  333


/* error codes */

#define DB3_ERROR_NONE                         0
#define DB3_ERROR_FILE_OPEN                    1
#define DB3_ERROR_OUT_OF_MEMORY                2
#define DB3_ERROR_DATA_CORRUPTED               3
#define DB3_ERROR_VERSION_UNSUPPORTED          4
#define DB3_ERROR_READING_DATA                 5
#define DB3_ERROR_WRONG_CHUNK_ORDER            6


/* Memory allocation and manipulation. */

#define DB3_SAMPLE_LOAD_BUFFER_SIZE            8192    /* in bytes */

#if (defined TARGET_MORPHOS) || (defined TARGET_AMIGAOS3) || (defined TARGET_AMIGAOS4)

#define __NOLIBBASE__
#include <proto/exec.h>
#include <exec/memory.h>

#ifdef TARGET_AMIGAOS4
extern struct ExecIFace *IExec;
#define db3_malloc(size) AllocVecTags(size, AVT_ClearWithValue, 0, TAG_END)
#else
extern struct Library *SysBase;
#define db3_malloc(size) AllocVec(size, MEMF_ANY | MEMF_CLEAR)
#endif

#define db3_free(ptr) FreeVec(ptr)
#define db3_memcpy(d, s, l) CopyMem(s, d, l)

#else

#include <stdlib.h>
#include <string.h>
#define db3_malloc(size) calloc(1, size)
#define db3_free(ptr) free(ptr)
#define db3_strlen(s) strlen(s)
#define db3_strcpy(d, s) strcpy(d, s)
#define db3_memcpy(d, s, l) memcpy(d, s, l)

#endif


/* File I/O */

#if (defined TARGET_MORPHOS) || (defined TARGET_AMIGAOS3) || (defined TARGET_AMIGAOS4)

#include <proto/dos.h>

#ifdef TARGET_AMIGAOS4
extern struct DOSIFace *IDOS;
#else
extern struct Library *DOSBase;
#endif

#define db3_fopen(path) Open((STRPTR)path, MODE_OLDFILE)
#define db3_fread(buffer, blklen, count, file) FRead(file, buffer, blklen, count)
#define db3_fclose(file) Close(file)
#define errno IoErr()

#else

#include <stdio.h>
#include <errno.h>
#define db3_fopen(path) fopen(path, "rb")
#define db3_fread(buffer, blklen, count, file) fread(buffer, blklen, count, file)
#define db3_fclose(file) fclose(file)
#define BPTR FILE*
#endif


#endif     /* LIBDIGIBOOSTER3_LIBDIGIBOOSTER3_H */
