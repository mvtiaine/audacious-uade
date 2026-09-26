;/***********************************************************************
 **
 **  Main file
 **
 **  (8bb: .ASM -> .C, some files removed)
 **
 **  Contents of DIG*.C files:
 **
 **  DIG.C          general/main
 **  DIGAMG.C       amiga note handling
 **  DIGADL.C       adlib note handling
 **  DIGCMD.C       commands (effects)
 **  DIGDATA.C      data variables
 **  DIGREAD.C      reading new stuff from pattern
 **
 ***********************************************************************/

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "dig.h"
#include "digcmd.h"
#include "digread.h"
#include "digdata.h"
#include "dig_gus.h"
#include "digadl.h"
#include "mixer/gus_gf1.h"
#include "mixer/sbpro.h"
#include "opl2/opl2.h"

static uint32_t randSeed;
static float fPrngStateL, fPrngStateR;

bool WAVRender_Flag; // global

static void setmasterflags(void)
{
	song.masterflags = song.header.flags;

	song.fastvolslide = !!(song.masterflags & 64);

	if (song.masterflags & 16)
	{
		// limit to amiga limits
		song.amigalimits = true;
		song.aspdmin = 453;
		song.aspdmax = 3424;
	}
	else
	{
		song.amigalimits = false;
		song.aspdmin = 64;
		song.aspdmax = 32767;
	}
}

static void checkheader(void)
{
	if (song.header.mastermul != 0)
	{
		audio.mastermul = song.header.mastermul & 127;

		if (song.stereomode) // multiply mastermul by 11/8 -2 (30->41) {if STEREO/SBPRO}
		{
			uint16_t mastermul = audio.mastermul;

			mastermul *= 11;
			mastermul >>= 3;
			mastermul -= 2; // (just to make 30 => 40)

			if (mastermul > 127)
				mastermul = 127;

			audio.mastermul = (uint8_t)mastermul;
		}
	}

	if (song.header.inittempo != 0)
		settempo(song.header.inittempo);
	else
		settempo(125);

	if (song.header.initspeed != 255)
		setspeed(song.header.initspeed);
	else
		setspeed(6);

	if (song.header.globalvol != 255)
		setglobalvol(song.header.globalvol);
	else
		setglobalvol(64);

	if (song.header.flags != 255)
		setmasterflags();
	else
		song.masterflags = 0;

	if (song.header.ultraclick != 16 && song.header.ultraclick != 24 && song.header.ultraclick != 32)
		song.header.ultraclick = 16;
}

void loadheaderparms(void) // and variables
{
	song.oldstvib = !!(song.header.flags & 1);
	checkheader();
}

void loadheaderpans(void) // pannings
{
	zchn_t *c = song._zchn;
	for (int32_t i = 0; i < 32; i++, c++)
	{
		if (song.defaultpan[i] & 32)
			c->apanpos = 0xF0 | (song.defaultpan[i] & 0xF); // 8bb: the 0xF0 part means that this channel has a pan set
	}
}

void setglobalvol(int8_t vol)
{
	song.globalvol = vol;

	if ((uint8_t)vol > 64)
		vol = 64;

	song.useglobalvol = (uint16_t)vol << 2; // 8bb: 0..256, for setvol()
}

void shutupsounds(void)
{
	lockMixer();

	memset(song._zchn, 0, sizeof (song._zchn));

	zchn_t *ch = song._zchn;
	for (int8_t i = 0; i < ACHANNELS; i++, ch++)
	{
		ch->channelnum = i;
		ch->achannelused = 128;
		ch->aguschannel = -1;
		ch->lastadlins = 101;
		ch->m_oldvol = 255; // 8bb: from shutupsounds2() (GUS), but let's put it here
		ch->m_oldpos = 0xFFFFFFFF; // 8bb: added this (also for GUS)
	}

	song.lastachannelused = 1;

	if (audio.soundcardtype == SOUNDCARD_GUS)
		gcmd_inittables();

	unlockMixer();
}

static void initmodule(void)
{
	song.jmptoord = -1;
	song.musiccount = 0;
	song.patterndelay = 0;
	song.patloopstart = 0;
	song.patloopcount = 0;
	song.np_row = 0;
	song.np_pat = 0;
	song.startrow = 0;
	song.breakpat = 0; // 8bb: added this one
	shutupsounds();

	song.KxyLxxVolslideType = 0; // 8bb: added this for Kxy/Lxx effect (ST3 uses BP register for this)
}

