// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <algorithm>
#include <set>
#include <string>

#include "common/extensions.h"
#include "common/logger.h"
#include "common/strings.h"
#include "songdb/songdb.h"

using namespace std;

namespace {

const set<string> extensions = []() {
    set<string> exts;
    for (const auto ext : common::plugin_extensions) {
        if (ext) {
            exts.insert(ext);
        }
    }
    return exts;
}();

// modland extensions blacklist
const set<string> extension_blacklist ({
    // No support
    ".ct", // Cybertracker
    ".dsm", // Dynamic Studio Professional
    ".fuchs", // Fuchs Tracker
    ".dux", // GT Game Systems
    ".mxtx", // MaxTrax
    ".stp", // SoundTracker Pro II
    ".spm", // Stonetracker
    ".symmod", // Symphonie
    // Not amiga
    ".ym", // YM
    // Sample etc. files
    ".instr",".x",".set",".ins",".nt",".as",".ip",".l",".n",".ssd",".sps",".smp",".smpl",
    // aminet etc. hack to speedup playlist population with content based detection
    ".lha",".lzh",".lzx",".zip",".adf",".adz",".dms",".bak",".tar",".gz",".tgz",".bz2",".pp",
    ".gif",".png",".jpg",".jpeg",".bmp",".tga",".ilbm",".anim",".pic",".gfx",".pbm",".raw",".pcx",".3ds",".lbm",
    ".info",".ico",".font",".fon",".lvl",".map",".inf",
    ".exe",".library",".device",".rexx",".prefs",".ini",".dll",".prg",".com",".debug",".cfg",".conf",".rev",".cmd",
    ".h",".hpp",".c",".s",".asm",".cpp",".cxx",".cc",".o",".obj",".po",".in",".am",".sh",".amos",".bas",".i",".a",".fd",
    ".txt",".readme",".rea",".guide",".diz",".nfo",".doc",".htm",".html",".hlp",".1st",".xml",
    ".catalog",".deutsch",".english",
    ".findlist", // Mods Anthology (hangs uade)
});

// prefix blacklist
const set<string> prefix_blacklist ({
    "669.",
    "AMF.",
    "AMS.",
    "DMF.",
    "DTM.",
    "FAR.",
    "GT2.", // atari(?)
    "MDL.",
    "MPTM.",
    "MT2.",
    "MTM.",
    "OCT.", // atari
    "PLM.",
    "STP.", // amiga
    "STP2.", // amiga
    "ULT.",
    "SMP.",
    "SMPL.",
    "smp.",
    "smpl.",
    "readme.",
    "README.",
});

// uade_play() or uade_stop() stuck, may leave zombie uadecore process around
const set<string> md5_blacklist ({
    "39ba477d233f9191a6fa9daf121a8c4a", // Delitracker Custom/Geoff Phillips/spy vs spy.cus
    "fcdafb42d12055eaca7943e0cdf5c58f", // Delitracker Custom/Geoff Phillips/spy vs spy - arctic antics.cus
    "20e6ca8777dc2cde7b2300388456b2c2", // Delitracker Custom/Stephan Wenzler/gravity-force.cus
    "bf1c23d06d95623060dd72fed7bc6f41", // Oktalyzer/- unknown/freestyle.okta
    "c30c27e6a0a32e10b5799d5566350f48", // Oktalyzer/Michael Tschogl/never ending story ii-unused.okta
    "54f3416311a9554e15a7cf35aafd2de9", // Oktalyzer/Mohr/1 love night dub.okta
    "4dba6e062959c23e3e893dee0ed551bb", // Protracker/Doh/drd is drd.mod
    "142c4d303e1b50a38a97423dc157636d", // Protracker/Gryzor/tbc-87 speed dance.mod
    "65ab9627534237f48555094292cddd15", // Ron Klaren/Ron Klaren/electricity.rk
    "bce1efa7c8811ab129b82f5543cc3856", // Soundtracker 2.6/Starbuck/test.mod
    "55d01a06206a97c4449fda17480ab943", // wantedteam/examples/TFMX_Anstoss/MDAT.anstoss3
    "944efba8363e4a2e9a6c96eae008f5ea", // wantedteam/examples/TFMX_Anstoss/MDAT.anstoss4 
    "0a14bdf6e11a16dd447ec2fa160fe3ae", // wantedteam/examples/TFMX_MrNutz/MDAT.title
    "c9373a252335a86c020c7a2524bf4908", // amp/H/Hardsequenzer/OKT.PartyInvitation
    "d35ab7d004eda2ff8c3c45ca1041be82", // amp/C/Curt Cool2/PRT.wigged
});

const set<string> songdb_blacklist ({
    // The main files have same MD5, but load different data files depending on file name :P
    // Mark Cooksey Old/Mark Cooksey/mcr.aquablast
    // Mark Cooksey Old/Mark Cooksey/mcr.mike reads comp pop quiz
    "099d60f22e44566fb45dea0ae70b0456",
});

} // namespace {}

namespace songdb::blacklist {

bool is_blacklisted_extension(const string &path, const string &ext) noexcept {
    string filename = common::split(path, "/").back();
    string lcfilename = filename;
    string lcext = ext;
    transform(lcfilename.begin(), lcfilename.end(), lcfilename.begin(), ::tolower);
    transform(lcext.begin(), lcext.end(), lcext.begin(), ::tolower);

    if (lcext.size() > 1 && extensions.count(lcext.substr(1))) {
        return false;
    }

    string lcprefix = common::split(lcfilename, ".").front();
    if (extensions.count(lcprefix)) {
        return false;
    }
    
    if (extension_blacklist.count(lcext)) {
        TRACE("Blacklisted extension %s for %s\n", lcext.c_str(), path.c_str());
        return true;
    }    
    // AMP hack to speedup playlist population
    for (const auto &prefix : prefix_blacklist) {
        if (common::starts_with(filename, prefix)) {
            TRACE("Blacklisted prefix %s for %s\n", prefix.c_str(), path.c_str());
            return true;
        }
    }

    return false;
}

bool is_blacklisted_md5(const string &md5hex) noexcept {
    const bool blacklisted = md5_blacklist.count(md5hex);
    if (blacklisted) {
        DEBUG("Blacklisted md5 %s\n", md5hex.c_str());
    }
    return blacklisted;
}

bool is_blacklisted_songdb_key(const string &md5hex) noexcept {
    const bool blacklisted = songdb_blacklist.count(md5hex);
    if (blacklisted) {
        DEBUG("Blacklisted songdb md5 %s\n", md5hex.c_str());
    }
    return blacklisted;
}

} // namespace songdb::blacklist
