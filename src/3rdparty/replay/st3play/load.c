// SPDX-License-Identifier: BSD-3-Clause
#ifndef AUDACIOUS_UADE
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dig.h"
#include "digread.h"
#endif

// mvtiaine: added big endian support (WORDS_BIGENDIAN must be defined by the build)
#define SWAP16(value) \
((uint16_t)( \
	((uint16_t)(value) << 8) | \
	((uint16_t)(value) >> 8) \
))
#define SWAP32(value) \
((uint32_t)( \
	((uint32_t)(value) << 24) | \
	(((uint32_t)(value) & 0x0000FF00U) << 8) | \
	(((uint32_t)(value) & 0x00FF0000U) >> 8) | \
	((uint32_t)(value) >> 24) \
))
#ifdef WORDS_BIGENDIAN
#define READ16LE(value) SWAP16(value)
#define READ32LE(value) SWAP32(value)
#else
#define READ16LE(value) value
#define READ32LE(value) value
#endif

// 8bb: added these so that we can have a "load from RAM" loader as well
typedef struct
{
	uint8_t *_ptr, *_base;
	bool _eof;
	size_t _cnt, _bufsiz;
} MEMFILE;

static MEMFILE *mopen(const uint8_t *src, uint32_t length);
static void mclose(MEMFILE **buf);
static size_t mread(void *buffer, size_t size, size_t count, MEMFILE *buf);
static bool meof(MEMFILE *buf);
static void mseek(MEMFILE *buf, int32_t offset, int32_t whence);
static void mrewind(MEMFILE *buf);
// ------------------------------------------------------------------------

static void checkins(ds_smp *ins)
{
	// 8bb: only check PCM samples (nasty, AdLib c2spd isn't clamped and is read as uint16_t in digadl.c)
	if (ins->type != 1)
		return;

	// audacious-uade: no sample data loaded, treat as empty
	if (ins->baseptr == NULL)
	{
		ins->length = 0;
		ins->flags &= 0xFE;
		return;
	}

	if (ins->length == 0)
	{
		ins->flags &= 0xFE;
		return;
	}

	if (ins->vol > 64) ins->vol = 64;
	if (ins->c2spd > 65535) ins->c2spd = 65535;
	// following three will be >999999 (8bb: ???)
	if (ins->length > 64000) ins->length = 64000;

	if (ins->lend == ins->lbeg)
		ins->flags &= 0xFE; // disable loop

	if (ins->lend < ins->lbeg)
		ins->lend = ins->lbeg+1;

	if (ins->lbeg > ins->length) ins->lbeg = ins->length;
	if (ins->lend > ins->length) ins->lend = ins->length;

	// do sample continuing for fast looping
	if (ins->flags & 1) // 8bb: loop enabled?
	{
		uint16_t u = (uint16_t)ins->lend;
		uint16_t v = (uint16_t)ins->lbeg;

		int8_t *p = ins->baseptr;

		if (ins->lend512 > 0)
		{
			for (uint32_t i = ins->lend512; i < ins->length; i++)
				p[i] = p[i+512];
		}

		for (uint32_t i = ins->length-1; i >= ins->lend; i--)
			p[i+512] = p[i];

		for (uint32_t i = 0; i < 512; i++)
			p[u+i] = p[v+i];

		ins->lend512 = (uint16_t)ins->lend;
	}
	else
	{
		int8_t *p = ins->baseptr + ins->length;

		if (ins->lend512 > 0)
		{
			for (uint32_t i = ins->lend512; i < ins->length; i++)
				p[i] = p[i+512];

			ins->lend512 = 0;
		}

		// 8bb: fade-out non-looping sample

		int8_t a = p[-1];
		for (uint32_t i = 0; i < 512; i++)
		{
			*p++ = a;
			if (a > 0)
			{
				a -= 4;
				if (a < 0)
					a = 0;
			}
			else if (a < 0)
			{
				a += 4;
				if (a > 0)
					a = 0;
			}
		}
	}
}

static void checkinstruments(void)
{
	if (song.header.ultraclick == 0)
		song.header.ultraclick = 16;

	ds_smp *ins = song.ins;
	for (int32_t i = 0; i < MAX_INSTRUMENTS; i++, ins++)
		checkins(ins);
}

