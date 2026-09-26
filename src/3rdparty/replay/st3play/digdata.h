#pragma once

#include <stdint.h>
#include <stdbool.h>

#define C2FREQ 8363

// total channels
// 16 DA
// 9 LAdlib + 5 LAdlibDrums + 2 unused
// 9 RAdlib + 5 RAdlibDrums + 2 unused
#define ACHANNELS 48

// 8bb: custom defines

#define MAX_ORDERS 256
#define MAX_INSTRUMENTS 99
#define MAX_PATTERNS 100

enum
{
	SOUNDCARD_GUS = 0,
	SOUNDCARD_SBPRO = 1,
};
// ---------------------------------

typedef struct zchn_t
{
	int8_t aorgvol, avol;
	bool atreon;
	uint8_t channelnum, achannelused, aglis, atremor, atrigcnt, anotecutcnt, anotedelaycnt, a0volcut;
	uint8_t avibtretype, note, ins, vol, cmd, info, lastins, lastnote, alastnfo, alasteff, alasteff1;
	int16_t avibcnt, asldspd, aspd, aorgspd;
	uint16_t astartoffset, astartoffset00, ac2spd;

	// 8bb: for AdLib
	uint8_t lastadlins, addherzretrig, addherzretrigvol;
	uint16_t addherzlo, addherzhi;

	// 8bb: for mixer and GUS
	int8_t amixtype, aguschannel;
	uint8_t apanpos;
	int8_t *m_base;
	uint8_t m_vol, m_oldvol;
	uint32_t m_pos, m_poslow, m_oldpos, m_end, m_loop, m_speed;
} zchn_t;

#ifdef _MSC_VER
#pragma pack(push)
#pragma pack(1)
#endif

typedef struct ds_fileheader
{
	char name[28];
	uint8_t _magic_pinit; // 1ah (or 0h in mem when initialized) (8bb: not used in st3play)
	uint8_t type; // 16=module, 17=song
	uint8_t _reserved1[2];
	uint16_t ordnum;
	uint16_t insnum;
	uint16_t patnum;
	uint16_t flags;
	uint16_t cwtv; // created with tracker version
	uint16_t ffv; // file format version
	char _magic_signature[4]; // SCRM
	uint8_t globalvol;
	uint8_t initspeed;
	uint8_t inittempo;
	uint8_t mastermul;
	uint8_t ultraclick; // 8bb: number of GUS voices to allocate (16, 24, 32). Used for anti-click (volume ramping)
	uint8_t defaultpan252;
	uint8_t _reserved[10];
	uint8_t channel[32]; // channel types
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
ds_fileheader;

typedef struct ds_smp // instrument block, sample
{
	uint8_t type; // 8bb: 1=sample, 2=AdLib memory, 3=AdLib drum, ... (see ds_adl struct below)
	char filename[12];
	uint8_t memseg2;
	uint16_t memseg;
	uint32_t length;
	uint32_t lbeg;
	uint32_t lend;
	uint8_t vol;
	uint8_t disk; // 8bb: not used in st3play
	uint8_t pack; // 0=normal(disk), 1=amiga(module), 2=ADPCM (8bb: not used in st3play)
	uint8_t flags; // +1=loop
	uint32_t c2spd; // 8bb: actually c4spd
	uint8_t _reserved2[4];
	uint16_t guspos; // 8bb: address of sample in GUS memory /32 (8bb: not used in st3play)
	uint16_t lend512; // 8bb: unrolled loop end (up to 512 bytes extra)
	uint32_t lastused; // in ilib, used as time/datestamp (8bb: not used in st3play)
	char name[28];
	char _magic[4]; // "SCRS"

	int8_t *baseptr; // 8bb: added this since we don't work with 16-bit segments
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
ds_smp;

typedef struct ds_adl // instrument block, AdLib (8bb: couldn't find definition for this, crafted from ST3 docs)
{
	uint8_t type; // 8bb: 2=melody, 3=bassdrum, 4=snare, 5=tom, 6=cymbal, 7=hihat
	char filename[12];
	uint8_t _reserved1[3];
	uint8_t D00;
	uint8_t D01;
	uint8_t D02;
	uint8_t D03;
	uint8_t D04;
	uint8_t D05;
	uint8_t D06;
	uint8_t D07;
	uint8_t D08;
	uint8_t D09;
	uint8_t D0A;
	uint8_t D0B;
	uint8_t vol;
	uint8_t disk; // 8bb: not used in st3play
	uint8_t _reserved2[2];
	uint32_t c2spd; // 8bb: actually c4spd
	uint8_t _reserved3[12];
	char name[28];
	char _magic[4]; // "SCRI"
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
ds_adl;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

// 8bb: custom structs for convenience

typedef struct song_t
{
	ds_fileheader header;
	uint8_t order[MAX_ORDERS+1], *patp[MAX_PATTERNS+1];
	ds_smp ins[MAX_INSTRUMENTS+1];
	zchn_t _zchn[ACHANNELS];

	bool oldstvib, fastvolslide, amigalimits, stereomode, adlibused;
	uint8_t *np_patseg;
	int16_t np_patoff, aspdmin, aspdmax, np_ord, np_row, np_pat, globalvol;
	uint16_t masterflags;
	int8_t patterndelay, patloopcount, lastachannelused;
	uint8_t musicmax, breakpat, startrow, musiccount;
	int16_t jmptoord, patloopstart, jumptorow;
	uint16_t useglobalvol, patmusicrand;

	uint8_t defaultpan[32]; // 8bb: GUS initial channel pans (ST3.20 & ST3.21)

	uint8_t KxyLxxVolslideType; // 8bb: added this, temporary variable used by Kxy/Lxx (instead of bp register)
	volatile bool moduleLoaded; // 8bb: added this
	
} song_t;

typedef struct audio_t
{
	volatile bool playing;
	int32_t soundcardtype;
	int8_t mastermul; // 8bb: used for SB mixer
	uint16_t notemixingspeed; // 8bb: ST3 SB/GUS mixing frequency
	uint32_t outputFreq; // 8bb: actual audio output speed
	uint32_t tickSampleCounter, samplesPerTickInt, bpm2SamplesPerTickInt[256], bpm2SamplesPerTickFrac[256];
	uint64_t tickSampleCounterFrac, samplesPerTickFrac;
	float *fMixBufferL, *fMixBufferR, fMixingVol;
} audio_t;

// ------------------------------------------------------------

// 8bb: customized data (different than ST3)
extern audio_t audio;
extern song_t song;
// -----------------------------------------

extern const int8_t retrigvoladd[32];
extern const uint8_t octavediv[8+8];
extern const uint16_t xfinetune_amiga[16];
extern const int16_t notespd[12+1+3];
extern const int16_t vibsin[64];
extern const uint8_t vibsqu[64];
extern const int16_t vibramp[64];
