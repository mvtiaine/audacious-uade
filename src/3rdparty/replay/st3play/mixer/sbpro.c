// channel mixer for Sound Blaster Pro mode

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "../dig.h"
#include "sinc.h"

#define ST3_PCM_CHANNELS 16

static const uint8_t xvol_st3[64+1] =
{
	0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,
	64,68,72,76,80,85,89,93,97,101,105,109,113,117,
	121,125,129,133,137,141,145,149,153,157,161,165,
	170,174,178,182,186,190,194,198,202,206,210,214,
	218,222,226,230,234,238,242,246,250,255,

	255 // overflow byte added just in case volume is >63 (shouldn't happen)
};

static int8_t postTable[2048];
static uint64_t resamplingFrac, resamplingDelta;
static float fSampleBufferL[SINC_TAPS], fSampleBufferR[SINC_TAPS];
static double dSBProOutputRate;

void SBPro_Init(int32_t audioOutputFrequency, uint8_t timeConstant)
{
	dSBProOutputRate = 1000000.0 / (256 - timeConstant);
	resamplingDelta = (uint64_t)round(RESAMPLING_FRAC_SCALE * (dSBProOutputRate / audioOutputFrequency));
	resamplingFrac = 0;

	// create post table (aka. "squeeze volume table")

	uint8_t mastervol = song.header.mastermul & 127;
	if (mastervol < 16)
		mastervol = 16;

	const uint16_t c = (2048 * 16) / mastervol;
	const uint16_t a = (2048 - c) / 2;
	const uint16_t b = a + c;
	const uint16_t delta16 = (uint16_t)(65536 / c);

	uint16_t smp16 = 0;
	for (int32_t i = 0; i < 2048; i++)
	{
		if (i < a)
		{
			postTable[i] = -128;
		}
		else if (i < b)
		{
			postTable[i] = (smp16 >> 8) ^ 0x80;
			smp16 += delta16;
		}
		else
		{
			postTable[i] = 127;
		}
	}
}

double SBPro_GetOutputRate(void)
{
	return dSBProOutputRate;
}

static void outputSBProSample(float *fOutL, float *fOutR)
{
	uint16_t L = 1024, R = 1024;

	zchn_t *ch = song._zchn;
	for (int32_t i = 0; i < ST3_PCM_CHANNELS; i++, ch++)
	{
		if (ch->m_speed == 0 || ch->m_pos == 0xFFFFFFFF || ch->m_base == NULL || ch->m_pos >= ch->m_end)
			continue;

		const int16_t smp = (ch->m_base[ch->m_pos] * (int16_t)xvol_st3[ch->m_vol]) >> 8;
		if (song.stereomode)
		{
			if (ch->amixtype == 0 || ch->amixtype == 2)
			{
				// normal mix
				if (ch->channelnum >= 8)
					L += smp;
				else
					R += smp;
			}
			else if (ch->amixtype == 1 || ch->amixtype == 3)
			{
				// swap L/R channels
				if (ch->channelnum < 8)
					L += smp;
				else
					R += smp;
			}
			else
			{
				// centered
				L += smp;
				R += smp;
			}
		}
		else
		{
			// centered (mono mode)
			L += smp;
			R += smp;
		}

		ch->m_poslow += ch->m_speed;
		ch->m_pos += ch->m_poslow >> 16;
		ch->m_poslow &= 0xFFFF;

		if (ch->m_pos >= ch->m_end)
		{
			if ((uint16_t)ch->m_loop != 65535) // loop enabled?
			{
				// ST3 does it like this. Safe because of loop unrolling in loader.
				ch->m_pos += (int16_t)(ch->m_loop - ch->m_end);
			}
			else // no loop
			{
				ch->m_speed = 0; // stop sample
			}
		}
	}

	// just in case of non-ST3 channel mapping (mix overflow)
	L &= 2047;
	R &= 2047;

	*fOutL = postTable[L] * (1.0f / 128.0f);
	*fOutR = postTable[R] * (1.0f / 128.0f);
}

static void SBPro_Output(float *outL, float *outR)
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
		outputSBProSample(&inL, &inR);

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

void SBPro_RenderSamples(float *fMixBufL, float *fMixBufR, int32_t numSamples)
{
	for (int32_t i = 0; i < numSamples; i++)
		SBPro_Output(fMixBufL++, fMixBufR++);
}
