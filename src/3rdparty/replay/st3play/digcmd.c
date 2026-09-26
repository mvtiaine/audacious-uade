/***********************************************************************
 **
 **  Special commands handling
 **
 ***********************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "dig.h"
#include "digread.h"
#include "digdata.h"
#include "digcmd.h"

#define GET_LAST_NFO if (ch->info == 0) ch->info = ch->alastnfo;

static void s_ret(zchn_t *ch);
static void s_setfilt(zchn_t *ch);
static void s_setgliss(zchn_t *ch);
static void s_setfinetune(zchn_t *ch);
static void s_setvibwave(zchn_t *ch);
static void s_settrewave(zchn_t *ch);
static void s_settrewave(zchn_t *ch);
static void s_setpanpos(zchn_t *ch);
static void s_stereocntr(zchn_t *ch);
static void s_patloop(zchn_t *ch);
static void s_notecut(zchn_t *ch);
static void s_notecutb(zchn_t *ch);
static void s_notedelay(zchn_t *ch);
static void s_notedelayb(zchn_t *ch);
static void s_patterdelay(zchn_t *ch);
static void s_setspeed(zchn_t *ch);
static void s_jmpto(zchn_t *ch);
static void s_break(zchn_t *ch);
static void s_volslide(zchn_t *ch);
static void s_slidedown(zchn_t *ch);
static void s_slideup(zchn_t *ch);
static void s_toneslide(zchn_t *ch);
static void s_vibrato(zchn_t *ch);
static void s_tremor(zchn_t *ch);
static void s_arp(zchn_t *ch);
static void s_vibvol(zchn_t *ch);
static void s_tonevol(zchn_t *ch);
static void s_retrig(zchn_t *ch);
static void s_tremolo(zchn_t *ch);
static void s_scommand1(zchn_t *ch);
static void s_scommand2(zchn_t *ch);
static void s_settempo(zchn_t *ch);
static void s_finevibrato(zchn_t *ch);
static void s_setgvol(zchn_t *ch);
static void s_zinfo(zchn_t *ch);

// command routine jump tables

typedef void (*effect_routine)(zchn_t *ch);

static const effect_routine ssoncejmp[16] = // when new note, S-commands (8bb: tick=0 only)
{
	s_setfilt,     // 0
	s_setgliss,    // 1
	s_setfinetune, // 2
	s_setvibwave,  // 3
	s_settrewave,  // 4
	s_ret,         // 5
	s_ret,         // 6
	s_ret,         // 7
	s_setpanpos,   // 8
	s_ret,         // 9
	s_stereocntr,  // A
	s_patloop,     // B
	s_notecut,     // C
	s_notedelay,   // D
	s_patterdelay, // E
	s_ret          // F
};

static const effect_routine ssotherjmp[16] = // when (8bb: not) new note, S-commands (8bb: tick>0 only)
{
	s_ret,        // 0
	s_ret,        // 1
	s_ret,        // 2
	s_ret,        // 3
	s_ret,        // 4
	s_ret,        // 5
	s_ret,        // 6
	s_ret,        // 7
	s_ret,        // 8
	s_ret,        // 9
	s_ret,        // A
	s_ret,        // B
	s_notecutb,   // C
	s_notedelayb, // D
	s_ret,        // E
	s_ret         // F
};

static const effect_routine soncejmp[27] = // when new note (8bb: tick=0 only)
{
	s_ret,       // .
	s_setspeed,  // A
	s_jmpto,     // B
	s_break,     // C
	s_volslide,  // D fine/musiccount=0
	s_slidedown, // E fine/musiccount=0
	s_slideup,   // F fine/musiccount=0
	s_ret,       // G
	s_ret,       // H
	s_tremor,    // I
	s_arp,       // J
	s_ret,       // K
	s_ret,       // L
	s_ret,       // M
	s_ret,       // N
	s_ret,       // O ADDPOS digamg.asm:ssa ( 8bb: handled in doamiga() )
	s_ret,       // P
	s_retrig,    // Q
	s_ret,       // R
	s_scommand1, // S
	s_settempo,  // T
	s_ret,       // U
	s_ret,       // V
	s_ret,       // W
	s_ret,       // X
	s_ret,       // Y
	s_zinfo      // Z
};

static const effect_routine sotherjmp[27] = // when not new note (8bb: tick>0 only)
{
	s_ret,         // .
	s_ret,         // A
	s_ret,         // B
	s_ret,         // C
	s_volslide,    // D
	s_slidedown,   // E
	s_slideup,     // F
	s_toneslide,   // G
	s_vibrato,     // H
	s_tremor,      // I
	s_arp,         // J
	s_vibvol,      // K
	s_tonevol,     // L
	s_ret,         // M
	s_ret,         // N
	s_ret,         // O
	s_ret,         // P
	s_retrig,      // Q
	s_tremolo,     // R
	s_scommand2,   // S
	s_ret,         // T
	s_finevibrato, // U
	s_setgvol,     // V
	s_ret,         // W
	s_ret,         // X
	s_ret,         // Y
	s_ret          // Z
};

static void s_ret(zchn_t *ch) // 8bb: dummy effect (for unused effects)
{
	(void)ch;
}

void docmd1(void) // cmds done once (&0vol cutting) (8bb: tick=0 commands)
{
	const int8_t oldKxyLxxVolslideType = song.KxyLxxVolslideType;

	zchn_t *ch = song._zchn;
	for (uint8_t i = 0; i <= song.lastachannelused; i++, ch++)
	{
		if (ch->achannelused != 0)
		{
			// 8bb: "a0clearcnt" handling is not ported, it's triggered by jamming notes in the tracker

			/* 8bb: "vol0 optimization" flag handling.
			** There's a label-bug with "ch->cmd != 0" in the original code,
			** but doing it like this gives the same result.
			*/
			if (song.masterflags & 8)
			{
				if (ch->cmd != 0 || ch->avol != 0 || ch->vol != 255 || ch->ins != 0 || ch->note != 255)
				{
					ch->a0volcut = 3;
				}
				else
				{
					ch->a0volcut--;
					if (ch->a0volcut == 0)
					{
						// shutdown channel
						ch->m_end = 0;
						ch->m_pos = 65535;
						ch->achannelused = 0;
						continue;
					}
				}
			}

			if (ch->info > 0)
				ch->alastnfo = ch->info;

			if (ch->cmd > 0)
			{
				ch->achannelused |= 0x80;

				if (ch->cmd == 'D'-64)
				{
					// fix trigger D

					ch->atrigcnt = 0;

					// fix speed if tone port noncomplete
					if (ch->aspd != ch->aorgspd)
					{
						ch->aspd = ch->aorgspd;
						setspd(ch);
					}
				}
				else
				{
					if (ch->cmd != 'I'-64)
					{
						ch->atremor = 0;
						ch->atreon = true;
					}

					if (ch->cmd != 'H'-64 && ch->cmd != 'U'-64 && ch->cmd != 'K'-64 && ch->cmd != 'R'-64)
						ch->avibcnt |= 128;
				}

				if (ch->cmd < 27)
				{
					song.KxyLxxVolslideType = 0;
					soncejmp[ch->cmd](ch);
				}
			}
			else
			{
				// fix trigger 0

				ch->atrigcnt = 0;

				// fix speed if tone port noncomplete
				if (ch->aspd != ch->aorgspd)
				{
					ch->aspd = ch->aorgspd;
					setspd(ch);
				}

				if (!song.amigalimits && ch->cmd < 27)
				{
					song.KxyLxxVolslideType = 0;
					soncejmp[ch->cmd](ch);
				}
			}
		}
	}

	song.KxyLxxVolslideType = oldKxyLxxVolslideType;
}

