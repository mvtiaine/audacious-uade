// SPDX-License-Identifier: BSD-3-Clause
/*
** st23play v0.35 - 31st of December 2020 - https://16-bits.org
** ===========================================================
**                - NOT BIG ENDIAN SAFE! -
**
** Very accurate C port of Scream Tracker 2.3's replayer,
** by Olav "8bitbubsy" Sørensen. Using the original asm source codes
** by Sami "PSI" Tammilehto (Future Crew) with permission, plus a
** disassembly of ST2.3 because the asm code I got was for a newer
** unreleased version.
**
** Keep in mind that the original ST2 replayer has been heavily changed
** between versions, so this one is accurate to how ST2.3 plays .STM files.
**
** Thanks to Sergei "x0r" Kolzun for pointing out that the replayer
** code differ between versions, and his ST2 disassembly help was handy.
**
** You need to link winmm.lib for this to compile (-lwinmm).
** Alternatively, you can change out the mixer functions at the bottom with
** your own for your OS.
**
** Example of st32play usage:
** #include "st23play.h"
** #include "songdata.h"
**
** st23play_PlaySong(songData, songDataLength, true, 48000);
** mainLoop();
** st23play_Close();
**
** To turn a song into an include file like in the example, you can use my
** win32 bin2h tool from here: https://16-bits.org/etc/bin2h.zip
**
** Changes in v0.35:
** - Note cut didn't work because of an earlier bug
**
** Changes in v0.34:
** - Added BLEP synthesis for better audio quality
**
** Changes in v0.33:
** - Resampling interpolation downgraded from 3-tap cubic to 2-tap linear.
**   This can sound sharper on lo-fi samples (which most STM songs have).
** - Audio mixer performance increase
**
** Changes in v0.32:
** - Audio mixer performance increase
** - Added an st23play_GetMasterVol() function
**
** Changes in v0.31:
** - Removed some unneeded logic in the audio channel mixer
**
** Changes in v0.3:
** - Handle illegal pattern notes (>B-4) as no note (ST2.3 doesn't do this, but I do!)
** - Added overflow data to notespd table for arpeggio overflow read bug
** - Handle yet another arpeggio LUT overflow case (thanks x0r)
**
** Changes in v0.2:
** - Vibrato and portamento accidentally shared the same memory (not correct)
** - Pattern separators (in the order list) were not handled correctly
** - Fixed some differences to match ST2.3's replayer (arp. + pitch up/down)
*/

/* st23play.h:

#pragma once

#include <stdint.h>
#include <stdbool.h>

bool st23play_PlaySong(const uint8_t *moduleData, uint32_t dataLength, uint32_t audioFreq);
void st23play_Close(void);
void st23play_PauseSong(bool flag); // true/false
void st23play_TogglePause(void);
void st23play_SetMasterVol(uint16_t vol); // 0..256
uint16_t st23play_GetMasterVol(void); // 0..256
char *st23play_GetSongName(void); // max 20 chars (21 with '\0'), string is in code page 437
uint32_t st23play_GetMixerTicks(void); // returns the amount of milliseconds of mixed audio (not realtime)
*/

#define MIX_BUF_SAMPLES 4096

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// don't change these!
#define BLEP_ZC 16
#define BLEP_OS 16
#define BLEP_SP 16
#define BLEP_NS (BLEP_ZC * BLEP_OS / BLEP_SP)
#define BLEP_RNS 31 // RNS = (2^ > NS) - 1

#define NUM_CHANNELS 4
#define MAX_SAMPLES 31
#define MAX_ORDERS 128
#define C1FREQ 8192
#define BASE_CLOCK 3546895 /* Amiga PAL Paula clock. ST2.3 uses this one */
#define PERIOD_FACTOR 10 /* x times higher resolution than Amiga periods */

#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

// fast 32-bit -> 16-bit clamp
#define CLAMP16(i) if ((int16_t)(i) != i) i = 0x7FFF ^ (i >> 31)

typedef struct st_ins_t
{
	uint16_t segm; // for loader
	// -----------
	int8_t *data;
	uint8_t vol;
	uint16_t length, lp_beg, lp_end, c1spd;
} st_ins_t;

typedef struct chn_t
{
	// mixer variables
	const int8_t *mdat;
	int32_t mpos, mend, mloopbeg, mlooplen;
	double dDelta, dPhase, dVolume, dLastDelta, dLastPhase, dDeltaMul, dLastDeltaMul;
	// -----------
	bool atreon;
	uint8_t mcnt, avol2, avibcnt, atremor, aorgvol;
	uint8_t alastnfo1, alastnfo2, coms, vols, notes, samples;
	uint8_t infos, lastnote;
	uint16_t mpnt, aspd, ac1spd, aorgspd;
} chn_t;

typedef struct blep_t
{
	int32_t index, samplesLeft;
	double dBuffer[BLEP_RNS + 1], dLastValue;
} blep_t;

static char songname[20+1];
static volatile bool musicPaused;
static bool breakpat;
static uint8_t vpnt, subcnt, gvolume = 64, tempo = 96, st1[MAX_ORDERS], st2[65536];
static int32_t mastervol = 256, samplesLeft, soundBufferSize, oversamplingFactor, randSeed = 0x12345000;
static uint32_t samplesPerTick, audioRate, sampleCounter;
static st_ins_t smp[MAX_SAMPLES];
static chn_t chn[NUM_CHANNELS];
static double *dMixBuffer, dPer2HzDiv, dPrngState;
static blep_t blep[NUM_CHANNELS], blepVol[NUM_CHANNELS];

