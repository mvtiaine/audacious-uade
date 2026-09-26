/***********************************************************************
 **
 **  ADLIB note handling
 **
 ** 8bb: ST3.21 has code for AdLib percussion mode, but in the end it's
 ** never handled, so I didn't port it. It's also impossible to make
 ** percussion instruments in public ST3 versions.
 **
 ***********************************************************************/

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "digdata.h"
#include "dig.h"
#include "opl2/opl2.h"

static const uint8_t emptyadlibins[12] = { 0,0,63,63,0,0,0,0,0,0,0,0 };
static const uint8_t adlibiadd[9] = { 0,1,2,8,9,10,16,17,18 }; // melodic sounds 0..8
static uint8_t adlibmem[256];

static void outaw(uint8_t reg, uint8_t data)
{
	if (data == adlibmem[reg])
		return;

	adlibmem[reg] = data;

	OPL2_WritePort(reg, data);
}

static void outnote(uint8_t channel, uint16_t note)
{
	outaw(0xA0+channel, (uint8_t)note);
	outaw(0xB0+channel, note >> 8);
}

static void adlibloadins(uint8_t channel, const uint8_t *adLibIns)
{
	uint8_t data;

	assert(channel <= 8);

	uint8_t reg = 0x20 + adlibiadd[channel];
	for (int32_t i = 0; i < 4; i++)
	{
		data = *adLibIns++;
		outaw(reg, data);
		reg += 3;

		data = *adLibIns++;
		outaw(reg, data);
		reg += 32-3;
	}

	reg += 64;
	data = *adLibIns++;
	outaw(reg, data);

	reg += 3;
	data = *adLibIns++;
	outaw(reg, data);

	reg = 0xC0 + channel;
	data = *adLibIns++;
	outaw(reg, data);
}

void initadlib(void)
{
	memset(adlibmem, 0xFC, 256);

	outaw(0x01, 0x20);
	outaw(0x08, 0x00);
	outaw(0xBD, 0x00);

	for (uint8_t ch = 0; ch < 9; ch++)
	{
		adlibloadins(ch, emptyadlibins);
		outnote(ch, 0);
	}
}

void doadlib(zchn_t *ch, uint8_t adLibCh)
{
	assert(adLibCh <= 8);

	song.adlibused = true;

	// INSTRUMENT***
	if (ch->ins != 0)
	{
		ch->addherzretrigvol = 1; // for default volume to be noticed

		if (ch->ins < MAX_INSTRUMENTS) // 8bb: added this protection!
		{
			bool reloadIns = false;
			if (ch->ins != ch->lastadlins)
			{
				ch->lastadlins = ch->ins;
				reloadIns = true;
			}

			ds_adl *ins = (ds_adl *)&song.ins[ch->ins-1];
			if (ins->type != 2) // adlibins
			{
				ch->lastadlins = 0;
				return;
			}

			uint16_t c2spd = (uint16_t)ins->c2spd; // 8bb: this will cause problems if adlib c2spd > 65535
			if (c2spd < 1000)
				c2spd = C2FREQ;

			ch->ac2spd = c2spd;

			ch->avol = ins->vol;
			setvol(ch);

			if (reloadIns)
				adlibloadins(adLibCh, (uint8_t *)&ins->D00);
		}
	}

	// NOTE***
	if (ch->note != 255)
	{
		if (ch->cmd != 'G'-64 && ch->note != 254)
			ch->addherzretrig = 1; // restart sample

		// calc note speed

		ch->lastnote = ch->note;

		/* 8bitbubsy:
		** Tone portamento is broken in OPL mode on ST3 releases after ST3.01.
		**
		** OpenMPT 1.31 and later also emulates this bug after discovering it,
		** so let's check for that too.
		*/
		const bool brokenPortamentos = (song.header.cwtv > 0x1301 && song.header.cwtv <= 0x1320)
			|| (song.header.cwtv >= 0x5131 && song.header.cwtv <= 0x5FFF);

		uint16_t spd = scalec2spd(ch, stnote2herz(ch->note));
		if (ch->cmd != 'G'-64)
		{
			ch->aspd = spd;
			setspd(ch);

			if (!brokenPortamentos)
				ch->aorgspd = spd; // original speed if true one changed with vibrato etc.
		}

		if (brokenPortamentos)
			ch->aorgspd = spd; // original speed if true one changed with vibrato etc.

		ch->asldspd = spd; // destination for toneslide (G)
	}

	// VOLUME***
	if (ch->vol != 255)
	{
		ch->avol = ch->vol;
		if ((uint8_t)ch->avol > 63)
			ch->avol = 63;

		ch->aorgvol = ch->avol;

		ch->addherzretrigvol = 1;
		setvol(ch);
	}
}

void updateadlib(void)
{
	// outputs all notes/freqs to ADLIB
	zchn_t *ch = &song._zchn[16];
	for (uint8_t i = 0; i < 9; i++, ch++)
	{
		if (!(ch->addherzhi & 32768))
		{
			// calc herz->adlib
			uint32_t hz = (ch->addherzhi << 16) | ch->addherzlo; // 8bb: ST3 hertz from period

			hz <<= 1;

			uint8_t block = 0;
			while (hz >= 3125)
			{
				block++;
				hz >>= 1;
			}

			block = (block << 2) | 32;
			hz <<= 10;

			uint16_t fnum = (uint16_t)(hz / 3125);
			uint16_t note = (block << 8) | fnum;

			// out the calculated note

			if (ch->addherzretrig != 0)
				outnote(i, note & 0xDFFF); // keyoff

			if (ch->addherzretrig != 254)
				outnote(i, note);
		}

		ch->addherzhi |= 32768;

		if (ch->addherzretrigvol != 0)
		{
			// retrig volume

			if (ch->lastadlins > 0) // 8bb: added this protection!
			{
				ds_adl *ins = (ds_adl *)&song.ins[ch->lastadlins-1];

				// calc volumes

				// modulator
				if (ins->D0A & 1)
				{
					uint8_t volOut = (0 - (ins->D02 & 63)) + 63;
					if (ch->avol < 63)
					{
						uint8_t vol = ch->avol;
						if (vol != 0)
							vol++;

						volOut = (volOut * vol) >> 6;
					}

					volOut = (0 - volOut) + 63;
					volOut |= ins->D02 & (64 | 128);

					outaw(0x40 + adlibiadd[i], volOut);
				}

				// carrier
				uint8_t volOut = (0 - (ins->D03 & 63)) + 63;
				if (ch->avol < 63)
				{
					uint8_t vol = ch->avol;
					if (vol != 0)
						vol++;

					volOut = (volOut * vol) >> 6;
				}

				volOut = (0 - volOut) + 63;
				volOut |= ins->D03 & (64 | 128);

				outaw(0x43 + adlibiadd[i], volOut);
			}
		}

		ch->addherzretrig = 0;
	}
}
