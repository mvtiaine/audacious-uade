// SPDX-License-Identifier: BSD-2-Clause
/*-----------------*/
/* libdigibooster3 */
/*-----------------*/

/*
  Copyright (c) 2014, Grzegorz Kraszewski
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met: 

  1. Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer. 
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution. 

  This software is provided by the copyright holders and contributors "as is" and
  any express or implied warranties, including, but not limited to the implied
  warranties of merchantability and fitness for a particular purpose are
  disclaimed. In no event shall the copyright owner or contributors be liable for
  any direct, indirect, incidental, special, exemplary, or consequential damages
  (including, but not limited to, procurement of substitute goods or services;
  loss of use, data, or profits; or business interruption) however caused and
  on any theory of liability, whether in contract  strict liability or tort
  (including negligence or otherwise) arising in any way out of the use of this
  software, even if advised of the possibility of such damage.
*/

#include "libdigibooster3.h"

/* Pattern decoder state machine states */

#define DBM0_TRACKNUM  1
#define DBM0_BITFIELD  2
#define DBM0_NOTE      3
#define DBM0_INSTR     4
#define DBM0_CMD1      5
#define DBM0_PARAM1    6
#define DBM0_CMD2      7
#define DBM0_PARAM2    8

/* Packed pattern bitmask fields. */

#define DBM0_PATTERN_HAVE_NOTE       0x01
#define DBM0_PATTERN_HAVE_INSTR      0x02
#define DBM0_PATTERN_HAVE_CMD1       0x04
#define DBM0_PATTERN_HAVE_PARAM1     0x08
#define DBM0_PATTERN_HAVE_CMD2       0x10
#define DBM0_PATTERN_HAVE_PARAM2     0x20

/* Envelope types. */

#define ENVTYPE_VOLUME   0
#define ENVTYPE_PANNING  1

/* Envelope flags */

#define DBM0_ENV_ENABLED             0x01
#define DBM0_ENV_SUSTAIN_A           0x02
#define DBM0_ENV_LOOP                0x04
#define DBM0_ENV_SUSTAIN_B           0x08

struct DataChunk
{
	int Size;
	int Pos;
};



static int assign_envelopes(struct DB3Module *m)
{
	int env;

	for (env = 0; env < m->NumVolEnv; env++)
	{
		int instr = m->VolEnvs[env].InstrNum;

		if (instr != ENVELOPE_DISABLED)
		{
			if ((instr > 0) && (instr <= m->NumInstr))
			{
				m->Instruments[instr - 1]->VolEnv = env;
			}
			else return 0;
		}
	}

	for (env = 0; env < m->NumPanEnv; env++)
	{
		int instr = m->PanEnvs[env].InstrNum;

		if (instr != ENVELOPE_DISABLED)
		{
			if ((instr > 0) && (instr <= m->NumInstr))
			{
				m->Instruments[instr - 1]->PanEnv = env;
			}
			else return 0;
		}
	}

	return 1;
}



static int verify_playlists(struct DB3Module *m)
{
	int song, order;

	for (song = 0; song < m->NumSongs; song++)
	{
		struct DB3ModSong *mso = m->Songs[song];

		for (order = 0; order < mso->NumOrders; order++)
		{
			if (mso->PlayList[order] >= m->NumPatterns) return 0;
		}
	}

	return 1;
}



static int verify_songs(struct DB3Module *m)
{
	int i;

	for (i = 0; i < m->NumSongs; i++)
	{
		if (!m->Songs[i]) return 0;
	}

	return 1;
}



static int verify_patterns(struct DB3Module *m)
{
	int i;

	for (i = 0; i < m->NumPatterns; i++)
	{
		if (!m->Patterns[i]) return 0;
	}

	return 1;
}



static int verify_sampled_instruments(struct DB3Module *m)
{
	int i;

	for (i = 0; i < m->NumInstr; i++)
	{
		if (!m->Instruments[i]) return 0;
	}

	for (i = 0; i < m->NumInstr; i++)
	{
		struct DB3ModInstr *mi = m->Instruments[i];

		if (mi->Type == ITYPE_SAMPLE)
		{
			struct DB3ModInstrS *mis = (struct DB3ModInstrS*)mi;
			struct DB3ModSample *msmp;

			if (mis->SampleNum >= m->NumSamples) mis->SampleNum = 0;     // workaround for broken modules
			msmp = m->Samples[mis->SampleNum];

			if (msmp && (msmp->Frames > 0))
			{
				// Loop verification. Negative loop start and loop length are rejected.
				// Loop start outside the sample is rejected too. For improved error tolerance
				// if calculated loop end is beyond the sample end, the loop length is clipped
				// and then accepted. It is ensured then that after clipping the loop will have
				// at least one frame.

				if ((mis->LoopStart < 0) || (mis->LoopLen < 0)) return 0;
				if (mis->LoopStart >= msmp->Frames) return 0;
				if (mis->LoopStart + mis->LoopLen > msmp->Frames) mis->LoopLen = msmp->Frames - mis->LoopStart;
			}
			else    // clear the loop for instrument having no frames
			{
				mis->LoopStart = 0;
				mis->LoopLen = 0;
				mis->Flags &= ~IF_LOOP_MASK;
			}

			if ((mis->C3Freq < 2000) || (mis->C3Freq > 192000)) return 0;
		}
	}

	return 1;
}