static const uint16_t notespd[80 + 11] =
{
	17120,16160,15240,14400,13560,12800,12080,11400,10760,10160, 9600, 9070, 0,0,0,0,
	 8560, 8080, 7620, 7200, 6780, 6400, 6040, 5700, 5380, 5080, 4800, 4535, 0,0,0,0,
	 4280, 4040, 3810, 3600, 3390, 3200, 3020, 2850, 2690, 2540, 2400, 2267, 0,0,0,0,
	 2140, 2020, 1905, 1800, 1695, 1600, 1510, 1425, 1345, 1270, 1200, 1133, 0,0,0,0,
	 1070, 1010,  952,  900,  847,  800,  755,  712,  672,  635,  600,  566, 0,0,0,0,
	 // +11 overflow values from vibsin, for buggy arpeggio routine
	    0,   24,   49,   74,   97,  120,  141,  161,  180,  197,  212
};

static const int16_t vibsin[64] =
{
	 0x00, 0x18, 0x31, 0x4A, 0x61, 0x78, 0x8D, 0xA1,
	 0xB4, 0xC5, 0xD4, 0xE0, 0xEB, 0xF4, 0xFA, 0xFD,
	 0xFF, 0xFD, 0xFA, 0xF4, 0xEB, 0xE0, 0xD4, 0xC5,
	 0xB4, 0xA1, 0x8D, 0x78, 0x61, 0x4A, 0x31, 0x18,
	 0x00,-0x18,-0x31,-0x4A,-0x61,-0x78,-0x8D,-0xA1,
	-0xB4,-0xC5,-0xD4,-0xE0,-0xEB,-0xF4,-0xFA,-0xFD,
	-0xFF,-0xFD,-0xFA,-0xF4,-0xEB,-0xE0,-0xD4,-0xC5,
	-0xB4,-0xA1,-0x8D,-0x78,-0x61,-0x4A,-0x31,-0x18
};

static const uint8_t slowdowns[16] = { 140,50,25,15,10,7,6,4,3,3,2,2,2,2,1,1 };