void updateregs(void) // adlib/gravis
{
	zchn_t *ch = song._zchn;
	for (int32_t i = 0; i < ACHANNELS; i++, ch++)
	{
		ch->achannelused &= 127;

		if (audio.soundcardtype == SOUNDCARD_GUS)
			gcmd_update(ch); // 8bb: update GUS registers (dig_gus.c)
	}

	if (audio.soundcardtype == SOUNDCARD_GUS)
		gcmd_update(NULL); // 8bb: trigger GUS voices (dig_gus.c)

	if (song.adlibused)
		updateadlib();
}

uint16_t roundspd(zchn_t *ch, uint16_t spd) // 8bb: for Gxx with semitones-slide enabled
{
	uint32_t newspd = spd * ch->ac2spd;
	if ((newspd >> 16) >= C2FREQ)
		return spd; // 8bb: div error

	newspd /= C2FREQ;

	// find octave

	int8_t octa = 0;
	uint16_t lastspd = (notespd[12] + notespd[11]) >> 1;

	while (lastspd >= newspd)
	{
		octa++;
		lastspd >>= 1;
	}

	// find note

	int8_t newnote = 0;
	int16_t notemin = 32767;

	for (int8_t i = 0; i < 11; i++)
	{
		int16_t note = notespd[i];
		if (octa > 0)
			note >>= octa;

		note -= (int16_t)newspd;
		if (note < 0)
			note = -note;

		if (note < notemin)
		{
			notemin = note;
			newnote = i;
		}
	}

	// get new speed from new note

	newspd = stnote2herz((octa << 4) | (newnote & 0x0F)) * C2FREQ;
	if ((newspd >> 16) >= ch->ac2spd)
		return spd; // 8bb: div error

	newspd /= ch->ac2spd;
	return (uint16_t)newspd;
}

uint16_t scalec2spd(zchn_t *ch, uint16_t spd)
{
	uint32_t tmpspd = spd * C2FREQ;
	if ((tmpspd >> 16) >= ch->ac2spd)
		return 32767; // 8bb: div error

	tmpspd /= ch->ac2spd;
	if (tmpspd > 32767)
		tmpspd = 32767;

	return (uint16_t)tmpspd;
}

void setspd(zchn_t *ch)
{
	ch->achannelused |= 128;

	const bool amigalimits = !!(song.masterflags & 16);

	if (amigalimits)
	{
		if ((uint16_t)ch->aorgspd > song.aspdmax)
			ch->aorgspd = song.aspdmax;

		if (ch->aorgspd < song.aspdmin)
			ch->aorgspd = song.aspdmin;
	}

	int16_t tmpspd = ch->aspd;
	if ((uint16_t)tmpspd > song.aspdmax)
	{
		tmpspd = song.aspdmax;
		if (amigalimits)
			ch->aspd = tmpspd;
	}

	if (tmpspd == 0)
	{
		ch->m_speed = 0;

		// 8bb: these two are for AdLib
		ch->addherzretrig = 254;
		ch->addherzhi &= 32767;

		return;
	}

	if (tmpspd < song.aspdmin)
	{
		tmpspd = song.aspdmin;
		if (amigalimits)
			ch->aspd = tmpspd;
	}

	const uint32_t hz = 14317056 / (uint16_t)tmpspd;
	if (hz < 65536)
	{
		// 8bb: fits in 32-bit division
		ch->m_speed = (hz << 16) / audio.notemixingspeed;
	}
	else
	{
		// 8bb: hz is above 65535, slow calculation needed
		const uint16_t quotient  = (uint16_t)(hz / audio.notemixingspeed);
		const uint16_t remainder = (uint16_t)(hz % audio.notemixingspeed);
		ch->m_speed = (quotient << 16) | ((remainder << 16) / audio.notemixingspeed);
	}

	// 8bb: for AdLib
	ch->addherzhi = hz >> 16;
	ch->addherzlo = (uint16_t)hz;
}

void setvol(zchn_t *ch)
{
	ch->achannelused |= 128;
	ch->m_vol = ((uint8_t)ch->avol * song.useglobalvol) >> 8;
}