void docmd2(void) // cmds done when not next row (8bb: tick>0 commands)
{
	int8_t oldKxyLxxVolslideType = song.KxyLxxVolslideType;

	zchn_t *ch = song._zchn;
	for (uint8_t i = 0; i <= song.lastachannelused; i++, ch++)
	{
		if (ch->achannelused != 0 && ch->cmd > 0)
		{
			ch->achannelused |= 0x80;
			if (ch->cmd < 27)
			{
				song.KxyLxxVolslideType = 0;
				sotherjmp[ch->cmd](ch);
			}
		}
	}

	song.KxyLxxVolslideType = oldKxyLxxVolslideType;
}

static void s_settempo(zchn_t *ch)
{
	if (!song.musiccount)
		settempo(ch->info);
}

void settempo(uint8_t bpm)
{
	// 8bb: ST3+SB + Txx <= 0x20 = do nothing
	if (audio.soundcardtype == SOUNDCARD_SBPRO && bpm <= 0x20)
		return;

	audio.samplesPerTickInt = audio.bpm2SamplesPerTickInt[bpm];
	audio.samplesPerTickFrac = audio.bpm2SamplesPerTickFrac[bpm];
}

void setspeed(uint8_t val)
{
	if (val > 0)
		song.musicmax = val;
}

