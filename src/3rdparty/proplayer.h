// SPDX-License-Identifier: LicenseRef-OctaMED-Soundstudio-Player
// https://aminet.net/package/dev/src/OcSS_src
/* C definitions for using the 'proplayer.a' and 'pro8player.a'
   play-routines of OctaMED (Pro/Ss). OctaMED Pro V5 specific
   features are marked as (V5), V6 features as (V6) and Sound-
   studio specific features as (V7)
*/

/* $VER: proplayer_h 7.0 (29.1.1996) */

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifdef LATTICE
#ifndef	OCTAPLR_LIB_PROTOS
/* In 'proplayer.a' */
LONG __asm InitPlayer(void);
void __asm RemPlayer(void);
void __asm PlayModule(register __a0 struct MMD0 *);
void __asm ContModule(register __a0 struct MMD0 *);
void __asm StopPlayer(void);
void __asm SetTempo(register __d0 UWORD);
/* In 'loadmod.a' */
struct MMD0 * __asm LoadModule(register __a0 char *);
struct MMD0 * __asm LoadModule_Fast(register __a0 char *);
void __asm RelocModule(register __a0 struct MMD0 *);
void __asm UnLoadModule(register __a0 struct MMD0 *);
ULONG __asm RequiredPlayRoutine(register __a0 struct MMD0 *);
ULONG __asm FastMemPlayRecommended(register __a0 struct MMD0 *);
/* These are the definitions for the 8-channel OctaMED routines,
   in 'pro8player.a' */
LONG __asm InitPlayer8(void);
void __asm RemPlayer8(void);
void __asm PlayModule8(register __a0 struct MMD0 *);
void __asm ContModule8(register __a0 struct MMD0 *);
void __asm StopPlayer8(void);
/* And the corresponding routines in 'promixplayer.a'... */
LONG __asm InitPlayerM(void);
void __asm RemPlayerM(void);
void __asm PlayModuleM(register __a0 struct MMD0 *);
void __asm ContModuleM(register __a0 struct MMD0 *);
void __asm StopPlayerM(void);
#endif
#endif

/* If you're playing multi-modules, set the 'modnum' variable to the
   number of the song you want to play before calling PlayModule(). */

#ifdef LATTICE_50
extern UWORD far modnum;

/* 'modnum8' is the equivalent in 'mod8player' */

extern UWORD far modnum8;
extern UBYTE far hq; /* (V6) */
extern ULONG far mixbuffsize,mixfreq; /* (V7, mix) */
extern UBYTE far mix14bit; /* (V7, mix) */
extern UWORD far fastmemplay,fastmemplay8; /* (V7) */
#else
extern UWORD modnum,modnum8; /* for less intelligent compilers */
extern UBYTE hq;
extern ULONG mixbuffsize,mixfreq;
extern UBYTE mix14bit;
extern UWORD fastmemplay,fastmemplay8;
#endif

/* This is the main module structure */
struct MMD0 {	/* Also for MMD1 and MMD2 */
	ULONG	id;			/* "MMD0"/"MMD1"/"MMD2" */
	ULONG	modlen;			/* module length (in bytes) */
	struct	MMD0song *song;		/* pointer to MMD0song */
	UWORD	psecnum;		/* (MMD2) - used by the player */
	UWORD	pseq;			/* (MMD2) - used by the player */
	struct	MMD0block **blockarr;	/* pointer to pointers of blocks */
	UBYTE	mmdflags;		/* see below... */
	UBYTE	reserved[3];
	struct	MMDSample **smplarr;	/* pointer to pointers of samples */
	ULONG	reserved2;
	struct	MMD0exp *expdata;	/* pointer to expansion data */
	ULONG	reserved3;
/* The following values are used by the play routine */
	UWORD	pstate;			/* the state of the player */
	UWORD	pblock;			/* current block */
	UWORD	pline;			/* current line */
	UWORD	pseqnum;		/* current # of playseqlist */
	WORD	actplayline;		/* OBSOLETE!! SET TO 0xFFFF! */
	UBYTE	counter;		/* delay between notes */
	UBYTE	extra_songs;		/* number of additional songs, see
					   expdata->nextmod */
};

/* MMD0.mmdflags bit definitions: */
/* this mod can always be loaded to fast memory (either uses the mixing
   routine or is a pure MIDI or 8-channel song) */
#define	MMD_LOADTOFASTMEM	0x1


/* These are the structures for future expansions */