static int verify_contents(struct DB3Module *m)
{
	if (verify_sampled_instruments(m))
	{
		if (verify_patterns(m))
		{
			if (verify_songs(m))
			{
				if (verify_playlists(m))
				{
					if (assign_envelopes(m)) return 1;
				}
			}
		}
	}

	return 0;
}



#if (defined TARGET_MORPHOS) || (defined TARGET_AMIGAOS3) || (defined TARGET_AMIGAOS4)

static int db3_strlen(char *s)
{
	char *v = s;

	while (*v) v++;
	return (int)(v - s);
}


static void db3_strcpy(char *d, char *s)
{
	while (*d++ = *s++);
}


static int runtime_little_endian_check(void)
{
	return 0;
}

#elif defined TARGET_WIN32

static int runtime_little_endian_check(void)
{
	return 1;
}

#elif defined TARGET_LINUX

static int runtime_little_endian_check(void)
{
	uint32_t x = 36;

	if (*((uint8_t*)&x) == 36) return 1;
	else return 0;
}

#endif


static int db3_bcd2bin(uint8_t x)
{
	return (x >> 4) * 10 + (x & 0x0F);
}



static int read_data(struct DataChunk *dc, struct AbstractHandle *ah, void *buf, int length)
{
	int error = 0;

	if (length <= dc->Size - dc->Pos)
	{
		if (ah->ah_Read(ah, buf, length)) dc->Pos += length;
		else error = DB3_ERROR_READING_DATA;
	}
	else error = DB3_ERROR_DATA_CORRUPTED;

	return error;
}



static int read_chunk_dspe(struct DB3Module *m, struct DataChunk *dc, struct AbstractHandle *ah)
{
	int error = 0, track_mask_length;
	uint8_t *track_mask;
	uint8_t b[8];

	if ((error = read_data(dc, ah, b, 2)) == 0)
	{
		track_mask_length = (b[0] << 8) | b[1];

		if (track_mask_length == m->NumTracks)
		{
			if (track_mask = db3_malloc(track_mask_length))
			{
				if ((error = read_data(dc, ah, track_mask, track_mask_length)) == 0)
				{
					if ((error = read_data(dc, ah, b, 8)) == 0)
					{
						int i;

						for (i = 0; i < track_mask_length; i++)
						{
							if (track_mask[i] == 0) m->DspDefaults.EffectMask[i] |= DSP_MASK_ECHO;
						}

						m->DspDefaults.EchoDelay = b[1];
						m->DspDefaults.EchoFeedback = b[3];
						m->DspDefaults.EchoMix = b[5];
						m->DspDefaults.EchoCross = b[7];
					}
				}

				db3_free(track_mask);
			}
			else error = DB3_ERROR_OUT_OF_MEMORY;
		}
		else error = DB3_ERROR_DATA_CORRUPTED;
	}

	return error;
}



static int read_envelope(struct DataChunk *dc, struct DB3ModEnvelope *menv, struct AbstractHandle *ah, int type, int creator)
{
	int error = 0;
	uint8_t b[136];

	if ((error = read_data(dc, ah, b, 136)) == 0)
	{
		int point, sections, instrument;
		uint8_t flags;
		uint8_t *pp;

		flags = b[2];
		sections = b[3];
		if (sections > 31) sections = 31;
		menv->NumSections = sections;
		menv->SustainA = ENV_SUSTAIN_DISABLED;
		menv->SustainB = ENV_SUSTAIN_DISABLED;
		menv->LoopFirst = ENV_LOOP_DISABLED;
		menv->LoopLast = ENV_LOOP_DISABLED;
		instrument = (b[0] << 8) | b[1];
		if ((instrument <= 0) || (instrument > 255)) menv->InstrNum = ENVELOPE_DISABLED;
		else menv->InstrNum = instrument;

		if (flags & DBM0_ENV_ENABLED)
		{
			if (flags & DBM0_ENV_SUSTAIN_B)
			{
				if (b[7] <= menv->NumSections) menv->SustainB = b[7];
				else error = DB3_ERROR_DATA_CORRUPTED;
			}

			if (flags & DBM0_ENV_SUSTAIN_A)
			{
				if (b[4] <= menv->NumSections) menv->SustainA = b[4];
				else error = DB3_ERROR_DATA_CORRUPTED;
			}

			if (flags & DBM0_ENV_LOOP)
			{
				if ((b[6] <= menv->NumSections) && (b[5] < b[6]))
				{
					menv->LoopFirst = b[5];
					menv->LoopLast = b[6];
				}
				else error = DB3_ERROR_DATA_CORRUPTED;
			}

			if ((menv->SustainA != ENV_SUSTAIN_DISABLED) && (menv->SustainB != ENV_SUSTAIN_DISABLED))
			{
				if (menv->SustainA > menv->SustainB)
				{
					uint16_t aux;
					aux = menv->SustainA;
					menv->SustainA = menv->SustainB;
					menv->SustainB = aux;
				}
			}
		}

		pp = &b[8];

		for (point = 0; point < sections + 1; point++)
		{
			int16_t pos, val;

			pos = (pp[0] << 8) | pp[1];
			val = (pp[2] << 8) | pp[3];
			pp += 4;

			if (type == ENVTYPE_VOLUME)
			{
				if (flags & DBM0_ENV_ENABLED)
				{
					if ((pos < 0) || (pos > 2048)) error = DB3_ERROR_DATA_CORRUPTED;
					if ((val < 0) || (val > 64)) error = DB3_ERROR_DATA_CORRUPTED;
				}
				else
				{
					pos = 0;
					val = 0;
				}
			}
			else   // ENVTYPE_PANNING
			{
				if (flags & DBM0_ENV_ENABLED)
				{
					if (creator == CREATOR_DIGIBOOSTER_2) val = (val << 2) - 128;
					if ((pos < 0) || (pos > 2048)) error = DB3_ERROR_DATA_CORRUPTED;
					if ((val < -128) || (val > 128)) error = DB3_ERROR_DATA_CORRUPTED;
				}
				else
				{
					pos = 0;
					val = 0;
				}
			}

			if (!error)
			{
				menv->Points[point].Position = pos;
				menv->Points[point].Value = val;
			}
			else break;
		}
	}

	return error;
}