bool load_st3_from_ram(const uint8_t *data, uint32_t dataLength, int32_t soundCardType)
{
	uint16_t insoff[101], patoff[101];
	ds_smp *ins;

	// 8bb: custom stuff not present in ST3.21 code...
	song.moduleLoaded = false;
	memset(&song.header, 0, sizeof (song.header)); // 8bb: clear song
	memset(song.order, 255, MAX_ORDERS); // 8bb: pad orderlist with 255
	// ---------------------------------------------

	MEMFILE *f = mopen(data, dataLength);
	if (f == NULL)
		goto loadError;

	mread(&song.header, sizeof (song.header), 1, f);
	if (meof(f))
		goto loadError;

	if (memcmp(song.header._magic_signature, "SCRM", 4) != 0)
		goto loadError; // 8bb: not a valid S3M

// mvtiaine: added big endian support
#ifdef WORDS_BIGENDIAN
	song.header.ordnum = SWAP16(song.header.ordnum);
	song.header.insnum = SWAP16(song.header.insnum);
	song.header.patnum = SWAP16(song.header.patnum);
	song.header.flags = SWAP16(song.header.flags);
	song.header.cwtv = SWAP16(song.header.cwtv);
	song.header.ffv = SWAP16(song.header.ffv);
#endif

	// 8bb: added sanity checking (ST3 doesn't do this!)
	if (song.header.ordnum > MAX_ORDERS || song.header.insnum > MAX_INSTRUMENTS || song.header.patnum > MAX_PATTERNS)
		goto loadError; // incompatible S3M

	bool songMadeWithST3; // audacious-uade: C++: goto must not cross initialization
	songMadeWithST3 = (song.header.cwtv >> 12) == 1;

	song.header.name[27] = '\0'; // 8bb: added sanitation, so that it's always safe to print this string

	if (song.header.cwtv == 0x1300)
		song.header.flags |= 64; // 8bb: fast volslide flag

	if (!songMadeWithST3 || song.header.cwtv < 0x1310)
		song.header.ultraclick = 16; // 8bb: controls the number of GUS voices to use

	if (song.header.ffv == 1)
	{
		switch (song.header.mastermul)
		{
			case 0: song.header.mastermul = 0x10; break;
			case 1: song.header.mastermul = 0x20; break;
			case 2: song.header.mastermul = 0x30; break;
			case 3: song.header.mastermul = 0x40; break;
			case 4: song.header.mastermul = 0x50; break;
			case 5: song.header.mastermul = 0x60; break;
			case 6: song.header.mastermul = 0x70; break;
			case 7: song.header.mastermul = 0x7F; break;
			default: break;
		}
	}

	if (song.header.mastermul == 2)
		song.header.mastermul = 0x20;

	if (song.header.mastermul == 2+16)
		song.header.mastermul = 0x20+128;

	mread(song.order, 1, song.header.ordnum, f);
	mread(insoff, 2, song.header.insnum, f);
	mread(patoff, 2, song.header.patnum, f);

// mvtiaine: added big endian support
#ifdef WORDS_BIGENDIAN
	for (int32_t i = 0; i < song.header.insnum; i++)
		insoff[i] = SWAP16(insoff[i]);
	for (int32_t i = 0; i < song.header.patnum; i++)
		patoff[i] = SWAP16(patoff[i]);
#endif

	if (song.header.defaultpan252 == 252)
		mread(song.defaultpan, 1, 32, f);

	// 8bb: load instrument headers
	ins = song.ins;
	for (int32_t i = 0; i < song.header.insnum; i++, ins++)
	{
		mseek(f, insoff[i] << 4, SEEK_SET);
		mread(ins, 0x50, 1, f);

// mvtiaine: added big endian support
#ifdef WORDS_BIGENDIAN
		// ds_adl overlaps ds_smp: length/lbeg/lend are the AdLib registers D00-D0B
		if (ins->type == 1)
		{
			ins->memseg = SWAP16(ins->memseg);
			ins->length = SWAP32(ins->length);
			ins->lbeg = SWAP32(ins->lbeg);
			ins->lend = SWAP32(ins->lend);
			ins->guspos = SWAP16(ins->guspos);
			ins->lend512 = SWAP16(ins->lend512);
		}
		ins->c2spd = SWAP32(ins->c2spd);
#endif
	}

	// 8bb: load pattern data
	for (int32_t i = 0; i < song.header.patnum; i++)
	{
		uint16_t patDataLen;

		song.patDataLens[i] = 0; // audacious-uade: bounds checking

		if (patoff[i] != 0)
		{
			// audacious-uade: sanity check
			uint32_t offs = (uint32_t)patoff[i] << 4;

			if (offs + 2 > dataLength)
				goto loadError;

			mseek(f, offs, SEEK_SET);
			mread(&patDataLen, 2, 1, f);
			patDataLen = READ16LE(patDataLen); // mvtiaine: added big endian support

			if (patDataLen < 2 || offs + patDataLen > dataLength)
				goto loadError;

			song.patp[i] = (uint8_t *)malloc(patDataLen);
			if (song.patp[i] == NULL)
				goto loadError;

			mread(song.patp[i], 1, patDataLen-2, f);
			song.patDataLens[i] = patDataLen-2;
		}
	}

	// 8bb: load sample data
	ins = song.ins;
	for (int32_t i = 0; i < song.header.insnum; i++, ins++)
	{
		if (ins->type == 1 && ins->memseg != 0)
		{
			// audacious-uade: sanity check, and avoid wrapping offs+length
			uint32_t offs = ((uint32_t)ins->memseg << 4) + ((uint32_t)ins->memseg2 << 20);

			if (offs >= dataLength)
				continue;

			mseek(f, offs, SEEK_SET);

			/* 8bb: clamp overflown sample lengths (f.ex. "miracle man.s3m").
			** ST3.21 doesn't do this, but we have to, or else it plays back wrongly.
			*/
			if (ins->length > dataLength-offs) // 8bb: dataLength is the filesize
				ins->length = dataLength-offs;

			ins->baseptr = (int8_t *)malloc(ins->length+512+1); // 8bb: +1 for GUS intrp. safety (ST3 doesn't do this)
			if (ins->baseptr == NULL)
				goto loadError;

			mread(ins->baseptr, 1, ins->length, f);

			// 8bb: we use signed samples, unlike ST3.01 and later. Convert to signed.
			if (song.header.ffv != 1)
			{
				for (uint32_t j = 0; j < ins->length; j++)
					ins->baseptr[j] ^= 0x80;
			}
		}
	}

	checkinstruments();

	// 8bb: custom stuff not present in ST3.21 loader code...

	audio.soundcardtype = SOUNDCARD_GUS;
	
	/* 8bb: detect if we want to use SB Pro mode (for S3Ms saved by ST3 only).
	** Thanks to Saga_Musix for this detection idea!
	**
	** Apparently the guspos field in the sample headers are all 1 when saved
	** by ST3 w/ SB. (or all zeroes in some very early ST3.00 modules).
	*/
	if (songMadeWithST3 && song.header.cwtv == 0x1320)
	{
		/* 8bb: Some non-ST3 trackers spoof as ST3.20, so do
		** some extra heuristics to determine if this really
		** wasn't saved by ST3.
		*/
		int32_t gusposOR = 0;

		ins = song.ins;
		for (int32_t i = 0; i < song.header.insnum; i++, ins++)
		{
			if (ins->type == 1)
				gusposOR |= ins->guspos;
		}

		if (gusposOR == 0) // 8bb: all guspos entries were zero, this is not ST3
			songMadeWithST3 = false;
	}

	if (songMadeWithST3)
	{
		// 8bb: find out how many PCM samples we have
		uint32_t numSamples = 0;

		ins = song.ins;
		for (int32_t i = 0; i < song.header.insnum; i++, ins++)
		{
			if (ins->type == 1)
				numSamples++;
		}

		/* 8bb: we need an ST3 song with at least two PCM samples to reliably
		** detect if this song was made with an SB Pro.
		*/
		if (numSamples >= 2)
		{
			int32_t gusposOR = 0;

			ins = song.ins;
			for (int32_t i = 0; i < song.header.insnum; i++, ins++)
			{
				if (ins->type == 1)
					gusposOR |= ins->guspos;
			}

			/* 8bb: ST3 stored a one in guspos when saved with Sound Blaster.
			** However, there are early cwtv 0x1300 (ST3.00) modules in the wild where
			** they are zero. Test for both 0 and 1 in the final ORed value.
			**/
			if (gusposOR <= 1)
				audio.soundcardtype = SOUNDCARD_SBPRO;
		}
	}

#ifdef FORCE_SOUNDCARD_TYPE
	audio.soundcardtype = FORCE_SOUNDCARD_TYPE;
#else
	if (soundCardType != -1)
		audio.soundcardtype = soundCardType;
#endif

	song.moduleLoaded = true;
	mclose(&f); // audacious-uade: fix stream leak
	return true;

loadError:
	if (f != NULL) mclose(&f);
	closeMusic();
	return false;
}

