/*
** OPL2 emulator based on Opal (OPL3), from Reality Adlib Tracker v2.0a.
** Opal was released under the public domain, which means I can safely set
** a BSD 3-Clause license on this.
**
** Modifications:
** - It has been ported from C++ to C, with OPL3 stuff removed (ST3 uses OPL2)
** - Interpolation has been changed from 2-point linear to 16-point windowed-sinc
** - Added a ~3.18Hz DC-blocking high-pass filter (from Sound Blaster 1.0 schematics)
** - Added some bugfixes from OpenMPT's custom Opal library (also BSD 3-Clause license)
*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "opl2.h"
#include "../dig.h" // 8bb: CLAMP(), etc.
#include "../mixer/sinc.h"

#define ISA_OSCPIN_CLK (157500000.0 / 11.0) /* 8bb: exact nominal clock */
#define OPL2_OUTPUT_RATE (ISA_OSCPIN_CLK / 288.0) /* 8bb: ~49715.9090Hz */
#define NUM_CHANNELS 9
#define OPERATORS_PER_CHANNEL 2
#define NUM_OPERATORS (NUM_CHANNELS*OPERATORS_PER_CHANNEL)

enum
{
	ENV_OFF     = -1,
	ENV_ATTACK  =  0,
	ENV_DECAY   =  1,
	ENV_SUSTAIN =  2,
	ENV_RELEASE =  3,
};

typedef struct rcFilter_t
{
	float lastSample, b1, a0;
} rcFilter_t;

typedef struct Operator_t
{
	void *ParentChan; // 8bb: Channel_t type
	bool KeyOn, KeyScaleRate, SustainMode, TremoloEnable, VibratoEnable;
	int16_t EnvelopeLevel, Out[2];
	const uint16_t *AttackTab, *DecayTab, *ReleaseTab;
	uint16_t Waveform, FreqMultTimes2, OutputLevel, AttackRate, DecayRate, SustainLevel, ReleaseRate;
	uint16_t AttackShift, AttackMask, AttackAdd, DecayShift, DecayMask, DecayAdd, ReleaseShift;
	uint16_t ReleaseMask, ReleaseAdd, KeyScaleShift, KeyScaleLevel;
	int32_t EnvelopeStage;
	uint32_t Phase;
} Operator_t;

typedef struct Channel_t
{
	uint16_t Freq, Octave, KeyScaleNumber, FeedbackShift, ModulationType;
	uint32_t PhaseStep;
	Operator_t *Op[OPERATORS_PER_CHANNEL];
} Channel_t;

static const uint16_t RateTables[4][8] =
{
	{ 1, 0, 1, 0, 1, 0, 1, 0 },
	{ 1, 0, 1, 0, 0, 0, 1, 0 },
	{ 1, 0, 0, 0, 1, 0, 0, 0 },
	{ 1, 0, 0, 0, 0, 0, 0, 0 }
};

static const uint16_t ExpTable[256] = // 8bb: from ROM, but modified for speed: x=x+1024
{
	2042,2037,2031,2026,2020,2015,2010,2004,1999,1993,1988,1983,1977,1972,1966,1961,
	1956,1951,1945,1940,1935,1930,1924,1919,1914,1909,1904,1898,1893,1888,1883,1878,
	1873,1868,1863,1858,1853,1848,1843,1838,1833,1828,1823,1818,1813,1808,1803,1798,
	1794,1789,1784,1779,1774,1769,1765,1760,1755,1750,1746,1741,1736,1732,1727,1722,
	1717,1713,1708,1704,1699,1694,1690,1685,1681,1676,1672,1667,1663,1658,1654,1649,
	1645,1640,1636,1631,1627,1623,1618,1614,1609,1605,1601,1596,1592,1588,1584,1579,
	1575,1571,1566,1562,1558,1554,1550,1545,1541,1537,1533,1529,1525,1520,1516,1512,
	1508,1504,1500,1496,1492,1488,1484,1480,1476,1472,1468,1464,1460,1456,1452,1448,
	1444,1440,1436,1433,1429,1425,1421,1417,1413,1409,1406,1402,1398,1394,1391,1387,
	1383,1379,1376,1372,1368,1364,1361,1357,1353,1350,1346,1342,1339,1335,1332,1328,
	1324,1321,1317,1314,1310,1307,1303,1300,1296,1292,1289,1286,1282,1279,1275,1272,
	1268,1265,1261,1258,1255,1251,1248,1244,1241,1238,1234,1231,1228,1224,1221,1218,
	1214,1211,1208,1205,1201,1198,1195,1192,1188,1185,1182,1179,1176,1172,1169,1166,
	1163,1160,1157,1154,1150,1147,1144,1141,1138,1135,1132,1129,1126,1123,1120,1117,
	1114,1111,1108,1105,1102,1099,1096,1093,1090,1087,1084,1081,1078,1075,1072,1069,
	1066,1064,1061,1058,1055,1052,1049,1046,1044,1041,1038,1035,1032,1030,1027,1024
};

