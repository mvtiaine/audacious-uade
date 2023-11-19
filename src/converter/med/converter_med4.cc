// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>
// 
// MED4 file format parsing based on sources included in MED 2.10 distribution, disassembled DeliTracker converter genie
// and comparing the MED V3.21 saved MED4 vs MMD0 files.

#include <cstdint>
#include <cstring>
#include <string>
#include <utility>
#include <vector>
#include "common/big_endian.h"
#include "converter/converter.h"
#include "common_med.h"

using namespace std;
using namespace converter;
using namespace converter::med;

namespace {

void readSong(const vector<char> &med4, MMD0song &song, MMD0exp0 &exp, size_t &offs) {
    ULONG samplemask[2], *smskptr = samplemask;
    UBYTE smskmsk, *smptr0 = (UBYTE *)samplemask;
    MMD0sample *ss;
    vector<vector<char>> samplenames;
    smskmsk = readu8be(med4, offs);
    for (int scnt = 0; scnt < 8; scnt++) {
        if (!(smskmsk & 0x80)) *(smptr0+scnt) = 0;
        else *(smptr0+scnt) = readu8be(med4, offs);
        smskmsk <<= 1;
    }
    for (int scnt = 0; scnt < MAX_SAMPLES; scnt++) {
        if (scnt == 32) smskptr++;
        ss = &song.sample[scnt];
        if (*smskptr & 0x80000000) {
            uint8_t scmask = readu8be(med4, offs) & 0x7f;
            uint8_t snamelen = readu8be(med4, offs) & 0x3f;
            if (scmask || snamelen) {
                if (snamelen) samplenames.push_back(readbytes(med4, offs, snamelen));
                else samplenames.push_back(vector<char>(0));
                if (scmask & 0x1) ss->rep = 0;
                else song.sample[scnt].rep = readu16be(med4, offs);
                if (scmask & 0x2) ss->replen = 0;
                else ss->replen = readu16be(med4, offs);
                if (scmask & 0x4) ss->midich = 0;
                else ss->midich = readu8be(med4, offs);
                if (scmask & 0x8) ss->midipreset = 0;
                else ss->midipreset = readu8be(med4, offs);
                if (scmask & 0x10) ss->svol = 0;
                else if (scmask & 0x20) ss->svol = 64;
                else ss->svol = readu8be(med4, offs);
                if (scmask & 0x40) ss->strans = 0;
                else ss->strans = reads8be(med4, offs);
            } else {
                samplenames.push_back(vector<char>(0));
                ss->rep = 0;
                ss->replen = 0;
                ss->midich = 0;
                ss->midipreset = 0;
                ss->strans = 0;
                ss->svol = 0;
            }
        }
        *smskptr = *smskptr << 1;
    }
    song.numblocks = readu16be(med4, offs);
    song.songlen = readu16be(med4, offs);
    copyu8bytes(med4, song.playseq, offs, song.songlen);
    song.deftempo = readu16be(med4, offs);
    song.playtransp = reads8be(med4, offs);
    song.flags = readu8be(med4, offs);
    song.flags2 = readu8be(med4, offs);
    song.tempo2 = readu8be(med4, offs);
    readbytes(med4, offs, 20); // skip jumpmask, rgb
    copyu8bytes(med4, song.trkvol, offs, 16);
    song.mastervol = readu8be(med4, offs);
    if (samplenames.size() > 0) {
        exp.i_ext_entries = samplenames.size();
        exp.i_ext_entrsz = 42; // MMDInstrInfo
        exp.iinfo = samplenames;
    }
}

void readBlock(const vector<char> &med4, MMD0Block &block, size_t &offs) {
    UBYTE hdrsz,hdr[80],trks,lines,mskmsks[8],msks,msk2,msk3 = 0;
    vector<UBYTE> conv;
    UWORD hdrsn = 0,convsz;
    ULONG lmsk[8],cmmsk[8];
    hdrsz = readu8be(med4, offs);
    copyu8bytes(med4, hdr, offs, hdrsz);
    trks = GetNibbles(hdr,&hdrsn,2);
    block.numtracks = trks;
    lines = GetNibbles(hdr,&hdrsn,2);
    block.lines = lines;
    convsz = GetNibbles(hdr,&hdrsn,4);
    msk2 = msks = (lines / 32) + 1; /* # of masks */
    while (msks--) mskmsks[msk3++] = GetNibble(hdr,&hdrsn);
    if(msk2 & 0x1) (void)GetNibble(hdr,&hdrsn); /* empty */
    for(msks = 0; msks < msk2; msks++) {
        if (mskmsks[msks] & 8) lmsk[msks] = 0xffffffff;
        else if (mskmsks[msks] & 4) lmsk[msks] = 0L;
        else lmsk[msks] = (GetNibbles(hdr,&hdrsn,4) << 16) |
                           GetNibbles(hdr,&hdrsn,4);
        if (mskmsks[msks] & 2) cmmsk[msks] = 0xffffffff;
        else if (mskmsks[msks] & 1) cmmsk[msks] = 0L;
        else cmmsk[msks] = (GetNibbles(hdr,&hdrsn,4) << 16) |
                            GetNibbles(hdr,&hdrsn,4);
    }
    // hlmask ignored
    block.data = vector<char>(3 * (lines + 1) * trks);
    if (convsz > 0) {
        conv = readu8bytes(med4, offs, convsz);
        UnpackData(lmsk,cmmsk,conv.data(),(UBYTE *)block.data.data(),(UWORD)(lines + 1),trks);
    }
    return;
}

void readSynthInstr(const vector<char> &med4, SynthInstr &instr, size_t &offs) {
    int instroffs = offs;
    (void)instroffs;
    readu32be(med4, offs); // synth instr header(?)
    reads16be(med4, offs); // FFFF (?)
    instr.defaultdecay = readu8be(med4, offs);
    readbytes(med4, offs, 3); // reserved(?)
    instr.rep = readu16be(med4, offs);
    instr.replen = readu16be(med4, offs);
    UWORD voltbllen = readu16be(med4, offs);
    UWORD wftbllen = readu16be(med4, offs);
    instr.volspeed = readu8be(med4, offs);
    instr.wfspeed = readu8be(med4, offs);
    instr.wforms = readu16be(med4, offs); 
    copybytes(med4, instr.voltbl.data(), offs, voltbllen);
    copybytes(med4, instr.wftbl.data(), offs, wftbllen);
    instr.wf = vector<SynthWF0>(instr.wforms);
    vector<ULONG> wfoffs(instr.wforms);
    for (int i = 0; i < instr.wforms; ++i) {
        ULONG wfo = readu32be(med4, offs);
        wfoffs[i] = wfo;
    }
    for (int i = 0; i < instr.wforms; ++i) {
        // assume data is just after pointers
        assert(offs - instroffs == wfoffs[i]);
        SynthWF0 wf {};
        if (instr.type == HYBRID && i == 0) {
            MMDSample0 sample {};
            sample.length = readu32be(med4, offs);
            sample.type = reads16be(med4, offs);
            sample.sample = readbytes(med4, offs, sample.length);
            wf.sample = sample;
        } else {
            SynthWF synth {};
            synth.length = readu16be(med4, offs); // length in words
            synth.wfdata = readbytes(med4, offs, synth.length * 2);
            wf.synthwf = synth;
        }
        instr.wf[i] = wf;
    }
    instr.length = 22 + instr.voltbl.size() + instr.wftbl.size() + instr.wf.size()* 4;
}

void readSamples(const vector<char> &med4, MMD0song &song, vector<Instr> &smplarr, size_t &offs) {
    ULONG imsk[2] = { 0,0 },*imptr = imsk;
    UBYTE snum = 0;
    imsk[0] = readu32be(med4, offs);
    if (*imsk & 0x80000000) imsk[1] = readu32be(med4, offs);
    int instrcnt = 0;
    while (snum < smplarr.size()) {
        *imptr = *imptr << 1;   /* There's no instrument #0 */
        if (snum == 31) imptr++;
        if (*imptr & 0x80000000) {
            Instr instr {};
            ULONG length = readu32be(med4, offs);
            WORD type = reads16be(med4, offs);
            if (type < 0) {
                SynthInstr synthinstr {};
                synthinstr.type = type;
                readSynthInstr(med4, synthinstr, offs);
                instr.synthinstr = synthinstr;
            } else {
                MMDSample0 sample {};
                sample.length = length;
                sample.type = type;
                sample.sample = readbytes(med4, offs, sample.length);
                instr.sample = sample;
            }
            smplarr[instrcnt++] = instr;
        }
        snum++;
    }
    song.numsamples = instrcnt;
}

vector<InstrExt> readIFF(const vector<char> &med4, MMD0song &song, MMD0exp0 &exp, size_t &offs) {
    constexpr uint32_t ANNO = 0x414e4e4f;
    constexpr uint32_t CHNS = 0x43484e53;
    constexpr uint32_t HLDC = 0x484c4443;
    constexpr uint32_t MEDV = 0x4d454456;
    constexpr uint32_t SMOF = 0x534d4f46;
    
    vector<InstrExt> exp_smp {};

    if (med4.size() <= offs) {
        song.tempo2 = 6;
        return exp_smp;
    }

    UWORD ext_entries = 0;
    ULONG medv = 0;

    while (med4.size() > offs) {
        uint32_t ID = readu32be(med4, offs);
        int32_t size = reads32be(med4, offs);
        switch (ID) {
            case ANNO: {
                auto annotxt = readbytes(med4, offs, size);
                exp.annolen = size;
                exp.annotxt = annotxt;
                break;
            }
            case CHNS:
                assert(size == 4);
                copyu8bytes(med4, exp.channelsplit, offs, 4);
                break;
            case HLDC: {
                assert(size <= MAX_SAMPLES * 2);
                for (int i = 0; i < size; i += 2) {
                    InstrExt ext {};
                    ext.hold = readu8be(med4, offs);
                    ext.decay = readu8be(med4, offs);
                    exp_smp.push_back(ext);
                }
                ext_entries = exp_smp.size();
                break;
            }
            case MEDV: {
                assert(size == 4);
                medv = readu32be(med4, offs);
                break;
            }
            case SMOF: {
                assert(size <= ext_entries);
                for (int i = 0; i < size; ++i) {
                    UBYTE smof = readu8be(med4, offs);
                    exp_smp[i].suppress_midi_off = smof;
                }
                break;
            }
            default:
                TRACE("Unxpected IFF type %u size %d\n", ID, size);
                break;
        }
    }
    exp.s_ext_entries = ext_entries;
    exp.s_ext_entrsz = ext_entries > 0 ? 4 : 0;
    if (!medv) {
        song.tempo2 = 6;
    }
    return exp_smp;
}
} // namespace {}

