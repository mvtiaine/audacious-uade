// SPDX-License-Identifier: BSD-3-Clause
/*
** ---- Custom high quality floating-point driver, by 8bitbubsy ----
**
** Behaves like the SB16 MMX driver when it comes to filter clamping,
** volume ramp speed and bidi looping.
**
** Features:
** - 4-tap cubic spline interpolation
** - Stereo sample support
** - 32.32 fixed-point sampling precision (32.16 if 32-bit CPU, for speed)
** - Ended non-looping samples are ramped out, like the WAV writer driver
**
** Compiling for 64-bit is ideal, for higher precision and support for'
** higher mixing frequencies than 48kHz.
*/

#ifndef AUDACIOUS_UADE
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../cpu.h"
#include "../it_structs.h"
#include "../it_music.h" // Update()
#include "hq_m.h"
#include "zerovol.h"
#endif

// fast 32-bit -> 16-bit clamp
#define CLAMP16(i) if ((int16_t)i != i) i = INT16_MAX ^ ((int32_t)i >> 31)

#define BPM_FRAC_BITS 31 /* absolute max for 32-bit arithmetics, don't change! */
#define BPM_FRAC_SCALE (1UL << BPM_FRAC_BITS)
#define BPM_FRAC_MASK (BPM_FRAC_SCALE-1)

static uint16_t MixVolume;
static int32_t RealBytesToMix, BytesToMix, MixTransferRemaining, MixTransferOffset;
static uint32_t BytesToMixFractional, CurrentFractional, RandSeed;
static uint32_t SamplesPerTickInt[256-LOWEST_BPM_POSSIBLE], SamplesPerTickFrac[256-LOWEST_BPM_POSSIBLE];
static float *fMixBuffer, fLastClickRemovalLeft, fLastClickRemovalRight;
static double dFreq2DeltaMul, dPrngStateL, dPrngStateR;

static bool InitCubicSplineLUT(void)
{
	Driver.fCubicLUT = (float *)malloc(CUBIC_PHASES * CUBIC_WIDTH * sizeof (float));
	if (Driver.fCubicLUT == NULL)
		return false;

	float *fLUTPtr = Driver.fCubicLUT;
	for (int32_t i = 0; i < CUBIC_PHASES; i++)
	{
		const double x1 = i * (1.0 / CUBIC_PHASES);
		const double x2 = x1 * x1; // x^2
		const double x3 = x2 * x1; // x^3

		*fLUTPtr++ = (float)(-0.5 * x3 + 1.0 * x2 - 0.5 * x1);
		*fLUTPtr++ = (float)( 1.5 * x3 - 2.5 * x2 + 1.0);
		*fLUTPtr++ = (float)(-1.5 * x3 + 2.0 * x2 + 0.5 * x1);
		*fLUTPtr++ = (float)( 0.5 * x3 - 0.5 * x2);
	}

	return true;
}

