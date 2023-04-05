// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <vector>
#include "common_med.h"
#include "converter.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#include "../3rdparty/SimpleBinStream.h"
#pragma GCC diagnostic pop

using namespace converter::med;

namespace {
const ULONG nil = 0;

vector<char> align(simple::mem_ostream<true_type> &out) {
    vector<char> data = out.get_internal_vec();
    if (data.size() % 2 != 0) {
        data.push_back(0);
    }
    return data;
}

void serializeBlocks(simple::mem_ostream<true_type> &out, const vector<MMD0Block> &blockarr, ULONG offs_blockarr) {
    assert(offs_blockarr % 2 == 0);
    vector<vector<char>> blocks(blockarr.size());
    int i = 0;
    for (const auto& block : blockarr) {
        simple::mem_ostream<true_type> out;
        out << block.numtracks;                                     TRACE("mmd0block[%d].numtracks: %d\n", i, block.numtracks);
        out << block.lines;                                         TRACE("mmd0block[%d].lines: %d\n", i, block.lines);
        out << block.data;                                          TRACE("mmd0block[%d].data[%lu]\n", i, block.data.size());
        blocks[i++] = align(out);
    }
    ULONG boffs = blockarr.size() * 4;
    i = 0;
    for (const auto& block : blocks) {
        ULONG offs = offs_blockarr + boffs;
        out << offs;                                                TRACE("mmd0block[%d].offs: 0x%.8x\n", i, (uint32_t)offs);
        boffs = boffs + block.size();
        i++;
    }
    for (const auto& block : blocks) {
        out << block;
    }
}

void serializeSamples(simple::mem_ostream<true_type> &out, const MMD0song& song, const vector<Instr> &smplarr, ULONG offs_smplarr) {
    assert(offs_smplarr % 2 == 0);
    vector<vector<char>> samples(smplarr.size());
    int i = 0;
    for (const auto& instr : smplarr) {
        simple::mem_ostream<true_type> out;
        if (instr.sample.has_value()) {
            const MMDSample0& sample = instr.sample.value();
            out << sample.length;                                   TRACE("mmd0sample[%d].length: %u\n", i, (uint32_t)sample.length);
            out << sample.type;                                     TRACE("mmd0sample[%d].type: %d\n", i, (int16_t)sample.type);
            out << sample.sample;                                   TRACE("mmd0sample[%d].data[%lu]\n", i, sample.sample.size());
            samples[i] = align(out);
        } else if (instr.synthinstr.has_value()) {
            const SynthInstr& synthinstr = instr.synthinstr.value();
            out << synthinstr.length;                               TRACE("mmd0sample[%d].length: %u\n", i, (uint32_t)synthinstr.length);
            out << synthinstr.type;                                 TRACE("mmd0sample[%d].type: %d\n", i, (int16_t)synthinstr.type);
            out << synthinstr.defaultdecay;                         TRACE("mmd0sample[%d].defaultdecay: %u\n", i, synthinstr.defaultdecay);
            out << synthinstr.reserved;                             TRACE("mmd0sample[%d].reserved[%lu]\n", i, synthinstr.reserved.size());
            out << synthinstr.rep;                                  TRACE("mmd0sample[%d].rep: %u\n", i, (uint16_t)synthinstr.rep);
            out << synthinstr.replen;                               TRACE("mmd0sample[%d].replen: %u\n", i, (uint16_t)synthinstr.replen);
            out << synthinstr.voltbllen;                            TRACE("mmd0sample[%d].voltbllen: %u\n", i, (uint16_t)synthinstr.voltbllen);
            out << synthinstr.wftbllen;                             TRACE("mmd0sample[%d].wftbllen: %u\n", i, (uint16_t)synthinstr.wftbllen);
            out << synthinstr.volspeed;                             TRACE("mmd0sample[%d].volspeed: %u\n", i, synthinstr.volspeed);
            out << synthinstr.wfspeed;                              TRACE("mmd0sample[%d].wfspeed: %u\n", i, synthinstr.wfspeed);
            out << synthinstr.wforms;                               TRACE("mmd0sample[%d].wforms: %u\n", i, (uint16_t)synthinstr.wforms);
            out << synthinstr.voltbl;                               TRACE("mmd0sample[%d].voltbl[%lu]\n", i, synthinstr.voltbl.size());
            out << synthinstr.wftbl;                                TRACE("mmd0sample[%d].wftbl[%lu]\n", i, synthinstr.wftbl.size());

            vector<vector<char>> wfs(synthinstr.wforms);
            int j = 0;
            if (synthinstr.wforms > 0) {
                if (synthinstr.type == HYBRID) {
                    const MMDSample0& sample = synthinstr.wf[0].sample.value();
                    simple::mem_ostream<true_type> wfout;
                    wfout << sample.length;                         TRACE("mmd0sample[%d].wf[0].length: %u\n", i, (uint32_t)sample.length);
                    wfout << sample.type;                           TRACE("mmd0sample[%d].wf[0].type: %d\n", i, (int16_t)sample.type);
                    wfout << sample.sample;                         TRACE("mmd0sample[%d].wf[0].sample[%lu]\n", i, sample.sample.size());
                    wfs[j] = align(wfout);
                    j++;
                }
                for (;j < synthinstr.wforms; j++) {
                    const SynthWF& wf = synthinstr.wf[j].synthwf.value();
                    simple::mem_ostream<true_type> wfout;
                    wfout << wf.length;                             TRACE("mmd0sample[%d].wf[%d].length: %u\n", i, j, (int16_t)wf.length);
                    wfout << wf.wfdata;                             TRACE("mmd0sample[%d].wf[%d].wfdata[%lu]\n", i, j, wf.wfdata.size());
                    wfs[j] = align(wfout);
                }
            }
            ULONG woffs = synthinstr.length;
            for (const auto &wf : wfs) {
                out << woffs;                                       TRACE("mmd0sample[%d].wf[%d].offs: 0x%.8x\n", i, j, (uint32_t)woffs);
                woffs = woffs + wf.size();
            }
            for (const auto &wf : wfs) {
                out << wf;
            }
            samples[i] = align(out);
        }
        i++;
    }
    ULONG soffs = song.numsamples * 4;
    i = 0;
    for (const auto& sample : samples) {
        if (sample.size() > 0) {
            ULONG offs = offs_smplarr + soffs;
            out << offs;                                            TRACE("mmd0sample[%d].offs: 0x%.8x\n", i, (uint32_t)offs);
            soffs = soffs + sample.size();
            i++;
        }
    }
    for (const auto& sample : samples) {
        if (sample.size() > 0) {
            out << sample;
        }
    }
}

void serializeExp(simple::mem_ostream<true_type> &out, const MMD0song &song, const MMD0exp0 &exp, const vector<InstrExt>& exp_smp, ULONG offs_expdata) {
    if (!exp.s_ext_entries && !exp.annolen && !exp.i_ext_entries && !(song.flags & FLAG_8CHANNEL)) return;

    const ULONG offs_exp_smp = offs_expdata + 84;
    const ULONG offs_annotxt = offs_exp_smp + exp.s_ext_entries * exp.s_ext_entrsz;
    vector<char> annotxt = exp.annotxt;
    if (annotxt.size() % 2 != 0) {
        annotxt.push_back(0);
    }
    const ULONG offs_iinfo = offs_annotxt + annotxt.size();

    out << nil; // offs_nextmod;
    out << (exp.s_ext_entries > 0 ? offs_exp_smp : nil);            TRACE("exp.exp_smp: 0x%.8x\n", (uint32_t)(exp.s_ext_entries > 0 ? offs_exp_smp : nil));
    out << exp.s_ext_entries;                                       TRACE("exp.s_ext_entries: %u\n", (uint16_t)exp.s_ext_entries);
    out << exp.s_ext_entrsz;                                        TRACE("exp.s_ext_entrsz: %u\n", (uint16_t)exp.s_ext_entrsz);
    out << (exp.annolen > 0 ? offs_annotxt : nil);                  TRACE("exp.annotxt: 0x%.8x\n", (uint32_t)(exp.annolen > 0 ? offs_annotxt : nil));
    out << exp.annolen;                                             TRACE("exp.annolen: %u\n", (uint32_t)exp.annolen);
    out << (exp.i_ext_entries > 0 ? offs_iinfo : nil);              TRACE("exp.iinfo: 0x%.8x\n", (uint32_t)(exp.i_ext_entries > +0 ? offs_iinfo : nil));
    out << exp.i_ext_entries;                                       TRACE("exp.i_ext_entries: %u\n", (uint16_t)exp.i_ext_entries);
    out << exp.i_ext_entrsz;                                        TRACE("exp.i_ext_entrsz: %u\n", (uint16_t)exp.i_ext_entrsz);
    out << nil; // exp.jumpmask;
    out << nil; // offs_rgbtable;
    for (size_t i = 0; i < size(exp.channelsplit); ++i) {
        out << exp.channelsplit[i];                                 TRACE("exp.channelsplit[%d]: %u\n", i, exp.channelsplit[i]);
    }
    out << nil; // offs_n_info;
    out << nil; // offs_songname;
    out << nil; // exp.songnamelen;
    out << nil; // offs_dumps;
    out << nil; // offs_mmdinfo;
    out << nil; // offs_mmdrexx;
    out << nil; // offs_mmdcmd3x;
    for (size_t i = 0; i < size(exp.reserved2); ++i) {
        out << exp.reserved2[i];                                    TRACE("exp.reserved2[%d]: %u\n", i, (uint32_t)b);
    }
    out << exp.tag_end;                                             TRACE("exp.tag_end: %u\n", (uint32_t)exp.tag_end);
    for (int i = 0; i < exp.s_ext_entries; ++i) {
        const InstrExt& ext = exp_smp[i];
        out << ext.hold;                                            TRACE("expdata.exp_smp[%d].hold: %u\n", i, ext.hold);
        out << ext.decay;                                           TRACE("expdata.exp_smp[%d].decay: %u\n", i, ext.decay);
        out << ext.suppress_midi_off;                               TRACE("expdata.exp_smp[%d].suppress_midi_off: %u\n", i, ext.suppress_midi_off);
        // unused, included for alighment
        out << ext.finetune;                                        TRACE("expdata.exp_smp[%d].finetune: %d\n", i, ext.finetune);
    }
    if (exp.annolen > 0) {
        out << annotxt;                                             TRACE("expdata.annotxt: %s\n", exp.annotxt.data());
    }
    for (int i = 0; i < exp.i_ext_entries; ++i) {
        out << exp.iinfo[i];                                        TRACE("expdata.iinfo[%d].name: %s\n", i, exp.iinfo[i].data());
        for (int j = exp.iinfo[i].size(); j < exp.i_ext_entrsz; ++j) {
            out << (char)0;
        }
    }
}

} // namespace {}