struct InstrExt {	/* This struct only for data required for playing */
/* NOTE: THIS STRUCTURE MAY GROW IN THE FUTURE, TO GET THE CORRECT SIZE,
   EXAMINE mmd0->expdata->s_ext_entrsz */
/* ALSO NOTE: THIS STRUCTURE MAY BE SHORTER THAN DESCRIBED HERE,
   EXAMINE mmd0->expdata->s_ext_entrsz */
	UBYTE hold;
	UBYTE decay;
	UBYTE suppress_midi_off;	/* 1 = suppress, 0 = don't */
	BYTE  finetune;
	UBYTE default_pitch;	/* (V5) */
	UBYTE instr_flags;	/* (V5) */
	UWORD long_midi_preset;	/* (V5), overrides the preset in the
		song structure, if this exists, MMD0sample/midipreset
		should not be used. */
	UBYTE output_device;	/* (V5.02, V6) */
	UBYTE reserved;		/* currently unused */
/* The following two variables override MMD0sample repeat settings. First,
   they allow >128 k samples; second, they allow byte resolution */
	ULONG long_repeat;	/* (V7) */
	ULONG long_replen;	/* (V7) */
};

/* Bits for instr_flags */
#define SSFLG_LOOP	0x01
#define	SSFLG_EXTPSET	0x02
#define	SSFLG_DISABLED	0x04
#define	SSFLG_PINGPONG	0X08	/* ping-pong loop [mixing only] */

/* Currently defined output_device values */
#define	OUTPUT_STD	0
#define	OUTPUT_MD16	1	/* Aura */
#define	OUTPUT_TOCC	2

struct MMDInstrInfo {
	UBYTE	name[40];
	UBYTE	pad0;	/* two pads? */
	UBYTE	pad1;
};

struct MMD0exp {
	struct MMD0 *nextmod;		/* for multi-modules */
	struct InstrExt *exp_smp;	/* pointer to an array of InstrExts */
	UWORD  s_ext_entries;		/* #�of InstrExts in the array */
	UWORD  s_ext_entrsz;		/* size of an InstrExt structure */
	UBYTE  *annotxt;		/* 0-terminated message string */
	ULONG  annolen;			/* length (including the 0-byte) */
/* MED V3.20 data below... */
	struct MMDInstrInfo *iinfo;	/* "secondary" InstrExt for info
					   that does not affect output */
	UWORD  i_ext_entries;		/* # of MMDInstrInfos */
	UWORD  i_ext_entrsz;		/* size of one */
	ULONG  jumpmask;		/* OBSOLETE in current OctaMEDs */
	UWORD  *rgbtable;		/* pointer to 8 UWORD values,
					   ignored by OctaMED V5 and later */
	UBYTE  channelsplit[4];	/* for OctaMED only (non-zero = NOT splitted) */
	struct NotationInfo *n_info;	/* OctaMED notation editor info data */
	UBYTE  *songname;	/* song name */
	ULONG  songnamelen;	/* length (including terminating zero) */
	struct MMDDumpData *dumps; /* MIDI message dump data */
	struct MMDInfo *mmdinfo;   /* (V6) annotation information */
	struct MMDARexx *mmdrexx;  /* (V7) ARexx information */
	struct MMDMIDICmd3x *mmdcmd3x; /* (V7) Command 3cxx settings */
/* These are still left, they must be 0 at the moment. */
	ULONG  reserved2[3];
/* When the above three reserved fields are all used, the following
   information will be represented as a tag list. Set this to zero
   [TAG_END]. */
	ULONG  tag_end;
};

/* Info for each instrument (mmd0->song.sample[xx]) */

struct MMD0sample {
	UWORD rep,replen;	/* repeat/repeat length */
	UBYTE midich;		/* midi channel for curr. instrument */
	UBYTE midipreset;	/* midi preset (1 - 128), 0 = no preset */
	UBYTE svol;		/* default volume */
	BYTE strans;		/* sample transpose */
};

/* The song structure (mmd0->song) */

struct MMD0song {
	struct MMD0sample sample[63];	/* info for each instrument */
	UWORD	numblocks;		/* number of blocks in this song */
	UWORD	songlen;		/* number of playseq entries */
	UBYTE	playseq[256];		/* the playseq list */
	UWORD	deftempo;		/* default tempo */
	BYTE	playtransp;		/* play transpose */
	UBYTE	flags;			/* flags (see below) */
	UBYTE	flags2;			/* for future expansion */
	UBYTE	tempo2;			/* 2ndary tempo (delay betw. notes) */
	UBYTE	trkvol[16];		/* track volume */
	UBYTE	mastervol;		/* master volume */
	UBYTE	numsamples;		/* number of instruments */
}; /* length = 788 bytes */