static void s_setspeed(zchn_t *ch)  // A - 1
{
	setspeed(ch->info);
}

static void s_jmpto(zchn_t *ch) // B - 2
{
	if (ch->info == 0xFF)
	{
		song.breakpat = 255;
	}
	else
	{
		song.breakpat = 1;
		song.jmptoord = ch->info;
	}
}

static void s_break(zchn_t *ch) // C - 3
{
	const uint8_t hi = ch->info >> 4;
	const uint8_t lo = ch->info & 0x0F;

	if (hi <= 9 && lo <= 9)
	{
		song.startrow = (hi * 10) + lo;
		song.breakpat = 1;
	}
}

static void s_slideup(zchn_t *ch) // F - 6
{
	if (ch->aorgspd == 0)
		return;

	GET_LAST_NFO

	if (song.musiccount > 0)
	{
		if (ch->info >= 0xE0)
			return; // no fine slides here

		ch->aspd -= ch->info << 2;
		if ((int16_t)ch->aspd < 0)
			ch->aspd = 0;
	}
	else
	{
		if (ch->info <= 0xE0)
			return; // only fine slides here

		if (ch->info <= 0xF0)
		{
			ch->aspd -= ch->info & 0x0F;
			if (ch->aspd < 0)
				ch->aspd = 0;
		}
		else
		{
			ch->aspd -= (ch->info & 0x0F) << 2;
			if (ch->aspd < 0)
				ch->aspd = 0;
		}
	}

	ch->aorgspd = ch->aspd;
	setspd(ch);
}

static void s_slidedown(zchn_t *ch) // E - 5
{
	if (ch->aorgspd == 0)
		return;

	GET_LAST_NFO

	if (song.musiccount > 0)
	{
		if (ch->info >= 0xE0)
			return; // no fine slides here

		ch->aspd += ch->info << 2;
		if ((uint16_t)ch->aspd > 32767)
			ch->aspd = 32767;
	}
	else
	{
		if (ch->info <= 0xE0)
			return; // only fine slides here

		if (ch->info <= 0xF0)
		{
			ch->aspd += ch->info & 0x0F;
			if ((uint16_t)ch->aspd > 32767)
				ch->aspd = 32767;
		}
		else
		{
			ch->aspd += (ch->info & 0x0F) << 2;
			if ((uint16_t)ch->aspd > 32767)
				ch->aspd = 32767;
		}
	}

	ch->aorgspd = ch->aspd;
	setspd(ch);
}

static void s_vibvol(zchn_t *ch) // K
{
	song.KxyLxxVolslideType = 2;
	s_volslide(ch);
}