static int read_chunk_venv(struct DB3Module *m, struct DataChunk *dc, struct AbstractHandle *ah)
{
	int error = 0;
	uint8_t b[2];

	if ((error = read_data(dc, ah, b, 2)) == 0)
	{
		m->NumVolEnv = (b[0] << 8) | b[1];

		if (m->NumVolEnv > 255)
		{
			error = DB3_ERROR_DATA_CORRUPTED;
		}
		else if (m->NumVolEnv > 0)
		{
			if (m->VolEnvs = db3_malloc(m->NumVolEnv * sizeof(struct DB3ModEnvelope)))
			{
				int envnum;

				for (envnum = 0; envnum < m->NumVolEnv; envnum++)
				{
					error = read_envelope(dc, &m->VolEnvs[envnum], ah, ENVTYPE_VOLUME, m->CreatorVer);
					if (error) break;
				}
			}
			else error = DB3_ERROR_OUT_OF_MEMORY;
		}
	}

	return error;
}



static int read_chunk_penv(struct DB3Module *m, struct DataChunk *dc, struct AbstractHandle *ah)
{
	int error = 0;
	uint8_t b[2];

	if ((error = read_data(dc, ah, b, 2)) == 0)
	{
		m->NumPanEnv = (b[0] << 8) | b[1];
		m->PanEnvs = NULL;

		if (m->NumPanEnv > 255)
		{
			error = DB3_ERROR_DATA_CORRUPTED;
		}
		else if (m->NumPanEnv > 0)
		{
			if (m->PanEnvs = db3_malloc(m->NumPanEnv * sizeof(struct DB3ModEnvelope)))
			{
				int envnum;

				for (envnum = 0; envnum < m->NumPanEnv; envnum++)
				{
					error = read_envelope(dc, &m->PanEnvs[envnum], ah, ENVTYPE_PANNING, m->CreatorVer);
					if (error) break;
				}
			}
			else error = DB3_ERROR_OUT_OF_MEMORY;
		}
	}

	return error;
}