static const uint64_t minblepdata[] =
{
	0x3FF000320C7E95A6,0x3FF00049BE220FD5,0x3FF0001B92A41ACA,0x3FEFFF4425AA9724,
	0x3FEFFDABDF6CF05C,0x3FEFFB5AF233EF1A,0x3FEFF837E2AE85F3,0x3FEFF4217B80E938,
	0x3FEFEEECEB4E0444,0x3FEFE863A8358B5F,0x3FEFE04126292670,0x3FEFD63072A0D592,
	0x3FEFC9C9CD36F56F,0x3FEFBA90594BD8C3,0x3FEFA7F008BA9F13,0x3FEF913BE2A0E0E2,
	0x3FEF75ACCB01A327,0x3FEF5460F06A4E8F,0x3FEF2C5C0389BD3C,0x3FEEFC8859BF6BCB,
	0x3FEEC3B916FD8D19,0x3FEE80AD74F0AD16,0x3FEE32153552E2C7,0x3FEDD69643CB9778,
	0x3FED6CD380FFA864,0x3FECF374A4D2961A,0x3FEC692F19B34E54,0x3FEBCCCFA695DD5C,
	0x3FEB1D44B168764A,0x3FEA59A8D8E4527F,0x3FE9814D9B10A9A3,0x3FE893C5B62135F2,
	0x3FE790EEEBF9DABD,0x3FE678FACDEE27FF,0x3FE54C763699791A,0x3FE40C4F1B1EB7A3,
	0x3FE2B9D863D4E0F3,0x3FE156CB86586B0B,0x3FDFCA8F5005B828,0x3FDCCF9C3F455DAC,
	0x3FD9C2787F20D06E,0x3FD6A984CAD0F3E5,0x3FD38BB0C452732E,0x3FD0705EC7135366,
	0x3FCABE86754E238F,0x3FC4C0801A6E9A04,0x3FBDECF490C5EA17,0x3FB2DFFACE9CE44B,
	0x3FA0EFD4449F4620,0xBF72F4A65E22806D,0xBFA3F872D761F927,0xBFB1D89F0FD31F7C,
	0xBFB8B1EA652EC270,0xBFBE79B82A37C92D,0xBFC1931B697E685E,0xBFC359383D4C8ADA,
	0xBFC48F3BFF81B06B,0xBFC537BBA8D6B15C,0xBFC557CEF2168326,0xBFC4F6F781B3347A,
	0xBFC41EF872F0E009,0xBFC2DB9F119D54D3,0xBFC13A7E196CB44F,0xBFBE953A67843504,
	0xBFBA383D9C597E74,0xBFB57FBD67AD55D6,0xBFB08E18234E5CB3,0xBFA70B06D699FFD1,
	0xBF9A1CFB65370184,0xBF7B2CEB901D2067,0x3F86D5DE2C267C78,0x3F9C1D9EF73F384D,
	0x3FA579C530950503,0x3FABD1E5FFF9B1D0,0x3FB07DCDC3A4FB5B,0x3FB2724A856EEC1B,
	0x3FB3C1F7199FC822,0x3FB46D0979F5043B,0x3FB47831387E0110,0x3FB3EC4A58A3D527,
	0x3FB2D5F45F8889B3,0x3FB145113E25B749,0x3FAE9860D18779BC,0x3FA9FFD5F5AB96EA,
	0x3FA4EC6C4F47777E,0x3F9F16C5B2604C3A,0x3F9413D801124DB7,0x3F824F668CBB5BDF,
	0xBF55B3FA2EE30D66,0xBF86541863B38183,0xBF94031BBBD551DE,0xBF9BAFC27DC5E769,
	0xBFA102B3683C57EC,0xBFA3731E608CC6E4,0xBFA520C9F5B5DEBD,0xBFA609DC89BE6ECE,
	0xBFA632B83BC5F52F,0xBFA5A58885841AD4,0xBFA471A5D2FF02F3,0xBFA2AAD5CD0377C7,
	0xBFA0686FFE4B9B05,0xBF9B88DE413ACB69,0xBF95B4EF6D93F1C5,0xBF8F1B72860B27FA,
	0xBF8296A865CDF612,0xBF691BEEDABE928B,0x3F65C04E6AF9D4F1,0x3F8035D8FFCDB0F8,
	0x3F89BED23C431BE3,0x3F90E737811A1D21,0x3F941C2040BD7CB1,0x3F967046EC629A09,
	0x3F97DE27ECE9ED89,0x3F98684DE31E7040,0x3F9818C4B07718FA,0x3F97005261F91F60,
	0x3F95357FDD157646,0x3F92D37C696C572A,0x3F8FF1CFF2BEECB5,0x3F898D20C7A72AC4,
	0x3F82BC5B3B0AE2DF,0x3F7784A1B8E9E667,0x3F637BB14081726B,0xBF4B2DACA70C60A9,
	0xBF6EFB00AD083727,0xBF7A313758DC6AE9,0xBF819D6A99164BE0,0xBF8533F57533403B,
	0xBF87CD120DB5D340,0xBF89638549CD25DE,0xBF89FB8B8D37B1BB,0xBF89A21163F9204E,
	0xBF886BA8931297D4,0xBF8673477783D71E,0xBF83D8E1CB165DB8,0xBF80BFEA7216142A,
	0xBF7A9B9BC2E40EBF,0xBF7350E806435A7E,0xBF67D35D3734AB5E,0xBF52ADE8FEAB8DB9,
	0x3F415669446478E4,0x3F60C56A092AFB48,0x3F6B9F4334A4561F,0x3F724FB908FD87AA,
	0x3F75CC56DFE382EA,0x3F783A0C23969A7B,0x3F799833C40C3B82,0x3F79F02721981BF3,
	0x3F7954212AB35261,0x3F77DDE0C5FC15C9,0x3F75AD1C98FE0777,0x3F72E5DACC0849F2,
	0x3F6F5D7E69DFDE1B,0x3F685EC2CA09E1FD,0x3F611D750E54DF3A,0x3F53C6E392A46D17,
	0x3F37A046885F3365,0xBF3BB034D2EE45C2,0xBF5254267B04B482,0xBF5C0516F9CECDC6,
	0xBF61E5736853564D,0xBF64C464B9CC47AB,0xBF669C1AEF258F56,0xBF67739985DD0E60,
	0xBF675AFD6446395B,0xBF666A0C909B4F78,0xBF64BE9879A7A07B,0xBF627AC74B119DBD,
	0xBF5F86B04069DC9B,0xBF597BE8F754AF5E,0xBF531F3EAAE9A1B1,0xBF496D3DE6AD7EA3,
	0xBF3A05FFDE4670CF,0xBF06DF95C93A85CA,0x3F31EE2B2C6547AC,0x3F41E694A378C129,
	0x3F4930BF840E23C9,0x3F4EBB5D05A0D47D,0x3F51404DA0539855,0x3F524698F56B3F33,
	0x3F527EF85309E28F,0x3F51FE70FE2513DE,0x3F50DF1642009B74,0x3F4E7CDA93517CAE,
	0x3F4A77AE24F9A533,0x3F45EE226AA69E10,0x3F411DB747374F52,0x3F387F39D229D97F,
	0x3F2E1B3D39AF5F8B,0x3F18F557BB082715,0xBEFAC04896E68DDB,0xBF20F5BC77DF558A,
	0xBF2C1B6DF3EE94A4,0xBF3254602A816876,0xBF354E90F6EAC26B,0xBF3709F2E5AF1624,
	0xBF379FCCB331CE8E,0xBF37327192ADDAD3,0xBF35EA998A894237,0xBF33F4C4977B3489,
	0xBF317EC5F68E887B,0xBF2D6B1F793EB773,0xBF2786A226B076D9,0xBF219BE6CEC2CA36,
	0xBF17D7F36D2A3A18,0xBF0AAEC5BBAB42AB,0xBEF01818DC224040,0x3EEF2F6E21093846,
	0x3F049D6E0060B71F,0x3F0E598CCAFABEFD,0x3F128BC14BE97261,0x3F148703BC70EF6A,
	0x3F1545E1579CAA25,0x3F14F7DDF5F8D766,0x3F13D10FF9A1BE0C,0x3F1206D5738ECE3A,
	0x3F0F99F6BF17C5D4,0x3F0AA6D7EA524E96,0x3F0588DDF740E1F4,0x3F0086FB6FEA9839,
	0x3EF7B28F6D6F5EED,0x3EEEA300DCBAF74A,0x3EE03F904789777C,0x3EC1BFEB320501ED,
	0xBEC310D8E585A031,0xBED6F55ECA7E151F,0xBEDFDAA5DACDD0B7,0xBEE26944F3CF6E90,
	0xBEE346894453BD1F,0xBEE2E099305CD5A8,0xBEE190385A7EA8B2,0xBEDF4D5FA2FB6BA2,
	0xBEDAD4F371257BA0,0xBED62A9CDEB0AB32,0xBED1A6DF97B88316,0xBECB100096894E58,
	0xBEC3E8A76257D275,0xBEBBF6C29A5150C9,0xBEB296292998088E,0xBEA70A10498F0E5E,
	0xBE99E52D02F887A1,0xBE88C17F4066D432,0xBE702A716CFF56CA,0x3E409F820F781F78,
	0x3E643EA99B770FE7,0x3E67DE40CDE0A550,0x3E64F4D534A2335C,0x3E5F194536BDDF7A,
	0x3E5425CEBE1FA40A,0x3E46D7B7CC631E73,0x3E364746B6582E54,0x3E21FC07B13031DE,
	0x3E064C3D91CF7665,0x3DE224F901A0AFC7,0x3DA97D57859C74A4,0x0000000000000000,

	// extra padding needed for interpolation
	0x0000000000000000
};

