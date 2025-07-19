// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2014-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <algorithm>
#include <set>
#include <string>

#include "common/logger.h"
#include "common/strings.h"
#include "songdb/songdb.h"

using namespace std;

namespace {

// modland extensions blacklist
const set<string> extension_blacklist ({
    // No support
    ".ct", // Cybertracker
    ".fuchs", // Fuchs Tracker
    ".dux", // GT Game Systems
    ".mxtx", // MaxTrax
    ".spm", // Stonetracker
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
    "SMP.",
    "SMPL.",
    "smp.",
    "smpl.",
    "readme.",
    "README.",
});

// uade_play() or uade_stop() stuck, may leave zombie uadecore process around
const set<string> hash_blacklist ({
    "e2e642be13fc", // "39ba477d233f9191a6fa9daf121a8c4a", // Delitracker Custom/Geoff Phillips/spy vs spy.cus
    "c12c2d131bcc", // "fcdafb42d12055eaca7943e0cdf5c58f", // Delitracker Custom/Geoff Phillips/spy vs spy - arctic antics.cus
    "69364c10fdf0", // "20e6ca8777dc2cde7b2300388456b2c2", // Delitracker Custom/Stephan Wenzler/gravity-force.cus
    "17c754bfa826", // "bf1c23d06d95623060dd72fed7bc6f41", // Oktalyzer/- unknown/freestyle.okta
    "58b790471c40", // "c30c27e6a0a32e10b5799d5566350f48", // Oktalyzer/Michael Tschogl/never ending story ii-unused.okta
    "15f2724de000", // "54f3416311a9554e15a7cf35aafd2de9", // Oktalyzer/Mohr/1 love night dub.okta
    "82247f6a6da4", // "4dba6e062959c23e3e893dee0ed551bb", // Protracker/Doh/drd is drd.mod
    "75d3a18b8752", // "142c4d303e1b50a38a97423dc157636d", // Protracker/Gryzor/tbc-87 speed dance.mod
    "4efe29ee2260", // "65ab9627534237f48555094292cddd15", // Ron Klaren/Ron Klaren/electricity.rk
    "c51668924406", // "bce1efa7c8811ab129b82f5543cc3856", // Soundtracker 2.6/Starbuck/test.mod
    "fc28103f0958", // "55d01a06206a97c4449fda17480ab943", // wantedteam/examples/TFMX_Anstoss/MDAT.anstoss3
    "beb7e0c00b50", // "944efba8363e4a2e9a6c96eae008f5ea", // wantedteam/examples/TFMX_Anstoss/MDAT.anstoss4 
    "b8d4b40c16f0", // "0a14bdf6e11a16dd447ec2fa160fe3ae", // wantedteam/examples/TFMX_MrNutz/MDAT.title
    "9d433b215e96", // "c9373a252335a86c020c7a2524bf4908", // amp/H/Hardsequenzer/OKT.PartyInvitation
    "064d9cb220c8", // "d35ab7d004eda2ff8c3c45ca1041be82", // amp/C/Curt Cool2/PRT.wigged
});

const set<string> songdb_blacklist ({
    // The main files have same MD5, but load different data files depending on file name :P
    // Mark Cooksey Old/Mark Cooksey/mcr.aquablast
    // Mark Cooksey Old/Mark Cooksey/mcr.mike reads comp pop quiz
    "af58e712052c", // "099d60f22e44566fb45dea0ae70b0456",
});

} // namespace {}

namespace songdb::blacklist {

bool is_blacklisted_extension(const string &path, const string &ext, const set<string> &whitelist) noexcept {
    string filename = common::split(path, "/").back();
    string lcfilename = filename;
    string lcext = ext;
    transform(lcfilename.begin(), lcfilename.end(), lcfilename.begin(), ::tolower);
    transform(lcext.begin(), lcext.end(), lcext.begin(), ::tolower);

    if (lcext.size() > 1 && whitelist.count(lcext.substr(1))) {
        return false;
    }

    string lcprefix = common::split(lcfilename, ".").front();
    if (whitelist.count(lcprefix)) {
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

bool is_blacklisted_hash(const string &hash) noexcept {
    const bool blacklisted = hash_blacklist.count(hash);
    if (blacklisted) {
        DEBUG("Blacklisted hash %s\n", hash.c_str());
    }
    return blacklisted;
}

bool is_blacklisted_songdb_hash(const string &hash) noexcept {
    const bool blacklisted = songdb_blacklist.count(hash);
    if (blacklisted) {
        DEBUG("Blacklisted songdb hash %s\n", hash.c_str());
    }
    return blacklisted;
}

} // namespace songdb::blacklist