uint16_t stnote2herz(uint8_t note)
{
	if (note == 254)
		return 0; // 0hertz/keyoff

	uint16_t noteVal = notespd[note & 0x0F];

	const uint8_t shiftVal = octavediv[note >> 4];
	if (shiftVal > 0)
		noteVal >>= shiftVal & 0x1F;

	return noteVal;
}

static inline int32_t random32(void) // 8bb: added this (LCG 32-bit random)
{
	randSeed *= 134775813;
	randSeed++;
	return (int32_t)randSeed;
}

void musmixer(int16_t *buffer, int32_t samples) // 8bb: not directly ported
{
	if (samples <= 0)
		return;

	if (!WAVRender_Flag && (!audio.playing || audio.samplesPerTickInt == 0))
	{
		memset(buffer, 0, samples * 2 * sizeof (int16_t));
		return;
	}

	float *fMixL = audio.fMixBufferL;
	float *fMixR = audio.fMixBufferR;

	uint32_t samplesLeft = samples;
	while (samplesLeft > 0)
	{
		if (audio.tickSampleCounter == 0)
		{
			dorow(); // 8bb: digread.c (replayer ticker)
			updateregs(); // 8bb: dig.c (GUS & AdLib updating)

			audio.tickSampleCounter = audio.samplesPerTickInt;

			audio.tickSampleCounterFrac += audio.samplesPerTickFrac;
			if (audio.tickSampleCounterFrac > UINT32_MAX)
			{
				audio.tickSampleCounterFrac &= UINT32_MAX;
				audio.tickSampleCounter++;
			}
		}

		uint32_t samplesToMix = samplesLeft;
		if (samplesToMix > audio.tickSampleCounter)
			samplesToMix = audio.tickSampleCounter;

		// 8bb: mix PCM voices
		if (audio.soundcardtype == SOUNDCARD_GUS)
			GUS_RenderSamples(fMixL, fMixR, samplesToMix);
		else
			SBPro_RenderSamples(fMixL, fMixR, samplesToMix);

		// 8bb: mix AdLib (OPL2) voices
		if (song.adlibused)
		{
			// lower gain a little before mixing in OPL2 samples
			for (uint32_t i = 0; i < samplesToMix; i++)
			{
				fMixL[i] *= 2.0f/3.0f;
				fMixR[i] *= 2.0f/3.0f;
			}

			OPL2_RenderSamples(fMixL, fMixR, samplesToMix);
		}

		fMixL += samplesToMix;
		fMixR += samplesToMix;

		audio.tickSampleCounter -= samplesToMix;
		samplesLeft -= samplesToMix;
	}

	float fOut, fPrng;
	int32_t out32;
	for (int32_t i = 0; i < samples; i++)
	{
		// 8bb: left channel - 1-bit triangular dithering
		fPrng = (float)random32() * (1.0f / (UINT32_MAX+1.0f)); // -0.5f .. 0.5f
		fOut = audio.fMixBufferL[i] * audio.fMixingVol;
		fOut = (fOut + fPrng) - fPrngStateL;
		fPrngStateL = fPrng;
		out32 = (int32_t)fOut;
		*buffer++ = (int16_t)(CLAMP(out32, INT16_MIN, INT16_MAX));

		// 8bb: right channel - 1-bit triangular dithering
		fPrng = (float)random32() * (1.0f / (UINT32_MAX+1.0f)); // -0.5f .. 0.5f
		fOut = audio.fMixBufferR[i] * audio.fMixingVol;
		fOut = (fOut + fPrng) - fPrngStateR;
		fPrngStateR = fPrng;
		out32 = (int32_t)fOut;
		*buffer++ = (int16_t)(CLAMP(out32, INT16_MIN, INT16_MAX));

		// 8bb: clear what we read from the mixing buffer
		audio.fMixBufferL[i] = audio.fMixBufferR[i] = 0.0f;
	}
}

void zgotosong(int16_t order, int16_t row)
{
	lockMixer();

	song.startrow = row & 0x3F;
	song.np_ord = order & 0xFF;
	song.breakpat = 1;
	song.musiccount = song.musicmax;

	unlockMixer();
	audio.playing = true;
}