static int read_pattern(struct DataChunk *dc, struct DB3ModPatt *mp, struct AbstractHandle *ah, int tracks)
{
	int error = 0;
	uint8_t b[6];

	if ((error = read_data(dc, ah, b, 6)) == 0)
	{
		int rows, packsize, bcount = 0;
		uint8_t *packed_data;

		rows = (b[0] << 8) | b[1];
		packsize = (b[2] << 24) | (b[3] << 16) | (b[4] << 8) | b[5];

		if (!rows) return DB3_ERROR_DATA_CORRUPTED;
		if (packsize <= 0) return DB3_ERROR_DATA_CORRUPTED;
		mp->NumRows = rows;

		if (mp->Pattern = db3_malloc(rows * tracks * sizeof(struct DB3ModEntry)))
		{
			if (packed_data = db3_malloc(packsize))
			{
				if ((error = read_data(dc, ah, packed_data, packsize)) == 0)
				{
					uint8_t byte, bitfield = 0;
					uint8_t *packed = packed_data;
					int dstate = DBM0_TRACKNUM;   // initial state
					int row = 0;
					int packcounter = packsize;
					struct DB3ModEntry *me = NULL;

					while (!error && packcounter-- && (row < mp->NumRows))    // main decoder loop
					{
						byte = *packed++;

						switch (dstate)
						{
							case DBM0_TRACKNUM:
								if (!byte) row++;
								else
								{
									if (byte <= tracks) me = &mp->Pattern[row * tracks + byte - 1];
									else
									{
										error = DB3_ERROR_DATA_CORRUPTED;
										me = NULL;
									}

									dstate = DBM0_BITFIELD;
								}
							break;

							case DBM0_BITFIELD:
								bitfield = byte;
								if (bitfield & DBM0_PATTERN_HAVE_NOTE) dstate = DBM0_NOTE;
								else if (bitfield & DBM0_PATTERN_HAVE_INSTR) dstate = DBM0_INSTR;
								else if (bitfield & DBM0_PATTERN_HAVE_CMD1) dstate = DBM0_CMD1;
								else if (bitfield & DBM0_PATTERN_HAVE_PARAM1) dstate = DBM0_PARAM1;
								else if (bitfield & DBM0_PATTERN_HAVE_CMD2) dstate = DBM0_CMD2;
								else if (bitfield & DBM0_PATTERN_HAVE_PARAM2) dstate = DBM0_PARAM2;
								else dstate = DBM0_TRACKNUM;
							break;

							case DBM0_NOTE:
							{
								if (me)
								{
									me->Octave = byte >> 4;
									me->Note = byte & 0x0F;
								}

								if (bitfield & DBM0_PATTERN_HAVE_INSTR) dstate = DBM0_INSTR;
								else if (bitfield & DBM0_PATTERN_HAVE_CMD1) dstate = DBM0_CMD1;
								else if (bitfield & DBM0_PATTERN_HAVE_PARAM1) dstate = DBM0_PARAM1;
								else if (bitfield & DBM0_PATTERN_HAVE_CMD2) dstate = DBM0_CMD2;
								else if (bitfield & DBM0_PATTERN_HAVE_PARAM2) dstate = DBM0_PARAM2;
								else dstate = DBM0_TRACKNUM;
							}
							break;

							case DBM0_INSTR:
								if (me) me->Instr = byte;
								if (bitfield & DBM0_PATTERN_HAVE_CMD1) dstate = DBM0_CMD1;
								else if (bitfield & DBM0_PATTERN_HAVE_PARAM1) dstate = DBM0_PARAM1;
								else if (bitfield & DBM0_PATTERN_HAVE_CMD2) dstate = DBM0_CMD2;
								else if (bitfield & DBM0_PATTERN_HAVE_PARAM2) dstate = DBM0_PARAM2;
								else dstate = DBM0_TRACKNUM;
							break;

							case DBM0_CMD1:
								if (me) me->Cmd1 = byte;
								if (bitfield & DBM0_PATTERN_HAVE_PARAM1) dstate = DBM0_PARAM1;
								else if (bitfield & DBM0_PATTERN_HAVE_CMD2) dstate = DBM0_CMD2;
								else if (bitfield & DBM0_PATTERN_HAVE_PARAM2) dstate = DBM0_PARAM2;
								else dstate = DBM0_TRACKNUM;
							break;

							case DBM0_PARAM1:
								if (me)	me->Param1 = byte;
								if (bitfield & DBM0_PATTERN_HAVE_CMD2) dstate = DBM0_CMD2;
								else if (bitfield & DBM0_PATTERN_HAVE_PARAM2) dstate = DBM0_PARAM2;
								else dstate = DBM0_TRACKNUM;
							break;

							case DBM0_CMD2:
								if (me)	me->Cmd2 = byte;
								if (bitfield & DBM0_PATTERN_HAVE_PARAM2) dstate = DBM0_PARAM2;
								else dstate = DBM0_TRACKNUM;
							break;

							case DBM0_PARAM2:
								if (me) me->Param2 = byte;
								dstate = DBM0_TRACKNUM;
							break;
						}

						bcount++;
					}
				}

				db3_free(packed_data);
			}
			else error = DB3_ERROR_OUT_OF_MEMORY;

			if (error)
			{
				db3_free(mp->Pattern);
				mp->Pattern = NULL;
			}
		}
		else error = DB3_ERROR_OUT_OF_MEMORY;
	}

	return error;
}



static int read_chunk_patt(struct DB3Module *m, struct DataChunk *dc, struct AbstractHandle *ah)
{
	int error = 0, pattnum;

	for (pattnum = 0; pattnum < m->NumPatterns; pattnum++)
	{
		struct DB3ModPatt *mp;

		if (mp = db3_malloc(sizeof(struct DB3ModPatt)))
		{
			if ((error = read_pattern(dc, mp, ah, m->NumTracks)) == 0)
			{
				m->Patterns[pattnum] = mp;
			}
			else
			{
				db3_free(mp);
				break;
			}
		}
		else
		{
			error = DB3_ERROR_OUT_OF_MEMORY;
			break;
		}
	}

	return error;
}



static int read_sample_data_8bit(struct DataChunk *dc, struct DB3ModSample *ms, struct AbstractHandle *ah, void *loadbuf)
{
	int error = 0, bytes = ms->Frames;
	int block;
	int16_t *pd = ms->Data;

	while (!error && (bytes > 0))
	{
		block = DB3_SAMPLE_LOAD_BUFFER_SIZE;
		if (block > bytes) block = bytes;

		if ((error = read_data(dc, ah, loadbuf, block)) == 0)
		{
			int frame;
			int8_t x, *ps = (int8_t*)loadbuf;

			for (frame = 0; frame < block; frame++)
			{
				x = *ps++;
				*pd++ = ((int16_t)x) << 8;
			}

			bytes -= block;
		}
	}

	return error;
}



static int read_sample_data_16bit(struct DataChunk *dc, struct DB3ModSample *ms, struct AbstractHandle *ah, void *loadbuf)
{
	int error = 0, bytes = ms->Frames << 1;

	if (runtime_little_endian_check())
	{
		int block;
		int16_t *pd = ms->Data;

		while (!error && (bytes > 0))
		{
			block = DB3_SAMPLE_LOAD_BUFFER_SIZE;
			if (block > bytes) block = bytes;

			if ((error = read_data(dc, ah, loadbuf, block)) == 0)
			{
				int frame;
				int16_t x, *ps = (int16_t*)loadbuf;

				for (frame = 0; frame < (block >> 1); frame++)
				{
					x = *ps++;
					*pd++ = ((x >> 8) & 0xFF) | (x << 8);
				}

				bytes -= block;
			}
		}
	}
	else        // read data straight to the sample buffer
	{
		error = read_data(dc, ah, ms->Data, bytes);
	}

	return error;
}



