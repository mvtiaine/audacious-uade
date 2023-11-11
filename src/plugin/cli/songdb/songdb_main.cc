// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

#include "3rdparty/md5.h"
#include "songdb/songdb.h"

using namespace std;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "File not given\n");
        return EXIT_FAILURE;
    }
    const char* fname = argv[1];
    ifstream input(fname, ios::in | ios::binary | ios::ate);
    if (!input.is_open()) {
        fprintf(stderr, "File not found: %s\n", fname);
        return EXIT_FAILURE;
    }
    vector<char> buffer(input.tellg());
    input.seekg(0, ios::beg);
    input.read(buffer.data(), buffer.size());

    MD5 md5; md5.update((const unsigned char *)buffer.data(), buffer.size()); md5.finalize();
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

    const auto print = [&subsongs](const string &prefix, songdb::SongInfo info, const string &path) -> string {
        char buf[1024];
        if (info.subsong == subsongs->first) {
            snprintf(buf, sizeof buf, "%s%s\t%d\t%d\t%s\t%zd\t%s",
                prefix.c_str(), info.md5.c_str(), info.subsong, info.length, info.status.c_str(), info.size, path.c_str());
        } else {
            snprintf(buf, sizeof buf, "%s%s\t%d\t%d\t%s",
                prefix.c_str(), info.md5.c_str(), info.subsong, info.length, info.status.c_str());
        }
        return buf;
    };

    vector<string> lines;
    for (int subsong = subsongs->first; subsong <= subsongs->second; ++subsong) {
        const auto infos = songdb::lookup_all(md5hex, subsong);
        for (const auto &info : infos) {
            if (info.modland_data)
                lines.push_back(print("modland.tsv:", info, info.modland_data->path));
            else if (info.amp_data)
                lines.push_back(print("amp.tsv:", info, info.amp_data->path));
            else if (info.unexotica_data)
                lines.push_back(print("unexotica.tsv:", info, info.unexotica_data->path));
            else
                lines.push_back(print(":", info, ""));
        }
    }

    sort(lines.begin(), lines.end());

    for (const auto &line : lines) {
        fprintf(stdout, "%s\n", line.c_str());
    }

    return EXIT_SUCCESS;
}