static bool openMixer(uint32_t audioFreq);
static void closeMixer(void);

// CODE START

const double *get_minblep_table(void) { return (const double *)minblepdata; }

#define LERP(x, y, z) ((x) + ((y) - (x)) * (z))

void blepAdd(blep_t *b, double dOffset, double dAmplitude)
{
	double f = dOffset * BLEP_SP;

	int32_t i = (int32_t)f; // get integer part of f
	const double *dBlepSrc = get_minblep_table() + i;
	f -= i; // remove integer part from f

	i = b->index;
	for (int32_t n = 0; n < BLEP_NS; n++)
	{
		b->dBuffer[i] += dAmplitude * LERP(dBlepSrc[0], dBlepSrc[1], f);
		dBlepSrc += BLEP_SP;

		i = (i + 1) & BLEP_RNS;
	}

	b->samplesLeft = BLEP_NS;
}

static void blepVolAdd(blep_t *b, double dAmplitude)
{
	const double *dBlepSrc = get_minblep_table();

	int32_t i = b->index;
	for (int32_t n = 0; n < BLEP_NS; n++)
	{
		b->dBuffer[i] += dAmplitude * (*dBlepSrc);
		dBlepSrc += BLEP_SP;

		i = (i + 1) & BLEP_RNS;
	}

	b->samplesLeft = BLEP_NS;
}

static double blepRun(blep_t *b, double dInput)
{
	double dBlepOutput = dInput + b->dBuffer[b->index];
	b->dBuffer[b->index] = 0.0;

	b->index = (b->index + 1) & BLEP_RNS;

	b->samplesLeft--;
	return dBlepOutput;
}

static void setspd(chn_t *c) // updates stspeeds with c->aspd
{
	if (c->aspd < 551)
	{
		c->dDelta = 0.0;
		c->dDeltaMul = 1.0;
	}
	else
	{
		c->dDelta = dPer2HzDiv / (int32_t)c->aspd;
		c->dDeltaMul = 1.0 / c->dDelta;
	}

	// for BLEP synthesis
	if (c->dLastDelta == 0.0) c->dLastDelta = c->dDelta;
	if (c->dLastDeltaMul == 0.0) c->dLastDeltaMul = c->dDeltaMul;
}

static void _znewtempo(uint8_t val)
{
	uint16_t hz = 50;

	tempo = val >> 4;

	hz -= (slowdowns[val >> 4] * (val & 15)) >> 4; // can and will underflow
	samplesPerTick = audioRate / hz;
}

static void nextpat(void) // 8bitbubsy: this routine is not directly ported, but it's even safer
{
	vpnt++;
	if (vpnt >= MAX_ORDERS || st1[vpnt] == 99) // 8bitbubsy: added vpnt>127 check (prevents Bxx (xx>127) LUT overflow)
		vpnt = 0;

	// 8bitbubsy: skip pattern separators
	while (vpnt < MAX_ORDERS && st1[vpnt] == 98)
		vpnt++;

	if (vpnt == MAX_ORDERS)
		vpnt = 0;

	if (st1[vpnt] == 98)
		return; // panic!

	for (int16_t i = 0; i < NUM_CHANNELS; i++)
	{
		chn[i].mpnt = (st1[vpnt] << 10) + (i << 2);
		chn[i].mcnt = 0;
	}
}

static void volslide(chn_t *c)
{
	if ((c->infos & 15) == 0)
	{
		c->avol2 += c->infos >> 4;
		if (c->avol2 > 64)
			c->avol2 = 64;
	}
	else
	{
		c->avol2 -= c->infos & 15;
		if ((int8_t)c->avol2 < 0)
			c->avol2 = 0;
	}
}

static void toneslide(chn_t *c, bool effectOFlag)
{
	uint8_t info = (effectOFlag || c->infos == 0) ? c->alastnfo2 : c->infos;
	c->alastnfo2 = info;

	if (c->aspd == c->aorgspd)
		return;

	if (c->aspd > c->aorgspd)
	{
		c->aspd -= info*PERIOD_FACTOR;
		if ((int16_t)c->aspd < (int16_t)c->aorgspd)
			c->aspd = c->aorgspd;
	}
	else
	{
		c->aspd += info*PERIOD_FACTOR;
		if ((int16_t)c->aspd > (int16_t)c->aorgspd)
			c->aspd = c->aorgspd;
	}

	setspd(c);
}

static void vibrato(chn_t *c, bool effectKFlag)
{
	uint8_t info = (effectKFlag || c->infos == 0) ? c->alastnfo1 : c->infos;
	c->alastnfo1 = info;

	c->aspd = c->aorgspd + (((vibsin[c->avibcnt] * (info & 15)) >> 7) * PERIOD_FACTOR);
	setspd(c);

	c->avibcnt = (c->avibcnt + (info >> 4)) & 63;
}