bool zplaysong(int16_t order)
{
	if (!song.moduleLoaded)
		return false;

	song.adlibused = false; // 8bb: set in digadl.c if AdLib channels are handled

	OPL2_Init(audio.outputFreq);
	initadlib(); // initialize adlib

	song.stereomode = !!(song.header.mastermul & 128);
	if (audio.soundcardtype == SOUNDCARD_GUS)
		audio.notemixingspeed = 38587; // 8bb: yes, ST3 sets this to 38587 regardless of active GUS voices
	else
		audio.notemixingspeed = song.stereomode ? 22000 : 43478; // 8bb: first constant is off! :-(

	// 8bb: calculate bpm2SamplesPerTick table
	for (int32_t i = 0; i <= 255; i++)
	{
		const int32_t bpm = (i == 0) ? 1 : i;

		double dHz;
		if (audio.soundcardtype == SOUNDCARD_GUS)
		{
			// 8bb: calculate ST3-lossy value
			int32_t hz = (bpm * 50) / 125;
			if (hz < 19) // 8bb: ST3 does this to fit the PIT period range (this limits low BPM to 47.5)
				hz = 19;

			const int32_t PIT_Period = 1193180 / hz; // 8bb: ST3 off-by-one PIT clock constant

			// 8bb: convert to actual hertz
			const double dNominalPITClk = 157500000.0 / 132.0;
			dHz = dNominalPITClk / (double)PIT_Period;
		}
		else
		{
			// 8bb: calculate ST3-lossy value
			const int32_t samplesPerTick = (audio.notemixingspeed * 125) / (bpm * 50);

			// 8bb: convert to actual hertz
			dHz = audio.notemixingspeed / (double)samplesPerTick;
		}

		const double dSamplesPerTick = audio.outputFreq / dHz;
		const uint64_t samplesPerTick64 = (uint64_t)((dSamplesPerTick * (UINT32_MAX+1.0)) + 0.5); // 8bb: rounded 32.32fp

		audio.bpm2SamplesPerTickInt[i] = samplesPerTick64 >> 32;
		audio.bpm2SamplesPerTickFrac[i] = (uint32_t)samplesPerTick64;
	}

	loadheaderparms();
	initmodule();
	loadheaderpans();

	if (audio.soundcardtype == SOUNDCARD_GUS)
	{
		uint8_t numGUSVoices = song.header.ultraclick;

		GUS_Init(audio.outputFreq, numGUSVoices);
		gcmd_setvoices(numGUSVoices);
		gcmd_setstereo();
	}
	else if (audio.soundcardtype == SOUNDCARD_SBPRO)
	{
		const uint8_t timeConstant = song.stereomode ? 210 : 233;
		SBPro_Init(audio.outputFreq, timeConstant);
	}

	// 8bb: added these two for protection
	song.np_patseg = NULL;
	song.np_patoff = -1;

	song.np_ord = order & 255;
	neworder();

	song.musiccount = 0; // 8bb: added this
	resetAudioDither();

	// 8bb: zero tick sample counter so that it will instantly initiate a tick
	audio.tickSampleCounterFrac = audio.tickSampleCounter = 0;

	audio.playing = true;
	return true;
}

static void freeinsmem(int32_t a)
{
	ds_smp *ins = &song.ins[a];
	if (ins->type == 1 && ins->baseptr != NULL)
	{
		free(ins->baseptr);
		ins->baseptr = NULL;
	}
}

// 8bb: custom routines

void resetAudioDither(void)
{
	randSeed = 0x12345000;
	fPrngStateL = fPrngStateR = 0.0f;
}

void closeMusic(void)
{
	closeMixer();

	if (audio.fMixBufferL != NULL)
	{
		free(audio.fMixBufferL);
		audio.fMixBufferL = NULL;
	}

	if (audio.fMixBufferR != NULL)
	{
		free(audio.fMixBufferR);
		audio.fMixBufferR = NULL;
	}

	// free pattern data
	for (int32_t i = 0; i < MAX_PATTERNS; i++)
	{
		if (song.patp[i] != NULL)
		{
			free(song.patp[i]);
			song.patp[i] = NULL;
		}
	}

	// free sample data
	for (int32_t i = 0; i < MAX_INSTRUMENTS; i++)
		freeinsmem(i);

	memset(&song.header, 0, sizeof (song.header));
	memset(song.order, 255, MAX_ORDERS);
	memset(song.ins, 0, sizeof (song.ins));

	song.adlibused = false;
}