static void s_tonevol(zchn_t *ch) // L
{
	song.KxyLxxVolslideType = 1;
	s_volslide(ch);
}

static void s_volslide(zchn_t *ch) // D - 4
{
	ch->addherzretrigvol = 1; // 8bb: for AdLib

	GET_LAST_NFO

	const uint8_t infohi = ch->info >> 4;
	const uint8_t infolo = ch->info & 0x0F;

	if (infolo == 0x0F)
	{
		if (infohi == 0)
			ch->avol -= infolo;
		else if (song.musiccount == 0)
			ch->avol += infohi;
	}
	else if (infohi == 0x0F)
	{
		if (infolo == 0)
			ch->avol += infohi;
		else if (song.musiccount == 0)
			ch->avol -= infolo;
	}
	else if (song.fastvolslide || song.musiccount > 0)
	{
		if (infolo == 0)
			ch->avol += infohi;
		else
			ch->avol -= infolo;
	}
	else
	{
		return; // illegal slide
	}

	ch->avol = CLAMP(ch->avol, 0, 63);
	setvol(ch);

	// 8bb: these are set on Kxy/Lxx
	if (song.KxyLxxVolslideType == 1)
		s_toneslide(ch);
	else if (song.KxyLxxVolslideType == 2)
		s_vibrato(ch);
}

static void s_toneslide(zchn_t *ch) // G - 7
{
	uint8_t toneinfo;

	if (song.KxyLxxVolslideType == 1) // 8bb: we came from an Lxy (toneslide+volslide)
	{
		toneinfo = ch->alasteff1;
	}
	else
	{
		if (ch->aorgspd == 0)
		{
			if (ch->asldspd == 0)
				return;

			ch->aorgspd = ch->asldspd;
			ch->aspd = ch->asldspd;
		}

		if (ch->info == 0)
			ch->info = ch->alasteff1;
		else
			ch->alasteff1 = ch->info;

		toneinfo = ch->info;
	}

	if (ch->aorgspd != ch->asldspd)
	{
		if (ch->aorgspd < ch->asldspd)
		{
			ch->aorgspd += toneinfo << 2;
			if ((uint16_t)ch->aorgspd > (uint16_t)ch->asldspd)
				ch->aorgspd = ch->asldspd;
		}
		else
		{
			ch->aorgspd -= toneinfo << 2;
			if (ch->aorgspd < ch->asldspd)
				ch->aorgspd = ch->asldspd;
		}

		if (ch->aglis)
			ch->aspd = roundspd(ch, ch->aorgspd);
		else
			ch->aspd = ch->aorgspd;

		setspd(ch);
	}
}

static void s_vibrato(zchn_t *ch) // H - 8
{
	uint8_t vibinfo;

	if (song.KxyLxxVolslideType == 2) // 8bb: we came from a Kxy (vibrato+volslide)
	{
		vibinfo = ch->alasteff;
	}
	else
	{
		if (ch->info == 0)
			ch->info = ch->alasteff;

		if ((ch->info & 0xF0) == 0)
			ch->info = (ch->alasteff & 0xF0) | (ch->info & 0x0F);

		vibinfo = ch->alasteff = ch->info;
	}

	if (ch->aorgspd == 0)
		return;

	int16_t cnt  = ch->avibcnt;
	const int8_t type = (ch->avibtretype & 0x0E) >> 1;
	int32_t dat = 0;

	// sine
	if (type == 0 || type == 4)
	{
		if (type == 4)
		{
			cnt &= 0x7F;
		}
		else
		{
			if (cnt & 0x80)
				cnt = 0;
		}

		dat = vibsin[cnt >> 1];
	}

	// ramp
	else if (type == 1 || type == 5)
	{
		if (type == 5)
		{
			cnt &= 0x7F;
		}
		else
		{
			if (cnt & 0x80)
				cnt = 0;
		}

		dat = vibramp[cnt >> 1];
	}

	// square
	else if (type == 2 || type == 6)
	{
		if (type == 6)
		{
			cnt &= 0x7F;
		}
		else
		{
			if (cnt & 0x80)
				cnt = 0;
		}

		dat = vibsqu[cnt >> 1];
	}

	// random
	else if (type == 3 || type == 7)
	{
		if (type == 7)
		{
			cnt &= 0x7F;
		}
		else
		{
			if (cnt & 0x80)
				cnt = 0;
		}

		dat = vibsin[cnt >> 1];
		cnt += (song.patmusicrand & 0x1E);
	}

	if (song.oldstvib)
		ch->aspd = ch->aorgspd + ((int16_t)(dat * (vibinfo & 0x0F)) >> 4);
	else
		ch->aspd = ch->aorgspd + ((int16_t)(dat * (vibinfo & 0x0F)) >> 5);

	setspd(ch);

	ch->avibcnt = (cnt + ((vibinfo >> 4) << 1)) & 126;
}