static int read_sample_data_32bit(struct DataChunk *dc, struct DB3ModSample *ms, struct AbstractHandle *ah, void *loadbuf)
{
	int error = 0, bytes = ms->Frames << 2;

	if (runtime_little_endian_check())
	{
		int block;
		int16_t *pd = ms->Data;

		while (!error && (bytes > 0))
		{
			block = DB3_SAMPLE_LOAD_BUFFER_SIZE;
			if (block > bytes) block = bytes;

			if ((error = read_data(dc, ah, loadbuf, block)) == 0)
			{
				int frame;
				int32_t x, *ps = (int32_t*)loadbuf;

				for (frame = 0; frame < (block >> 2); frame++)
				{
					x = *ps++;
					*pd++ = ((x >> 24) & 0xFF) | ((x >> 8) & 0xFF00);
				}

				bytes -= block;
			}
		}
	}
	else
	{
		int block;
		int16_t *pd = ms->Data;

		while (!error && (bytes > 0))
		{
			block = DB3_SAMPLE_LOAD_BUFFER_SIZE;
			if (block > bytes) block = bytes;

			if ((error = read_data(dc, ah, loadbuf, block)) == 0)
			{
				int frame;
				int32_t x, *ps = (int32_t*)loadbuf;

				for (frame = 0; frame < (block >> 2); frame++)
				{
					x = *ps++;
					*pd++ = x >> 16;
				}

				bytes -= block;
			}
		}
	}

	return error;
}



static int read_sample(struct DataChunk *dc, struct DB3ModSample *ms, struct AbstractHandle *ah, void *loadbuf)
{
	uint8_t b[8];
	int error = 0;

	if ((error = read_data(dc, ah, b, 8)) == 0)
	{
		ms->Frames = (b[4] << 24) | (b[5] << 16) | (b[6] << 8) | b[7];

		if ((ms->Frames >= 0) && (ms->Frames < 0x40000000))   // negative length means corrupted module, limit sample to 2 GB.
		{
			if (ms->Frames > 0)   // there may be samples of 0 length
			{
				if (ms->Data = db3_malloc(ms->Frames * sizeof(int16_t)))
				{
					switch (b[3] & 0x07)  // bytes per sample
					{
						case 1:   error = read_sample_data_8bit(dc, ms, ah, loadbuf);    break;
						case 2:   error = read_sample_data_16bit(dc, ms, ah, loadbuf);   break;
						case 4:   error = read_sample_data_32bit(dc, ms, ah, loadbuf);   break;
						default:  error = DB3_ERROR_DATA_CORRUPTED;                     break;
					}
				}
				else error = DB3_ERROR_OUT_OF_MEMORY;
			}
			else ms->Data = NULL;
		}
		else error = DB3_ERROR_DATA_CORRUPTED;
	}

	return error;
}



static int read_chunk_smpl(struct DB3Module *m, struct DataChunk *dc, struct AbstractHandle *ah)
{
	int error = 0, sampnum;
	void *loadbuf = NULL;

	if (loadbuf = db3_malloc(DB3_SAMPLE_LOAD_BUFFER_SIZE))
	{
		for (sampnum = 0; sampnum < m->NumSamples; sampnum++)
		{
			struct DB3ModSample *ms;

			if (ms = db3_malloc(sizeof(struct DB3ModSample)))
			{
				if ((error = read_sample(dc, ms, ah, loadbuf)) == 0)
				{
					m->Samples[sampnum] = ms;
				}
				else
				{
					if (ms->Data) db3_free(ms->Data);
					db3_free(ms);
					break;
				}
			}
			else
			{
				error = DB3_ERROR_OUT_OF_MEMORY;
				break;
			}
		}

		db3_free(loadbuf);
	}
	else error = DB3_ERROR_OUT_OF_MEMORY;

	return error;
}



static int read_instrument(struct DataChunk *dc, struct DB3ModInstrS *mi, struct AbstractHandle *ah)
{
	uint8_t b[50];
	int error = 0;

	if ((error = read_data(dc, ah, b, 50)) == 0)
	{
		int namelen;

		if (b[29]) namelen = 30;
		else namelen = db3_strlen((char*)b);

		if (mi->Instr.Name = db3_malloc(namelen + 1))
		{
			db3_memcpy(mi->Instr.Name, b, namelen);
			mi->Instr.Name[namelen] = 0x00;
			mi->SampleNum = ((b[30] << 8) | b[31]) - 1;                 // samples from 1 in the mod, from 0 in the app
			mi->Instr.Volume = (b[32] << 8) | b[33];
			mi->Instr.Type = ITYPE_SAMPLE;
			mi->Instr.VolEnv = ENVELOPE_DISABLED;
			mi->Instr.PanEnv = ENVELOPE_DISABLED;
			mi->C3Freq = (b[34] << 24) | (b[35] << 16) | (b[36] << 8) | b[37];
			mi->LoopStart = (b[38] << 24) | (b[39] << 16) | (b[40] << 8) | b[41];
			mi->LoopLen = (b[42] << 24) | (b[43] << 16) | (b[44] << 8) | b[45];
			mi->Instr.Panning = (b[46] << 8) | b[47];
			mi->Flags = (b[48] << 8) | b[49];
			if (mi->LoopLen == 0) mi->Flags &= ~IF_LOOP_MASK;           // set loop type to none for 0 length

			if ((mi->Flags & IF_LOOP_MASK) == IF_NO_LOOP)
			{
				mi->LoopLen = 0;
				mi->LoopStart = 0;
			}
		}
		else error = DB3_ERROR_OUT_OF_MEMORY;
	}

	return error;
}



