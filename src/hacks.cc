// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <libgen.h>
#include <sys/stat.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "config.h"
#if SYSTEM_LIBUADE
#include <uade/uade.h>
#else
#include "../uade/src/frontends/include/uade/options.h"
#include "../uade/src/frontends/include/uade/uadeconfstructure.h"
#include "../uade/src/frontends/include/uade/uade.h"
#endif

#include "common.h"

using namespace std;

namespace {

// modland extensions blacklist
const set<string> extension_blacklist ({
    // non-Amiga tracker, avoid using UADE
    ".ft", // Fast Tracker
    // No support
    ".ct", // Cybertracker
    ".dbm", // Digibooster Pro
    ".dsm", // Dynamic Studio Professional
    ".ftm", // Face The Music
    ".fuchs", // Fuchs Tracker
    ".dux", // GT Game Systems
    ".hvl", // HivelyTracker
    ".mxtx", // MaxTrax
    ".mmd3", // Octamed SoundStudio
    ".ptm", // Protracker IFF
    ".stp", // SoundTracker Pro II
    ".spm", // Stonetracker
    ".symmod", // Symphonie
    // Not amiga?
    ".ym" // YM
});

// OctaMED sets <no songtitle> or similar as the modulename if there's no title given
const set<string> octamed_title_blacklist ({
    "<no songtitle>",
    "<sans titre>",
    "<ohne Namen>",
    "<unnamed>"
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
});

const set<string> songdb_blacklist ({
    // The main files have same MD5, but load different data files depending on file name :P
    // Mark Cooksey Old/Mark Cooksey/mcr.aquablast
    // Mark Cooksey Old/Mark Cooksey/mcr.mike reads comp pop quiz
    "099d60f22e44566fb45dea0ae70b0456",
});

// Modland hack for missing WantedTeam.bin files
const map<pair<string,string>, string> bundled_extfiles ({
    {{"1ad8cd8a8e799fe7f1f27e00f925d9e0","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/GeorgGlaxo/WantedTeam.bin"},
    {{"81b641316c9af33154bea71eb97fa916","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/GeorgGlaxo/WantedTeam.bin"},
    {{"a33a4ef23f43543ae09ee4fecc4a0a61","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/GeorgGlaxo/WantedTeam.bin"},
    {{"153c555fdc2c72e5a62e21c131f4b635","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"159581910f064311d7fc72b8ccf29ab9","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"19ed56bde9033c6fb1c88731ce4fd2d1","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"23aa0bce4f4e6293ad6565eda2c78347","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"283a7612f6afc3a54666423bdfccb594","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"301a523cb5686a4226fe0a62f4820779","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"37006b2614276015f32f866565bf2ea2","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"3ecf6d705b44906eaecf7630c05751a0","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"3f897211dc0f17c2cc13db4c3698de78","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"43be812e51102ff8e76424b9c37614f9","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"478bf4b89695454d067e34e05a22cacc","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"4a08b49732bfb2f2f2acd01b2c15dde3","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"5254492b6ca356a8c81054befaa0b1db","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"53f7ebaa4edf178c31d214f928d75934","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"557550edf2c96732bc992785f9907a15","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"55f682366163f34b528f9b7c709b64a8","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"59aaf90869419c55ac9e26cc6c0a014e","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"6b13749109816261295ef3c08dee35f4","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"7187fca3ce63d75bc08a5fea917c3b6e","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"7c2a31d9c62154a7a85a4f7e0bf17b06","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"7d8082ab2b8bf21586712591ad8213d5","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"7dc142146e6946a6a0e61af40ee762b7","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"7f119629fc9a51d6e78e8ae7662d62b8","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"80b7a1e3039d808aaca66f78d49c1f41","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"854201a7ceb6e497ffd6c742dd8faf54","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"859c834231a3efa428ac5c49ceb42481","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"884f0124a90a1b7f805af8df61b47f5d","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"8de8e2c0e1d23edd6d84bfdfcf7efb26","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"998c371cecbb9b2592ec110298258295","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"a0b78c1b40738913f370019034090543","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"a29d573748bcdee158d462556ce1b79b","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"ac6ee85f36542340f2b0dc8300938439","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"b069ee530330bfde8346a98e7a7c2619","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"bbb1f74fbdcbb36be6a768251807086b","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"c61bfb3bc7e04c593dcd1b45689c7ceb","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"c9b430c12ba35598f8e51a569a1b510d","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"cd64ac2c1892666c85f65600ccfa181d","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"d84d3fe127fd9980f7a4c7a91db407c5","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"dc099b0c587c9e92c03e2ba93076c49d","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"e18c7366eadcf2e171b39816918408f0","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"ef9c33a9ecb0e14c68a09a44f08b6616","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"fe771473508f0efcb23dcacc1eb85fe9","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"607fa8d7a49c0b373580fd629acd40e8","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/AncientArtOfWar/WantedTeam.bin"},
    {{"7795cf2a65a25a099f509640087dac15","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/AncientArtOfWar/WantedTeam.bin"},
    {{"19435767e2bf68b6c04941385d505745","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/AncientArtOfWar/WantedTeam.bin"},
    {{"12178bce42c9e8702881c14993789163","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/AncientArtOfWar/WantedTeam.bin"},
    {{"1e35824e46acb0f89e25ab8d01d73a30","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/AncientArtOfWar/WantedTeam.bin"},
    {{"5ef88969b20d52d9408c3d78d604cd1e","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/AncientArtOfWar/WantedTeam.bin"},
    {{"a52948cd3d62978a6b9570ae21b7401d","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/AncientArtOfWar/WantedTeam.bin"},
    {{"f9c3e244ada1a0ca701211a2cefa26b0","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/AncientArtOfWar/WantedTeam.bin"},
    {{"a717edf8c6257e3d2d0a0d27b9798616","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/AncientArtOfWar/WantedTeam.bin"},
    {{"2ec0c17d505d95da48fdf8a7728a46e2","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/Dogfight/WantedTeam.bin"},
    {{"61d746d9fa2766b803e58ac7766c3f6d","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/Dogfight/WantedTeam.bin"},
    {{"7ee7a0176b92e62ebc29cc1506b48434","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/Dogfight/WantedTeam.bin"},
    {{"cf8e54284a53a44f83bd28a17ecbe84e","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/Dogfight/WantedTeam.bin"},
});

struct uade_file *uade_load(const char *name, const char*playerdir, struct uade_state *state) {
    struct uade_file *amiga_file = uade_load_amiga_file(name, playerdir, state);
    if (amiga_file) {
        DEBUG("amiga_loader_wrapper found file: %s\n", name);
    }
    return amiga_file;
}

struct uade_file *sample_loader_wrapper(const char *name, const char *playerdir, void *context, struct uade_state *state) {
    struct uade_file *amiga_file = uade_load(name, playerdir, state);
    if (!amiga_file) {
        vector<string> stokens = split(name, "/");
        if (stokens.size() >= 3 && stokens[0] != ".") {
            const string parent = stokens[stokens.size() - 3] + "/";
            const string sample = stokens[stokens.size() - 2] + "/" + stokens[stokens.size() - 1];
            string newpath = name;
            newpath.replace(newpath.find(parent + sample), (parent + sample).size(), sample);
            amiga_file = uade_load(newpath.c_str(), playerdir, state);

        } else if ((stokens.size() == 3 && stokens[0] == ".") || stokens.size() == 2) {
            const string samplepath = stokens[stokens.size() - 2] + "/" + stokens[stokens.size() - 1];
            amiga_file = uade_load(("../"+samplepath).c_str(), playerdir, state);
        }
    }
    if (!amiga_file) {
        ERROR("sample_loader_wrapper could NOT find file: %s\n", name);
    }
    return amiga_file;
}

bool is_octamed(const struct uade_song_info *info) {
    const string formatname = info->formatname;
    return formatname.find("type: MMD0") == 0 ||
           formatname.find("type: MMD1") == 0 ||
           formatname.find("type: MMD2") == 0;
}

bool is_vss(const struct uade_song_info *info) {
    const string playername = info->playername;
    return playername == "VSS";
}

} // namespace

bool is_blacklisted_extension(const string &ext) {
    const bool blacklisted = extension_blacklist.count(ext);
    if (blacklisted) {
        DEBUG("Blacklisted extension %s\n", ext.c_str());
    }
    return blacklisted;
}

bool is_blacklisted_title(const struct uade_song_info *info) {
    if (is_octamed(info)) {
        const string modulename = info->modulename;
        const bool blacklisted = octamed_title_blacklist.count(modulename);
        if (blacklisted) {
            DEBUG("Blacklisted title %s\n", modulename.c_str());
        }
        return blacklisted;
    }
    return false;
}

bool is_blacklisted_md5(const string &md5hex) {
    const bool blacklisted = md5_blacklist.count(md5hex);
    if (blacklisted) {
        WARN("Blacklisted md5 %s\n", md5hex.c_str());
    }
    return blacklisted;
}

bool is_blacklisted_songdb(const string &md5hex) {
    const bool blacklisted = songdb_blacklist.count(md5hex);
    if (blacklisted) {
        DEBUG("Blacklisted songdb md5 %s\n", md5hex.c_str());
    }
    return blacklisted;
}

bool allow_songend_error(const struct uade_song_info *info) {
    return is_octamed(info) || is_vss(info);
}

#if __cplusplus
extern "C"
{
#endif

struct uade_file *amiga_loader_wrapper(const char *name, const char *playerdir, void *context, struct uade_state *state) {
    const struct uade_song_info *info = uade_get_song_info(state);
    const string player = info->playerfname;
    TRACE("amiga_loader_wrapper name: %s player: %s\n", name, player.c_str());

    const string fname = split(name, "/").back();
    const auto key = pair(string(info->modulemd5),fname);
    if (bundled_extfiles.count(key)) {
        const auto bundled = bundled_extfiles.at(key);
        TRACE("amiga_loader_wrapper loading bundled extfile for %s bundled: %s\n", name, bundled.c_str());
        return uade_load(bundled.c_str(), playerdir, state);
    }

    if (player.find("/ZoundMonitor") != player.npos) {
        return sample_loader_wrapper(name, playerdir, context, state);
    }

    // for example cust.zoids (aka zoids.src) and other SunTronic customs will first request instr/ directory (for listing?)
    // just return empty file for directories to make them happy
    struct stat s;
    if (stat(name,&s) == 0 && s.st_mode & S_IFDIR) {
        DEBUG("amiga_loader_wrapper returning dummy file for directory %s\n", name);
        struct uade_file *dummyfile = (struct uade_file *)malloc(sizeof(struct uade_file));
        dummyfile->name = nullptr;
        dummyfile->data = nullptr;
        dummyfile->size = 0;
        return dummyfile;
    }

    return uade_load(name, playerdir, state);
}

#if __cplusplus
}
#endif
