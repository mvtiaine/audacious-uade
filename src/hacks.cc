// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <libgen.h>
#include <sys/stat.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
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

const set<string> md5_blacklist ({
    // uade_play() or uade_stop() stuck
    "bf1c23d06d95623060dd72fed7bc6f41", // Oktalyzer/- unknown/freestyle.okta
    "c30c27e6a0a32e10b5799d5566350f48", // Oktalyzer/Michael Tschogl/never ending story ii-unused.okta
    "54f3416311a9554e15a7cf35aafd2de9", // Oktalyzer/Mohr/1 love night dub.okta
    "142c4d303e1b50a38a97423dc157636d", // Protracker/Gryzor/tbc-87 speed dance.mod
    "bce1efa7c8811ab129b82f5543cc3856" // Soundtracker 2.6/Starbuck/test.mod
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
    constexpr int MAX_PARENTS = 4u;
    if (!amiga_file) {
        vector<string> stokens = split(name, "/");
        if (stokens.size() >= 2) {
            const struct uade_song_info *info = uade_get_song_info(state);
            vector<string> mtokens = split(info->modulefname, "/");
            // try from parents with last two token (assume it's sampledir/filename)
            string samplepath = stokens[stokens.size() - 2] + "/" + stokens[stokens.size() - 1];
            auto count = 0u;
            while (!amiga_file && count++ < MAX_PARENTS && count < mtokens.size() - 2) {
                string parent;
                for_each(mtokens.begin(), mtokens.end() - 1 - count, [&parent](const string& token) {
                    parent += "/" + token;
                });
                amiga_file = uade_load((parent + "/" + samplepath).c_str(), playerdir, state);
            }
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