namespace converter::med {

bool isMED4(const char *buf, const size_t size) {
    constexpr char magic[] = {'M','E','D',4};
    if (size < sizeof(magic)) {
        return false;
    }
    return memcmp(magic, buf, sizeof(magic)) == 0;
}

ConverterResult convertMED4(const char *buf, const size_t size) {
    // TODO add metadata about conversion (annotxt?)
    assert(isMED4(buf, size));

    ConverterResult res;
    size_t med4offs = 4; // skip id

    MMD0 mmd0 {};
    MMD0song song {};
    MMD0exp0 exp {};

    mmd0.id = MMD0ID;
    mmd0.actplayline = -1;

    vector<char> med4;
    med4.assign(buf, buf + size);

    readSong(med4, song, exp, med4offs);

    if (!(song.flags & FLAG_INSTRSATT)) {
        // TODO maybe try load instruments ?
        res.success = false;
        res.reason_failed = "no sample data";
        return res;
    }

    vector<MMD0Block> blockarr(song.numblocks);
    for (int i = 0; i < song.numblocks; i++) {
        MMD0Block block {};
        readBlock(med4, block, med4offs);
        blockarr[i] = block;
    }
    
    vector<Instr> smplarr(MAX_SAMPLES);
    readSamples(med4, song, smplarr, med4offs);

    const vector<InstrExt> exp_smp = readIFF(med4, song, exp, med4offs);

    res.ext = "mmd0";
    if (song.flags & FLAG_8CHANNEL) {
        res.format = "MED4 (8ch)";
    } else {
        res.format = "MED4 (4ch)";
    }

    res.data = serializeMMD0(mmd0, song, blockarr, smplarr, exp, exp_smp);
    res.success = true;

    return res;
}

} // namespace converter::med