static void s_finevibrato(zchn_t *ch) // U
{
	if (ch->info == 0)
		ch->info = ch->alasteff;

	if ((ch->info & 0xF0) == 0)
		ch->info = (ch->alasteff & 0xF0) | (ch->info & 0x0F);

	ch->alasteff = ch->info;

	if (ch->aorgspd == 0)
		return;

	int16_t cnt = ch->avibcnt;
	const int8_t type = (ch->avibtretype & 0x0E) >> 1;
	int32_t dat = 0;

	// sine
	if (type == 0 || type == 4)
	{
		if (type == 4)
		{
			cnt &= 0x7F;
		}
		else
		{
			if (cnt & 0x80)
				cnt = 0;
		}

		dat = vibsin[cnt >> 1];
	}

	// ramp
	else if (type == 1 || type == 5)
	{
		if (type == 5)
		{
			cnt &= 0x7F;
		}
		else
		{
			if (cnt & 0x80)
				cnt = 0;
		}

		dat = vibramp[cnt >> 1];
	}

	// square
	else if (type == 2 || type == 6)
	{
		if (type == 6)
		{
			cnt &= 0x7F;
		}
		else
		{
			if (cnt & 0x80)
				cnt = 0;
		}

		dat = vibsqu[cnt >> 1];
	}

	// random
	else if (type == 3 || type == 7)
	{
		if (type == 7)
		{
			cnt &= 0x7F;
		}
		else
		{
			if (cnt & 0x80)
				cnt = 0;
		}

		dat = vibsin[cnt >> 1];
		cnt += (song.patmusicrand & 0x1E);
	}

	if (song.oldstvib)
		ch->aspd = ch->aorgspd + ((int16_t)(dat * (ch->info & 0x0F)) >> 6);
	else
		ch->aspd = ch->aorgspd + ((int16_t)(dat * (ch->info & 0x0F)) >> 7);

	setspd(ch);

	ch->avibcnt = (cnt + ((ch->info >> 4) << 1)) & 126;
}

