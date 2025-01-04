// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

// Outputs song lengths in milliseconds to stdout in TSV format (one line for each subsong in the module):
// <md5>\t<subsong>\t<length-milliseconds>\t<songend-reason>
// Optionally also the file size and file name is included:
// <md5>\t<subsong>\t<length-milliseconds>\t<songend-reason>\t<size>\t<filename>

// Errored songlengths are not recorded, except for whitelisted players (OctaMED,VSS) which always crash just at songend.

// Example usage:
// ./precalc <input-file> >> /tmp/Songlengths.csv 2>/dev/null

// NOTE: requires the audacious plugin to be installed to find some conf/data files

#include <vector>

#include "common/md5.h"
#include "player/player.h"
#include "songend/precalc.h"
#include "songdb/songdb.h"

#include <sys/stat.h>
#include <unistd.h>

using namespace std;

namespace {

void print(const common::SongEnd &songend, const player::ModuleInfo &info, int subsong, vector<char> &buf, bool includepath, const string &md5hex) {
    const auto reason = songend.status_string();
    if (subsong == info.minsubsong) {
        const auto pl = player::name(info.player);
        if (includepath) {
            fprintf(stdout, "%s\t%d\t%d\t%s\t%s\t%s\t%d\t%zu\t%s\n", md5hex.c_str(),subsong,songend.length,reason.c_str(),pl.c_str(),info.format.c_str(),info.channels,buf.size(),info.path.c_str());
        } else {
            fprintf(stdout, "%s\t%d\t%d\t%s\t%s\t%s\t%d\t%zu\n", md5hex.c_str(),subsong,songend.length,reason.c_str(),pl.c_str(),info.format.c_str(),info.channels,buf.size());
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
        if (songend.status == common::SongEnd::ERROR && !songend::precalc::allow_songend_error(info->format)) {
            songend.length = 0;
        }
        print(songend, info.value(), subsong, buf, includepath, md5hex);
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

    FILE *f = fopen(path, "rb"); 
    if (!f) {
        fprintf(stderr, "File not found: %s\n", path);
        return EXIT_FAILURE;
    }
    int fd = fileno(f);

    struct stat st;
    if (fstat(fd, &st)) {
        close(fd);
        fprintf(stderr, "Failed to read file size for %s\n", path);
        return EXIT_FAILURE;
    }

    uint8_t buf[4096];
    vector<char> buffer;
    buffer.reserve(st.st_size);

    ssize_t count;
    MD5 md5;
    while ((count = read(fd, buf, sizeof buf)) > 0) {
        md5.update(buf, count);
        buffer.insert(buffer.end(), buf, buf + count);
    }
    close(fd);
    md5.finalize();
    string md5hex = md5.hexdigest();

    if (songdb::blacklist::is_blacklisted_songdb_key(md5hex)) {
        fprintf(stderr, "Blacklisted songdb md5 for %s\n", path);
        return EXIT_FAILURE;
    }

    if (songdb::blacklist::is_blacklisted_md5(md5hex)) {
        fprintf(stderr, "Blacklisted md5 for %s\n", path);
        if (includepath) {
            fprintf(stdout, "%s\t%d\t%d\t%s\t\t\t\t%zu\t%s\n", md5hex.c_str(),0,0,"error",buffer.size(),path);
        } else {
            fprintf(stdout, "%s\t%d\t%d\t%s\t\t\t\t%zu\n", md5hex.c_str(),0,0,"error",buffer.size());
        }
        return EXIT_FAILURE;
    }

#ifdef __MINGW32__
    _setmode(_fileno(stdout), 0x8000);
#endif

    const player::support::PlayerScope p;
    if (player::check(path, buffer.data(), buffer.size()) != player::Player::NONE) {
        int res = player_songend(buffer, path, includepath, md5hex);
        return res;
    } else {
        fprintf(stderr, "Could not recognize %s md5 %s\n", path, md5hex.c_str());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
