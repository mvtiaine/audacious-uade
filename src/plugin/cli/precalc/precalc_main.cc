// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

// Outputs song lengths in milliseconds to stdout in TSV format (one line for each subsong in the module):
// <md5>\t<subsong>\t<length-milliseconds>\t<songend-reason>
// Optionally also the file size and file name is included:
// <md5>\t<subsong>\t<length-milliseconds>\t<songend-reason>\t<size>\t<filename>

// Example usage:
// ./precalc <input-file> >> /tmp/Songlengths.csv 2>/dev/null

// NOTE: requires the audacious plugin to be installed to find some conf/data files

#include <algorithm>
#include <vector>

#include "3rdparty/xxhash/xxhash.h"
#include "common/md5.h"
#include "common/strings.h"
#include "player/player.h"
#include "songend/precalc.h"
#include "songdb/songdb.h"

#include <sys/stat.h>
#include <unistd.h>

using namespace std;

namespace {

void print(const common::SongEnd &songend, const player::ModuleInfo &info, int subsong, vector<char> &buf, bool includepath, const string &md5hex, const XXH32_hash_t xxh32) {
    const auto reason = songend.status_string();
    if (subsong == info.minsubsong) {
        const auto pl = player::name(info.player);
        if (includepath) {
            fprintf(stdout, "%s\t%d\t%d\t%s\t%s\t%s\t%d\t%zu\t%08x\t%s\n", md5hex.c_str(),subsong,songend.length,reason.c_str(),pl.data(),info.format.c_str(),info.channels,buf.size(),xxh32,info.path.c_str());
        } else {
            fprintf(stdout, "%s\t%d\t%d\t%s\t%s\t%s\t%d\t%zu\t%08x\n", md5hex.c_str(),subsong,songend.length,reason.c_str(),pl.data(),info.format.c_str(),info.channels,buf.size(),xxh32);
        }
    } else {
        fprintf(stdout, "%s\t%d\t%d\t%s\n", md5hex.c_str(),subsong,songend.length,reason.c_str());
    }
}

int player_songend(const vector<player::Player> &players, vector<char> &buf, const char *path, bool includepath, const string &md5hex, const XXH32_hash_t xxh32) {
    for (const auto &player : players) {
        const auto &info = player::parse(path, buf.data(), buf.size(), player);
        if (!info) continue;
        const int minsubsong = info->minsubsong;
        const int maxsubsong = info->maxsubsong;
        for (int subsong = minsubsong; subsong <= maxsubsong; subsong++) {
            auto songend = songend::precalc::precalc_song_end(info.value(), buf.data(), buf.size(), subsong, md5hex);
            if (songend.status == common::SongEnd::ERROR && !songend::precalc::allow_songend_error(info->format)) {
                songend.length = 0;
            }
            print(songend, info.value(), subsong, buf, includepath, md5hex, xxh32);
        }
        return EXIT_SUCCESS;
    }
    fprintf(stderr, "Could not parse %s md5 %s\n", path, md5hex.c_str());
    return EXIT_FAILURE;
}

} // namespace {}

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

    if (argc >= 3 && string(argv[2]) == "subsongs") {
        // just report subsongs
        uint8_t buf[4096];
        vector<char> buffer;
        buffer.reserve(st.st_size);
        ssize_t count;
        while ((count = read(fd, buf, sizeof buf)) > 0) {
            buffer.insert(buffer.end(), buf, buf + count);
        }
        close(fd);

        const player::support::PlayerScope p;
        const auto players = player::check(path, buffer.data(), buffer.size(), buffer.size());
        if (players.empty()) {
            return EXIT_FAILURE;
        }
        for (const auto &player : players) {
            const auto &info = player::parse(path, buffer.data(), buffer.size(), player);
            if (!info) continue;
            const int minsubsong = info->minsubsong;
            const int maxsubsong = info->maxsubsong;
            fprintf(stdout, "%d", minsubsong);
            for (int subsong = minsubsong + 1; subsong <= maxsubsong; subsong++) {
                fprintf(stdout, " %d", subsong);
            }
            break;
        }
        return EXIT_SUCCESS;
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

    const auto xxh32 = XXH32(buffer.data(), min(songdb::XXH_MAX_BYTES, buffer.size()), 0);
    const string hash = common::to_hex(xxh32) + common::to_hex((uint16_t)(st.st_size & 0xFFFF));

    if (songdb::blacklist::is_blacklisted_songdb_hash(hash)) {
        fprintf(stderr, "Blacklisted songdb hash for %s xxh32 %s\n", path, hash.c_str());
        return EXIT_FAILURE;
    }

    if (songdb::blacklist::is_blacklisted_hash(hash)) {
        fprintf(stderr, "Blacklisted hash %s for %s\n", hash.c_str(), path);
        if (includepath) {
            fprintf(stdout, "%s\t%d\t%d\t%s\t\t\t\t%zu\t%08x\t%s\n", md5hex.c_str(),0,0,"error",buffer.size(),xxh32,path);
        } else {
            fprintf(stdout, "%s\t%d\t%d\t%s\t\t\t\t%zu\t%08x\n", md5hex.c_str(),0,0,"error",buffer.size(),xxh32);
        }
        return EXIT_FAILURE;
    }

#ifdef __MINGW32__
    _setmode(_fileno(stdout), 0x8000);
#endif

    const player::support::PlayerScope p;
    vector<player::Player> players;
    if (getenv("PLAYER")) {
        player::Player player = player::player(getenv("PLAYER"));
        if (player == player::Player::NONE) {
            fprintf(stderr, "Unknown player %s\n", getenv("PLAYER"));
            return EXIT_FAILURE;
        }
        players.push_back(player::player(getenv("PLAYER")));
    } else {
        players = player::check(path, buffer.data(), buffer.size(), buffer.size());
    }
    if (players.empty()) {
        fprintf(stderr, "Could not recognize %s md5 %s\n", path, md5hex.c_str());
        return EXIT_FAILURE;
    }

    return player_songend(players, buffer, path, includepath, md5hex, xxh32);
}
