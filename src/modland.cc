// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <libaudcore/audstrings.h>
#include <libaudcore/multihash.h>
#include <libaudcore/runtime.h>

#include "common.h"
#include "modland.h"

#define LINE_MAX_ 1024
#define AUTHOR_MAX 128
#define FORMAT_MAX 128

#define COOP "coop-"
#define UNKNOWN "- unknown"
#define NOTBY "not by "

#define UNKNOWN_AUTHOR "<Unknown>"

String previous_md5_file("");
bool initialized = false;

SimpleHash<String,modland_data_t> ml_table;

int is_amiga_format(char * format) {
    int i = 0;
    const char *amiga_format;
    while ((amiga_format = modland_amiga_formats[i++]) != NULL) {
        switch (strncmp(format, amiga_format, FORMAT_MAX)) {
            case 0:
                return 1;
            case -1:
                return 0;
            default:
                continue;
        }
    }
    return 0;
}

int parse_modland_path(char *path, modland_data_t &item) {
    const char *sep = "/";
    char *format, author[AUTHOR_MAX], *album = NULL, *token;
    size_t count = 0;
    char *str = path;
    while(*str) if (*str++ == '/') ++count;

    if (count < 2) {
        DEBUG("Unexpected path: %s\n", path);
        return 1;
    }

    format = strtok(path, sep);
    if (!is_amiga_format(format)) {
        TRACE("Skipping format %s\n", format);
        return 1;
    }

    token = strtok(NULL, sep);

    strlcpy(author, token, sizeof(author));

    switch (count) {
        case 2:
            // nothing to do
            break;
        case 3:
            token = strtok(NULL, sep);
            if (!strncmp(COOP, token, strlen(COOP))) {
                strlcat(author, " & ", sizeof(author));
                strlcat(author, token + strlen(COOP), sizeof(author));
            } else if (strncmp(NOTBY, token, strlen(NOTBY))) {
                album = token;
            }
            break;
        case 4:
            token = strtok(NULL, sep);
            if (!strncmp(COOP, token, strlen(COOP))) {
                strlcat(author, " & ", sizeof(author));
                strlcat(author, token + strlen(COOP), sizeof(author));
            } else {
                WARN("Skipped path: %s\n", path);
                return 1;
            }
            album = strtok(NULL, sep);
            break;
        default:
            TRACE("Skipping path %s, token count %ld\n", path, count);
            break;
    }

    item.format = String(format);
    if (!strncmp(UNKNOWN, author, strlen(UNKNOWN))) {
        item.author = String(UNKNOWN_AUTHOR);
    } else {
        item.author = String(author);
    }
    if (album) {
        item.album = String(album);
    }

    return 0;
}

void try_init(void) {
    bool init_success = true;
    String md5_file = aud_get_str (PLUGIN_NAME, MODLAND_ALLMODS_MD5_FILE);

    if (!strnlen(md5_file, FILENAME_MAX)) {
        const char *home = getenv ("HOME");
        md5_file = String((std::string(home) + "/.uade/allmods_md5_amiga.txt").c_str());
    }

    char line[LINE_MAX_];

    if (!strncmp(previous_md5_file, md5_file, FILENAME_MAX)) {
        return;
    }

    DEBUG("Modland allmods_md5.txt location changed\n");

    previous_md5_file = md5_file;
    initialized = false;
    modland_cleanup();

    FILE *file = fopen(uri_to_filename(md5_file), "r");
    if (!file) {
        DEBUG("Could not open modland file %s\n", (const char *)md5_file);
        return;
    }

    while(fgets(line, LINE_MAX_ - 1, file)) {
        char md5[33];
        char path[LINE_MAX_];

        // sanity check
        if (strnlen(line, LINE_MAX_) <= 34) {
            ERROR("Too short line %s\n", line);
            init_success = false;
            goto out;
        }

        strlcpy(md5, line, sizeof(md5));

        if (ml_table.lookup(String(md5))) {
            DEBUG("Duplicate md5: %s", line);
            continue;
        }

        strlcpy(path, line + 33, sizeof(path));

        modland_data_t item = {};

        switch (parse_modland_path(path, item)) {
            case 0:
                ml_table.add(String(md5), std::move(item));
                break;
            case 1:
                continue;
            default:
                init_success = false;
                goto out;
        }

//        TRACE("%s -> format = %s, author = %s, album = %s\n", line, static_cast<const char*>(item->format), static_cast<const char*>(item->author), static_cast<const char*>(item->album));
    }

out:
    fclose(file);

    if (init_success) {
        initialized = true;
    } else {
        modland_cleanup();
    }

    return;
}

void modland_cleanup() {
    ml_table.clear();
}

modland_data_t *modland_lookup(const char *md5) {
    try_init();

    if (!initialized) {
        return NULL;
    }

    return ml_table.lookup(String(md5));
}