static void spec2(chn_t *c) // special codes excecuted 5 times
{
	if (c->coms == 9) // tremor
	{
		if (c->atremor != 0)
		{
			c->atremor--;
			return;
		}

		if (c->atreon)
		{
			// set to off
			c->atreon = false;
			c->avol2 = 0;
			c->atremor = c->infos & 15;
		}
		else
		{
			// set to on
			c->atreon = true;
			c->avol2 = c->aorgvol;
			c->atremor = c->infos >> 4;
		}

		return;
	}
	else if (c->coms == 10) // arpeggio
	{
		uint8_t note, octa, arptick = subcnt % 3;

		// 8bitbubsy: weird ST2.3 arpeggio order
		if (arptick == 0)
			note = c->infos & 15;
		else if (arptick == 1)
			note = 0;
		else 
			note = c->infos >> 4;

		// check for octave overflow
		note += c->lastnote & 15;
		octa = c->lastnote & (255-15);

		if (note >= 11) // 8bitbubsy: this ought to be >= 12, a bug in ST2.3
		{
			note -= 12;
			octa += 16;
		}

		if (note == 255) // bug
			c->aspd = 59079; // LUT overflow junk (thanks x0r)
		else
			c->aspd = notespd[note | octa];

		if (c->ac1spd != 0)
			c->aspd = (c->aspd * C1FREQ) / c->ac1spd;

		c->aorgspd = c->aspd;
		setspd(c);

		return;
	}

	c->atremor = 0;
	c->atreon = true;

	if (c->coms == 7) // toneslide
	{
		toneslide(c, false);
		return;
	}
	else if (c->coms == 8) // vibrato
	{
		vibrato(c, false);
		return;
	}

	c->avibcnt = 0;

	if (c->coms == 5) // pitch slide down
	{
		c->aspd += c->infos * PERIOD_FACTOR;
		setspd(c);
		return;
	}
	else if (c->coms == 6) // pitch slide up
	{
		c->aspd -= c->infos * PERIOD_FACTOR;
		setspd(c);
		return;
	}
	else if (c->coms == 15) // volslide + toneslide
	{
		volslide(c);
		toneslide(c, true);
		return;
	}
	else if (c->coms == 11) // voslide + vibrato
	{
		volslide(c);
		vibrato(c, true);
		return;
	}

	if (c->aorgspd != c->aspd)
	{
		c->aspd = c->aorgspd;
		setspd(c);
	}

	if (c->coms == 4) // volslide
	{
		volslide(c);
		return;
	}
}

static void spec1(chn_t *c) // special codes excecuted only once.
{
	if (c->coms == 1) // A - set tempo
	{
		if (c->infos > 0)
			_znewtempo(c->infos);
	}
	else if (c->coms == 2) // B - position jump
	{
		vpnt = c->coms;
	}
	else if (c->coms == 3) // C - pattern break
	{
		breakpat = true;
	}
}

static void donote1(chn_t *c)
{
	st_ins_t *s;

	if (c->vols != 65) // custom volume set
		c->avol2 = c->aorgvol = c->vols;

	if (c->coms == 7) // toneslide
	{
		if (c->notes != 255)
		{
			// original speed if true one changed with vibrato etc.
			c->aorgspd = notespd[c->notes];
			if (c->ac1spd != 0)
				c->aorgspd = (c->aorgspd * C1FREQ) / c->ac1spd;
		}

		return;
	}

	if (c->samples > 0)
	{
		s = &smp[c->samples-1];
		if (c->vols == 65) // default volume specified
			c->avol2 = c->aorgvol = s->vol;

		c->ac1spd = s->c1spd;
		if (s->data == NULL)
			c->notes = 254; // quit the note if no sample

		c->mdat = s->data;
		c->mpos = 0; // reset pos

		if (s->lp_end == 65535) // no loop?
		{ 
			c->mloopbeg = 65535;
			c->mend = s->length;
		}
		else
		{
			// looping sound
			c->mend = s->lp_end;
			c->mloopbeg = s->lp_beg;
			c->mlooplen = s->lp_end - s->lp_beg; // 8bitbubsy: added for my mixer
		}
	}

	// volume set, now calc the speed

	if (c->notes == 254)
	{
		// quiet
		c->dPhase = 0.0;
		c->mdat = NULL;
	}
	else if (c->notes != 255)
	{
		c->lastnote = c->notes;

		c->aorgspd = notespd[c->notes];
		if (c->ac1spd != 0)
			c->aorgspd = (c->aorgspd * C1FREQ) / c->ac1spd;

		c->aspd = c->aorgspd;
		setspd(c);

		c->dPhase = 0.0;
		c->mpos = 0;
	}

	spec1(c);
}

static void getnewnote(chn_t *c)
{
	uint8_t *pattptr = &st2[c->mpnt];

	c->mcnt++;
	if (c->mcnt >= 64)
		breakpat = true;

	c->notes = pattptr[0];

	// 8bitbubsy: added clamping to prevent buffer overflow
	if (c->notes > 75 && c->notes < 254) // B-4 (16*4)+11
		c->notes = 75;

	c->samples = pattptr[1] >> 3;
	c->vols = ((pattptr[2] >> 1) & 0x78) | (pattptr[1] & 7);

	// 8bitbubsy: sanity clamping
	if (c->vols > 65)
		c->vols = 65; // no volume

	c->coms = pattptr[2] & 15;
	c->infos = pattptr[3];

	c->mpnt += 16;
	donote1(c);

	if (c->coms == 9)
		spec2(c);
}