static const uint16_t LogSinTable[256] = // 8bb: from ROM
{
	2137,1731,1543,1419,1326,1252,1190,1137,1091,1050,1013, 979, 949, 920, 894, 869,
	 846, 825, 804, 785, 767, 749, 732, 717, 701, 687, 672, 659, 646, 633, 621, 609,
	 598, 587, 576, 566, 556, 546, 536, 527, 518, 509, 501, 492, 484, 476, 468, 461,
	 453, 446, 439, 432, 425, 418, 411, 405, 399, 392, 386, 380, 375, 369, 363, 358,
	 352, 347, 341, 336, 331, 326, 321, 316, 311, 307, 302, 297, 293, 289, 284, 280,
	 276, 271, 267, 263, 259, 255, 251, 248, 244, 240, 236, 233, 229, 226, 222, 219,
	 215, 212, 209, 205, 202, 199, 196, 193, 190, 187, 184, 181, 178, 175, 172, 169,
	 167, 164, 161, 159, 156, 153, 151, 148, 146, 143, 141, 138, 136, 134, 131, 129,
	 127, 125, 122, 120, 118, 116, 114, 112, 110, 108, 106, 104, 102, 100,  98,  96,
	  94,  92,  91,  89,  87,  85,  83,  82,  80,  78,  77,  75,  74,  72,  70,  69,
	  67,  66,  64,  63,  62,  60,  59,  57,  56,  55,  53,  52,  51,  49,  48,  47,
	  46,  45,  43,  42,  41,  40,  39,  38,  37,  36,  35,  34,  33,  32,  31,  30,
	  29,  28,  27,  26,  25,  24,  23,  23,  22,  21,  20,  20,  19,  18,  17,  17,
	  16,  15,  15,  14,  13,  13,  12,  12,  11,  10,  10,   9,   9,   8,   8,   7,
	   7,   7,   6,   6,   5,   5,   5,   4,   4,   4,   3,   3,   3,   2,   2,   2,
	   2,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,   0,   0,   0,   0
};

static const int chan_ops[NUM_CHANNELS] = { 0, 1, 2, 6, 7, 8, 12, 13, 14 };

