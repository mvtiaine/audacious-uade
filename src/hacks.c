/*
    Copyright (C) 2014  Matti Tiainen <mvtiaine@cc.hut.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <libgen.h>
#include <stdio.h>
#include <strings.h>

#include <libaudcore/core.h>

#include <uade/uade.h>

#include "common.h"

// modland extensions blacklist
static const char * const extension_blacklist[] = {
    // PC trackers
    ".ft", // Fast Tracker
    // No support
    ".mmd3", // Octamed SoundStudio
    // Not amiga?
    ".ym", // YM
    NULL
};

bool_t is_blacklisted_extension(const char *ext) {
    int i;
    for (i = 0; extension_blacklist[i]; ++i) {
        if (!strncasecmp(extension_blacklist[i], ext, FILENAME_MAX)) {
            return TRUE;
        }
    }
    return FALSE;
}

// OctaMED sets <no songtitle> or similar as the modulename if there's no title given
static const char * const octamed_title_blacklist[] = {
    "<no songtitle>",
    "<sans titre>",
    "<ohne Namen>",
    "<unnamed>",
    NULL
};

bool_t is_blacklisted_title(const struct uade_song_info *info) {
    bool_t is_octamed = !strncmp("type: MMD0", info->formatname, 10) ||
                        !strncmp("type: MMD1", info->formatname, 10) ||
                        !strncmp("type: MMD2", info->formatname, 10);
    if (is_octamed) {
        int i;
        // check if extension is blacklisted
        for (i = 0; octamed_title_blacklist[i]; ++i) {
            if (!strncmp(octamed_title_blacklist[i], info->modulename, FILENAME_MAX)) {
                DEBUG("Blacklisted title %s\n", info->modulename);
                return TRUE;
            }
        }
    }
    return FALSE;
}

// hack to work around modland TFMX files using a suffix
// which causes them not to play due to not finding the sample file
struct uade_file *amiga_loader_wrapper(const char *name, const char *playerdir, void *context, struct uade_state *state) {
    TRACE("amiga_loader_wrapper name:%s playerdir:%s\n", name, playerdir);

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

        TRACE("prefix:%s middle:%s suffix:%s\n", prefix, middle, suffix);

        // change smpl.*.mdat to *.smpl
        if (!strncasecmp(prefix, "smpl", 4) && !strncasecmp(suffix, "mdat", 4)) {
            char new_filename[FILENAME_MAX];
            char *path = dirname((char *)name);
            snprintf(new_filename, sizeof(new_filename), "%s/%s.%s", path, middle, prefix);
            DEBUG("amiga_loader_wrapper changed %s to %s\n", filename, new_filename);
            return uade_load_amiga_file(new_filename, playerdir, state);
        }
        if (!buf[0]) {
            strlcat(buf, middle, sizeof(buf));
            middle = buf;
        }
        strlcat(buf, sep, sizeof(buf));
        strlcat(buf, suffix, sizeof(buf));
    }

    struct uade_file *amiga_file = uade_load_amiga_file(name, playerdir, state);

    // try set.smpl instead of smpl.set :P
    if (!amiga_file && !strncasecmp(prefix, "smpl", 4) && !strncasecmp(middle, "set", 4)){
        char new_filename[FILENAME_MAX];
        char *path = dirname((char *)name);
        snprintf(new_filename, sizeof(new_filename), "%s/set.smpl", path);
        DEBUG("amiga_loader_wrapper changed %s.%s to set.smpl\n", prefix, middle);
        amiga_file = uade_load_amiga_file(new_filename, playerdir, state);
    }

    return amiga_file;
}