static void imusic(void)
{
	int32_t i;

	if (subcnt == 0)
	{
		if (breakpat)
		{
			breakpat = false;
			nextpat();
		}

		for (i = 0; i < NUM_CHANNELS; i++)
			getnewnote(&chn[i]);

		subcnt = tempo;
		if (subcnt != 0)
			subcnt--;
	}
	else
	{
		subcnt--;

		// no new note
		for (i = 0; i < NUM_CHANNELS; i++)
			spec2(&chn[i]);
	}

	for (i = 0; i < NUM_CHANNELS; i++)
		chn[i].dVolume = (chn[i].avol2 * gvolume) * (1.0 / 4096.0); // 8bitbubsy: mvol=0..4096 (0..64 in original code)
}

/* 8bitbubsy: The ST2.3 mixer has been heavily improved:
** - No interpolation -> BLEP synthesis
** - Mixing rate can be higher than 23863Hz
** - Higher volume precision
** - Proper wrapping of loop (takes overflow samples into account)
** - Sample position is now increased _after_ mixing the sample
*/

static void mixChannels(int32_t numSamples)
{
	double dSmp, dVol;
	blep_t *bSmp, *bVol;
	chn_t *c;

	memset(dMixBuffer, 0, numSamples * sizeof (double) * oversamplingFactor);

	c = chn;
	bSmp = blep;
	bVol = blepVol;

	const int32_t samplesToMix = numSamples * oversamplingFactor;
	for (int32_t i = 0; i < NUM_CHANNELS; i++, c++, bSmp++, bVol++)
	{
		if (c->mdat == NULL)
			continue;

		for (int32_t j = 0; j < samplesToMix; j++)
		{
			dSmp = c->mdat[c->mpos] * (1.0 / 128.0);
			dVol = c->dVolume;

			if (dSmp != bSmp->dLastValue)
			{
				if (c->dLastDelta > c->dLastPhase)
				{
					// div->mul trick: v->dLastDeltaMul is 1.0 / v->dLastDelta
					blepAdd(bSmp, c->dLastPhase * c->dLastDeltaMul, bSmp->dLastValue - dSmp);
				}

				bSmp->dLastValue = dSmp;
			}

			if (dVol != bVol->dLastValue)
			{
				blepVolAdd(bVol, bVol->dLastValue - dVol);
				bVol->dLastValue = dVol;
			}

			if (bSmp->samplesLeft > 0) dSmp = blepRun(bSmp, dSmp);
			if (bVol->samplesLeft > 0) dVol = blepRun(bVol, dVol);

			dMixBuffer[j] += dSmp * dVol;

			c->dPhase += c->dDelta;
			if (c->dPhase >= 1.0)
			{
				c->dPhase -= 1.0;

				c->dLastPhase = c->dPhase;
				c->dLastDelta = c->dDelta;
				c->dLastDeltaMul = c->dDeltaMul;

				if (++c->mpos >= c->mend)
				{
					if (c->mloopbeg == 65535)
					{
						// no loop
						c->mdat = NULL;
						break;
					}
					else
					{
						c->mpos = c->mloopbeg;
					}
				}
			}
		}
	}
}

static inline int32_t random32(void)
{
	// LCG random 32-bit generator (quite good and fast)
	randSeed *= 134775813;
	randSeed++;
	return randSeed;
}

static void mixAudio(int16_t *stream, int32_t sampleBlockLength)
{
	if (musicPaused)
	{
		memset(stream, 0, sampleBlockLength * (sizeof (int16_t) * oversamplingFactor));
		return;
	}

	mixChannels(sampleBlockLength);

	const double *dMixBufferPtr = dMixBuffer;
	if (oversamplingFactor == 1)
	{
		for (int32_t i = 0; i < sampleBlockLength; i++)
		{
			double dOut = dMixBufferPtr[i] * (16384.0 * 0.5);

			// 1-bit triangular dithering (high-pass filtered)
			const double dPrng = random32() * (0.5 / INT32_MAX); // -0.5..0.5
			dOut = (dOut + dPrng) - dPrngState;
			dPrngState = dPrng;
			int32_t out32 = (int32_t)dOut;
			CLAMP16(out32);

			const int16_t out16 = (int16_t)out32;
			*stream++ = out16;
			*stream++ = out16;
		}
	}
	else
	{
		for (int32_t i = 0; i < sampleBlockLength; i++)
		{
			const double dSample = ((*dMixBufferPtr++) + (*dMixBufferPtr++)) * 0.5;
			double dOut = dSample * (16384.0 * 0.5);

			// 1-bit triangular dithering (high-pass filtered)
			const double dPrng = random32() * (0.5 / INT32_MAX); // -0.5..0.5
			dOut = (dOut + dPrng) - dPrngState;
			dPrngState = dPrng;
			int32_t out32 = (int32_t)dOut;
			CLAMP16(out32);

			const int16_t out16 = (int16_t)out32;
			*stream++ = out16;
			*stream++ = out16;
		}
	}
}

static void st23play_FillAudioBuffer(int16_t *buffer, int32_t samples)
{
	int32_t a, b;

	a = samples;
	while (a > 0)
	{
		if (samplesLeft == 0)
		{
			// new replayer tick
			if (!musicPaused)
				imusic();

			samplesLeft = samplesPerTick;
		}

		b = a;
		if (b > samplesLeft)
			b = samplesLeft;

		mixAudio(buffer, b);
		buffer += (uint32_t)b * 2;

		a -= b;
		samplesLeft -= b;
	}

	sampleCounter += samples;
}

