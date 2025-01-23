// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#define UADE_MIMETYPE "audio/x-amiga"

namespace common {

static constexpr const char *plugin_mimes[] = {
    UADE_MIMETYPE,
    nullptr
};

// hybrid of UADE eagleplayer.conf and modland file extensions
static constexpr const char *plugin_extensions[] = {
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
    "bd", // Ben Daglish
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
    "dw","dwold", // David Whittaker
    "dlm1","dm1","dm" // Delta Music 1.3
    "dlm2","dm2", // Delta Music 2.0
    "dp","trc","tro","tronic" // Delta Packer / Tronic
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
    "hip","mcmd", // Hippel / MCMD
    "hip7","s7g", // Hippel 7V
    "hipc", // Hippel COSO
    "hst","sog", // Hippel ST
    "soc", // Hippel ST COSO
    "hd", // Howie Davis
    "hn","mtp2","thn","arp", // Major Tom / The Holy Noise
    "ims", // Images Music System
    "dum", // Infogrames / Rob Hubbard 2
    "is", // InStereo!
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

    "ymst", // YM-2149

    // converted to UADE compatible
    "med", // Music Editor MED4 -> MMD0
    
    "adpcm", // ADPCM (Mono)
    "ftm", // Face The Music
    "prt", // Pretracker
    "ptm", "mod3", // Protracker IFF (Protracker4)
    "mmd3", "oss", // Octamed SoundStudio / MMD3

    // non-UADE/native player
    "hvl", // HivelyTracker
    "dbm", // Digibooster Pro
    "ntk", // NoiseTrekker
    "ptk", // ProTrekkr

    // PC-tracker
    "ft", "fst", // Fasttracker 1
    "xm", // Fasttracker 2
    "stm", // Scream Tracker 2
    "s3m", // Scream Tracker 3
    "it", // Impulse Tracker 1 & 2

    // no replay
    //"ct", Cybertracker
    //"dsm", // Dynamic Studio Professional
    //"fuchs", // Fuchs Tracker
    //"dux", // GT Game Systems
    //"mxtx", // MaxTrax
    //"stp", // SoundTracker Pro II
    //"spm", // Stonetracker
    //"symmod", // Symphonie

    nullptr
};

}