/* The new PlaySeq structure of MMD2 */

struct PlaySeq {
	char	name[32];	/* (0)  31 chars + \0 */
	ULONG	reserved[2];	/* (32) for possible extensions */
	UWORD	length;		/* (40) # of entries */
/* Commented out, not all compilers may like it... */
/*	UWORD	seq[0];	*/	/* (42) block numbers.. */
/* Note: seq[] values above 0x7FFF are reserved for future expansion! */
};

/* This structure is used in MMD2s, instead of the above one.
   (Be sure to cast the pointer.) */

struct MMD2song {
	struct MMD0sample sample[63];
	UWORD	numblocks;
	UWORD	songlen;	/* NOTE: number of sections in MMD2 */
	struct	PlaySeq **playseqtable;
	UWORD	*sectiontable;	/* UWORD section numbers */
	UBYTE	*trackvols;	/* UBYTE track volumes */
	UWORD	numtracks;	/* max. number of tracks in the song
				   (also the number of entries in
				    'trackvols' table) */
	UWORD	numpseqs;	/* number of PlaySeqs in 'playseqtable' */
	BYTE	*trackpans;	/* NULL means 'all centered */
	ULONG	flags3;		/* see defs below */
	UWORD	voladj;		/* volume adjust (%), 0 means 100 */
	UWORD	channels;	/* mixing channels, 0 means 4 */
	UBYTE	mix_echotype;	/* 0 = nothing, 1 = normal, 2 = cross */
	UBYTE	mix_echodepth;	/* 1 - 6, 0 = default */
	UWORD	mix_echolen;	/* echo length in milliseconds */
	BYTE	mix_stereosep;	/* stereo separation */
	UBYTE	pad0[223];	/* reserved for future expansion */
/* Below fields are MMD0/MMD1-compatible (except pad1[]) */
	UWORD	deftempo;
	BYTE	playtransp;
	UBYTE	flags;
	UBYTE	flags2;
	UBYTE	tempo2;
	UBYTE	pad1[16];	/* used to be trackvols, in MMD2 reserved */
	UBYTE	mastervol;
	UBYTE	numsamples;
};

 /* FLAGS of the above structure */
#define	FLAG_FILTERON	0x1	/* hardware low-pass filter */
#define	FLAG_JUMPINGON	0x2	/* OBSOLETE now, but retained for compatibility */
#define	FLAG_JUMP8TH	0x4	/* also OBSOLETE */
#define	FLAG_INSTRSATT	0x8	/* instruments are attached (sng+samples)
				   used only in saved MED-songs */
#define	FLAG_VOLHEX	0x10	/* volumes are represented as hex */
#define FLAG_STSLIDE	0x20	/* no effects on 1st timing pulse (STS) */
#define FLAG_8CHANNEL	0x40	/* OctaMED 8 channel song, examine this bit
				   to find out which routine to use */
#define	FLAG_SLOWHQ	0x80	/* HQ slows playing speed (V2-V4 compatibility) */
/* flags2 */
#define FLAG2_BMASK	0x1F
#define FLAG2_BPM	0x20
#define	FLAG2_MIX	0x80	/* uses Mixing (V7+), this is IMPORTANT! */
/* flags3 */
#define	FLAG3_STEREO	0x1	/* mixing in Stereo mode */
#define	FLAG3_FREEPAN	0x2	/* free panning */

struct MMDDump {
	ULONG	length;		/* dump data length */
	UBYTE	*data;		/* data pointer */
	UWORD	ext_len;	/* bytes remaining in this struct */
/* ext_len >= 20: */
	UBYTE	name[20];	/* message name (null-terminated) */
};

struct MMDDumpData {
	UWORD	numdumps;	/* number of message dumps */
	UWORD	reserved[3];	/* not currently used */
};	// Followed by <numdumps> pointers to struct MMDDump

/* Designed so that several info items can exist (in V6 only one supported),
   you must also check the data type before using it, currently only text is
   supported, but more types can be added in the future.

   Text is stored in plain Amiga ASCII, lines separated by \n characters.
   The last byte is \0.
*/
struct MMDInfo {
	struct MMDInfo *next;	/* next info (currently not supported) */
	UWORD	reserved;	/* 0 */
	UWORD	type;		/* 1 = text, ignore ALL other types */
	ULONG	length;		/* length of the following data */
/*	UBYTE	data[0]; */	/* Comments may be removed in SAS/C V6 */
};

