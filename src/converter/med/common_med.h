// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <optional>
#include <string>
#include <vector>
#include "common/big_endian.h"

namespace converter::med {

typedef common::be_int32_t      LONG;           /* signed 32-bit quantity */
typedef common::be_uint32_t     ULONG;          /* unsigned 32-bit quantity */
typedef common::be_int16_t      WORD;           /* signed 16-bit quantity */
typedef common::be_uint16_t     UWORD;          /* unsigned 16-bit quantity */
typedef int8_t          BYTE;           /* signed 8-bit quantity */
typedef uint8_t         UBYTE;          /* unsigned 8-bit quantity */
typedef char *          STRPTR;         /* string pointer (NUL-terminated) */
// XXX needed to fix Windows typedef conflict
#define LONG converter::med::LONG
#define ULONG converter::med::ULONG
#define WORD converter::med::WORD
#define UWORD converter::med::UWORD
#define BYTE converter::med::BYTE
#define UBYTE converter::med::UBYTE
#define STRPTR converter::med::STRPTR
#define EXEC_TYPES_H
#include "3rdparty/proplayer.h"
#undef EXEC_TYPES_H

const ULONG MMD0ID = 0x4d4d4430; // "MMD0"
constexpr int MAX_SAMPLES = 63;

struct MMD0Block {
    UBYTE numtracks;
    UBYTE lines;
    std::vector<char> data; // size = 3 * (lines + 1) * trks
};

struct MMDSample0 : MMDSample {
    std::vector<char> sample; // size = length
};

struct SynthWF {
    UWORD length;   /* length in words */
    std::vector<char> wfdata; /* the waveform */
};

struct SynthWF0 {
    std::optional<SynthWF> synthwf;
    std::optional<MMDSample0> sample; // wf[0] when type == HYBRID
};

struct SynthInstr {
    ULONG   length;     /* length of this struct */
    WORD    type;       /* -1 or -2 (offs: 4) */
    UBYTE   defaultdecay;
    std::vector<char> reserved{0,0,0};
    UWORD   rep;
    UWORD   replen;
    UWORD   voltbllen;  /* offs: 14 */
    UWORD   wftbllen;   /* offs: 16 */
    UBYTE   volspeed;   /* offs: 18 */
    UBYTE   wfspeed;    /* offs: 19 */
    UWORD   wforms;     /* offs: 20 */
    std::vector<char> voltbl; /* offs: 22 */
    std::vector<char> wftbl; /* offs: 150 */
    std::vector<SynthWF0> wf; /* offs: 278 */
    SynthInstr() : voltbllen(128), wftbllen(128), voltbl(128), wftbl(128) {};
};

struct Instr {
    std::optional<MMDSample0> sample;
    std::optional<SynthInstr> synthinstr;
};

struct MMD0exp0 : MMD0exp {
    std::vector<char> annotxt;
    std::vector<std::vector<char>> iinfo; // sample names
};

UBYTE GetNibble(UBYTE *mem, UWORD *nbnum);
UWORD GetNibbles(UBYTE *mem, UWORD *nbnum, UBYTE nbs);
void UnpackData(ULONG *lmptr, ULONG *cmptr, UBYTE *from, UBYTE *to, UWORD lines, UBYTE trkn);

std::vector<char> serializeMMD0(
    const MMD0 &mmd0,
    const MMD0song &song,
    const std::vector<MMD0Block> &blockarr,
    const std::vector<Instr> &smplarr,
    const MMD0exp0 &exp,
    const std::vector<InstrExt> &exp_smp
);

} // namespace converter::med
