// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <vector>

#include "common/strings.h"
#include "songdb/songdb.h"
#include "3rdparty/xxhash/xxhash.h"

#include <sys/stat.h>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "File not given\n");
        return EXIT_FAILURE;
    }
    const char *fname = argv[1];
    FILE *f = fopen(fname, "rb"); 
    if (!f) {
        fprintf(stderr, "File not found: %s\n", fname);
        return EXIT_FAILURE;
    }
    int fd = fileno(f);

    struct stat st;
    if (fstat(fd, &st)) {
        close(fd);
        fprintf(stderr, "Failed to read file size for %s\n", fname);
        return EXIT_FAILURE;
    }

    uint8_t buf[4096];
    vector<char> buffer;
    const ssize_t bytes = min(songdb::XXH_MAX_BYTES, (size_t)st.st_size);
    buffer.reserve(bytes);

    ssize_t count;
    while ((count = read(fd, buf, sizeof buf)) > 0 && count < bytes) {
        buffer.insert(buffer.end(), buf, buf + count);
    }
    close(fd);

    const auto xxh32 = XXH32(buffer.data(), bytes, 0);
    const string hash = common::to_hex(xxh32) + common::to_hex((uint16_t)(st.st_size & 0xFFFF));

#ifdef SONGDB_DIR
    const char *songdbdir = SONGDB_DIR;
#else
    const char *songdbdir = getenv("SONGDB_DIR");
    if (!songdbdir) {
        songdbdir = UADEDIR "/songdb";
    }
#endif
    songdb::init(songdbdir);

    const auto info = songdb::lookup(hash);
    if (!info) {
        fprintf(stderr, "File %s does not exist in songdb\n", fname);
        return EXIT_FAILURE;
    }

    vector<string> songends;
    for (const auto &songinfo : info->subsongs) {
        const auto songend = common::split(songinfo.songend.status_string(),"+");
        string se = songend.front().substr(0,1);
        if (songend.size() > 1) {
            se = se + "+" + songend.back().substr(0,1);
        }
        char buf[1024];
        snprintf(buf, sizeof buf, "%u,%s%s", songinfo.songend.length, se.c_str(), songinfo.is_duplicate ? ",!" : "");
        songends.push_back(buf);
    }

#ifdef __MINGW32__
    _setmode(_fileno(stdout), 0x8000);
#endif

    if (!info->subsongs.empty()) {
        fprintf(stdout, "songlengths.tsv:%s\t%d\t%s\n", hash.c_str(), info->subsongs.front().subsong, common::mkString(songends, " ").c_str());
    }

    if (info->modinfo) {
        fprintf(stdout, "modinfos.tsv:%s\t%s\t%d\n", hash.c_str(), info->modinfo->format.c_str(), info->modinfo->channels);
    }

    if (info->metadata) {
        fprintf(stdout, "metadata.tsv:%s\t%s\t%s\t%s\t%u\n", hash.c_str(), info->metadata->author.c_str(), info->metadata->publisher.c_str(), info->metadata->album.c_str(), info->metadata->year);
    }

    return EXIT_SUCCESS;
}
