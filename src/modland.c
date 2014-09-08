#include <stdlib.h>
#include <string.h>

#include <libaudcore/multihash.h>

#include "common.h"
#include "modland.h"

// TODO configuration, some validation
#define MD5_FILE "/Users/tundrah/allmods_md5_amiga.txt"
#define LINE_MAX 1024
#define AUTHOR_MAX 64
#define FORMAT_MAX 64

#define COOP "coop-"
#define UNKNOWN "- unknown"
#define NOTBY "not by "

#define ADLIB "Ad Lib"
#define VIDEOGAMEMUSIC "Video Game Music"

typedef struct {
    MultihashNode node;
    unsigned hash;
    char *md5;
    modland_data_t *item;
} MLNode;

static unsigned md5_hash(const char* md5) {
    //DBG("Hash for %s -> %lld, %u\n", md5 + 17, strtoll(md5 + 17, NULL, 16), (unsigned)strtoll(md5 + 17, NULL, 16));
    return (unsigned) strtoll(md5 + 17, NULL, 16);
}

static unsigned hash_cb (const MultihashNode *node) {
    return md5_hash(((const MLNode *) node)->md5);
}

static bool_t match_cb (const MultihashNode *node_, const void *data, unsigned hash) {
    const MLNode *node = (const MLNode *) node_;
    //DBG("Matching %u == %u, %s == %s\n", node->hash, hash, node->md5, data);
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

    //DBG("Added hash: %u md5: %s\n", node->hash, node->md5);

    return (MultihashNode *) node;
}

static bool_t ref_cb (MultihashNode *node_, void *state) {
    MLNode * node = (MLNode *) node_;

    * ((modland_data_t * *) state) = node->item;
    return FALSE;
}

static bool_t cleanup_cb (MultihashNode *node_, void *state) {
    MLNode * node = (MLNode *) node_;
    //DBG("Cleaning up %s %s %s\n", node->md5, node->item->format, node->item->author);
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

modland_data_t *modland_lookup(const char *md5) {
    modland_data_t *item = NULL;
    //DBG("Looking up md5: %s hash: %u\n", md5, md5_hash(md5));
    multihash_lookup (&ml_table, md5, md5_hash(md5), NULL, ref_cb, &item);
    return item;
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

int modland_init(void) {
    int ret = 0;
    char line[LINE_MAX];
    FILE *file = fopen(MD5_FILE, "r");
    if (!file) {
        ERR("Could not open %s\n", MD5_FILE);
        return -1;
    }

    while(fgets(line, LINE_MAX, file)) {
        char md5[33];
        char path[LINE_MAX];
        char *sep = "/";
        char *format, author[AUTHOR_MAX], *album = NULL, *token;

        // sanity check
        if (strnlen(line, LINE_MAX) <= 34) {
            ERR("Too short line %s\n", line);
            ret = -1;
            goto out;
        }

        strlcpy(md5, line, sizeof(md5));
        strlcpy(path, line + 33, sizeof(path));

        size_t count = 0;
        char *str = path;
        while(*str) if (*str++ == '/') ++count;

        if (count < 2) {
            ERR("Unexpected line: %s", line);
            ret = -1;
            goto out;
        }

        format = strtok(path, sep);
        if (!is_amiga_format(format)) {
            //DBG("Skipping line %s\n", line);
            continue;
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
                    strlcat(author, " and ", sizeof(author));
                    strlcat(author, token + strlen(COOP), sizeof(author));
                } else if (strncmp(NOTBY, token, strlen(NOTBY))) {
                    album = token;
                }
                break;
            case 4:
                token = strtok(NULL, sep);
                if (!strncmp(COOP, token, strlen(COOP))) {
                    strlcat(author, " and ", sizeof(author));
                    strlcat(author, token + strlen(COOP), sizeof(author));
                } else {
                    WRN("Skipped line: %s", line);
                    continue;
                    /*
                    ret = -1;
                    goto out;
                    */
                }
                album = strtok(NULL, sep);
                break;
            default:
                //DBG("Skipping line: %s\n", line);
                break;
        }

        if (modland_lookup(md5)) {
            WRN("Duplicate md5: %s", line);
            continue;
        }

        modland_data_t *item = calloc(1, sizeof(modland_data_t));
        item->format = str_get(format);
        item->author = str_get(author);
        if (album) {
            item->album = str_get(album);
        }

        multihash_lookup (&ml_table, str_get(md5), md5_hash(md5), add_cb, NULL, item);

        //DBG("%s -> format = %s, author = %s, album = %s, hash:%u\n", line, format, author, album, md5_hash(md5));
    }

out:
    fclose(file);

    return ret;
}

void modland_cleanup() {
    multihash_iterate (&ml_table, cleanup_cb, NULL);
}