static void s_tremolo(zchn_t *ch) // R
{
	GET_LAST_NFO

	if ((ch->info & 0xF0) == 0)
		ch->info = (ch->alastnfo & 0xF0) | (ch->info & 0x0F);

	ch->alastnfo = ch->info;

	if (ch->aorgvol <= 0)
		return;

	int16_t cnt = ch->avibcnt;
	const int8_t type = ch->avibtretype >> 5;
	int16_t dat = 0;

	// sine
	if (type == 0 || type == 4)
	{
		if (type == 4)
		{
			cnt &= 0x7F;
		}
		else
		{
			if (cnt & 0x80)
				cnt = 0;
		}

		dat = vibsin[cnt >> 1];
	}

	// ramp
	else if (type == 1 || type == 5)
	{
		if (type == 5)
		{
			cnt &= 0x7F;
		}
		else
		{
			if (cnt & 0x80)
				cnt = 0;
		}

		dat = vibramp[cnt >> 1];
	}

	// square
	else if (type == 2 || type == 6)
	{
		if (type == 6)
		{
			cnt &= 0x7F;
		}
		else
		{
			if (cnt & 0x80)
				cnt = 0;
		}

		dat = vibsqu[cnt >> 1];
	}

	// random
	else if (type == 3 || type == 7)
	{
		if (type == 7)
		{
			cnt &= 0x7F;
		}
		else
		{
			if (cnt & 0x80)
				cnt = 0;
		}

		dat = vibsin[cnt >> 1];
		cnt += (song.patmusicrand & 0x1E);
	}

	dat = ch->aorgvol + (int8_t)((dat * (ch->info & 0x0F)) >> 7);
	dat = CLAMP(dat, 0, 63);

	ch->avol = (int8_t)dat;
	setvol(ch);

	ch->avibcnt = (cnt + ((ch->info & 0xF0) >> 3)) & 126;
}

static void s_tremor(zchn_t *ch) // I - 9
{
	GET_LAST_NFO

	if (ch->atremor > 0)
	{
		ch->atremor--;
		return;
	}

	if (ch->atreon)
	{
		// set to off
		ch->atreon = false;

		ch->avol = 0;
		setvol(ch);

		ch->atremor = ch->info & 0x0F;
	}
	else
	{
		// set to on
		ch->atreon = true;

		ch->avol = ch->aorgvol;
		setvol(ch);

		ch->atremor = ch->info >> 4;
	}
}

static void s_arp(zchn_t *ch)
{
	GET_LAST_NFO

	const uint8_t tick = song.musiccount % 3;

	int8_t noteadd = 0;
	if (tick == 1)
		noteadd = ch->info >> 4;
	else if (tick == 2)
		noteadd = ch->info & 0x0F;

	// check for octave overflow
	int8_t octa = ch->lastnote & 0xF0;
	int8_t note = (ch->lastnote & 0x0F) + noteadd;

	while (note >= 12)
	{
		note -= 12;
		octa += 16;
	}

	ch->aspd = scalec2spd(ch, stnote2herz(octa | note));
	setspd(ch);
}

static void s_retrig(zchn_t *ch)
{
	GET_LAST_NFO
	const uint8_t infohi = ch->info >> 4;

	if ((ch->info & 0x0F) == 0 || (ch->info & 0x0F) > ch->atrigcnt)
	{
		ch->atrigcnt++;
		return;
	}

	ch->atrigcnt = 0;

	ch->m_pos = 0;
	ch->m_oldpos = 0xFFFFFFFF; // 8bb: this forces a GUS trigger

	if (retrigvoladd[infohi+16] == 0)
		ch->avol += retrigvoladd[infohi]; // add/sub
	else
		ch->avol = (int8_t)((ch->avol * retrigvoladd[infohi+16]) >> 4);

	ch->avol = CLAMP((int8_t)ch->avol, 0, 63);
	setvol(ch);

	ch->atrigcnt++;
}

// 8bb: Sets a variable that isn't read by the replayer. Maybe meant for demo fx syncing?
static void s_zinfo(zchn_t *ch)
{
	(void)ch;
}

static void s_scommand1(zchn_t *ch) // once (8bb: tick=0 only)
{
	GET_LAST_NFO
	ssoncejmp[ch->info >> 4](ch);
}

static void s_scommand2(zchn_t *ch) // often (8bb: tick>0 only)
{
	GET_LAST_NFO
	ssotherjmp[ch->info >> 4](ch);
}

static void s_patterdelay(zchn_t *ch)
{
	if (song.patterndelay == 0)
		song.patterndelay = ch->info & 0xF;
}

static void s_notecut(zchn_t *ch)
{
	ch->anotecutcnt = ch->info & 0xF;
}