static void HQ_MixSamples(void)
{
	MixTransferOffset = 0;

	RealBytesToMix = BytesToMix;

	CurrentFractional += BytesToMixFractional;
	if (CurrentFractional > BPM_FRAC_SCALE)
	{
		CurrentFractional &= BPM_FRAC_MASK;
		RealBytesToMix++;
	}

	// click removal (also clears buffer)

	float *fMixBufPtr = fMixBuffer;
	for (int32_t i = 0; i < RealBytesToMix; i++)
	{
		*fMixBufPtr++ = fLastClickRemovalLeft;
		*fMixBufPtr++ = fLastClickRemovalRight;

		fLastClickRemovalLeft  -= fLastClickRemovalLeft  * (1.0f / 4096.0f);
		fLastClickRemovalRight -= fLastClickRemovalRight * (1.0f / 4096.0f);
	}

	slaveChn_t *sc = sChn;
	for (uint32_t i = 0; i < Driver.NumChannels; i++, sc++)
	{
		if (!(sc->Flags & SF_CHAN_ON) || sc->Smp == 100)
			continue;

		sample_t *s = sc->SmpPtr;
		assert(s != NULL);

		if (sc->Flags & SF_NOTE_STOP) // note cut
		{
			sc->Flags &= ~SF_CHAN_ON;

			sc->FinalVol32768 = 0;
			sc->Flags |= SF_RECALC_FINALVOL;
		}

		if (sc->Flags & SF_FREQ_CHANGE)
		{
			if ((uint32_t)sc->Frequency >= INT32_MAX/2) // non-IT2 limit, but required for safety
			{
				sc->Flags = SF_NOTE_STOP;
				if (!(sc->HostChnNum & CHN_DISOWNED))
					((hostChn_t *)sc->HostChnPtr)->Flags &= ~HF_CHAN_ON; // turn off channel

				continue;
			}

#if CPU_32BIT
			sc->Delta32 = (int32_t)((int32_t)sc->Frequency * dFreq2DeltaMul); // mixer delta (16.16fp)
#else
			sc->Delta64 = (int64_t)((int32_t)sc->Frequency * dFreq2DeltaMul); // mixer delta (32.32fp)
#endif
		}

		if (sc->Flags & SF_NEW_NOTE)
		{
			sc->fOldLeftVolume = sc->fOldRightVolume = 0.0f;

			sc->fCurrVolL = sc->fCurrVolR = 0.0f; // ramp in current voice (old note is ramped out in another voice)

			// clear filter state and filter coeffs
			sc->fOldSamples[0] = sc->fOldSamples[1] = sc->fOldSamples[2] = sc->fOldSamples[3] = 0.0f;
			sc->fFiltera = 1.0f;
			sc->fFilterb = sc->fFilterc = 0.0f;
		}

		if (sc->Flags & (SF_RECALC_FINALVOL | SF_LOOP_CHANGED | SF_PAN_CHANGED))
		{
			uint8_t FilterQ;

			if (sc->HostChnNum & CHN_DISOWNED)
			{
				FilterQ = sc->MIDIBank >> 8; // if disowned, use channel filters
			}
			else
			{
				uint8_t filterCutOff = Driver.FilterParameters[sc->HostChnNum];
				FilterQ = Driver.FilterParameters[64+sc->HostChnNum];

				sc->VolEnvState.CurNode = (filterCutOff << 8) | (sc->VolEnvState.CurNode & 0x00FF);
				sc->MIDIBank = (FilterQ << 8) | (sc->MIDIBank & 0x00FF);
			}

			// FilterEnvVal (0..255) * CutOff (0..127)
			const uint16_t FilterFreqValue = (sc->MIDIBank & 0x00FF) * (uint8_t)((uint16_t)sc->VolEnvState.CurNode >> 8);
			if (FilterFreqValue != 127*255 || FilterQ != 0)
			{
				assert(FilterFreqValue <= 127*255 && FilterQ <= 127);
				const float r = powf(2.0f, (float)FilterFreqValue * Driver.FreqParameterMultiplier) * Driver.FreqMultiplier;
				const float p = Driver.QualityFactorTable[FilterQ];
				const float d = (p * r) + (p - 1.0f);
				const float e = r * r;

				sc->fFiltera = 1.0f / (1.0f + d + e);
				sc->fFilterb = (d + e + e) * sc->fFiltera;
				sc->fFilterc = 1.0f - sc->fFiltera - sc->fFilterb;
			}

			if (sc->Flags & SF_CHN_MUTED)
			{
				sc->fLeftVolume = sc->fRightVolume = 0.0f;
			}
			else
			{
				const int32_t Vol = sc->FinalVol32768 * MixVolume;
				if (!(Song.Header.Flags & ITF_STEREO)) // mono?
				{
					sc->fLeftVolume = sc->fRightVolume = Vol * (1.0f / (32768.0f * 128.0f));
				}
				else if (sc->FinalPan == PAN_SURROUND)
				{
					sc->fLeftVolume = sc->fRightVolume = Vol * (0.5f / (32768.0f * 128.0f));
				}
				else // normal (panned)
				{
					sc->fLeftVolume  = ((64-sc->FinalPan) * Vol) * (1.0f / (64.0f * 32768.0f * 128.0f));
					sc->fRightVolume = ((   sc->FinalPan) * Vol) * (1.0f / (64.0f * 32768.0f * 128.0f));
				}
			}
		}

		// just in case (shouldn't happen)
#if CPU_32BIT
		if (sc->Delta32 == 0)
#else
		if (sc->Delta64 == 0)
#endif
			continue;

		uint32_t MixBlockSize = RealBytesToMix;
		const bool FilterActive = (sc->fFilterb > 0.0f) || (sc->fFilterc > 0.0f);

		if (sc->fLeftVolume == 0.0f && sc->fRightVolume == 0.0f &&
			sc->fOldLeftVolume <= 0.000001f && sc->fOldRightVolume <= 0.000001f &&
			!FilterActive)
		{
			// use position update routine (zero voice volume and no filter)

			const uint32_t LoopLength = sc->LoopEnd - sc->LoopBegin; // also length for non-loopers
			if ((int32_t)LoopLength > 0)
			{
				if (sc->LoopMode == LOOP_PINGPONG)
					UpdatePingPongLoopHQ(sc, MixBlockSize);
				else if (sc->LoopMode == LOOP_FORWARDS)
					UpdateForwardsLoopHQ(sc, MixBlockSize);
				else
					UpdateNoLoopHQ(sc, MixBlockSize);
			}
		}
		else // regular mixing
		{
			const bool Surround = (sc->FinalPan == PAN_SURROUND);
			const bool Stereo = !!(s->Flags & SMPF_STEREO);
			
			MixFunc_t Mix = HQ_MixFunctionTables[(FilterActive << 3) + (Stereo << 2) + (Surround << 1) + sc->SmpIs16Bit];
			assert(Mix != NULL);

			const uint32_t LoopLength = sc->LoopEnd - sc->LoopBegin; // also actual length for non-loopers
			if ((int32_t)LoopLength > 0)
			{
				float *fMixBufferPtr = fMixBuffer;
				if (sc->LoopMode == LOOP_PINGPONG)
				{
					while (MixBlockSize > 0)
					{
						uint32_t NewLoopPos, SamplesToMix;

						if (sc->LoopDirection == DIR_BACKWARDS)
						{
							SamplesToMix = sc->SamplingPosition - (sc->LoopBegin + 1);
#if CPU_32BIT
							if (SamplesToMix > UINT16_MAX) // 8bb: limit it so we can do a hardware 32-bit div (instead of slow software 64-bit div)
								SamplesToMix = UINT16_MAX;

							SamplesToMix = (((SamplesToMix << 16) | (uint16_t)sc->Frac32) / sc->Delta32) + 1;
							Driver.Delta32 = 0 - sc->Delta32;
#else
							SamplesToMix = (uint32_t)(((((uint64_t)SamplesToMix << 32) | (uint32_t)sc->Frac64) / sc->Delta64) + 1);
							Driver.Delta64 = 0 - sc->Delta64;
#endif
						}
						else // forwards
						{
							SamplesToMix = (sc->LoopEnd - 1) - sc->SamplingPosition;
#if CPU_32BIT
							if (SamplesToMix > UINT16_MAX) // 8bb: limit it so we can do a hardware 32-bit div (instead of slow software 64-bit div)
								SamplesToMix = UINT16_MAX;

							SamplesToMix = (((SamplesToMix << 16) | (uint16_t)(sc->Frac32 ^ UINT16_MAX)) / sc->Delta32) + 1;
							Driver.Delta32 = sc->Delta32;
#else
							SamplesToMix = (uint32_t)(((((uint64_t)SamplesToMix << 32) | ((uint32_t)sc->Frac64 ^ UINT32_MAX)) / sc->Delta64) + 1);
							Driver.Delta64 = sc->Delta64;
#endif
						}

						if (SamplesToMix > MixBlockSize)
							SamplesToMix = MixBlockSize;

						Mix(sc, fMixBufferPtr, SamplesToMix);

						MixBlockSize -= SamplesToMix;
						fMixBufferPtr += SamplesToMix << 1;

						if (sc->LoopDirection == DIR_BACKWARDS)
						{
							if (sc->SamplingPosition <= sc->LoopBegin)
							{
								NewLoopPos = (uint32_t)(sc->LoopBegin - sc->SamplingPosition) % (LoopLength << 1);
								if (NewLoopPos >= LoopLength)
								{
									sc->SamplingPosition = (sc->LoopEnd - 1) - (NewLoopPos - LoopLength);

									if (sc->SamplingPosition <= sc->LoopBegin) // 8bb: non-IT2 edge-case safety for extremely high pitches
										sc->SamplingPosition = sc->LoopBegin + 1;
								}
								else
								{
									sc->LoopDirection = DIR_FORWARDS;
									sc->SamplingPosition = sc->LoopBegin + NewLoopPos;
#if CPU_32BIT
									sc->Frac32 = (uint16_t)(0 - sc->Frac32);
#else
									sc->Frac64 = (uint32_t)(0 - sc->Frac64);
#endif
								}
							}
						}
						else // forwards
						{
							if ((uint32_t)sc->SamplingPosition >= (uint32_t)sc->LoopEnd)
							{
								NewLoopPos = (uint32_t)(sc->SamplingPosition - sc->LoopEnd) % (LoopLength << 1);
								if (NewLoopPos >= LoopLength)
								{
									sc->SamplingPosition = sc->LoopBegin + (NewLoopPos - LoopLength);
								}
								else
								{
									sc->LoopDirection = DIR_BACKWARDS;
									sc->SamplingPosition = (sc->LoopEnd - 1) - NewLoopPos;
#if CPU_32BIT
									sc->Frac32 = (uint16_t)(0 - sc->Frac32);
#else
									sc->Frac64 = (uint32_t)(0 - sc->Frac64);
#endif
									if (sc->SamplingPosition <= sc->LoopBegin) // 8bb: non-IT2 edge-case safety for extremely high pitches
										sc->SamplingPosition = sc->LoopBegin + 1;
								}
							}
						}
					}
				}
				else if (sc->LoopMode == LOOP_FORWARDS)
				{
					while (MixBlockSize > 0)
					{
						uint32_t SamplesToMix = (sc->LoopEnd - 1) - sc->SamplingPosition;
#if CPU_32BIT
						if (SamplesToMix > UINT16_MAX) // 8bb: limit it so we can do a hardware 32-bit div (instead of slow software 64-bit div)
							SamplesToMix = UINT16_MAX;

						SamplesToMix = (((SamplesToMix << 16) | (uint16_t)(sc->Frac32 ^ UINT16_MAX)) / sc->Delta32) + 1;
						Driver.Delta32 = sc->Delta32;
#else
						SamplesToMix = (uint32_t)(((((uint64_t)SamplesToMix << 32) | ((uint32_t)sc->Frac64 ^ UINT32_MAX)) / sc->Delta64) + 1);
						Driver.Delta64 = sc->Delta64;
#endif
						if (SamplesToMix > MixBlockSize)
							SamplesToMix = MixBlockSize;

						Mix(sc, fMixBufferPtr, SamplesToMix);

						MixBlockSize -= SamplesToMix;
						fMixBufferPtr += SamplesToMix << 1;

						if ((uint32_t)sc->SamplingPosition >= (uint32_t)sc->LoopEnd)
							sc->SamplingPosition = sc->LoopBegin + ((uint32_t)(sc->SamplingPosition - sc->LoopEnd) % LoopLength);
					}
				}
				else // no loop
				{
					while (MixBlockSize > 0)
					{
						uint32_t SamplesToMix = (sc->LoopEnd - 1) - sc->SamplingPosition;
#if CPU_32BIT
						if (SamplesToMix > UINT16_MAX) // 8bb: limit it so we can do a hardware 32-bit div (instead of slow software 64-bit div)
							SamplesToMix = UINT16_MAX;

						SamplesToMix = (((SamplesToMix << 16) | (uint16_t)(sc->Frac32 ^ UINT16_MAX)) / sc->Delta32) + 1;
						Driver.Delta32 = sc->Delta32;
#else
						SamplesToMix = (uint32_t)(((((uint64_t)SamplesToMix << 32) | ((uint32_t)sc->Frac64 ^ UINT32_MAX)) / sc->Delta64) + 1);
						Driver.Delta64 = sc->Delta64;
#endif
						if (SamplesToMix > MixBlockSize)
							SamplesToMix = MixBlockSize;

						Mix(sc, fMixBufferPtr, SamplesToMix);

						MixBlockSize -= SamplesToMix;
						fMixBufferPtr += SamplesToMix << 1;

						if ((uint32_t)sc->SamplingPosition >= (uint32_t)sc->LoopEnd)
						{
							sc->Flags = SF_NOTE_STOP;
							if (!(sc->HostChnNum & CHN_DISOWNED))
								((hostChn_t *)sc->HostChnPtr)->Flags &= ~HF_CHAN_ON;

							// sample ended, ramp out very last sample point for the remaining samples
							for (; MixBlockSize > 0; MixBlockSize--)
							{
								*fMixBufferPtr++ += Driver.fLastLeftValue;
								*fMixBufferPtr++ += Driver.fLastRightValue;

								Driver.fLastLeftValue  -= Driver.fLastLeftValue  * (1.0f / 4096.0f);
								Driver.fLastRightValue -= Driver.fLastRightValue * (1.0f / 4096.0f);
							}

							// update anti-click value for next mixing session
							fLastClickRemovalLeft  += Driver.fLastLeftValue;
							fLastClickRemovalRight += Driver.fLastRightValue;

							break;
						}
					}
				}
			}

			sc->fOldLeftVolume = sc->fCurrVolL;
			if (!Surround)
				sc->fOldRightVolume = sc->fCurrVolR;
		}

		sc->Flags &= ~(SF_RECALC_PAN      | SF_RECALC_VOL | SF_FREQ_CHANGE |
		               SF_RECALC_FINALVOL | SF_NEW_NOTE   | SF_NOTE_STOP   |
		               SF_LOOP_CHANGED    | SF_PAN_CHANGED);
	}
}