static const int8_t op_lookup[32] =
{
	 0,  1,  2,  3,  4,  5, -1, -1,  6,  7,  8,  9, 10, 11, -1, -1,
	12, 13, 14, 15, 16, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

static const uint16_t mul_times_2[16] =
{
	1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 20, 24, 24, 30, 30
};

static const uint8_t kslShift[4] = { 8, 1, 2, 0 };

static const uint8_t levtab[128] = // 8bb: based on KSL table from ROM, but modified for speed
{
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   8,  12,  16,  20,  24,  28,  32,
	  0,   0,   0,   0,   0,  12,  20,  28,  32,  40,  44,  48,  52,  56,  60,  64,
	  0,   0,   0,  20,  32,  44,  52,  60,  64,  72,  76,  80,  84,  88,  92,  96,
	  0,   0,  32,  52,  64,  76,  84,  92,  96, 104, 108, 112, 116, 120, 124, 128,
	  0,  32,  64,  84,  96, 108, 116, 124, 128, 136, 140, 144, 148, 152, 156, 160,
	  0,  64,  96, 116, 128, 140, 148, 156, 160, 168, 172, 176, 180, 184, 188, 192,
	  0,  96, 128, 148, 160, 172, 180, 188, 192, 200, 204, 208, 212, 216, 220, 224
};

static bool NoteSel, TremoloDepth, VibratoDepth;
static uint16_t Clock, TremoloClock, TremoloLevel, VibratoTick, VibratoClock;
static float fSampleBuffer[SINC_TAPS];
static uint64_t resamplingFrac, resamplingDelta;
static Channel_t Channel[NUM_CHANNELS];
static Operator_t Operator[NUM_OPERATORS];
static rcFilter_t filter;

static int16_t OperatorOutput(Operator_t *Op, uint32_t phase_step, int16_t vibrato, int16_t mod, int16_t fbshift)
{
	// Advance wave phase
	if (Op->VibratoEnable)
		phase_step += vibrato;

	Op->Phase += (phase_step * Op->FreqMultTimes2) >> 1;

	const uint16_t level = (Op->EnvelopeLevel + Op->OutputLevel + Op->KeyScaleLevel + (Op->TremoloEnable ? TremoloLevel : 0)) << 3;

	switch (Op->EnvelopeStage)
	{
		// Attack stage
		case ENV_ATTACK:
		{
			uint16_t add = ((Op->AttackAdd >> Op->AttackTab[(Clock >> Op->AttackShift) & 7]) * ~Op->EnvelopeLevel) >> 3;
			if (Op->AttackRate == 0)
				add = 0;

			if (Op->AttackMask && (Clock & Op->AttackMask))
				add = 0;

			Op->EnvelopeLevel += add;
			if (Op->EnvelopeLevel <= 0)
			{
				Op->EnvelopeLevel = 0;
				Op->EnvelopeStage = ENV_DECAY;
			}
		}
		break;

		// Decay stage
		case ENV_DECAY:
		{
			uint16_t add = Op->DecayAdd >> Op->DecayTab[(Clock >> Op->DecayShift) & 7];
			if (Op->DecayRate == 0)
				add = 0;

			if (Op->DecayMask && (Clock & Op->DecayMask))
				add = 0;

			Op->EnvelopeLevel += add;
			if (Op->EnvelopeLevel >= Op->SustainLevel)
			{
				Op->EnvelopeLevel = Op->SustainLevel;
				Op->EnvelopeStage = ENV_SUSTAIN;
			}
		}
		break;

		// Sustain stage
		case ENV_SUSTAIN:
		{
			if (Op->SustainMode)
				break;

			// Note: fall-through!
		}

		// Release stage
		case ENV_RELEASE:
		{
			uint16_t add = Op->ReleaseAdd >> Op->ReleaseTab[(Clock >> Op->ReleaseShift) & 7];
			if (Op->ReleaseRate == 0)
				add = 0;

			if (Op->ReleaseMask && (Clock & Op->ReleaseMask))
				add = 0;

			Op->EnvelopeLevel += add;
			if (Op->EnvelopeLevel >= 0x1FF)
			{
				Op->EnvelopeLevel = 0x1FF;
				Op->EnvelopeStage = ENV_OFF;
				Op->Out[0] = Op->Out[1] = 0;
				return 0;
			}
		}
		break;

		// Envelope, and therefore the operator, is not running
		default:
			Op->Out[0] = Op->Out[1] = 0;
			return 0;
	}

	// Feedback? In that case we modulate by a blend of the last two samples
	if (fbshift != 0)
		mod += (Op->Out[0] + Op->Out[1]) >> fbshift;

	const uint16_t phase = (uint16_t)(Op->Phase >> 10) + mod;
	uint16_t offset = phase & 0xFF;
	uint16_t logsin;
	bool negate = false;

	switch (Op->Waveform)
	{
		case 0: // Standard sine wave
		{
			if (phase & 0x100)
				offset ^= 0xFF;

			logsin = LogSinTable[offset];
			negate = !!(phase & 0x200);
		}
		break;

		case 1: // Half sine wave
		{
			if (phase & 0x200)
				logsin = 0x1000;
			else if (phase & 0x100)
				logsin = LogSinTable[offset ^ 0xFF];
			else
				logsin = LogSinTable[offset];
		}
		break;

		case 2: // Positive sine wave
		{
			if (phase & 0x100)
				offset ^= 0xFF;

			logsin = LogSinTable[offset];
		}
		break;

		default:
		case 3: // Quarter positive sine wave
		{
			if (phase & 0x100)
				logsin = 0x1000;
			else
				logsin = LogSinTable[offset];
		}
		break;
	}

	uint16_t mix = logsin + level;
	if (mix > 0x1FFF)
		mix = 0x1FFF;

	int16_t v = ExpTable[mix & 0xFF] >> (mix >> 8);

	v += v;
	if (negate)
		v = ~v;

	// Keep last two results for feedback calculation
	Op->Out[1] = Op->Out[0];
	Op->Out[0] = v;

	return v;
}

static int16_t ChannelOutput(Channel_t *Ch)
{
	int16_t out, vibrato = (Ch->Freq >> 7) & 7;
	if (!VibratoDepth)
		vibrato >>= 1;

	// 0  3  7  3  0  -3  -7  -3
	uint16_t clk = VibratoClock;
	if (!(clk & 3))
	{
		vibrato = 0; // Position 0 and 4 is zero
	}
	else
	{
		if (clk & 1)
			vibrato >>= 1; // Odd positions are half the magnitude

		vibrato <<= Ch->Octave;
		if (clk & 4)
			vibrato = -vibrato; // The second half positions are negative
	}

	// Combine individual operator outputs
	if (Ch->ModulationType == 0)
	{
		// Frequency modulation (well, phase modulation technically)
		out = OperatorOutput(Ch->Op[0], Ch->PhaseStep, vibrato, 0, Ch->FeedbackShift);
		out = OperatorOutput(Ch->Op[1], Ch->PhaseStep, vibrato, out, 0);
	}
	else
	{
		// Additive
		out  = OperatorOutput(Ch->Op[0], Ch->PhaseStep, vibrato, 0, Ch->FeedbackShift);
		out += OperatorOutput(Ch->Op[1], Ch->PhaseStep, vibrato, 0, 0);
	}

	return out;
}

static float OutputOPL2Sample(void)
{
	int32_t mix = 0;

	// Sum the output of each channel
	Channel_t *Ch = Channel;
	for (int32_t i = 0; i < NUM_CHANNELS; i++, Ch++)
		mix += ChannelOutput(Ch);
	mix = CLAMP(mix, INT16_MIN, INT16_MAX);

	Clock++;

	TremoloClock = (TremoloClock + 1) % 13440;
	TremoloLevel = ((TremoloClock < 13440/2) ? TremoloClock : (13440 - TremoloClock)) >> 8;
	if (!TremoloDepth)
		TremoloLevel >>= 2;

	if (++VibratoTick >= 1024)
	{
		VibratoTick = 0;
		VibratoClock = (VibratoClock + 1) & 7;
	}

	float fMix = (float)mix * (1.0f / 32768.0f);

	// 8bb: apply DC-centering high-pass filter
	filter.lastSample = (fMix * filter.a0) + (filter.lastSample * filter.b1);
	fMix -= filter.lastSample;

	return fMix;
}

static void ComputePhaseStep(Channel_t *Ch)
{
	Ch->PhaseStep = (uint32_t)Ch->Freq << Ch->Octave;
}

static void ComputeRates(Channel_t *Ch, Operator_t *Op)
{
	int32_t combined_rate = Op->AttackRate * 4 + (Ch->KeyScaleNumber >> (Op->KeyScaleRate ? 0 : 2));
	int32_t rate_high = combined_rate >> 2;
	int32_t rate_low = combined_rate & 3;

	Op->AttackShift = (uint16_t)(rate_high < 12 ? (12 - rate_high) : 0);
	Op->AttackMask = (1 << Op->AttackShift) - 1;
	Op->AttackAdd = (rate_high < 12) ? 1 : (1 << (rate_high - 12));
	Op->AttackTab = RateTables[rate_low];

	if (Op->AttackRate == 15) // Attack rate of 15 is always instant
		Op->AttackAdd = 0xFFF;

	combined_rate = Op->DecayRate * 4 + (Ch->KeyScaleNumber >> (Op->KeyScaleRate ? 0 : 2));
	rate_high = combined_rate >> 2;
	rate_low = combined_rate & 3;

	Op->DecayShift = (uint16_t)(rate_high < 12 ? (12 - rate_high) : 0);
	Op->DecayMask = (1 << Op->DecayShift) - 1;
	Op->DecayAdd = (rate_high < 12) ? 1 : (1 << (rate_high - 12));
	Op->DecayTab = RateTables[rate_low];

	combined_rate = Op->ReleaseRate * 4 + (Ch->KeyScaleNumber >> (Op->KeyScaleRate ? 0 : 2));
	rate_high = combined_rate >> 2;
	rate_low = combined_rate & 3;

	Op->ReleaseShift = (uint16_t)(rate_high < 12 ? (12 - rate_high) : 0);
	Op->ReleaseMask = (1 << Op->ReleaseShift) - 1;
	Op->ReleaseAdd = (rate_high < 12) ? 1 : (1 << (rate_high - 12));
	Op->ReleaseTab = RateTables[rate_low];
}

static void ComputeKeyScaleLevel(Channel_t *Ch, Operator_t *Op)
{
	const int32_t i = (Ch->Octave << 4) | (Ch->Freq >> 6);
	Op->KeyScaleLevel = levtab[i & 127] >> Op->KeyScaleShift;
}

static void ComputeKeyScaleNumber(Channel_t *Ch)
{
	uint16_t lsb = (NoteSel ? (Ch->Freq >> 9) : (Ch->Freq >> 8)) & 1;
	Ch->KeyScaleNumber = (Ch->Octave << 1) | lsb;

	// Get the channel operators to recompute their rates as they're dependent on this number.
	// They also need to recompute their key scale level.
	for (int32_t i = 0; i < OPERATORS_PER_CHANNEL; i++)
	{
		Operator_t *Op = Ch->Op[i];
		if (Op == NULL)
			continue;

		Channel_t *OpCh = (Channel_t *)Op->ParentChan;
		ComputeRates(OpCh, Op);
		ComputeKeyScaleLevel(OpCh, Op);
	}
}

static void SetFrequencyLow(Channel_t *Ch, uint16_t frequency)
{
	Ch->Freq = (Ch->Freq & 0x300) | (frequency & 0xFF);
	ComputePhaseStep(Ch);
}

static void SetFrequencyHigh(Channel_t *Ch, uint16_t frequency)
{
	Ch->Freq = (Ch->Freq & 0xFF) | ((frequency & 3) << 8);
	ComputePhaseStep(Ch);
	ComputeKeyScaleNumber(Ch);
}

static void SetOctave(Channel_t *Ch, uint16_t octave)
{
	Ch->Octave = octave & 7;
	ComputePhaseStep(Ch);
	ComputeKeyScaleNumber(Ch);
}

static void OperatorSetKeyOn(Operator_t *Op, bool on)
{
	if (Op->KeyOn == on) // Already on/off?
		return;

	Op->KeyOn = on;
	if (on)
	{
		// The highest attack rate is instant; it bypasses the attack phase
		if (Op->AttackRate == 15)
		{
			Op->EnvelopeStage = ENV_DECAY;
			Op->EnvelopeLevel = 0;
		}
		else
		{
			Op->EnvelopeStage = ENV_ATTACK;
		}

		Op->Phase = 0;
	}
	else
	{
		// Stopping current sound?
		if (Op->EnvelopeStage != ENV_OFF && Op->EnvelopeStage != ENV_RELEASE)
			Op->EnvelopeStage = ENV_RELEASE;
	}
}

static void SetKeyOn(Channel_t *Ch, bool on)
{
	OperatorSetKeyOn(Ch->Op[0], on);
	OperatorSetKeyOn(Ch->Op[1], on);
}

static void SetFeedback(Channel_t *Ch, uint16_t val)
{
	Ch->FeedbackShift = val ? (9 - val) : 0;
}

static void SetEnvelopeScaling(Channel_t *Ch, Operator_t *Op, bool on)
{
	Op->KeyScaleRate = on;
	ComputeRates(Ch, Op);
}

static void SetKeyScale(Channel_t *Ch, Operator_t *Op, uint16_t scale)
{
	Op->KeyScaleShift = kslShift[scale];
	ComputeKeyScaleLevel(Ch, Op);
}

static void SetAttackRate(Channel_t *Ch, Operator_t *Op, uint16_t rate)
{
	Op->AttackRate = rate;
	ComputeRates(Ch, Op);
}

static void SetDecayRate(Channel_t *Ch, Operator_t *Op, uint16_t rate)
{
	Op->DecayRate = rate;
	ComputeRates(Ch, Op);
}

static void SetSustainLevel(Operator_t *Op, uint16_t level)
{
	Op->SustainLevel = (level < 15 ? level : 31) << 4;
}

static void SetReleaseRate(Channel_t *Ch, Operator_t *Op, uint16_t rate)
{
	Op->ReleaseRate = rate;
	ComputeRates(Ch, Op);
}

void OPL2_Init(int32_t audioOutputFrequency)
{
	if (audioOutputFrequency <= 0)
		audioOutputFrequency = 44100;

	// 8bb: OPL DC-blocking high-pass filter
	const double cutoffHz = 3.18309886184; // 8bb: based on RC values from Sound Blaster 1.0 schematics
	filter.b1 = (float)exp((-2.0 * PI) * cutoffHz / OPL2_OUTPUT_RATE);
	filter.a0 = 1.0f - filter.b1;
	filter.lastSample = 0.0f;

	TremoloClock = TremoloLevel = VibratoTick = VibratoClock = Clock = 0;
	NoteSel = TremoloDepth = VibratoDepth = false;

	// Initialize operators
	memset(Operator, 0, sizeof (Operator));
	Operator_t *Op = Operator;
	for (int32_t i = 0; i < NUM_OPERATORS; i++, Op++)
	{
		Op->FreqMultTimes2 = 1;
		Op->EnvelopeStage = ENV_OFF;
		Op->EnvelopeLevel = 0x1FF;
	}

	// Initialize channels
	memset(Channel, 0, sizeof (Channel));
	Channel_t *Ch = Channel;
	for (int32_t i = 0; i < NUM_CHANNELS; i++, Ch++)
	{
		const int32_t op = chan_ops[i];
		Ch->Op[0] = &Operator[op+0];
		Ch->Op[1] = &Operator[op+3];
		Ch->Op[0]->ParentChan = Ch;
		Ch->Op[1]->ParentChan = Ch;
	}

	// Initialize operator rates
	Op = Operator;
	for (int32_t i = 0; i < NUM_OPERATORS; i++, Op++)
		ComputeRates((Channel_t *)Op->ParentChan, Op);

	resamplingDelta = (uint64_t)round(RESAMPLING_FRAC_SCALE * (OPL2_OUTPUT_RATE / (double)audioOutputFrequency));
	resamplingFrac = 0;
}

void OPL2_WritePort(uint16_t reg_num, uint8_t val)
{
	uint16_t type = reg_num & 0xE0;

	// Is it 0xBD, the one-off register stuck in the middle of the register array?
	if (reg_num == 0xBD)
	{
		TremoloDepth = !!(val & 0x80);
		VibratoDepth = !!(val & 0x40);
		return;
	}

	// Global registers
	if (type == 0x00)
	{
		if (reg_num == 0x08) // CSW / Note-sel
		{
			NoteSel = !!(val & 0x40);

			// Get the channels to recompute the Key Scale No. as this varies based on NoteSel
			Channel_t *Ch = Channel;
			for (int32_t i = 0; i < NUM_CHANNELS; i++, Ch++)
				ComputeKeyScaleNumber(Ch);
		}
	}
	else if (type >= 0xA0 && type <= 0xC0) // Channel registers
	{
		// Convert to channel number
		int32_t chan_num = reg_num & 15;
		if (chan_num >= 9) // Valid channel?
			return;

		Channel_t *Ch = &Channel[chan_num];

		// Do specific registers
		switch (reg_num & 0xF0)
		{
			default:
				break;
			
			// Frequency low
			case 0xA0:
				SetFrequencyLow(Ch, val);
				break;
	
			// Key-on / Octave / Frequency High
			case 0xB0:
				SetKeyOn(Ch, !!(val & 0x20));
				SetOctave(Ch, (val >> 2) & 7);
				SetFrequencyHigh(Ch, val & 3);
				break;

			// Feedback Factor / Modulation Type
			case 0xC0:
				SetFeedback(Ch, (val >> 1) & 7);
				Ch->ModulationType = val & 1;
				break;
		}
	}
	else if ((type >= 0x20 && type <= 0x80) || type == 0xE0) // Operator registers
	{
		// Convert to operator number
		int32_t op_num = op_lookup[reg_num & 0x1F];
		if (op_num < 0) // Valid register?
			return;

		Operator_t *Op = &Operator[op_num];
		Channel_t *OpCh = (Channel_t *)Op->ParentChan;

		// Do specific registers
		switch (type)
		{
			// Tremolo Enable / Vibrato Enable / Sustain Mode / Envelope Scaling / Frequency Multiplier
			case 0x20:
				Op->TremoloEnable = !!(val & 0x80);
				Op->VibratoEnable = !!(val & 0x40);
				Op->SustainMode = !!(val & 0x20);
				SetEnvelopeScaling(OpCh, Op, !!(val & 0x10));
				Op->FreqMultTimes2 = mul_times_2[val & 15];
				break;

			// Key Scale / Output Level
			case 0x40:
				SetKeyScale(OpCh, Op, val >> 6);
				Op->OutputLevel = (val & 0x3F) << 2;
				break;

			// Attack Rate / Decay Rate
			case 0x60:
				SetAttackRate(OpCh, Op, val >> 4);
				SetDecayRate(OpCh, Op, val & 15);
				break;

			// Sustain Level / Release Rate
			case 0x80:
				SetSustainLevel(Op, val >> 4);
				SetReleaseRate(OpCh, Op, val & 15);
				break;
			
			// Waveform
			case 0xE0:
				Op->Waveform = val & 3;
				break;
		}
	}
}

static float OPL2_Output(void)
{
	resamplingFrac += resamplingDelta;
	while (resamplingFrac >= RESAMPLING_FRAC_SCALE)
	{
		resamplingFrac -= RESAMPLING_FRAC_SCALE;

		// 8bb: advance resampling ring buffer
		for (int32_t i = 0; i < SINC_TAPS-1; i++)
			fSampleBuffer[i] = fSampleBuffer[1+i];
		fSampleBuffer[SINC_TAPS-1] = OutputOPL2Sample();
	}

	const uint32_t frac32 = (uint32_t)resamplingFrac;
	const uint32_t lutPhase = frac32 >> INTRP_PHASE_SHIFT; // 8bb: 0 .. SINC_OVERSAMPLING-1
	const float fIntrpFrac = (int32_t)(frac32 & INTRP_PHASE_MASK) * (1.0f / INTRP_PHASE_SCALE);

	// 8bb: it may look like we go out of bounds for fSinc_2, but we have an extra phase after LUT
	const float *fSinc_1 = fSincLUT + ( lutPhase    << SINC_TAPS_BITS);
	const float *fSinc_2 = fSincLUT + ((lutPhase+1) << SINC_TAPS_BITS);

	float fSum = 0.0f;
	for (int32_t i = 0; i < SINC_TAPS; i++)
	{
		// 8bb: do linear interpolation between phases
		const float y1 = fSinc_1[i];
		const float y2 = fSinc_2[i];
		fSum += fSampleBuffer[i] * (y1 + ((y2 - y1) * fIntrpFrac));
	}

	return fSum;
}

void OPL2_RenderSamples(float *fMixBufL, float *fMixBufR, int32_t numSamples)
{
	for (int32_t i = 0; i < numSamples; i++)
	{
		const float sample = OPL2_Output();

		fMixBufL[i] += sample;
		fMixBufR[i] += sample;
	}
}
