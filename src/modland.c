#include <string.h>

#include "common.h"

// TODO configuration, some validation
#define MD5_FILE "/Users/tundrah/allmods_md5_amiga.txt"
#define LINE_MAX 1024
#define AUTHOR_MAX 64

#define COOP "coop-"
#define UNKNOWN "- unknown"
#define NOTBY "not by "

int modland_init_md5_db(void) {
    int ret = 0;
    char line[LINE_MAX];
    FILE *file = fopen(MD5_FILE, "r");
    if (!file) {
        ERR("Could not open %s\n", MD5_FILE);
        return -1;
    }

    while(fgets(line, LINE_MAX, file)) {
        char md5[32];
        char path[LINE_MAX];
        char *sep = "/";
        char *format, author[AUTHOR_MAX], *game = NULL, *token;

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
            ERR("Unexpected line: %s\n", line);
            ret = -1;
            goto out;
        }

        format = strtok(path, sep);
        // TODO Ad Lib, Video Game Music secondary format
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
                    game = token;
                }
                break;
            case 4:
                token = strtok(NULL, sep);
                if (!strncmp(COOP, token, strlen(COOP))) {
                    strlcat(author, " and ", sizeof(author));
                    strlcat(author, token + strlen(COOP), sizeof(author));
                } else {
                    ERR("Unexpected line: %s\n", line);
                    continue;
                    /*
                    ret = -1;
                    goto out;
                    */
                }
                game = strtok(NULL, sep);
                break;
            default:
                DBG("Skipping line: %s\n", line);
                break;
        }
        DBG("%s -> format = %s, author = %s, game = %s\n", line, format, author, game);
    }

out:
    fclose(file);

    return ret;
}