void st23play_Close(void)
{
	closeMixer();

	if (dMixBuffer != NULL)
	{
		free(dMixBuffer);
		dMixBuffer = NULL;
	}

	for (uint8_t i = 0; i < MAX_SAMPLES; i++)
	{
		if (smp[i].data != NULL)
		{
			free(smp[i].data);
			smp[i].data = NULL;
		}
	}
}

void st23play_PauseSong(bool flag)
{
	musicPaused = flag;
}

void st23play_TogglePause(void)
{
	musicPaused ^= 1;
}

void st23play_SetMasterVol(uint16_t vol)
{
	mastervol = CLAMP(vol, 0, 256);
}

uint16_t st23play_GetMasterVol(void)
{
	return (uint16_t)mastervol;
}

char *st23play_GetSongName(void)
{
	return songname;
}

uint32_t st23play_GetMixerTicks(void)
{
	if (audioRate < 1000)
		return 0;

	return sampleCounter / (audioRate / 1000);
}

static bool loadSTM(const uint8_t *dat, uint32_t modLen)
{
	uint8_t pats, *pattdata;
	uint16_t a, ver;
	st_ins_t *s;

	ver = (dat[30] * 100) + dat[31];
	if (dat[29] != 2 || (ver != 200 && ver != 210 && ver != 220 && ver != 221))
		return false; // unsupported

	memset(smp, 0, sizeof (smp));
	memset(st1, 99, sizeof (st1)); // 8bitbubsy: added for safety

	memcpy(songname, dat, 20);
	songname[20] = '\0';

	gvolume = 64;

	tempo = dat[32];
	if (ver < 221)
		tempo = (tempo % 10) + (tempo / 10) * 16;

	pats = dat[33];

	if (ver > 210)
	{
		gvolume = dat[34];
		if (gvolume > 64) // 8bitbubsy: added for safety
			gvolume = 64;
	}

	// load sample information
	s = smp;
	for (a = 0; a < MAX_SAMPLES; a++, s++)
	{
		uint8_t *sdat = (uint8_t *)&dat[48 + (a * 32)];

		s->segm = *(uint16_t *)&sdat[14];
		s->length = *(uint16_t *)&sdat[16];
		s->lp_beg = *(uint16_t *)&sdat[18];
		s->lp_end = *(uint16_t *)&sdat[20];
		s->vol = sdat[22];
		s->c1spd = *(uint16_t *)&sdat[24];

		if (s->lp_end == 0)
			s->lp_end = 65535;

		// 8bitbubsy: added loop overflow handling (not present in ST2.3)
		if (s->lp_end != 65535)
		{
			if (s->lp_end > s->length)
				s->lp_end = s->length;

			if (s->lp_beg >= s->lp_end)
			{
				s->lp_beg = 0;
				s->lp_end = 65535;
			}
		}

		if (s->vol > 64) // 8bitbubsy: added for safety
			s->vol = 64;
	}

	memcpy(st1, &dat[1040], (ver == 200) ? 64 : 128); // read order list
	memset(st2, 0, sizeof (st2)); // wipe pattern data

	// load pattern data
	pattdata = (uint8_t *)&dat[(ver == 200) ? (1040+64) : (1040+128)];
	for (a = 0; a < pats; a++)
	{
		for (uint16_t u = 0; u < 1024;)
		{
			uint8_t c = *pattdata++;
			if (c == 251)
			{
				st2[a*1024+u]=0; u++;
				st2[a*1024+u]=0; u++;
				st2[a*1024+u]=0; u++;
				st2[a*1024+u]=0; u++;
			}
			else if (c == 253)
			{
				st2[a*1024+u]=254; u++;
				st2[a*1024+u]=1; u++;
				st2[a*1024+u]=128; u++;
				st2[a*1024+u]=0; u++;
			}
			else if (c==252)
			{
				st2[a*1024+u]=255; u++;
				st2[a*1024+u]=1; u++;
				st2[a*1024+u]=128; u++;
				st2[a*1024+u]=0; u++;
			}
			else
			{
				st2[a*1024+u]=c; u++;
				st2[a*1024+u]=*pattdata++; u++;
				st2[a*1024+u]=*pattdata++; u++;
				st2[a*1024+u]=*pattdata++; u++;

				if (ver < 221 && (st2[a*1024+u-2] & 15) == 1)
				{
					uint8_t d, b;
					d=st2[a*1024+u-1];
					b=d/10;
					d%=10;
					d+=b*16;
					st2[a*1024+u-1]=d;
				}
			}
		}
	}

	// load sample data
	s = smp;
	for (a = 0; a < MAX_SAMPLES; a++, s++)
	{
		uint32_t smpOffsetInFile = s->segm << 4;

		if (s->vol == 0 || s->length == 0 || s->c1spd == 0 || smpOffsetInFile >= modLen)
			continue;

		s->data = (int8_t *)malloc(s->length);
		if (s->data == NULL)
			return false;

		memcpy(s->data, &dat[smpOffsetInFile], s->length); // copy over sample data
	}

	return true;
}

