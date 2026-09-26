/***********************************************************************
 **
 **  Reading new notes etc. from pattern
 **
 ***********************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "digdata.h"
#include "digread.h"
#include "digcmd.h"
#include "digamg.h"
#include "digadl.h"
#include "dig.h"

static uint8_t getnote1(void);
static void donotes(void);

void dorow(void) // 8bb: replayer ticker
{
	if (song.np_pat == 255) // 8bb: bugfix if there are no patterns in the song!
		return;

	song.patmusicrand = (((song.patmusicrand * 0xCDEF) >> 16) + 0x1727) & 0xFFFF;

	if (song.musiccount == 0)
	{
		if (song.patterndelay > 0)
		{
			song.np_row--;
			docmd1();
			song.patterndelay--;
		}
		else
		{
			donotes(); // new notes
			docmd1(); // also does 0volcut
		}
	}
	else
	{
		docmd2(); // effects only
	}

	song.musiccount++;
	if (song.musiccount >= song.musicmax)
	{
		// next row
		song.np_row++;
		if (song.jumptorow != -1)
		{
			song.np_row = song.jumptorow;
			song.jumptorow = -1;
		}

		if (song.np_row >= 64 || (song.patloopcount == 0 && song.breakpat > 0))
		{
			// next pattern
			if (song.breakpat == 255)
			{
				song.breakpat = 0;
				return;
			}

			song.breakpat = 0;
			if (song.jmptoord != -1)
			{
				song.np_ord = song.jmptoord;
				song.jmptoord = -1;
			}

			song.np_row = neworder(); // if breakpat, np_row = break row
		}

		song.musiccount = 0;
	}
}

int16_t neworder(void) // 8bb: rewritten to be more safe
{
	uint8_t patt;

	uint16_t numSep = 0;
	while (true)
	{
		song.np_ord++;

		patt = song.order[song.np_ord-1];
		if (patt == 254)
		{
			/* 8bb: Added security that is not present in ST3.21: check
			** if a song has pattern separators only, prevent endless loop!
			*/
			numSep++;
			if (numSep >= song.header.ordnum)
			{
				WAVRender_Flag = false;
				return 0;
			}

			continue;
		}

		if (patt == 255)
		{
			// restart song
			song.np_ord = 0;
			WAVRender_Flag = false;

			if (song.order[0] == 255)
				return 0;

			continue;
		}

		break;
	}

	song.np_pat = patt;
	song.np_patoff = -1; // force reseek
	song.np_row = song.startrow;
	song.startrow = 0;
	song.patmusicrand = 0;
	song.patloopstart = -1;
	song.jumptorow = -1;

	return song.np_row;
}

static void seekpat(void)
{
	// find np_row from pattern

	if (song.np_patoff != -1)
		return;

	song.np_patseg = song.patp[song.np_pat];
	if (song.np_patseg != NULL)
	{
		int16_t j = 0;
		if (song.np_row > 0)
		{
			int16_t i = song.np_row;
			while (i > 0)
			{
				const uint8_t dat = song.np_patseg[j++];
				if (dat == 0)
				{
					i--;
				}
				else
				{
					if (dat & 0x20) j += 2;
					if (dat & 0x40) j += 1;
					if (dat & 0x80) j += 2;
				}
			}
		}

		song.np_patoff = j;
	}
}

static uint8_t getnote1(void) // getnote for DA notes
{
	uint8_t dat;

	if (song.np_patseg == NULL)
		return 255;

	if (song.np_pat >= song.header.patnum) // 8bb: added security that is not present in ST3.21
		return 255;

	uint8_t channel = 0;

	int16_t i = song.np_patoff;
	while (true)
	{
		dat = song.np_patseg[i++];
		if (dat == 0)
		{
			song.np_patoff = i;
			return 255;
		}

		uint8_t tmpChannel = song.header.channel[dat & 0x1F];
		if (!(tmpChannel & 128)) // 8bb: channel not muted?
		{
			channel = tmpChannel;
			break;
		}

		// channel off, skip
		if (dat &  32) i += 2;
		if (dat &  64) i += 1;
		if (dat & 128) i += 2;
	}

	zchn_t *ch = &song._zchn[channel];

	// NOTE/INSTRUMENT
	if (dat & 32)
	{
		ch->note = song.np_patseg[i++];
		ch->ins = song.np_patseg[i++];

		if (ch->note != 255)
			ch->lastnote = ch->note;

		if (ch->ins > 0)
			ch->lastins = ch->ins;
	}

	// VOLUME
	if (dat & 64)
		ch->vol = song.np_patseg[i++];

	// COMMAND/INFO
	if (dat & 128)
	{
		ch->cmd = song.np_patseg[i++];
		ch->info = song.np_patseg[i++];
	}

	song.np_patoff = i;
	return channel;
}

static void clearnotes(void)
{
	zchn_t *ch = song._zchn;
	for (int32_t i = 0; i < ACHANNELS; i++, ch++)
	{
		ch->note = 255;
		ch->vol = 255;
		ch->ins = 0;
		ch->cmd = 0;
		ch->info = 0;
	}
}

static void donotes(void)
{
	clearnotes();
	seekpat();

	while (true)
	{
		const uint8_t channel = getnote1();
		if (channel == 255)
			break; // end of row/channels

		donewnote(channel, false); // 8bb: false = we didn't come from notedelay effect
	}
}

void donewnote(uint8_t channel, bool fromNoteDelayEfx)
{
	zchn_t *ch = &song._zchn[channel];

	if (fromNoteDelayEfx)
	{
		ch->achannelused = 1 | 128;
	}
	else
	{
		if (ch->channelnum > song.lastachannelused)
			song.lastachannelused = ch->channelnum + 1;

		ch->achannelused = 1;

		if (ch->cmd == 'S'-64 && (ch->info & 0xF0) == 0xD0)
			return; // we have a note delay, do nothing yet
	}

	if (ch->ins > 101) // 101 used by inslib
		ch->ins = 0;

	// 8bb: volume column
	if (ch->vol != 255 && ch->vol > 63)
		ch->vol = 63;

	if (ch->channelnum <= 15)
		doamiga(ch);
	else if (ch->channelnum <= 16+9)
		doadlib(ch, ch->channelnum-16); // melody 0..8
}