static void HQ_SetTempo(uint8_t Tempo)
{
	if (Tempo < LOWEST_BPM_POSSIBLE)
		Tempo = LOWEST_BPM_POSSIBLE;

	const uint32_t index = Tempo - LOWEST_BPM_POSSIBLE;

	BytesToMix = SamplesPerTickInt[index];
	BytesToMixFractional = SamplesPerTickFrac[index];
}

static void HQ_SetMixVolume(uint8_t Vol)
{
	MixVolume = Vol;
	RecalculateAllVolumes();
}

static void HQ_ResetMixer(void)
{
	MixTransferRemaining = 0;
	MixTransferOffset = 0;
	CurrentFractional = 0;
	RandSeed = 0x12345000;
	dPrngStateL = dPrngStateR = 0.0;
	fLastClickRemovalLeft = fLastClickRemovalRight = 0.0f;
}

static inline int32_t Random32(void)
{
	// LCG 32-bit random
	RandSeed *= 134775813;
	RandSeed++;

	return (int32_t)RandSeed;
}

static int32_t HQ_PostMix(int16_t *AudioOut16, int32_t SamplesToOutput)
{
	int32_t out32;
#if !CPU_32BIT
	double dOut, dPrng;
#endif

	int32_t SamplesTodo = (SamplesToOutput == 0) ? RealBytesToMix : SamplesToOutput;
	for (int32_t i = 0; i < SamplesTodo; i++)
	{
#if CPU_32BIT // if 32-bit CPU, use single-precision float + no dithering (speed)

		// left channel
		out32 = (int32_t)(fMixBuffer[MixTransferOffset++] * 32768.0f);
		CLAMP16(out32);
		*AudioOut16++ = (int16_t)out32;

		// right channel
		out32 = (int32_t)(fMixBuffer[MixTransferOffset++] * 32768.0f);
		CLAMP16(out32);
		*AudioOut16++ = (int16_t)out32;
#else

		// left channel - 1-bit triangular dithering
		dPrng = Random32() * (0.5 / INT32_MAX); // -0.5 .. 0.5
		dOut = (double)fMixBuffer[MixTransferOffset++] * 32768.0;
		dOut = (dOut + dPrng) - dPrngStateL;
		dPrngStateL = dPrng;
		out32 = (int32_t)dOut;
		CLAMP16(out32);
		*AudioOut16++ = (int16_t)out32;

		// right channel - 1-bit triangular dithering
		dPrng = Random32() * (0.5 / INT32_MAX); // -0.5 .. 0.5
		dOut = (double)fMixBuffer[MixTransferOffset++] * 32768.0;
		dOut = (dOut + dPrng) - dPrngStateR;
		dPrngStateR = dPrng;
		out32 = (int32_t)dOut;
		CLAMP16(out32);
		*AudioOut16++ = (int16_t)out32;

#endif
	}

	return SamplesTodo;
}