bool load_st3(const char *fileName, int32_t soundCardType)
{
	FILE *f = fopen(fileName, "rb");
	if (f == NULL)
		return false;

	fseek(f, 0, SEEK_END);
	const uint32_t fileSize = (uint32_t)ftell(f);
	rewind(f);

	uint8_t *fileBuffer = (uint8_t *)malloc(fileSize);
	if (fileBuffer == NULL)
	{
		fclose(f);
		return false;
	}

	if (fread(fileBuffer, 1, fileSize, f) != fileSize)
	{
		free(fileBuffer);
		fclose(f);
		return false;
	}

	fclose(f);

	if (!load_st3_from_ram((const uint8_t *)fileBuffer, fileSize, soundCardType))
	{
		free(fileBuffer);
		return false;
	}

	free(fileBuffer);
	return true;
}

// 8bb: added these so that we can have a "load from RAM" loader as well

static MEMFILE *mopen(const uint8_t *src, uint32_t length)
{
	if (src == NULL || length == 0)
		return NULL;

	MEMFILE *b = (MEMFILE *)malloc(sizeof (MEMFILE));
	if (b == NULL)
		return NULL;

	b->_base = (uint8_t *)src;
	b->_ptr = (uint8_t *)src;
	b->_cnt = length;
	b->_bufsiz = length;
	b->_eof = false;
 
	return b;
}