static void s_notecutb(zchn_t *ch)
{
	if (ch->anotecutcnt > 0)
	{
		ch->anotecutcnt--;
		if (ch->anotecutcnt == 0)
			ch->m_speed = 0; // 8bb: shut down voice (recoverable by using pitch effects)
	}
}

static void s_notedelay(zchn_t *ch)
{
	ch->anotedelaycnt = ch->info & 0xF;
}

static void s_notedelayb(zchn_t *ch)
{
	if (ch->anotedelaycnt > 0)
	{
		ch->anotedelaycnt--;
		if (ch->anotedelaycnt == 0)
			donewnote(ch->channelnum, true);
	}
}

static void s_setfilt(zchn_t *ch) // S0x
{
	/* "Amiga" low-pass filter
	**
	** Yet another mixer effect that is broken in ST3.21.
	** First of all, this needs the "+32:enable filter/sfx with sb"
	** legacy flag to be set (which you can't set in ST3.01 and later),
	** and secondly, it completely messes up the sound when ran in ST3.21.
	** In other words, not worth implementing...
	*/
	(void)ch;
}

static void s_setgvol(zchn_t *ch)
{
	if (ch->info <= 64)
		setglobalvol(ch->info);
}

static void s_setfinetune(zchn_t *ch) // S2x
{
	/* 8bb: In ST3.21, this effect is bugged in a way where the finetune is
	** actually not applied (period not updated), but the channel's internal
	** c2spd is still updated. This affects portamentos/arpeggios, and also
	** following notes without a sample number.
	**
	** I only implemented the code that was needed to simulate the quirk,
	** instead of implemending the buggy code that attempted to update the
	** channel period (but fails).
	*/

	ch->ac2spd = xfinetune_amiga[ch->info & 0xF];
}

static void s_setvibwave(zchn_t *ch)
{
	ch->avibtretype = (ch->avibtretype & 0xF0) | ((ch->info << 1) & 0x0F);
}

static void s_settrewave(zchn_t *ch)
{
	ch->avibtretype = ((ch->info << 5) & 0xF0) | (ch->avibtretype & 0x0F);
}

static void s_stereocntr(zchn_t *ch) // SAx
{
	/* 8bb: Sound Blaster Pro L/R channel output selector (undocumented ST3 effect):
	** - SA0 = normal
	** - SA1 = swapped (L<->R)
	** - SA2 = normal  (bugged, has DC offset)
	** - SA3 = swapped (bugged, has DC offset)
	** - SA4 = center  (bugged, has DC offset)
	** - SA5 = center  (bugged, has DC offset)
	** - SA6 = center
	** - SA7 = center  (bugged, has DC offset)
	*/

	if ((ch->info & 0xF) <= 7)
		ch->amixtype = ch->info & 0xF;
}

static void s_patloop(zchn_t *ch)
{
	if ((ch->info & 0xF) == 0)
	{
		song.patloopstart = song.np_row;
		return;
	}

	if (song.patloopcount == 0)
	{
		song.patloopcount = (ch->info & 0xF) + 1;
		if (song.patloopstart == -1)
			song.patloopstart = 0; // default loopstart
	}

	if (song.patloopcount > 1)
	{
		song.patloopcount--;
		song.jumptorow = song.patloopstart;
		song.np_patoff = -1; // force reseek
	}
	else
	{
		song.patloopcount = 0;
		song.patloopstart = song.np_row + 1;
	}
}

static void s_setgliss(zchn_t *ch)
{
	ch->aglis = ch->info & 0xF;
}

static void s_setpanpos(zchn_t *ch)
{
	// 8bb: the 0xF0 part means that this channel has a pan set
	ch->apanpos = 0xF0 | (ch->info & 0xF);

	// 8bb: this forces a GUS update, and sample trigger (pos set to last ch->m_pos (?) ), so you get a click!
	ch->m_oldpos = 0xFFFFFFFF;
}