static int read_chunk_inst(struct DB3Module *m, struct DataChunk *dc, struct AbstractHandle *ah)
{
	int error = 0;
	int instrnum;

	for (instrnum = 0; instrnum < m->NumInstr; instrnum++)
	{
		struct DB3ModInstrS *mi;

		if (mi = db3_malloc(sizeof(struct DB3ModInstrS)))
		{
			if ((error = read_instrument(dc, mi, ah)) == 0)
			{
				m->Instruments[instrnum] = &mi->Instr;
			}
			else
			{
				db3_free(mi);
				break;
			}
		}
		else
		{
			error = DB3_ERROR_OUT_OF_MEMORY;
			break;
		}
	}

	return error;
}




static int read_song(struct DataChunk *dc, struct DB3ModSong *ms, struct AbstractHandle *ah)
{
	uint8_t b[46];
	int i, error = 0;

	if ((error = read_data(dc, ah, b, 46)) == 0)
	{
		int namelen;

		if (b[43]) namelen = 44;
		else namelen = db3_strlen((char*)b);

		if (ms->Name = db3_malloc(namelen + 1))
		{
			db3_memcpy(ms->Name, b, namelen);
			ms->Name[namelen] = 0x00;
			ms->NumOrders = (b[44] << 8) | b[45];

			if (ms->PlayList = db3_malloc(ms->NumOrders * sizeof(uint16_t)))
			{
				if ((error = read_data(dc, ah, ms->PlayList, ms->NumOrders * sizeof(uint16_t))) == 0)
				{
					uint8_t *p;

					for (i = 0; i < ms->NumOrders; i++)
					{
						p = (uint8_t*)&ms->PlayList[i];
						ms->PlayList[i] = (p[0] << 8) | p[1];
					}
				}
			}
			else error = DB3_ERROR_OUT_OF_MEMORY;
		}
		else error = DB3_ERROR_OUT_OF_MEMORY;
	}

	return error;
}



static int read_chunk_song(struct DB3Module *m, struct DataChunk *dc, struct AbstractHandle *ah)
{
	int error = 0;
	int songnum;

	for (songnum = 0; !error && (songnum < m->NumSongs); songnum++)
	{
		struct DB3ModSong *ms;

		if (ms = db3_malloc(sizeof(struct DB3ModSong)))
		{
			error = read_song(dc, ms, ah);
			m->Songs[songnum] = ms;
		}
		else error = DB3_ERROR_OUT_OF_MEMORY;
	}

	return error;
}



static int read_chunk_info(struct DB3Module *m, struct DataChunk *dc, struct AbstractHandle *ah)
{
	uint8_t b[10];
	int error = 0;

	if ((error = read_data(dc, ah, b, 10)) == 0)
	{
		m->NumInstr = (b[0] << 8) | b[1];
		m->NumSamples = (b[2] << 8) | b[3];
		m->NumSongs = (b[4] << 8) | b[5];
		m->NumPatterns = (b[6] << 8) | b[7];
		m->NumTracks = (b[8] << 8) | b[9];

		if ((m->NumInstr == 0) || (m->NumInstr > 255)) error = DB3_ERROR_DATA_CORRUPTED;
		if ((m->NumSamples == 0) || (m->NumSamples > 255)) error = DB3_ERROR_DATA_CORRUPTED;
		if ((m->NumTracks == 0) || (m->NumTracks > 254) || (m->NumTracks & 1)) error = DB3_ERROR_DATA_CORRUPTED;
		if ((m->NumSongs == 0) || (m->NumSongs > 255)) error = DB3_ERROR_DATA_CORRUPTED;
		if (m->NumPatterns == 0) error = DB3_ERROR_DATA_CORRUPTED;

		if (!(m->Instruments = db3_malloc(m->NumInstr * sizeof(void*)))) error = DB3_ERROR_OUT_OF_MEMORY;
		if (!(m->Samples = db3_malloc((m->NumSamples + 1) * sizeof(void*)))) error = DB3_ERROR_OUT_OF_MEMORY;
		if (!(m->Songs = db3_malloc(m->NumSongs * sizeof(void*)))) error = DB3_ERROR_OUT_OF_MEMORY;
		if (!(m->Patterns = db3_malloc(m->NumPatterns * sizeof(void*)))) error = DB3_ERROR_OUT_OF_MEMORY;
		if (!(m->DspDefaults.EffectMask = db3_malloc(m->NumTracks * sizeof(uint32_t)))) error = DB3_ERROR_OUT_OF_MEMORY;

		m->DspDefaults.EchoDelay = 0x40;
		m->DspDefaults.EchoFeedback = 0x80;
		m->DspDefaults.EchoMix = 0x80;
		m->DspDefaults.EchoCross = 0xFF;
	}

	return error;
}