/* flags in struct NotationInfo */
#define NFLG_FLAT 1
#define NFLG_3_4  2

struct NotationInfo {
	UBYTE n_of_sharps;	/* number of #'s (or b's) */
	UBYTE flags;		/* flags (see above) */
	WORD  trksel[5];	/* selected track for each preset (-1 = none) */
	UBYTE trkshow[16];	/* which tracks to show (bit 0 = for preset 0,
				bit 1 for preset 1 and so on..) */
	UBYTE trkghost[16];	/* ghosted tracks (like trkshow[]) */
	BYTE  notetr[63];   	/* -24 - +24 (if bit #6 is negated, hidden) */
	UBYTE pad;	/* perhaps info about future extensions */
};

/* ctrlr_types */
#define	MCS_TYPE_STD_MSB	0
#define	MCS_TYPE_STD_LSB	1
#define	MCS_TYPE_RPN_MSB	2
#define	MCS_TYPE_RPN_LSB	3
#define	MCS_TYPE_NRPN_MSB	4
#define	MCS_TYPE_NRPN_LSB	5

struct MMDMIDICmd3x {
	UBYTE struct_vers;	// current version = 0
	UBYTE pad;
	UWORD num_of_settings;	// number of Cmd3x settings (currently set to 15)
	UBYTE *ctrlr_types;	// controller types [ignore unknown types!!]
	UWORD *ctrlr_numbers;	// controller numbers
};

struct MMDARexxTrigCmd {
	struct MMDARexxTrigCmd *next; /* the next command, or NULL */
	UBYTE cmdnum; /* command number (01..FF) */
	UBYTE pad;
	WORD cmdtype; /* command type (OMACTION_...) */
	STRPTR cmd; /* command, or NULL */
	STRPTR port; /* port, or NULL */
	UWORD cmd_len; /* length of 'cmd' string (without term. 0) */
	UWORD port_len; /* length of 'port' string (without term. 0) */
}; /* current (V7) structure size: 20 */

/* command type definitions... ignore unrecognized cmdtypes! */
#define	OMACTION_CMDSELF	0
#define	OMACTION_RUNREXXPRG	1
#define	OMACTION_CMDEXT		2
#define	OMACTION_RUNPRG		3

struct MMDARexx {
	UWORD	res;	/* reserved, must be zero! */
	UWORD	trigcmdlen; /* size of trigcmd entries (MUST be used!!) */
	struct	MMDARexxTrigCmd *trigcmd; /* a chain of MMDARexxTrigCmds, or NULL */
};

/* This structure exists in V6+ blocks with multiple command pages */
struct BlockCmdPageTable {
	UWORD	num_pages;	// number of command pages
	UWORD	reserved;	// zero = compatibility
	UWORD	*page[0];	// page pointers follow...
};

/* Below structs for MMD1 only! */
struct BlockInfo {
	ULONG	*hlmask;  	/* highlight data */
	UBYTE	*blockname;	/* block name */
	ULONG	blocknamelen;	/* length of block name (including term. 0) */
	struct	BlockCmdPageTable *pagetable;	/* (V6) command page table */
	ULONG	reserved[5];	/* future expansion */
};

struct MMD1Block {
	UWORD numtracks;
	UWORD lines;
	struct BlockInfo *info;
};
#define MMD1BLKHDRSZ 8

/* This header exists in the beginning of each sample */
struct MMDSample {
/* length of one channel in bytes */
	ULONG	length;
/* see definitions below */
	WORD	type;
/* 8- or 16-bit data follows */
};

/* Type definitions: */
#define	SAMPLE		0
#define	IFF5OCT		1
#define	IFF3OCT		2
#define IFF2OCT		3
#define IFF4OCT		4
#define IFF6OCT		5
#define IFF7OCT		6
/* low octaves usable */
#define	EXTSAMPLE	7
/* a standard synthsound */
#define	SYNTHETIC	-1
/* sample with synthetic information */
#define	HYBRID		-2
/* 16-bit (flag), only type SAMPLE supported */
#define	S_16		0x10
/* stereo (flag) */
#define	STEREO		0x20
/* only supported while reading... V5 Aura sample */
#define	OBSOLETE_MD16	0x18

/* Please refer to 'MMD.txt' for a complete description of MMD file format. */
