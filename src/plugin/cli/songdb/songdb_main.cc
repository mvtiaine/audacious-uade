// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <vector>

#include "common/md5.h"
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

    const char *songdbdir = getenv("SONGDB_DIR");
    if (!songdbdir) {
        songdbdir = UADEDIR "/songdb";
    }
    songdb::init(songdbdir);

    const auto subsongs = songdb::subsong_range(md5hex);

    if (!subsongs) {
        fprintf(stderr, "File %s does not exist in songdb\n", fname);
        return EXIT_FAILURE;
    }

    const auto infos = songdb::lookup_all(md5hex);
    if (infos.empty()) {
        fprintf(stderr, "File %s does not exist in songdb\n", fname);
        return EXIT_FAILURE;
    }

    const auto md5short = md5hex.substr(0,12);

    const auto &info = infos.front();
    vector<string> songends;
    for (const auto &info : infos) {
        const auto songend = common::split(info.songend,"+");
        string se = songend.front().substr(0,1);
        if (songend.size() > 1) {
            se = se + "+" + songend.back().substr(0,1);
        }
        char buf[1024];
        snprintf(buf, sizeof buf, "%u,%s", info.songlength, se.c_str());
        songends.push_back(buf);
    }

#ifdef __MINGW32__
    _setmode(_fileno(stdout), 0x8000);
#endif

    fprintf(stdout, "songlengths.tsv:%s\t%s\t%d\t%d\t%s\n", md5short.c_str(), info.format.c_str(), info.channels, info.subsong, common::mkString(songends, " ").c_str());

    if (info.amp_data) {
        // TODO handle extra authors
        fprintf(stdout, "amp.tsv:%s\t%s\t\n", md5short.c_str(), info.amp_data->author.c_str());
    }

    if (info.demozoo_data) {
        fprintf(stdout, "demozoo.tsv:%s\t%u\t%s\t%s\t%s\n", md5short.c_str(), info.demozoo_data->year, info.demozoo_data->author.c_str(), info.demozoo_data->publisher.c_str(), info.demozoo_data->album.c_str());
    }

    if (info.modland_data) {
        fprintf(stdout, "modland.tsv:%s\t%s\n", md5short.c_str(), info.modland_data->author.c_str());
    }

    if (info.unexotica_data) {
        const auto author_path = songdb::unexotica::author_path(info.unexotica_data->author);
        const auto album_path = common::mkString(common::split(info.unexotica_data->album, " "), "_");
        fprintf(stdout, "unexotica.tsv:%s\t%s/%s\t%s\t%u\n", md5short.c_str(), author_path.c_str(), album_path.c_str(), info.unexotica_data->publisher.c_str(), info.unexotica_data->year);
    }

    return EXIT_SUCCESS;
}
