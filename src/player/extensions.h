// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2014-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include "config.h"

namespace player {

// TODO add more
static constexpr const char *mimetypes[] = {
    "audio/x-amiga",
    nullptr
};

static constexpr const char *exts[] = {
#if PLAYER_uade
    "aps", // AProSys
    "ast", // Actionamics
    "avp","mw", // Activision Pro / Martin Walker
    "ahx","thx", // AHX / THX
    "amc", // AM Composer
    "abk", // AMOS
    "hot", // Anders 0land
    "aam", // Art & Magic
    "alp", // Alcatraz Packer
    "aon","aon4","aon8", // Art Of Noise
    "adsc","mod_adsc4", // Audio Sculpture / Startrekker AM
    "bss", // Beathoven Synthesizer
    "bd","mth" // Ben Daglish
    "bds", // Ben Daglish SID
    "uds", // Blade Packer / Unique Development
    "bp","sndmon", // BP SoundMon2.0
    "bp3", // BP SoundMon2.2
    "cin", // Cinemaware
    "core", // Core Design
    "cus","cust","custom", // Custom
    "cm","rk","rkb", // Custom Made  / Ron Klaren
    "dz","mkiio", // Darius Zendeh
    "dl", // Dave Lowe
    "dl_deli", // Dave Lowe Deli
    "dln", // Dave Lowe New
    "dh", // David Hanney
    "dw","dwold","oldw" // David Whittaker
    "dlm1","dm1","dm" // Delta Music 1.3
    "dlm2","dm2", // Delta Music 2.0
    "dp","trc","tro","tronic","tron", // Delta Packer / Tronic
    "dsr", // Desire
    "db","digi", // DIGI Booster
    "dmu","mug", // Digital Mugician
    "dmu2","mug2", // Digital Mugician II
    "dsc", // Digital Sonix & Chrome
    "dss", // Digital Sound Studio
    "tpu", // Dirk Bialluch
    "dns", // Dynamic Synthesizer
    "ea","mg", // EarAche
    "ems", // Electronic Music System
    "emsv6", // Electronic Music System v6
    "ex", // Fashion Tracker
    "fc13","fc3","smod", // Future Composer 1.3
    "fc","fc14","fc4", // Future Composer 1.4
    "bfc","bsi","fc-bsi", // Future Composer BSI
    "fw", // Forgotten Worlds
    "tf", // Tim Follin
    "fred", // FredMon
    "gray", // Fred Gray
    "fp", // Future Player
    "glue","gm", // GlueMon
    "hip","mcmd","sog", // Hippel / MCMD
    "hip7","s7g", // Hippel 7V
    "hipc", // Hippel COSO
    "hst","sdc", // Hippel ST
    "soc", // Hippel ST COSO
    "hd", // Howie Davis
    "hn","mtp2","thn","arp", // Major Tom / The Holy Noise
    "ims", // Images Music System
    "dum", // Infogrames / Rob Hubbard 2
    "is","ism" // InStereo!
    "is20", // InStereo! 2.0
    "jam","jc", // JamCracker
    "jmf", // Janko Mrsic-Flogel
    "jcb","jcbo", "jb", // Jason Brooke
    "jpn","jpnd", // Jason Page
    "jpo","jpold", // Jason Page Old / Steve Turner
    "jp", // Jason Page JP
    "jt","mon_old", // Jeroen Tel / Maniacs Of Noise Old
    "jo", // Jesper Olsen
    "jd", // Special FX
    "kh", // Kris Hatlelid
    "powt","pt", // Laxity  / Powertracker
    "lme", // Leggless Music Editor
    "lion","sp-a", // Lionheart (SonicArranger)
    "mfp", // Magnetic Fields Packer
    "mon", // Maniacs Of Noise
    "mc","mcr", // Mark Cooksey
    "mco", // Mark Cooksey Old
    "mk2","mkii", // Mark II
    "mcmd_org", // MCMD
    "mso", // Medley
    "md", // Mike Davies
    "mms","sfx20", // MultiMedia Sound
    "ma", // Music Assembler
    "mmd0","mmd1","mmd2", // OctaMED
    "ml", // Musicline Editor
    "mm4","sdata", // Music Maker 8V Old
    "mm8", // Music Maker 8V
    "max", // Maximum Effect
    "mmdc", // MMDC / MED Packer
    "midi", // MIDI - Loriciel

    // PTK-Prowiz
    "mod_doc", // st24
    "mod15","mod15_msg", // st20
    "mod_ntk","mod_ntk1", // nt10
    "mod_ntk2", // nt20
    "mod_ntkamp", // m&k
    "mod_flt4",  // flt4
    // "mod_flt8", // no support ?
    "mod", // pt30
    "mod_comp", // comp

    "!pm!","40a","40b","41a","50a","60a","61a","ac1","ac1d","aval","chan","cp",
    "cplx","crb","di","eu","fc-m","fcm","fuz","fuzz","gv","hmc",
    "hrt","hrt!","ice","it1","kef","kef7","krs","ksm","lax","mexxmp","mpro",
    "np","np1","np2","noisepacker2","np3","noisepacker3","nr","nru","ntpk",
    "p10","p21","p30","p40a","p40b","p41a","p4x","p50a","p5a","p5x","p60",
    "p60a","p61","p61a","p6x","pha","pin","pm","pm0","pm01","pm1","pm10c",
    "pm18a","pm2","pm20","pm4","pm40","pmz","polk","pp10","pp20","pp21","pp30",
    "ppk","pr1","pr2","prom","pru","pru1","pru2","prun","prun1","prun2","pwr",
    "pyg","pygm","pygmy","skt","skyt","snt","snt!","st2","st26","st30","star",
    "stpk","tp","tp1","tp2","tp3","un2","unic","unic2","wn","xan","xann","zen",

    "ntp", // NovoTrade Packer
    "two", // NTSP-System

    "octamed", // OctaMED
    "okt","okta", // Oktalyzer
    "one", // onEscapee
    "dat", // Paul Robotham
    "ps", // Paul Shields
    "snk", // Paul Summers
    "pvp", // Peter Verswyvelen
    "pap", // Pierre Adane Packer
    "pn", // PokeyNoise
    "psa", // Professional Sound Artists
    "puma", // Pumatracker
    "emod","qc", // Quadra Composer
    "qpa", // Quartet
    "sqt", // Quartet PSG
    "qts", // Quartet ST
    "rjp","sng", // Richard Joseph / Vectordean
    "riff", // Riff Raff
    "rh", // Rob Hubbard
    "rho", // Rob Hubbard Old / Rob Hubbard ST
    "scumm", // SCUMM
    "s-c","scn", // Sean Connolly
    "scr", // Sean Conran
    "sid1","smn", "sid", // SIDMon1.0
    "sid2", // SIDMon2.0
    "mok", // Silmarils
    "sa","sonic", // Sonic Arranger
    "sa_old", // Sonic Arranger pc-all
    "tw", // Sound Images / Tiny Williams
    "sm","sm1","sm2","sm3","smpro", // Sound Master / Sound Master II / Sound Master II v3
    "spl", // Sound Programming Language
    "sc","sct", // SoundControl
    "psf", // SoundFactory
    "sfx","sfx13", // SoundFX /SoundFX 2
    "sjs", // SoundPlayer
    "jd", // Special FX
    "doda", // Special FX ST
    "sas", // Speedy A1 System
    "ss", // Speedy System
    "sb", // Steve Barrett
    "sun", // SunTronic
    "sdr", // Synth Dream
    "osp", // Synth Pack
    "syn", // Synthesis
    "st","synmod", // SynTracker
    "smus","snx","tiny", // Sonix Music Driver
    "tfmx1.5", // TFMX
    "tfhd1.5", // TFMX-1.5-TFHD
    "tfmx7v", // TFMX-7V
    "tfhd7v", // TFMX-7V-TFHD
    "mdat","tfmxpro", // TFMX-Pro
    "tfhdpro", "tfmx", // TFMX-Pro-TFHD
    "mdst", // TFMX_ST
    "tme", // The Musical Enlightenment
    "thm", // Thomas Hermann
    "sg", // Tomy Tracker
    "mus","ufo", // UFO
    "mod15_ust", // Ultimate Soundtracker
    "vss", // Voodoo Supreme Synthesizer
    "wb", // Wally Beben
    "zmon","sng", // ZoundMonitor
    "mod15_st-iv", // Soundtracker-IV
    "agi", // Sierra AGI
    "tmk", // TimeTracker
    "tcb", // TCB Tracker
    "js", // Janne Salmijarvi Optimizer	
    "kim", // Kim Christensen
    "pat", // Paul Tonge
    "npp", // Nick Pelling Packer
    "mosh", // Mosh Packer
    "gmc", // Game Music Creator
    "ash", // Ashley Hogg
    "bye", // Andrew Parton
    "sj", // ScottJohnston

    "ymst", // YM-2149

    // converted to UADE compatible
    "med", // Music Editor MED4 -> MMD0
    
    "adpcm", // ADPCM (Mono)
    "ftm", // Face The Music
    "prt", // Pretracker
    "ptm", "mod3", // Protracker IFF (Protracker4)
    "mmd3", "oss", // Octamed SoundStudio / MMD3
    "stk", // Soundtracker

    // no replay
    //"ct", Cybertracker
    //"fuchs", // Fuchs Tracker
    //"dux", // GT Game Systems
    //"mxtx", // MaxTrax
    //"stp", // SoundTracker Pro II
    //"spm", // Stonetracker
    //"symmod", // Symphonie
#endif

    // non-UADE/native player
#if PLAYER_hivelytracker
    "hvl", // HivelyTracker
#endif
#if PLAYER_libdigibooster3
    "dbm", // Digibooster Pro
#endif
#if PLAYER_noisetrekker2 || PLAYER_protrekkr1 || PLAYER_protrekkr2
    "ntk", // NoiseTrekker
#endif
#if PLAYER_protrekkr1 || PLAYER_protrekkr2
    "ptk", // ProTrekkr
#endif
    // PC-tracker
#if PLAYER_ft2play
    "ft", "fst", "mod", // Fasttracker 1
    "xm", // Fasttracker 2
#endif
#if PLAYER_st23play
    "stm", // Scream Tracker 2
#endif
#if PLAYER_st3play
    "s3m", // Scream Tracker 3
#endif
#if PLAYER_it2play
    "it", // Impulse Tracker 1 & 2
#endif
#if PLAYER_libopenmpt
    // from Tables.cpp
    "mptm", // OpenMPT
    "mod", // ProTracker, ChipTracker, TCB Tracker
    "s3m", // Scream Tracker 3
    "xm", // FastTracker 2
    "it", // Impulse Tracker
    "667", // Composer 667
    "669", // Composer 669 / UNIS 669
    "amf", // ASYLUM Music Format
    "ams", // Extreme's Tracker, Velvet Studio
    "c67", // CDFM / Composer 670
    "cba", // Chuck Biscuits / Black Artist
    "dbm", // DigiBooster Pro
    "digi", // DigiBooster
    "dmf", // X-Tracker, DSMI Advanced Music Format (Compact)
    "dsm", // DSIK Format, Dynamic Studio
    "dsym", // Digital Symphony
    "dtm", // Digital Tracker
    "etx", // EasyTrax
    "far", // Farandole Composer
    "fc", // Future Composer
    "fmt", // FM Tracker
    "fst", // ProTracker
    "ftm", // Face The Music
    "imf", // Imago Orpheus
    "ims", // Images Music System
    "ice", // Ice Tracker
    "j2b", // Galaxy Sound System
//#ifdef MPT_EXTERNAL_SAMPLES
    "itp", // Impulse Tracker Project
//#endif
    "m15", // Soundtracker
    "mdl", // Digitrakker
    "med", // OctaMED
    "mms", // MultiMedia Sound
    "mt2", // MadTracker 2
    "mtm", // MultiTracker
    "mus", // Karl Morton Music Format
    "nst", // NoiseTracker
    "okt", // Oktalyzer
    "plm", // Disorder Tracker 2
    "psm", // Epic Megagames MASI
    "pt36", // ProTracker
    "ptm", // PolyTracker
    "puma", // Puma Tracker
    "rtm", // Real Tracker 2
    "sfx", // SoundFX
    "sfx2", // SoundFX
    "smod", // Future Composer
    "st26", // Soundtracker 2.6
    "stk", // Soundtracker
    "stm", // Scream Tracker 2
    "stx", // Scream Tracker Music Interface Kit
    "stp", // Soundtracker Pro II
    "symmod", // Symphonie
    "gmc", // Game Music Creator
    "gtk", // Graoumf Tracker
    "gt2", // Graoumf Tracker 1 / 2
    "ult", // UltraTracker
    "unic", // UNIC Tracker
    "wow", // Mod's Grave
    "xmf", // Astroidea XMF
    // converted formats (no MODTYPE)
    "gdm", // General Digital Music
    "mo3", // Un4seen MO3
    "oxm", // OggMod FastTracker 2
    // Compressed modules
//#ifndef NO_ARCHIVE_SUPPORT
    "mdz", // Compressed ProTracker
    "mdr", // Compressed Module
    "s3z", // Compressed Scream Tracker 3
    "xmz", // Compressed FastTracker 2
    "itz", // Compressed Impulse Tracker
    "mptmz", // Compressed OpenMPT
//#endif
    // not in openmpt
    "as3m", // Screamtracker 3 AdLib
#endif
#if PLAYER_libxmp
    // from docs/formats.txt
    // Common suffix   Tracker/packer          Recognized variants
    // Soundtracker and variants:
    "mod", // Sound/Noise/Protracker    M.K., M!K!, M&K!, N.T., CD81
    "m15", // Soundtracker              2.2, UST
    /*MOD*/"nt", // Startrekker/ADSC    FLT4/8/M, EXO4/8
    //MOD             Digital Tracker   FA04, FA06, FA08
    //MOD             Fast/Taketracker  xCHN, xxCH, TDZx
    "flx", // Flextrax                  M.K., xCHN (no dsp effects)
    "wow", // Mod's Grave               M.K.
    // Amiga packed formats:
    /*
    -               AC1D Packer             -
    -               FC-M Packer             1.0
    -               Fuchs Tracker           -
    -               Heatseeker              mc1.0
    -               Hornet Packer           HRT!
    -               Images Music System     ?
    -               Kefrens Sound Machine   -
    -               Module Protector        -
    -               NoisePacker             1.0, 2.0, 3.0
    -               NoiseRunner             -
    -               Pha Packer              -
    -               Power Music             -
    -               ProPacker               2.1
    -               ProRunner               1.0, 2.0
    -               Promizer                0.1, 1.0c, 1.8a, 2.0, 4.0
    -               SKYT Packer             -
    -               StarTrekker Packer      -
    -               The Player              4.x, 5.0a, 6.0a, 6.1a
    -               Titanics Player         -
    -               Tracker Packer          3
    MOD             Unic Tracker            1.0, 2.0
    -               Wanton Packer           -
    -               XANN Packer             -
    -               Zen Packer              -
    */
    // Other Amiga tracker formats:
    "dbm", // DigiBooster Pro           DBM0
    "digi", // DIGI Booster             1.4, 1.5, 1.6, 1.7
    "emod", // Quadra Composer          0001
    //MOD       ChipTracker             KRIS
    //MOD       Protracker 3.59         PTDT
    "med", // MED 1.12/2.10/3.00        MED2, MED3, MED4
    //MED       MED 3.00/OctaMED        MMD0, MMD1, MMD2, MMD3
    /*MOD*/"mtn", // ST 2.6, Ice Tracker  MTN, IT10
    "okt", // Oktalyzer                 -
    "sfx", // SoundFX                   1.3, 2.0?
    // Atari tracker formats:
    //MOD       Octalyser               CD61, CD81
    "dtm", // Digital Tracker           2.015, 2.03, 2.04, 1.x
    "mgt", // Megatracker               -
    // Acorn tracker formats:
    /*
    -               Archimedes Tracker      V1.0+++
    -               Coconizer               -
    -               Digital Symphony        0, 1
    */
    // PC tracker formats:
    "669", // Composer 669/UNIS 669     if, JN
    "far", // Farandole Composer        1.0
    "fnk", // Funktracker               R0, R1, R2
    "imf", // Imago Orpheus             1.0
    "it", // Impulse Tracker            1.00, 2.00, 2.14, 2.15
    "liq", // Liquid Tracker            NO, 0.00, 1.00
    "mdl", // Digitrakker               0.0, 1.0, 1.1
    "mtm", // Multitracker              1.0
    "ptm", // Poly Tracker              2.03
    "rtm", // Real Tracker              1.00
    "s3m", // Scream Tracker 3          3.00, 3.01+
    "stm", // Scream Tracker 2          !Scream!, BMOD2STM
    "ult", // Ultra Tracker             V001, V002, V003, V004
    "xm", // Fast Tracker II            1.02, 1.03, 1.04
    // PC packed formats:
    "amf", // DSMI (DMP)                0.1, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4
    "gdm", // General Digital Music     1.0
    "stx", // ST Music Interface Kit    1.0, 1.1
    // Game formats:
    "abk", // AMOS Sound Bank           -
    "amf", // Asylum Music Format       1.0
    //-       Digital Illusions         -
    //-       Game Music Creator        -
    "psm", // Epic MegaGames MASI       epic, sinaria
    /*PSM*/"ps16", // Epic MegaGames MASI 16  Epic Pinball (old), Silverball
    "j2b", // Galaxy Music System 5.0   -
    //-       Galaxy Music System 4.0   -
    "mfp","smp", // Magnetic Fields Packer  -
    "mmdc", // MED Packer               -
    //-       Novotrade Packer          -
    "stim", // Slamtilt                 -
    "umx", // Epic Games Unreal/UT      IT, S3M, MOD, XM
    "xmf", // Imperium Galactica        -
#endif
    nullptr
};

}