static int read_chunk_name(struct DB3Module *m, struct DataChunk *dc, struct AbstractHandle *ah)
{
	char name[48];
	int error = 0, i, namelen = 0;

	for (i = 0; i < 48; i++) name[i] = 0;

	if ((error = read_data(dc, ah, name, 44)) == 0)
	{
		namelen = db3_strlen(name) + 1;

		if (m->Name = db3_malloc(namelen))
		{
			db3_strcpy(m->Name, name);
		}
		else error = DB3_ERROR_OUT_OF_MEMORY;
	}

	return error;
}



static int skip_to_chunk_end(struct AbstractHandle *ah, struct DataChunk *dc)
{
	char b[512];
	int error = 0, block;

	while (!error && (dc->Pos < dc->Size))
	{
		block = dc->Size - dc->Pos;
		if (block > 512) block = 512;
		if (!ah->ah_Read(ah, b, block)) error = DB3_ERROR_READING_DATA;
		dc->Pos += block;
	}

	return error;
}



static int strequ(char *str1, char *str2, int count)
{
	while (count--) if (*str1++ != *str2++) return 0;
	return 1;
}



static int read_contents(struct DB3Module *m, struct AbstractHandle *ah)
{
	int have_info = 0;
	int error = 0;
	uint8_t h[8];                 // chunk header, contains id, then length, length is big endian
	struct DataChunk dc;

	while (!error && (ah->ah_Read(ah, h, 8) == 1))
	{
		dc.Size = (h[4] << 24) | (h[5] << 16) | (h[6] << 8) | h[7];
		dc.Pos = 0;

		if (strequ((char*)h, "NAME", 4))
		{
			error = read_chunk_name(m, &dc, ah);
		}
		else if (strequ((char*)h, "INFO", 4))
		{
			if ((error = read_chunk_info(m, &dc, ah)) == 0) have_info = 1;
		}
		else if (strequ((char*)h, "SONG", 4))
		{
			if (have_info) error = read_chunk_song(m, &dc, ah);
			else error = DB3_ERROR_WRONG_CHUNK_ORDER;
		}
		else if (strequ((char*)h, "INST", 4))
		{
			if (have_info) error = read_chunk_inst(m, &dc, ah);
			else error = DB3_ERROR_WRONG_CHUNK_ORDER;
		}
		else if (strequ((char*)h, "PATT", 4))
		{
			if (have_info) error = read_chunk_patt(m, &dc, ah);
			else error = DB3_ERROR_WRONG_CHUNK_ORDER;
		}
		else if (strequ((char*)h, "SMPL", 4))
		{
			if (have_info) error = read_chunk_smpl(m, &dc, ah);
			else error = DB3_ERROR_WRONG_CHUNK_ORDER;
		}
		else if (strequ((char*)h, "VENV", 4))
		{
			if (have_info) error = read_chunk_venv(m, &dc, ah);
			else error = DB3_ERROR_WRONG_CHUNK_ORDER;
		}
		else if (strequ((char*)h, "PENV", 4))
		{
			if (have_info) error = read_chunk_penv(m, &dc, ah);
			else error = DB3_ERROR_WRONG_CHUNK_ORDER;
		}
		else if (strequ((char*)h, "DSPE", 4))
		{
			if (have_info) error = read_chunk_dspe(m, &dc, ah);
			else error = DB3_ERROR_WRONG_CHUNK_ORDER;
		}

		if (!error) error = skip_to_chunk_end(ah, &dc);
	}

	if (errno) error = DB3_ERROR_READING_DATA;
	return error;
}



static int read_header(struct DB3Module *m, struct AbstractHandle *ah)
{
	uint8_t h[8];
	int err = 0;

	if (ah->ah_Read(ah, h, 8) == 1)
	{
		if ((h[0] == 'D') && (h[1] == 'B') && (h[2] == 'M') && (h[3] == '0'))
		{
			m->CreatorRev = db3_bcd2bin(h[5]);
			if (h[4] == 2) m->CreatorVer = CREATOR_DIGIBOOSTER_2;
			else if (h[4] == 3) m->CreatorVer = CREATOR_DIGIBOOSTER_3;
			else err = DB3_ERROR_VERSION_UNSUPPORTED;
		}
		else err = DB3_ERROR_DATA_CORRUPTED;
	}
	else err = DB3_ERROR_READING_DATA;

	return err;
}



/****** libdigibooster3/DB3_Unload ******************************************
*
* NAME
*   DB3_Unload -- unloads DigiBooster 3 module from memory.
*
* SYNOPSIS
*   void DB3_Unload(struct DB3Module *m);
*
* FUNCTION
*   Properly unloads DigiBooster 3 module loaded with DB3_Load().
*
* INPUTS
*   m - pointer to the module returned by DB3_Load(). NULL is valid input,
*     function is a no-op in this case.
*
* RESULT
*   None.
*
* SEE ALSO
*   DB3_Load
*
*****************************************************************************
*
*/