static void HQ_Mix(int32_t numSamples, int16_t *audioOut)
{
	int32_t SamplesLeft = numSamples;
	while (SamplesLeft > 0)
	{
		if (MixTransferRemaining == 0)
		{
			Update();
			HQ_MixSamples();
			MixTransferRemaining = RealBytesToMix;
		}

		int32_t SamplesToTransfer = SamplesLeft;
		if (SamplesToTransfer > MixTransferRemaining)
			SamplesToTransfer = MixTransferRemaining;

		HQ_PostMix(audioOut, SamplesToTransfer);
		audioOut += SamplesToTransfer * 2;

		MixTransferRemaining -= SamplesToTransfer;
		SamplesLeft -= SamplesToTransfer;
	}
}

/* Fixes sample end bytes for interpolation (yes, we have room after the data).
** Samples with sustain loop are not fixed (too complex to get right).
*/
static void HQ_FixSamples(void)
{
	sample_t *s = Song.Smp;
	for (int32_t i = 0; i < Song.Header.SmpNum; i++, s++)
	{
		if (s->Data == NULL || s->Length == 0)
			continue;

		const bool Sample16Bit = !!(s->Flags & SMPF_16BIT);
		const bool HasLoop = !!(s->Flags & SMPF_USE_LOOP);

		int16_t *Data16 = (int16_t *)s->Data;
		int16_t *Data16R = (int16_t *)s->DataR;
		int8_t *Data8 = (int8_t *)s->Data;
		int8_t *Data8R = (int8_t *)s->DataR;

		/* All negative taps should be equal to the first sample point when at sampling
		** position #0 (on sample trigger).
		*/
		if (Sample16Bit)
		{
			Data16[-1] = Data16[0];
			if (Data16R != NULL) // right sample (if present)
				Data16R[-1] = Data16R[0];
		}
		else
		{
			Data8[-1] = Data8[0];
			if (Data8R != NULL) // right sample (if present)
				Data8R[-1] = Data8R[0];
		}

		if (Sample16Bit)
		{
			// 16 bit

			if (HasLoop)
			{
				if (s->Flags & SMPF_LOOP_PINGPONG)
				{
					int32_t LastSample = s->LoopEnd;
					if (LastSample < 0)
						LastSample = 0;

					Data16[s->LoopEnd+0] = Data16[LastSample];
					if (LastSample > 0)
						Data16[s->LoopEnd+1] = Data16[LastSample-1];
					else
						Data16[s->LoopEnd+1] = Data16[LastSample];

					// right sample (if present)
					if (Data16R != NULL)
					{
						Data16R[s->LoopEnd+0] = Data16R[LastSample];
						if (LastSample > 0)
							Data16R[s->LoopEnd+1] = Data16R[LastSample-1];
						else
							Data16R[s->LoopEnd+1] = Data16R[LastSample];
					}

					/* For bidi loops:
					** The loopstart point is never read after having looped once.
					** IT2 behaves like that. It loops loopstart+1 to loopend-1.
					** As such, there's no point in modifying the -1 point.
					** We already set the -1 point to the 0 point above.
					*/
				}
				else
				{
					if (s->LoopBegin == 0)
						Data16[-1] = Data16[s->LoopEnd-1];

					Data16[s->LoopEnd+0] = Data16[s->LoopBegin+0];
					Data16[s->LoopEnd+1] = Data16[s->LoopBegin+1];

					// right sample (if present)
					if (Data16R != NULL)
					{
						if (s->LoopBegin == 0)
							Data16R[-1] = Data16R[s->LoopEnd-1];

						Data16R[s->LoopEnd+0] = Data16R[s->LoopBegin+0];
						Data16R[s->LoopEnd+1] = Data16R[s->LoopBegin+1];
					}
				}
			}
			else
			{
				Data16[s->Length+0] = Data16[s->Length-1];
				Data16[s->Length+1] = Data16[s->Length-1];

				// right sample (if present)
				if (Data16R != NULL)
				{
					Data16R[s->Length+0] = Data16R[s->Length-1];
					Data16R[s->Length+1] = Data16R[s->Length-1];
				}
			}
		}
		else
		{
			// 8 bit

			if (HasLoop)
			{
				if (s->Flags & SMPF_LOOP_PINGPONG)
				{
					int32_t LastSample = s->LoopEnd - 1;
					if (LastSample < 0)
						LastSample = 0;

					Data8[s->LoopEnd+0] = Data8[LastSample];
					if (LastSample > 0)
						Data8[s->LoopEnd+1] = Data8[LastSample-1];
					else
						Data8[s->LoopEnd+1] = Data8[LastSample];

					// right sample (if present)
					if (Data8R != NULL)
					{
						Data8R[s->LoopEnd+0] = Data8R[LastSample];
						if (LastSample > 0)
							Data8R[s->LoopEnd+1] = Data8R[LastSample-1];
						else
							Data8R[s->LoopEnd+1] = Data8R[LastSample];

					}

					/* For bidi loops:
					** The loopstart point is never read after having looped once.
					** IT2 behaves like that. It loops loopstart+1 to loopend-1.
					** As such, there's no point in modifying the -1 point.
					** We already set the -1 point to the 0 point above.
					*/
				}
				else
				{
					if (s->LoopBegin == 0)
						Data8[-1] = Data8[s->LoopEnd-1];

					Data8[s->LoopEnd+0] = Data8[s->LoopBegin+0];
					Data8[s->LoopEnd+1] = Data8[s->LoopBegin+1];

					// right sample (if present)
					if (Data8R != NULL)
					{
						if (s->LoopBegin == 0)
							Data8R[-1] = Data8R[s->LoopEnd-1];

						Data8R[s->LoopEnd+0] = Data8R[s->LoopBegin+0];
						Data8R[s->LoopEnd+1] = Data8R[s->LoopBegin+1];
					}
				}
			}
			else
			{
				Data8[s->Length+0] = Data8[s->Length-1];
				Data8[s->Length+1] = Data8[s->Length-1];

				// right sample (if present)
				if (Data8R != NULL)
				{
					Data8R[s->Length+0] = Data8R[s->Length-1];
					Data8R[s->Length+1] = Data8R[s->Length-1];
				}
			}
		}
	}
}

