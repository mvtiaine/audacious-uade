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
#ifdef SYSTEM_LIBUADE
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
    ".prt", // Pretracker
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

// Ignore some files which hang UADE or Audacious playlist completely when trying to add
const set<string> filename_blacklist ({
    // uade_play() or uade_stop() stuck
    "freestyle.okta", // Oktalyzer/- unknown
    "never ending story ii-unused.okta", // Oktalyzer/Michael Tschogl
    "1 love night dub.okta", // Oktalyzer/Mohr
    "tbc-87 speed dance.mod", // Protracker/Gryzor
    "electricity.rk", // Ron Klaren/Ron Klaren
    "test.mod" // Soundtracker 2.6/Starbuck
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
    constexpr int MAX_PARENTS = 4;
    if (!amiga_file) {
        vector<string> tokens = split(name, "/");
        // try from parents with last two token (assume it's sampledir/filename)
        string samplepath = tokens[tokens.size() - 2] + "/" + tokens[tokens.size() - 1];
        int count = 0;
        while (!amiga_file && count++ < MAX_PARENTS) {
            string parent;
            for_each(tokens.begin(), tokens.end() - 2 - count, [&parent](const string& token) {
                parent += "/" + token;
            });
            amiga_file = uade_load((parent + "/" + samplepath).c_str(), playerdir, state);
        }
    }
    if (!amiga_file) {
        ERROR("sample_loader_wrapper could NOT find file: %s\n", name);
    }
    return amiga_file;
}

// hack to work around modland TFMX files using a suffix
// which causes them not to play due to not finding the sample file
// should be obsolete
struct uade_file *tfmx_loader_wrapper(const char *name, const char *playerdir, void *context, struct uade_state *state) {
    TRACE("tfmx_loader_wrapper file: %s\n", name);

    char *filename = basename((char *)name);
    const char *sep = ".";
    char *prefix;
    char *middle;
    char *suffix;
    char buf[PATH_MAX];
    buf[0] = 0;

    for((prefix = strtok(filename, sep)) &&
        (middle = strtok(nullptr, sep)) &&
        (suffix = strtok(nullptr, sep));
        prefix && middle && suffix;
        suffix = strtok(nullptr, sep)) {

        TRACE("tfmx_loader_wrapper prefix:%s middle:%s suffix:%s\n", prefix, middle, suffix);

        // change smpl.*.mdat to *.smpl
        if (!strncasecmp(prefix, "smpl", 4) && !strncasecmp(suffix, "mdat", 4)) {
            char new_filename[PATH_MAX + 3];
            char *path = dirname((char *)name);
            snprintf(new_filename, sizeof(new_filename), "%s/%s.%s", path, middle, prefix);
            DEBUG("amiga_loader_wrapper changed %s to %s\n", filename, new_filename);
            return uade_load(new_filename, playerdir, state);
        }
        if (!buf[0]) {
            strncat(buf, middle, sizeof(buf) - strlen(buf) - 1);
            middle = buf;
        }
        strncat(buf, sep, sizeof(buf) - strlen(buf) - 1);
        strncat(buf, suffix, sizeof(buf) - strlen(buf) - 1);
    }

    struct uade_file *amiga_file = uade_load(name, playerdir, state);

    // try set.smpl instead of smpl.set :P
    if (!amiga_file && prefix && middle && !strncasecmp(prefix, "smpl", 4) && !strncasecmp(middle, "set", 4)){
        char new_filename[PATH_MAX];
        char *path = dirname((char *)name);
        snprintf(new_filename, sizeof(new_filename), "%s/set.smpl", path);
        DEBUG("amiga_loader_wrapper changed %s.%s to set.smpl\n", prefix, middle);
        amiga_file = uade_load(new_filename, playerdir, state);
    }

    if (!amiga_file) {
        ERROR("tfmx_loader_wrapper could NOT find file: %s\n", name);
    }
    return amiga_file;
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
    const string formatname = info->formatname;
    const bool is_octamed = formatname.find("type: MMD0") == 0 ||
                            formatname.find("type: MMD1") == 0 ||
                            formatname.find("type: MMD2") == 0;
    if (is_octamed) {
        const string modulename = info->modulename;
        const bool blacklisted = octamed_title_blacklist.count(modulename);
        if (blacklisted) {
            DEBUG("Blacklisted title %s\n", modulename.c_str());
        }
        return blacklisted;
    }
    return false;
}

bool is_blacklisted_filename(const string &name) {
    const bool blacklisted = filename_blacklist.count(name);
    if (blacklisted) {
        WARN("Blacklisted filename %s\n", name.c_str());
    }
    return blacklisted;
}

#ifdef __cplusplus
extern "C"
{
#endif

struct uade_file *amiga_loader_wrapper(const char *name, const char *playerdir, void *context, struct uade_state *state) {
    const struct uade_song_info *info = uade_get_song_info(state);
    const string player = info->playerfname;
    TRACE("amiga_loader_wrapper name: %s player: %s\n", name, player.c_str());

    // should be obsolete hack
    if (player.find("/TFMX") != player.npos) {
        return tfmx_loader_wrapper(name, playerdir, context, state);
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

#ifdef __cplusplus
}
#endif