void DB3_Unload(struct DB3Module *m)
{
	if (m)
	{
		int i;

		if (m->VolEnvs) db3_free(m->VolEnvs);
		if (m->PanEnvs) db3_free(m->PanEnvs);

		for (i = 0; i < m->NumPatterns; i++)
		{
			if (m->Patterns[i])
			{
				if (m->Patterns[i]->Pattern) db3_free(m->Patterns[i]->Pattern);
				db3_free(m->Patterns[i]);
			}
		}

		for (i = 0; i < m->NumSamples; i++)
		{
			if (m->Samples[i])
			{
				if (m->Samples[i]->Data) db3_free(m->Samples[i]->Data);
				db3_free(m->Samples[i]);
			}
		}

		for (i = 0; i < m->NumInstr; i++)
		{
			if (m->Instruments[i])
			{
				if (m->Instruments[i]->Name) db3_free(m->Instruments[i]->Name);
				db3_free(m->Instruments[i]);
			}
		}

		for (i = 0; i < m->NumSongs; i++)
		{
			if (m->Songs[i])
			{
				if (m->Songs[i]->Name) db3_free(m->Songs[i]->Name);
				if (m->Songs[i]->PlayList) db3_free(m->Songs[i]->PlayList);
				db3_free(m->Songs[i]);
			}
		}

		if (m->Instruments) db3_free(m->Instruments);
		if (m->Samples) db3_free(m->Samples);
		if (m->Songs) db3_free(m->Songs);
		if (m->Patterns) db3_free(m->Patterns);
		if (m->DspDefaults.EffectMask) db3_free(m->DspDefaults.EffectMask);
		if (m->Name) db3_free(m->Name);
		db3_free(m);
	}
}


/****** libdigibooster3/DB3_LoadFromHandle **********************************
*
* NAME
*   DBM0_Load() -- loads a DigiBooster3 module from abstract handle
*
* SYNOPSIS
*   struct DB3Module* DB3_LoadFromHandle(struct AbstractHandle *handle,
*   int *errptr);
*
* FUNCTION
*   Loads and parses a DigiBooster 3 (DBM0 type) module from an abstract
*   stream handle. The handle is defined by an abstract pointer to data
*   stream descriptor and a callback function pointer reading the stream.
*   It is assumed that opening and closing this abstract stream is done by
*   application.
*
*   Before calling the function, structure AbstractHandle should be
*   initialized. ah_Handle should contain stream handle used inside the
*   read callback. ah_Read is a pointer to the callback. The callback is
*   defined as follows:
*
*   int read_callback(struct AbstractHandle*, void*, int);
*
*   Arguments are: pointer to AbstractHandle, pointer to read buffer, and
*   number of bytes to load. It should return 1 if it succesfully reads
*   required number of bytes, 0 otherwise.
*
* INPUTS
*   handle - pointer to initialized AbstractHandle structure.
*   errptr - optional pointer to a variable where error code will be stored.
*     Note that the variable is not cleared if the call succeeds.
*
* RESULT
*   Module structure as defined in "musicmodule.h" with complete set of
*   patterns, songs, samples and instruments. Samples are converted to 16
*   bits. In case of failure, function returns NULL. May be caused by
*   NULL filename, corrupted module data, out of memory.
*
* SEE ALSO
*   DB3_Load
*
*****************************************************************************
*
*/

struct DB3Module *DB3_LoadFromHandle(struct AbstractHandle *ah, int *errptr)
{
	struct DB3Module *m = NULL;
	int err = 0;

	if (m = db3_malloc(sizeof(struct DB3Module)))
	{
		if (!(err = read_header(m, ah)))
		{
			if (!(err = read_contents(m, ah)))
			{
				if (!(verify_contents(m))) err = DB3_ERROR_DATA_CORRUPTED;
			}
		}
	}
	else err = DB3_ERROR_OUT_OF_MEMORY;

	if (err)
	{
		if (m)
		{
			DB3_Unload(m);
			m = NULL;
		}
	}

	if (!m)
	{
		if (errptr) *errptr = err;
	}

	return m;
}


/****** libdigibooster3/DB3_Load ********************************************
*
* NAME
*   DBM0_Load -- loads a DigiBooster3 module from a file.
*
* SYNOPSIS
*   struct DB3Module* DB3_Load(char *filename, int *errptr);
*
* FUNCTION
*   Loads and parses a DigiBooster 3 (DBM0 type) module stored as a file.
*
* INPUTS
*   filename - path to a file
*   errptr - optional pointer to a variable where error code will be stored.
*     Note that the variable is not cleared if the call succeeds.
*
* RESULT
*   Module structure as defined in "musicmodule.h" with complete set of
*   patterns, songs, samples and instruments. Samples are converted to 16
*   bits. In case of failure, function returns NULL. May be caused by
*   NULL filename, corrupted module data, out of memory.
*
* SEE ALSO
*   DB3_LoadFromHandle
*
*****************************************************************************
*
*/


static int file_read(struct AbstractHandle *ah, void *buf, int bytes)
{
	int k;

	k = db3_fread(buf, bytes, 1, (BPTR)ah->ah_Handle);
	return k ? 1 : 0;
}


struct DB3Module *DB3_Load(char *filename, int *errptr)
{
	struct DB3Module *m = NULL;
	struct AbstractHandle ah;

	ah.ah_Read = file_read;

	if (ah.ah_Handle = (void*)db3_fopen(filename))
	{
		m = DB3_LoadFromHandle(&ah, errptr);
		db3_fclose((BPTR)ah.ah_Handle);
	}
	else *errptr = DB3_ERROR_FILE_OPEN;

	return m;
}