static void HQ_CloseDriver(void)
{
	if (fMixBuffer != NULL)
	{
		free(fMixBuffer);
		fMixBuffer = NULL;
	}

	if (Driver.fCubicLUT != NULL)
	{
		free(Driver.fCubicLUT);
		Driver.fCubicLUT = NULL;
	}

	DriverClose = NULL;
	DriverMix = NULL;
	DriverSetTempo = NULL;
	DriverSetMixVolume = NULL;
	DriverFixSamples = NULL;
	DriverResetMixer = NULL;
	DriverPostMix = NULL;
	DriverMixSamples = NULL;
}

bool HQ_InitDriver(int32_t mixingFrequency)
{
	if (mixingFrequency < 8000)
		mixingFrequency = 8000;

#if CPU_32BIT
	if (mixingFrequency > 48000) // higher means bigger pitch errors (.16fp limitation)!
		mixingFrequency = 48000;
#else
	if (mixingFrequency > 768000)
		mixingFrequency = 768000;
#endif

	const int32_t MaxSamplesToMix = (int32_t)ceil((mixingFrequency * 2.5) / LOWEST_BPM_POSSIBLE) + 1;

	fMixBuffer = (float *)malloc(MaxSamplesToMix * 2 * sizeof (float));
	if (fMixBuffer == NULL)
		return false;

	Driver.Flags = DF_SUPPORTS_MIDI | DF_USES_VOLRAMP | DF_HAS_RESONANCE_FILTER;
	Driver.NumChannels = 256;
	Driver.MixSpeed = mixingFrequency;
	Driver.Type = DRIVER_HQ;

	// calculate samples-per-tick tables
	for (int32_t i = LOWEST_BPM_POSSIBLE; i <= 255; i++)
	{
		const double dSamplesPerTick = (Driver.MixSpeed * 2.5) / i;

		// break into int/frac parts
		double dInt;
		const double dFrac = modf(dSamplesPerTick, &dInt);

		const uint32_t index = i - LOWEST_BPM_POSSIBLE;
		SamplesPerTickInt[index] = (uint32_t)dInt;
		SamplesPerTickFrac[index] = (uint32_t)((dFrac * BPM_FRAC_SCALE) + 0.5);
	}

	// setup driver functions
	DriverClose = HQ_CloseDriver;
	DriverMix = HQ_Mix;
	DriverSetTempo = HQ_SetTempo;
	DriverSetMixVolume = HQ_SetMixVolume;
	DriverFixSamples = HQ_FixSamples;
	DriverResetMixer = HQ_ResetMixer;
	DriverPostMix = HQ_PostMix;
	DriverMixSamples = HQ_MixSamples;

#if CPU_32BIT
	dFreq2DeltaMul = (double)(UINT16_MAX+1.0) / mixingFrequency; // .16fp
#else
	dFreq2DeltaMul = (double)(UINT32_MAX+1.0) / mixingFrequency; // .32fp
#endif

	return InitCubicSplineLUT();
}
