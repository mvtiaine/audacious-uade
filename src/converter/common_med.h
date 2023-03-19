// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#ifndef COMMON_MED_H_
#define COMMON_MED_H_

#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include "big_endian.h"
#include "../3rdparty/SimpleBinStream.h"

namespace converter::med {

typedef be_int32_t      LONG;           /* signed 32-bit quantity */
typedef be_uint32_t     ULONG;          /* unsigned 32-bit quantity */
typedef be_int16_t      WORD;           /* signed 16-bit quantity */
typedef be_uint16_t     UWORD;          /* unsigned 16-bit quantity */
typedef int8_t          BYTE;           /* signed 8-bit quantity */
typedef uint8_t         UBYTE;          /* unsigned 8-bit quantity */
typedef char *          STRPTR;         /* string pointer (NUL-terminated) */
#define EXEC_TYPES_H
#include "../3rdparty/proplayer.h"
#undef EXEC_TYPES_H

constexpr ULONG MMD0ID = 0x4d4d4430; // "MMD0"
constexpr int MAX_SAMPLES = 63;

struct MMD0Block {
    UBYTE numtracks;
    UBYTE lines;
    vector<char> data; // size = 3 * (lines + 1) * trks
};

struct MMDSample0 : MMDSample {
    vector<char> sample; // size = length
};

struct SynthWF {
    UWORD length;   /* length in words */
    vector<char> wfdata; /* the waveform */
};

struct SynthWF0 {
    optional<SynthWF> synthwf;
    optional<MMDSample0> sample; // wf[0] when type == HYBRID
};

struct SynthInstr {
    ULONG   length;     /* length of this struct */
    WORD    type;       /* -1 or -2 (offs: 4) */
    UBYTE   defaultdecay;
    vector<char> reserved{0,0,0};
    UWORD   rep;
    UWORD   replen;
    UWORD   voltbllen;  /* offs: 14 */
    UWORD   wftbllen;   /* offs: 16 */
    UBYTE   volspeed;   /* offs: 18 */
    UBYTE   wfspeed;    /* offs: 19 */
    UWORD   wforms;     /* offs: 20 */
    vector<char> voltbl; /* offs: 22 */
    vector<char> wftbl; /* offs: 150 */
    vector<SynthWF0> wf; /* offs: 278 */
    SynthInstr() : voltbllen(128), wftbllen(128), voltbl(128), wftbl(128) {};
};

struct Instr {
    optional<MMDSample0> sample;
    optional<SynthInstr> synthinstr;
};

struct MMD0exp0 : MMD0exp {
    vector<char> annotxt;
    vector<vector<char>> iinfo; // sample names
};

UBYTE GetNibble(UBYTE *mem, UWORD *nbnum);
UWORD GetNibbles(UBYTE *mem, UWORD *nbnum, UBYTE nbs);
void UnpackData(ULONG *lmptr, ULONG *cmptr, UBYTE *from, UBYTE *to, UWORD lines, UBYTE trkn);

vector<char> serializeMMD0(
    const MMD0 &mmd0,
    const MMD0song &song,
    const vector<MMD0Block> &blockarr,
    const vector<Instr> &smplarr,
    const MMD0exp0 &exp,
    const vector<InstrExt> &exp_smp
);

} // namespace converter::med

#endif // COMMON_MED_H_