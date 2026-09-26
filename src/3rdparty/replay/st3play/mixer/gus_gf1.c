/*
** Simple 'Gravis Ultrasound' GF1 emulator for st3play
**
** This emulator has just enough logic implemented to support st3play.
** In other words, it's lacking a lot of stuff (which we don't need).
**
** Built using the excellent low-level hardware documentation found in
** "InterWave IC Am78C201 Programmers Guide v2, (SDK) 1996.pdf"
** (https://16-bits.org/etc/guspnp.pdf).
**
** WARNING: This code is *not* thread-safe, and GUS functions shall
** only be called from the thread that calls GUS_RenderSamples() itself!
** We only use it like this in st3play anyway.
*/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "gus_gf1.h"
#include "../dig.h" // CLAMP(), etc.
#include "../digread.h"
#include "sinc.h"

#define GF1_MIN_VOICES 14
#define GF1_MAX_VOICES 32
#define GF1_SMP_ADD_FRAC_BITS 9
#define GF1_SMP_ADD_FRAC_MASK ((1 << GF1_SMP_ADD_FRAC_BITS)-1)
#define GF1_VOL_FRAC_BITS 3

enum // GUS GF1 voice/volume control flags (the ones needed for ST3 playback, that is)
{
	SACI_STOPPED         = 1,
	SACI_STOP            = 2,
	SACI_LOOP_FWD        = 8,
	SVCI_STOPPED         = 1,
	SVCI_STOP            = 2,
	SVCI_DECREASING_RAMP = 64
};

typedef struct gusVoice_t
{
	const int8_t *SA; // current address
	const int8_t *SAS; // start address (used when loop is enabled)
	const int8_t *SAE; // end address
	uint16_t SA_frac; // current address fraction (6.9fp)
	uint8_t SACI, SVCI; // voice/volume control (flags)
	uint16_t LOff, ROff; // current pan offsets
	uint16_t SVRI; // volume rate
	uint16_t SVLI; // current volume
	uint16_t SVSI; // volume start
	uint16_t SVEI; // volume end
	uint16_t SFCI; // frequency/delta (6.9fp)
} gusVoice_t;

/* Values taken from GUS PnP documentation.
**
** Formula:
**  for (int32_t i = 0; i < 16; i++)
**      panOffsTable[i] = (i == 15) ? 4095 : round(-128.0 * log2((15 - i) / 15.0))
*/
static const uint16_t panOffsTable[16] =
{
	   0,   13,   26,   41,
	  57,   75,   94,  116,
	 141,  169,  203,  244,
	 297,  372,  500, 4095
};

static int32_t activeVoices = 14;
static uint64_t resamplingFrac, resamplingDelta;
static float fSampleBufferL[SINC_TAPS], fSampleBufferR[SINC_TAPS];
static double dGUSOutputRate = 44100.0;
static gusVoice_t gusVoice[GF1_MAX_VOICES];
static gusVoice_t *gv = gusVoice; // initialize to voice #0

void GUS_VoiceSelect(int32_t voiceNum)
{
	gv = &gusVoice[CLAMP(voiceNum, 0, activeVoices-1)];
}

void GUS_SetFrequency(uint16_t freq) // 6.10fp
{
	gv->SFCI = freq >> 1; // GUS GF1: LSB is not used
}

void GUS_SetCurrVolume(uint16_t volume) // 12.4fp
{
	gv->SVLI = volume >> 1; // GUS GF1: LSB is not used
}

void GUS_SetStartVolume(uint8_t volume)
{
	gv->SVSI = volume << (4+GF1_VOL_FRAC_BITS);
}

void GUS_SetEndVolume(uint8_t volume)
{
	gv->SVEI = volume << (4+GF1_VOL_FRAC_BITS);
}

void GUS_SetVolumeRate(uint8_t rate)
{
	gv->SVRI = (rate & 63) << (GF1_VOL_FRAC_BITS - (rate >> 6));
}

void GUS_SetBalance(uint8_t balance) // 0..15
{
	balance &= 15;
	gv->LOff = panOffsTable[   balance];
	gv->ROff = panOffsTable[15-balance];
}