static void mclose(MEMFILE **buf)
{
	if (*buf != NULL)
	{
		free(*buf);
		*buf = NULL;
	}
}

static size_t mread(void *buffer, size_t size, size_t count, MEMFILE *buf)
{
	if (buf == NULL || buf->_ptr == NULL)
		return 0;

	size_t wrcnt = size * count;
	if (size == 0 || buf->_eof)
		return 0;

	int32_t pcnt = (buf->_cnt > wrcnt) ? (int32_t)wrcnt : (int32_t)buf->_cnt;
	memcpy(buffer, buf->_ptr, pcnt);

	buf->_cnt -= pcnt;
	buf->_ptr += pcnt;

	if (buf->_cnt <= 0)
	{
		buf->_ptr = buf->_base + buf->_bufsiz;
		buf->_cnt = 0;
		buf->_eof = true;
	}

	return pcnt / size;
}

static bool meof(MEMFILE *buf)
{
	if (buf == NULL)
		return true;

	return buf->_eof;
}

static void mseek(MEMFILE *buf, int32_t offset, int32_t whence)
{
	if (buf == NULL)
		return;

	if (buf->_base)
	{
		switch (whence)
		{
			case SEEK_SET: buf->_ptr = buf->_base + offset; break;
			case SEEK_CUR: buf->_ptr += offset; break;
			case SEEK_END: buf->_ptr = buf->_base + buf->_bufsiz + offset; break;
			default: break;
		}

		buf->_eof = false;
		if (buf->_ptr >= buf->_base+buf->_bufsiz)
		{
			buf->_ptr = buf->_base + buf->_bufsiz;
			buf->_eof = true;
		}

		buf->_cnt = (buf->_base + buf->_bufsiz) - buf->_ptr;
	}
}

static void mrewind(MEMFILE *buf)
{
	mseek(buf, 0, SEEK_SET);
}

// ------------------------------------------------------------------------
