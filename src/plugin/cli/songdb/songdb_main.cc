// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <vector>

#include "common/md5.h"
#include "common/strings.h"
#include "songdb/songdb.h"

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

    uint8_t buf[4096];
    ssize_t count;
    MD5 md5;
    while ((count = read(fd, buf, sizeof buf)) > 0) {
        md5.update(buf, count);
    }
    close(fd);

    md5.finalize();
    string md5hex = md5.hexdigest();

#ifdef SONGDB_DIR
    const char *songdbdir = SONGDB_DIR;
#else
    const char *songdbdir = getenv("SONGDB_DIR");
    if (!songdbdir) {
        songdbdir = UADEDIR "/songdb";
    }
#endif
    songdb::init(songdbdir);

    const auto info = songdb::lookup(md5hex);
    if (!info) {
        fprintf(stderr, "File %s does not exist in songdb\n", fname);
        return EXIT_FAILURE;
    }

    const auto md5short = md5hex.substr(0,12);

    vector<string> songends;
    for (const auto &songinfo : info->subsongs) {
        const auto songend = common::split(songinfo.songend.status_string(),"+");
        string se = songend.front().substr(0,1);
        if (songend.size() > 1) {
            se = se + "+" + songend.back().substr(0,1);
        }
        char buf[1024];
        snprintf(buf, sizeof buf, "%u,%s", songinfo.songend.length, se.c_str());
        songends.push_back(buf);
    }

#ifdef __MINGW32__
    _setmode(_fileno(stdout), 0x8000);
#endif

    fprintf(stdout, "songlengths.tsv:%s\t%d\t%s\n", md5short.c_str(), info->subsongs.front().subsong, common::mkString(songends, " ").c_str());
    fprintf(stdout, "modinfos.tsv:%s\t%s\t%d\n", md5short.c_str(), info->modinfo->format.c_str(), info->modinfo->channels);

    if (info->amp) {
        fprintf(stdout, "amp.tsv:%s\t%s\n", md5short.c_str(), info->amp->author.c_str());
    }

    if (info->demozoo) {
        fprintf(stdout, "demozoo.tsv:%s\t%s\t%s\t%s\t%u\n", md5short.c_str(), info->demozoo->author.c_str(), info->demozoo->publisher.c_str(), info->demozoo->album.c_str(), info->demozoo->year);
    }

    if (info->modland) {
        fprintf(stdout, "modland.tsv:%s\t%s\n", md5short.c_str(), info->modland->author.c_str());
    }

    if (info->unexotica) {
        const auto author_path = songdb::unexotica::author_path(info->unexotica->author);
        const auto album_path = common::mkString(common::split(info->unexotica->album, " "), "_");
        fprintf(stdout, "unexotica.tsv:%s\t%s/%s\t%s\t%u\n", md5short.c_str(), author_path.c_str(), album_path.c_str(), info->unexotica->publisher.c_str(), info->unexotica->year);
    }

    return EXIT_SUCCESS;
}
