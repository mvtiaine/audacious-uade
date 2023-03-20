// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <libgen.h>
#include <cstring>
#include <set>
#include <string>

#include <uade/uade.h>

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
        TRACE("amiga_loader_wrapper found file: %s\n", name);
    } else {
        TRACE("amiga_loader_wrapper NOT found file: %s\n", name);
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
    char buf[FILENAME_MAX];
    buf[0] = 0;

    for((prefix = strtok(filename, sep)) &&
        (middle = strtok(NULL, sep)) &&
        (suffix = strtok(NULL, sep));
        prefix && middle && suffix;
        suffix = strtok(NULL, sep)) {

        TRACE("tfmx_loader_wrapper prefix:%s middle:%s suffix:%s\n", prefix, middle, suffix);

        // change smpl.*.mdat to *.smpl
        if (!strncasecmp(prefix, "smpl", 4) && !strncasecmp(suffix, "mdat", 4)) {
            char new_filename[FILENAME_MAX];
            char *path = dirname((char *)name);
            snprintf(new_filename, sizeof(new_filename), "%s/%s.%s", path, middle, prefix);
            DEBUG("amiga_loader_wrapper changed %s to %s\n", filename, new_filename);
            return uade_load(new_filename, playerdir, state);
        }
        if (!buf[0]) {
            strlcat(buf, middle, sizeof(buf));
            middle = buf;
        }
        strlcat(buf, sep, sizeof(buf));
        strlcat(buf, suffix, sizeof(buf));
    }

    struct uade_file *amiga_file = uade_load(name, playerdir, state);

    // try set.smpl instead of smpl.set :P
    if (!amiga_file && !strncasecmp(prefix, "smpl", 4) && !strncasecmp(middle, "set", 4)){
        char new_filename[FILENAME_MAX];
        char *path = dirname((char *)name);
        snprintf(new_filename, sizeof(new_filename), "%s/set.smpl", path);
        DEBUG("amiga_loader_wrapper changed %s.%s to set.smpl\n", prefix, middle);
        amiga_file = uade_load(new_filename, playerdir, state);
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

    return uade_load(name, playerdir, state);
}

#ifdef __cplusplus
}
#endif