bool initMusic(int32_t audioFrequency, int32_t audioBufferSize)
{
	audio.outputFreq = CLAMP(audioFrequency, 8000, 768000);

	if (!renderToWavFlag)
		closeMixer();

	closeMusic();
	memset(song._zchn, 0, sizeof (song._zchn));

	// zero tick sample counter so that it will instantly initiate a tick
	audio.tickSampleCounterFrac = audio.tickSampleCounter = 0;

	audio.fMixBufferL = (float *)calloc(audioBufferSize, sizeof (float));
	audio.fMixBufferR = (float *)calloc(audioBufferSize, sizeof (float));

	if (audio.fMixBufferL == NULL || audio.fMixBufferR == NULL)
	{
		closeMusic();
		return false;
	}

	if (!renderToWavFlag)
	{
		if (!openMixer(audio.outputFreq, audioBufferSize))
		{
			closeMusic();
			return false;
		}

		audio.playing = true;
	}

	return true;
}

void togglePause(void)
{
	audio.playing ^= 1;
}

int32_t activePCMVoices(void)
{
	if (audio.soundcardtype == SOUNDCARD_GUS)
	{
		return GUS_GetNumberOfRunningVoices();
	}
	else
	{
		int32_t activeVoices = 0;

		zchn_t *ch = song._zchn;
		for (int32_t i = 0; i < 16; i++, ch++)
		{
			if (ch->m_base != NULL && ch->m_speed != 0 && ch->m_pos != 0xFFFFFFFF && ch->m_vol > 0)
				activeVoices++;
		}

		return activeVoices;
	}
}

int32_t activeAdLibVoices(void)
{
	int32_t activeVoices = 0;

	zchn_t *ch = &song._zchn[16];
	for (int32_t i = 0; i < 9; i++, ch++)
	{
		if (ch->m_speed != 0 && ch->m_vol > 0 && ch->lastadlins > 0)
			activeVoices++;
	}

	return activeVoices;
}

// 8bb: added these WAV rendering routines

static void WAV_WriteHeader(FILE *f, int32_t frq)
{
	uint16_t w;
	uint32_t l;

	const uint32_t RIFF = 0x46464952;
	fwrite(&RIFF, 4, 1, f);
	fseek(f, 4, SEEK_CUR);
	const uint32_t WAVE = 0x45564157;
	fwrite(&WAVE, 4, 1, f);

	const uint32_t fmt = 0x20746D66;
	fwrite(&fmt, 4, 1, f);
	l = 16; fwrite(&l, 4, 1, f);
	w = 1; fwrite(&w, 2, 1, f);
	w = 2; fwrite(&w, 2, 1, f);
	l = frq; fwrite(&l, 4, 1, f);
	l = frq*2*2; fwrite(&l, 4, 1, f);
	w = 2*2; fwrite(&w, 2, 1, f);
	w = 8*2; fwrite(&w, 2, 1, f);

	const uint32_t DATA = 0x61746164;
	fwrite(&DATA, 4, 1, f);
	fseek(f, 4, SEEK_CUR);
}

static void WAV_WriteEnd(FILE *f, uint32_t size)
{
	fseek(f, 4, SEEK_SET);
	uint32_t l = size+4+24+8;
	fwrite(&l, 4, 1, f);
	fseek(f, 12+24+4, SEEK_SET);
	fwrite(&size, 4, 1, f);
}

bool Dig_RenderToWAV(uint32_t audioRate, uint32_t bufferSize, const char *filenameOut)
{
	int16_t *AudioBuffer = (int16_t *)malloc(bufferSize * 2 * sizeof (int16_t));
	if (AudioBuffer == NULL)
	{
		WAVRender_Flag = false;
		return false;
	}

	FILE *f = fopen(filenameOut, "wb");
	if (f == NULL)
	{
		WAVRender_Flag = false;
		free(AudioBuffer);
		return false;
	}

	WAV_WriteHeader(f, audioRate);
	uint32_t TotalSamples = 0;

	WAVRender_Flag = true;
	while (WAVRender_Flag)
	{
		musmixer(AudioBuffer, bufferSize);
		fwrite(AudioBuffer, 2, bufferSize * 2, f);
		TotalSamples += bufferSize * 2;
	}
	WAVRender_Flag = false;

	WAV_WriteEnd(f, TotalSamples * sizeof (int16_t));
	free(AudioBuffer);
	fclose(f);

	return true;
}
