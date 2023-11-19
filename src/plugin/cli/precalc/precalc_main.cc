// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

// Outputs song lengths in milliseconds to stdout in TSV format (one line for each subsong in the module):
// <md5>\t<subsong>\t<length-milliseconds>\t<songend-reason>
// Optionally also the file size and file name is included:
// <md5>\t<subsong>\t<length-milliseconds>\t<songend-reason>\t<size>\t<filename>

// Errored songlengths are not recorded, except for whitelisted players (OctaMED,VSS) which always crash just at songend.

// Example usage:
// ./precalc <input-file> >> /tmp/Songlengths.csv 2>/dev/null

// NOTE: requires the audacious plugin to be installed to find some conf/data files

#include <fstream>
#include <iostream>
#include <vector>

#include "common/md5.h"
#include "player/player.h"
#include "songend/precalc.h"
#include "songdb/songdb.h"

using namespace std;

namespace {

void print(const player::SongEnd &songend, int subsong, int minsubsong, vector<char> &buf, const char *path, bool includepath, const string &md5hex) {
    const auto reason = songend.status_string();
    if (subsong == minsubsong) {
        if (includepath) {
            fprintf(stdout, "%s\t%d\t%d\t%s\t%zu\t%s\n", md5hex.c_str(),subsong,songend.length,reason.c_str(),buf.size(),path);
        } else {
            fprintf(stdout, "%s\t%d\t%d\t%s\t%zu\n", md5hex.c_str(),subsong,songend.length,reason.c_str(),buf.size());
        }
    } else {
        fprintf(stdout, "%s\t%d\t%d\t%s\n", md5hex.c_str(),subsong,songend.length,reason.c_str());
    }
}

int player_songend(vector<char> &buf, const char *path, bool includepath, const string &md5hex) {
    const auto &info = player::parse(path, buf.data(), buf.size());
    if (!info) {
        fprintf(stderr, "Could not parse %s md5 %s\n", path, md5hex.c_str());
        return EXIT_FAILURE;
    }
    
    const int minsubsong = info->minsubsong;
    const int maxsubsong = info->maxsubsong;
    for (int subsong = minsubsong; subsong <= maxsubsong; subsong++) {
        auto songend = songend::precalc::precalc_song_end(info.value(), buf.data(), buf.size(), subsong, md5hex);
        if (songend.status == player::SongEnd::ERROR && !songend::precalc::allow_songend_error(info.value())) {
            songend.length = 0;
        }
        print(songend, subsong, minsubsong, buf, path, includepath, md5hex);
    }

    return EXIT_SUCCESS;
}

}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "File not given\n");
        return EXIT_FAILURE;
    }
    const char* path = argv[1];
    bool includepath = argc >= 3;

    ifstream input(path, ios::in | ios::binary | ios::ate);
    if (!input.is_open()) {
        fprintf(stderr, "File not found: %s\n", path);
        return EXIT_FAILURE;
    }
    vector<char> buf(input.tellg());
    input.seekg(0, ios::beg);
    input.read(buf.data(), buf.size());

    MD5 md5; md5.update((const unsigned char *)buf.data(), buf.size()); md5.finalize();
    string md5hex = md5.hexdigest();

    if (songdb::blacklist::is_blacklisted_songdb_key(md5hex)) {
        fprintf(stderr, "Blacklisted songdb md5 for %s\n", path);
        return EXIT_FAILURE;
    }

    if (songdb::blacklist::is_blacklisted_md5(md5hex)) {
        fprintf(stderr, "Blacklisted md5 for %s\n", path);
        if (includepath) {
            fprintf(stdout, "%s\t%d\t%d\t%s\t%zu\t%s\n", md5hex.c_str(),0,0,"error",buf.size(),path);
        } else {
            fprintf(stdout, "%s\t%d\t%d\t%s\t%zu\n", md5hex.c_str(),0,0,"error",buf.size());
        }
        return EXIT_FAILURE;
    }

    const player::support::PlayerScope p;
    if (player::check(path, buf.data(), buf.size()) != player::Player::NONE) {
        int res = player_songend(buf, path, includepath, md5hex);
        return res;
    } else {
        fprintf(stderr, "Could not recognize %s md5 %s\n", path, md5hex.c_str());
        return EXIT_FAILURE;
    }
}