const int8_t *GUS_GetCurrAddress(void)
{
	return (int8_t *)gv->SA;
}

void GUS_SetCurrAddress(const int8_t *address)
{
	gv->SA = address;
	gv->SA_frac = 0;
}

void GUS_SetStartAddress(const int8_t *address) // loop start
{
	gv->SAS = address;
}

void GUS_SetEndAddress(const int8_t *address)
{
	gv->SAE = (int8_t *)address;
}

void GUS_SetVolumeCtrl(uint8_t flags)
{
	gv->SVCI = flags;
}

void GUS_SetVoiceCtrl(uint8_t flags)
{
	gv->SACI = flags;
}

uint8_t GUS_GetVoiceCtrl(void)
{
	return gv->SACI;
}

void GUS_Init(int32_t audioOutputFrequency, int32_t numVoices)
{
	if (audioOutputFrequency <= 0)
		audioOutputFrequency = 44100;

	numVoices = CLAMP(numVoices, GF1_MIN_VOICES, GF1_MAX_VOICES);

	lockMixer();

	// set defaults register values

	gusVoice_t *v = gusVoice;
	for (int32_t i = 0; i < GF1_MAX_VOICES; i++, v++)
	{
		v->SA = v->SAS = v->SAE = NULL;
		v->SA_frac = 0;
		v->SACI = SACI_STOPPED;
		v->SVCI = SVCI_STOPPED;
		v->LOff = panOffsTable[   7];
		v->ROff = panOffsTable[15-7];
		v->SVRI = v->SVLI = v->SVSI = v->SVEI = 0;
		v->SFCI = 1 << GF1_SMP_ADD_FRAC_BITS; // 1.0
	}

	gv = gusVoice; // currently selected voice = first voice
	activeVoices = numVoices;
	dGUSOutputRate = (double)(14 * 44100) / activeVoices;
	resamplingDelta = (uint64_t)round(RESAMPLING_FRAC_SCALE * (dGUSOutputRate / audioOutputFrequency));
	resamplingFrac = 0;

	unlockMixer();
}

double GUS_GetOutputRate(void)
{
	return dGUSOutputRate;
}

int32_t GUS_GetNumberOfVoices(void)
{
	return activeVoices;
}

int32_t GUS_GetNumberOfRunningVoices(void)
{
	int32_t voices = 0;

	gusVoice_t *v = gusVoice;
	for (int32_t i = 0; i < activeVoices; i++, v++)
	{
		if (!(v->SACI & SACI_STOPPED) && (v->SVLI >> GF1_VOL_FRAC_BITS) > 256)
			voices++;
	}

	return voices;
}