namespace converter::med {

UBYTE GetNibble(UBYTE *mem, UWORD *nbnum) {
    UBYTE *mloc = mem + (*nbnum / 2),res;
    if (*nbnum & 0x1) res = *mloc & 0x0f;
    else res = *mloc >> 4;
    *nbnum = *nbnum + 1;
    return res;
}

UWORD GetNibbles(UBYTE *mem, UWORD *nbnum, UBYTE nbs) {
    UWORD res = 0;
    while (nbs--) {
        res = res << 4; 
        res = res | GetNibble(mem, nbnum);
    }
    return res;
}

void UnpackData(ULONG *lmptr, ULONG *cmptr, UBYTE *from, UBYTE *to, UWORD lines, UBYTE trkn) {
    UBYTE *fromst = from, *tmpto, bcnt;
    UWORD fromn = 0, lmsk;
    for(UWORD lcnt = 0; lcnt < lines; lcnt = lcnt + 1) {
        if(!(lcnt % 32) && lcnt) { lmptr++; cmptr++; }
        if(*lmptr & 0x80000000) {
            lmsk = GetNibbles(fromst, &fromn, (UBYTE)(trkn / 4));
            lmsk = lmsk << (16 - trkn);
            tmpto = to;
            for(bcnt = 0; bcnt < trkn; bcnt++) {
                if(lmsk & 0x8000) {
                    *tmpto = (UBYTE)GetNibbles(fromst, &fromn, 2);
                    *(tmpto+1) = (GetNibble(fromst, &fromn) << 4);
                }
                lmsk = lmsk << 1; tmpto += 3;
            }
        }
        if(*cmptr & 0x80000000) {
            lmsk = GetNibbles(fromst,&fromn,(UBYTE)(trkn / 4));
            lmsk = lmsk << (16 - trkn);
            tmpto = to;
            for(bcnt = 0; bcnt < trkn; bcnt++) {
                if(lmsk & 0x8000) {
                    *(tmpto+1) |= GetNibble(fromst, &fromn);
                    *(tmpto+2) = (UBYTE)GetNibbles(fromst, &fromn, 2);
                }
                lmsk = lmsk << 1; tmpto += 3;
            }
        }
        to += 3 * trkn;
        *lmptr = *lmptr << 1; *cmptr = *cmptr << 1;
    }
}

vector<char> serializeMMD0(
    const MMD0 &mmd0,
    const MMD0song &song,
    const vector<MMD0Block> &blockarr,
    const vector<Instr> &smplarr,
    const MMD0exp0 &exp,
    const vector<InstrExt> &exp_smp
) {
    // MMD0 structs are internally big endian

    const ULONG offs_song = 52;
    const ULONG offs_blockarr = offs_song + 788;

    simple::mem_ostream<true_type> sb;
    serializeBlocks(sb, blockarr, offs_blockarr);
    const vector<char> blockdata = sb.get_internal_vec();

    const ULONG offs_smplarr = offs_blockarr + blockdata.size();
    simple::mem_ostream<true_type> ss;
    serializeSamples(ss, song, smplarr, offs_smplarr);
    const vector<char> sampledata = ss.get_internal_vec();

    const ULONG offs_expdata = offs_smplarr + sampledata.size();
    simple::mem_ostream<true_type> sexp;
    serializeExp(sexp, song, exp, exp_smp, offs_expdata);
    const vector<char> expdata = sexp.get_internal_vec();

    const ULONG modlen = offs_expdata + expdata.size();

    simple::mem_ostream<true_type> out;

    out << mmd0.id;                                                 TRACE("mmd0.id: 0x%.8x\n", (uint32_t)mmd0.id);
    out << modlen;                                                  TRACE("mmd0.modlen: 0x%.8x\n", (uint32_t)modlen);
    out << offs_song;                                               TRACE("mmd0.song: 0x%.8x\n", (uint32_t)offs_song);
    out << mmd0.psecnum;                                            TRACE("mmd0.psecnum: %u\n", (uint32_t)mmd0.psecnum);
    out << mmd0.pseq;                                               TRACE("mmd0.pseq: %u\n", (uint32_t)mmd0.pseq);
    assert(blockdata.size() > 0);
    out << offs_blockarr;                                           TRACE("mmd0.blockarr: 0x%.8x\n", (uint32_t)offs_blockarr);
    out << mmd0.mmdflags;                                           TRACE("mmd0.mmdflags: 0x%.2x\n", mmd0.mmdflags);
    for (size_t i = 0; i < size(mmd0.reserved); ++i) {
        out << mmd0.reserved[i];                                    TRACE("mmd0.reserved[%d]: %u\n", i, mmd0.reserved[i]);
    }
    out << (sampledata.size() > 0 ? offs_smplarr: nil);             TRACE("mmd0.smplarr: 0x%.8x\n", (uint32_t)(sampledata.size() > 0 ? offs_smplarr: nil));
    out << mmd0.reserved2;                                          TRACE("mmd0.reserved2: %u\n", (uint32_t)mmd0.reserved2);
    out << (expdata.size() > 0 ? offs_expdata : nil);               TRACE("mmd0.expdata: 0x%.8x\n", (uint32_t)(expdata.size() > 0 ? offs_expdata : nil));
    out << mmd0.reserved3;                                          TRACE("mmd0.reserved3: %u\n", (uint32_t)mmd0.reserved3);
    out << mmd0.pstate;                                             TRACE("mmd0.pstate: %u\n", (uint16_t)mmd0.pstate);
    out << mmd0.pblock;                                             TRACE("mmd0.pblock: %u\n", (uint16_t)mmd0.pblock);
    out << mmd0.pline;                                              TRACE("mmd0.pline: %u\n", (uint16_t)mmd0.pline);
    out << mmd0.pseqnum;                                            TRACE("mmd0.pseqnum: %u\n", (uint16_t)mmd0.pseqnum);
    out << mmd0.actplayline;                                        TRACE("mmd0.actplayline %d\n", (int16_t)mmd0.actplayline);
    out << mmd0.counter;                                            TRACE("mmd0.counter: %u\n", mmd0.counter);
    out << mmd0.extra_songs;                                        TRACE("mmd0.extra_songs: %u\n", mmd0.extra_songs);
    for (int i = 0; i < MAX_SAMPLES; ++i) {
        out << song.sample[i].rep;
        out << song.sample[i].replen;
        out << song.sample[i].midich;
        out << song.sample[i].midipreset;
        out << song.sample[i].svol;
        out << song.sample[i].strans;
        if (i < song.numsamples) {
                                                                    TRACE("mmd0song.sample[%d].rep: %u\n", i, (uint16_t)song.sample[i].rep);
                                                                    TRACE("mmd0song.sample[%d].replen: %u\n", i, (uint16_t)song.sample[i].replen);
                                                                    TRACE("mmd0song.sample[%d].midich: %u\n", i, song.sample[i].midich);
                                                                    TRACE("mmd0song.sample[%d].midipreset: %u\n", i, song.sample[i].midipreset);
                                                                    TRACE("mmd0song.sample[%d].svol: %u\n", i, song.sample[i].svol);
                                                                    TRACE("mmd0song.sample[%d].strans: %d\n", i, song.sample[i].strans);
        }
     }
    out << song.numblocks;                                          TRACE("mmd0song.numblocks %u\n", (uint16_t)song.numblocks);
    out << song.songlen;                                            TRACE("mmd0song.songlen %u\n", (uint16_t)song.songlen);
    for (size_t i = 0; i < size(song.playseq); ++i) {
        out << song.playseq[i];
        if (song.playseq[i] != 0)                                   TRACE("mmd0song.playseq[%d]: %u\n", i, song.playseq[i]);
    }
    out << song.deftempo;                                           TRACE("mmd0song.deftempo %u\n", (uint16_t)song.deftempo);
    out << song.playtransp;                                         TRACE("mmd0song.playtransp %d\n", song.playtransp);
    out << song.flags;                                              TRACE("mmd0song.flags 0x%.2x\n", song.flags);
    out << song.flags2;                                             TRACE("mmd0song.flags2 0x%.2x\n", song.flags2);
    out << song.tempo2;                                             TRACE("mmd0song.tempo2 %u\n", song.tempo2);
    for (size_t i = 0; i < size(song.trkvol); ++i) {
        out << song.trkvol[i];                                      TRACE("mmd0song.trkvol[%d]: %u\n", i, song.trkvol[i]);
    }
    out << song.mastervol;                                          TRACE("mmd0song.mastervol %u\n", song.mastervol);
    out << song.numsamples;                                         TRACE("mmd0song.numsamples %u\n", song.numsamples);

    out << blockdata;
    
    out << sampledata;
    
    out << expdata;

    return out.get_internal_vec();
}

} // namespace converter::med