bool st23play_PlaySong(const uint8_t *moduleData, uint32_t dataLength, uint32_t audioFreq)
{
	st23play_Close();
	memset(songname, 0, sizeof (songname));

	if (audioFreq == 0)
		audioFreq = 44100;

	audioFreq = CLAMP(audioFreq, 44100, 96000);

	oversamplingFactor = 2;
	if (audioFreq >= 64372)
		oversamplingFactor = 1;

	dPer2HzDiv = (double)(BASE_CLOCK * PERIOD_FACTOR) / (audioFreq * oversamplingFactor);

	sampleCounter = 0;
	musicPaused = true;
	audioRate = audioFreq;
	soundBufferSize = MIX_BUF_SAMPLES;

	dMixBuffer = (double *)malloc(MIX_BUF_SAMPLES * sizeof (double) * oversamplingFactor);
	if (dMixBuffer == NULL || !openMixer(audioRate) || !loadSTM(moduleData, dataLength))
		goto playError;

	memset(chn, 0, sizeof (chn));
	_znewtempo(tempo);
	subcnt = 0;
	breakpat = false;
	vpnt = 127-1; // will be set to 0 in nextpat()
	nextpat();
	dPrngState = 0.0;
	musicPaused = false;
	return true;

playError:
	st23play_Close();
	return false;
}

// the following must be changed if you want to use another audio API than WinMM

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <mmsystem.h>

#define MIX_BUF_NUM 2

static volatile BOOL audioRunningFlag;
static uint8_t currBuffer;
static int16_t *mixBuffer[MIX_BUF_NUM];
static HANDLE hThread, hAudioSem;
static WAVEHDR waveBlocks[MIX_BUF_NUM];
static HWAVEOUT hWave;

static DWORD WINAPI mixThread(LPVOID lpParam)
{
	WAVEHDR *waveBlock;

	(void)lpParam;

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	while (audioRunningFlag)
	{
		waveBlock = &waveBlocks[currBuffer];
		st23play_FillAudioBuffer((int16_t *)waveBlock->lpData, MIX_BUF_SAMPLES);
		waveOutWrite(hWave, waveBlock, sizeof (WAVEHDR));
		currBuffer = (currBuffer + 1) % MIX_BUF_NUM;

		// wait for buffer fill request
		WaitForSingleObject(hAudioSem, INFINITE);
	}

	return 0;
}

static void CALLBACK waveProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	(void)hWaveOut;
	(void)uMsg;
	(void)dwInstance;
	(void)dwParam1;
	(void)dwParam2;

	if (uMsg == WOM_DONE)
		ReleaseSemaphore(hAudioSem, 1, NULL);
}

static void closeMixer(void)
{
	int32_t i;

	audioRunningFlag = false; // make thread end when it's done

	if (hAudioSem != NULL)
		ReleaseSemaphore(hAudioSem, 1, NULL);

	if (hThread != NULL)
	{
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
		hThread = NULL;
	}

	if (hAudioSem != NULL)
	{
		CloseHandle(hAudioSem);
		hAudioSem = NULL;
	}

	if (hWave != NULL)
	{
		waveOutReset(hWave);
		for (i = 0; i < MIX_BUF_NUM; i++)
		{
			if (waveBlocks[i].dwUser != 0xFFFF)
				waveOutUnprepareHeader(hWave, &waveBlocks[i], sizeof (WAVEHDR));
		}

		waveOutClose(hWave);
		hWave = NULL;
	}

	for (i = 0; i < MIX_BUF_NUM; i++)
	{
		if (mixBuffer[i] != NULL)
		{
			free(mixBuffer[i]);
			mixBuffer[i] = NULL;
		}
	}
}

static bool openMixer(uint32_t audioFreq)
{
	int32_t i;
	DWORD threadID;
	WAVEFORMATEX wfx;

	// don't unprepare headers on error
	for (i = 0; i < MIX_BUF_NUM; i++)
		waveBlocks[i].dwUser = 0xFFFF;

	closeMixer();

	ZeroMemory(&wfx, sizeof (wfx));
	wfx.nSamplesPerSec = audioFreq;
	wfx.wBitsPerSample = 16;
	wfx.nChannels = 2;
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nBlockAlign = wfx.nChannels * (wfx.wBitsPerSample / 8);
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

	samplesLeft = 0;
	currBuffer = 0;

	if (waveOutOpen(&hWave, WAVE_MAPPER, &wfx, (DWORD_PTR)&waveProc, 0, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
		goto omError;

	// create semaphore for buffer fill requests
	hAudioSem = CreateSemaphore(NULL, MIX_BUF_NUM - 1, MIX_BUF_NUM, NULL);
	if (hAudioSem == NULL)
		goto omError;

	// allocate WinMM mix buffers
	for (i = 0; i < MIX_BUF_NUM; i++)
	{
		mixBuffer[i] = (int16_t *)calloc(MIX_BUF_SAMPLES, wfx.nBlockAlign);
		if (mixBuffer[i] == NULL)
			goto omError;
	}

	// initialize WinMM mix headers
	memset(waveBlocks, 0, sizeof (waveBlocks));
	for (i = 0; i < MIX_BUF_NUM; i++)
	{
		waveBlocks[i].lpData = (LPSTR)mixBuffer[i];
		waveBlocks[i].dwBufferLength = MIX_BUF_SAMPLES * wfx.nBlockAlign;
		waveBlocks[i].dwFlags = WHDR_DONE;

		if (waveOutPrepareHeader(hWave, &waveBlocks[i], sizeof (WAVEHDR)) != MMSYSERR_NOERROR)
			goto omError;
	}

	// create main mixer thread
	audioRunningFlag = true;
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)mixThread, NULL, 0, &threadID);
	if (hThread == NULL)
		goto omError;

	return TRUE;

omError:
	closeMixer();
	return FALSE;
}

// ---------------------------------------------------------------------------

// END OF FILE
