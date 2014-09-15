#include <stdlib.h>
#include <string.h>

#include <audacious/misc.h>
#include <libaudcore/multihash.h>

#include "common.h"
#include "modland.h"

#define LINE_MAX 1024
#define AUTHOR_MAX 128
#define FORMAT_MAX 128

#define COOP "coop-"
#define UNKNOWN "- unknown"
#define NOTBY "not by "

#define UNKNOWN_AUTHOR "<Unknown>"

char *previous_md5_file = "";
bool_t initialized = FALSE;

typedef struct {
    MultihashNode node;
    unsigned hash;
    char *md5;
    modland_data_t *item;
} MLNode;

static unsigned md5_hash(const char* md5) {
    unsigned hash = (unsigned) strtoll(md5 + 17, NULL, 16);
    return hash;
}

static unsigned hash_cb (const MultihashNode *node) {
    return md5_hash(((const MLNode *) node)->md5);
}

static bool_t match_cb (const MultihashNode *node_, const void *data, unsigned hash) {
    const MLNode *node = (const MLNode *) node_;
    return node->hash == hash && !strncmp(node->md5, data, 32);
}

static MultihashTable ml_table = {
    .hash_func = hash_cb,
    .match_func = match_cb
};

static MultihashNode * add_cb (const void *data, unsigned hash, void *state) {
    modland_data_t *item = (modland_data_t *)state;
    MLNode *node = calloc (1, sizeof(MLNode));
    node->hash = hash;
    node->md5 = (char *)data;
    node->item = item;

    return (MultihashNode *) node;
}

static bool_t ref_cb (MultihashNode *node_, void *state) {
    MLNode * node = (MLNode *) node_;

    *((modland_data_t * *) state) = node->item;
    return FALSE;
}

static bool_t cleanup_cb (MultihashNode *node_, void *state) {
    MLNode * node = (MLNode *) node_;

    str_unref(node->md5);
    str_unref(node->item->format);
    str_unref(node->item->author);
    if (node->item->album) {
        str_unref(node->item->album);
    }
    free(node->item);
    free(node);
    return TRUE;
}

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

int parse_modland_path(char *path, modland_data_t *item) {
    const char *sep = "/";
    char *format, author[AUTHOR_MAX], *album = NULL, *token;
    size_t count = 0;
    char *str = path;
    while(*str) if (*str++ == '/') ++count;

    if (count < 2) {
        ERROR("Unexpected path: %s\n", path);
        return -1;
    }

    format = strtok(path, sep);
    if (!is_amiga_format(format)) {
        TRACE("Skipping path %s\n", path);
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
            TRACE("Skipping path: %s\n", path);
            break;
    }

    item->format = str_get(format);
    if (!strncmp(UNKNOWN, author, strlen(UNKNOWN))) {
        item->author = str_get(UNKNOWN_AUTHOR);
    } else {
        item->author = str_get(author);
    }
    if (album) {
        item->album = str_get(album);
    }

    return 0;
}

modland_data_t *modland_internal_lookup(const char *md5) {
    modland_data_t *item = NULL;
    multihash_lookup (&ml_table, md5, md5_hash(md5), NULL, ref_cb, &item);
    return item;
}

void try_init(void) {
    bool_t init_success = TRUE;
    char *md5_file = aud_get_str (PLUGIN_NAME, MODLAND_ALLMODS_MD5_FILE);
    str_unref(md5_file);

    char line[LINE_MAX];

    if (!strncmp(previous_md5_file, md5_file, FILENAME_MAX)) {
        return;
    }

    DEBUG("Modland allmods_md5.txt location changed\n");

    previous_md5_file = md5_file;
    initialized = FALSE;
    modland_cleanup();

    if (!strnlen(md5_file, FILENAME_MAX)) {
        DEBUG("Modland allmods_md5.txt location not defined\n");
        return;
    }

    FILE *file = fopen(md5_file, "r");
    if (!file) {
        ERROR("Could not open modland file %s\n", md5_file);
        return;
    }

    while(fgets(line, LINE_MAX, file)) {
        char md5[33];
        char path[LINE_MAX];

        // sanity check
        if (strnlen(line, LINE_MAX) <= 34) {
            ERROR("Too short line %s\n", line);
            init_success = FALSE;
            goto out;
        }

        strlcpy(md5, line, sizeof(md5));

        if (modland_internal_lookup(md5)) {
            DEBUG("Duplicate md5: %s", line);
            continue;
        }

        strlcpy(path, line + 33, sizeof(path));

        modland_data_t *item = calloc(1, sizeof(modland_data_t));

        switch (parse_modland_path(path, item)) {
            case 0:
                multihash_lookup (&ml_table, str_get(md5), md5_hash(md5), add_cb, NULL, item);
                break;
            case 1:
                continue;
            default:
                init_success = FALSE;
                goto out;
        }

        TRACE("%s -> format = %s, author = %s, album = %s\n", line, item->format, item->author, item->album);
    }

out:
    fclose(file);

    if (init_success) {
        initialized = TRUE;
    } else {
        modland_cleanup();
    }

    return;
}

void modland_cleanup() {
    multihash_iterate (&ml_table, cleanup_cb, NULL);
}

modland_data_t *modland_lookup(const char *md5) {
    try_init();

    if (!initialized) {
        return NULL;
    }

    return modland_internal_lookup(md5);
}