static void outputGUSSample(float *fOutL, float *fOutR)
{
	int32_t L = 0, R = 0;

	gusVoice_t *v = gusVoice;
	for (int32_t i = 0; i < activeVoices; i++, v++)
	{
		if (v->SACI & SACI_STOP) v->SACI |= SACI_STOPPED;
		if (v->SVCI & SVCI_STOP) v->SVCI |= SVCI_STOPPED;

		if (!(v->SACI & SACI_STOPPED)) // run sample engine
		{
			if (v->SA != NULL && v->SAE != NULL)
			{
				// handle end-of-sample
				if (v->SA >= v->SAE)
				{
					if ((v->SACI & SACI_LOOP_FWD) && v->SAS != NULL)
					{
						const uint32_t overflowSamples = (uint32_t)(v->SA - v->SAE);

						uint32_t loopLength = (uint32_t)(v->SAE - v->SAS);
						if (loopLength == 0)
							v->SA = v->SAS;
						else
							v->SA = v->SAS + (overflowSamples % loopLength);
					}
					else // no loop
					{
						v->SACI |= SACI_STOPPED;
					}
				}

				if (!(v->SACI & SACI_STOPPED))
				{
					// linear interpolation
					int16_t smp = v->SA[0] << 8, smp2 = v->SA[1] << 8;
					smp += ((smp2-smp) * (int16_t)v->SA_frac) >> GF1_SMP_ADD_FRAC_BITS;

					// this is how GUS GF1 (or at least GUS PnP) does its volume conversion
					const uint16_t vol = v->SVLI >> GF1_VOL_FRAC_BITS;
					int16_t volL = vol - v->LOff;
					int16_t volR = vol - v->ROff;
					volL &= ~((int16_t)volL >> 15); // if (volL < 0) volL = 0;
					volR &= ~((int16_t)volR >> 15); // if (volR < 0) volR = 0;
					L += (smp * (256 + (volL & 0xFF))) >> (24 - (volL >> 8));
					R += (smp * (256 + (volR & 0xFF))) >> (24 - (volR >> 8));

					v->SA_frac += v->SFCI;
					v->SA += v->SA_frac >> GF1_SMP_ADD_FRAC_BITS;
					v->SA_frac &= GF1_SMP_ADD_FRAC_MASK;
				}
			}
		}

		if (!(v->SVCI & SVCI_STOPPED)) // run volume ramping engine
		{
			if (v->SVCI & SVCI_DECREASING_RAMP)
			{
				v->SVLI -= v->SVRI;
				if ((int16_t)v->SVLI <= (int16_t)v->SVSI)
				{
					v->SVLI = v->SVSI;
					v->SVCI |= SVCI_STOPPED;
				}
			}
			else
			{
				v->SVLI += v->SVRI;
				if (v->SVLI >= v->SVEI)
				{
					v->SVLI = v->SVEI;
					v->SVCI |= SVCI_STOPPED;
				}
			}
		}
	}

	L = CLAMP(L, INT16_MIN, INT16_MAX);
	R = CLAMP(R, INT16_MIN, INT16_MAX);

	*fOutL = (float)L * (1.0f / 32768.0f);
	*fOutR = (float)R * (1.0f / 32768.0f);
}

static void GUS_Output(float *outL, float *outR)
{
	resamplingFrac += resamplingDelta;
	while (resamplingFrac >= RESAMPLING_FRAC_SCALE)
	{
		resamplingFrac -= RESAMPLING_FRAC_SCALE;

		// advance resampling ring buffer
		for (int32_t i = 0; i < SINC_TAPS-1; i++)
		{
			fSampleBufferL[i] = fSampleBufferL[1+i];
			fSampleBufferR[i] = fSampleBufferR[1+i];
		}

		float inL, inR;
		outputGUSSample(&inL, &inR);

		fSampleBufferL[SINC_TAPS-1] = inL;
		fSampleBufferR[SINC_TAPS-1] = inR;
	}

	const uint32_t frac32 = (uint32_t)resamplingFrac;
	const uint32_t lutPhase = frac32 >> INTRP_PHASE_SHIFT; // 0 .. SINC_OVERSAMPLING-1
	const float fIntrpFrac = (int32_t)(frac32 & INTRP_PHASE_MASK) * (1.0f / INTRP_PHASE_SCALE);

	// it may look like we go out of bounds for fSinc_2, but we have an extra phase after LUT
	const float *fSinc_1 = fSincLUT + ( lutPhase    << SINC_TAPS_BITS);
	const float *fSinc_2 = fSincLUT + ((lutPhase+1) << SINC_TAPS_BITS);

	float fSumL = 0.0f, fSumR = 0.0f;
	for (int32_t i = 0; i < SINC_TAPS; i++)
	{
		// do linear interpolation between phases
		const float y1 = fSinc_1[i];
		const float y2 = fSinc_2[i];
		const float y = y1 + ((y2 - y1) * fIntrpFrac);

		fSumL += fSampleBufferL[i] * y;
		fSumR += fSampleBufferR[i] * y;
	}
	
	*outL = fSumL;
	*outR = fSumR;
}

void GUS_RenderSamples(float *fMixBufL, float *fMixBufR, int32_t numSamples)
{
	for (int32_t i = 0; i < numSamples; i++)
		GUS_Output(fMixBufL++, fMixBufR++);
}
