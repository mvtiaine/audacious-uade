// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

// Outputs song lengths in milliseconds to stdout in TSV format (one line for each subsong in the module):
// <md5>\t<subsong>\t<length-milliseconds>\t<songend-reason>
// Optionally also the filename is included:
// <md5>\t<subsong>\t<length-milliseconds>\t<songend-reason>\t<filename>

// Errored songlengths are not recorded, except for whitelisted players (OctaMED,VSS) which always crash just at songend.

// Example usage:
// make uade_songend
// ./uade_songend <input-file> >> /tmp/Songlengths.csv 2>/dev/null

// NOTE: requires the audacious plugin to be installed to find some conf/data files

#include <fstream>
#include <iostream>
#include <vector>

#include "songend/songend.h"
#include "uade/uade_common.h"
#include "uade/hacks.h"
#include "converter/converter.h"
#include "player/player.h"
#include "3rdparty/md5.h"

using namespace std;

namespace {

void print(const song_end &songend, int subsong, int minsubsong, vector<char> &buf, const char *fname, bool includefname, const string &md5hex) {
    const auto reason = songend.status_string();
    if (subsong == minsubsong) {
        if (includefname) {
            fprintf(stdout, "%s\t%d\t%d\t%s\t%zu\t%s\n", md5hex.c_str(),subsong,songend.length,reason.c_str(),buf.size(),fname);
        } else {
            fprintf(stdout, "%s\t%d\t%d\t%s\t%zu\n", md5hex.c_str(),subsong,songend.length,reason.c_str(),buf.size());
        }
    } else {
        fprintf(stdout, "%s\t%d\t%d\t%s\n", md5hex.c_str(),subsong,songend.length,reason.c_str());
    }
}

int uade_songend(vector<char> &buf, const char *fname, bool includefname, const string &md5hex) {
    constexpr int freq = songend::PRECALC_FREQ_UADE;
    uade_state *state = create_uade_probe_state(freq);

    bool converted = false;
    if (converter::needs_conversion(buf.data(), converter::MAGIC_SIZE)) {
        auto res = converter::convert(buf.data(), buf.size());
        if (res.success) {
            buf = res.data;
            converted = true;
        } else {
            fprintf(stderr, "Could not convert %s md5 %s\n", fname, md5hex.c_str());
            return EXIT_FAILURE;
        }
    } else if (!uade_is_our_file_from_buffer(fname, buf.data(), buf.size(), state)) {
        fprintf(stderr, "Could not recognize %s md5 %s\n", fname, md5hex.c_str());
        uade_cleanup_state(state);
        return EXIT_FAILURE;
    }

    const auto play_uade = [&](const int subsong) {
        if (converted) {
            return uade_play_from_buffer(fname, buf.data(), buf.size(), subsong, state);
        } else {
            return uade_play(fname, subsong, state);
        }
    };

    if (play_uade(-1) != 1) {
        fprintf(stderr, "Could not play %s md5 %s\n", fname, md5hex.c_str());
        uade_cleanup_state(state);
        return EXIT_FAILURE;
    }

    const auto *info = uade_get_song_info(state);

    const int minsubsong = info->subsongs.min;
    // vaoid uade_subsong_control: Assertion `subsong >= 0 && subsong < 256' failed.
    const int maxsubsong = min(info->subsongs.max, 255);

    if (uade_stop(state) != 0) {
        fprintf(stderr, "Could not play (stop) %s md5 %s\n", fname, md5hex.c_str());
        uade_cleanup_state(state);
        return EXIT_FAILURE;
    }

    for (int subsong = minsubsong; subsong <= maxsubsong; subsong++) {
        int res = play_uade(subsong);
        if (res == 0) {
            fprintf(stderr, "Could not play %s subsong %d md5 %s\n", fname, subsong, md5hex.c_str());
            uade_stop(state);
            continue;
        } else if (res != 1) {
            fprintf(stderr, "Could not play %s subsong %d md5 %s\n", fname, subsong, md5hex.c_str());
            uade_cleanup_state(state);
            state = create_uade_probe_state(freq);
            continue;
        }
        const auto *info = uade_get_song_info(state);
        const song_end songend = precalc_song_length(state, info);

        if (songend.status == song_end::ERROR) {
            uade_cleanup_state(state);
            state = create_uade_probe_state(freq);
        } else if (uade_stop(state)) {
            fprintf(stderr, "Could not stop %s subsong %d md5 %s\n", fname, subsong, md5hex.c_str());
            uade_cleanup_state(state);
            state = create_uade_probe_state(freq);
        }

        print(songend, subsong, minsubsong, buf, fname, includefname, md5hex);
    }

    return EXIT_SUCCESS;
}

int player_songend(vector<char> &buf, const char *fname, bool includefname, const string &md5hex) {
    constexpr int freq = songend::PRECALC_FREQ_PLAYER;

    const auto info = player::parse(fname, buf.data(), buf.size());
    if (!info.has_value()) {
        fprintf(stderr, "Could not parse %s md5 %s\n", fname, md5hex.c_str());
        return EXIT_FAILURE;
    }

    const int minsubsong = 0;
    const int maxsubsong = info->maxsubsong;

    for (int subsong = minsubsong; subsong <= maxsubsong; subsong++) {
        auto state = player::play(fname, buf.data(), buf.size(), subsong, freq);
        if (!state.has_value()) {
            fprintf(stderr, "Could not play %s subsong %d md5 %s\n", fname, subsong, md5hex.c_str());
            continue;
        }
        const song_end songend = precalc_song_length_player(state.value(), fname);
        player::stop(state.value());
        print(songend, subsong, minsubsong, buf, fname, includefname, md5hex);
    }

    return EXIT_SUCCESS;
}

}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "File not given\n");
        return EXIT_FAILURE;
    }
    const char* fname = argv[1];
    bool includefname = argc >= 3;

    ifstream input(fname, ios::in | ios::binary | ios::ate);
    if (!input.is_open()) {
        fprintf(stderr, "File not found: %s\n", fname);
        return EXIT_FAILURE;
    }
    vector<char> buf(input.tellg());
    input.seekg(0, ios::beg);
    input.read(buf.data(), buf.size());

    MD5 md5; md5.update((const unsigned char *)buf.data(), buf.size()); md5.finalize();
    string md5hex = md5.hexdigest();

    if (is_blacklisted_songdb(md5hex)) {
        fprintf(stderr, "Blacklisted songdb md5 for %s\n", fname);
        return EXIT_FAILURE;
    }

    if (is_blacklisted_md5(md5hex)) {
        fprintf(stderr, "Blacklisted md5 for %s\n", fname);
        if (includefname) {
            fprintf(stdout, "%s\t%d\t%d\t%s\t%zu\t%s\n", md5hex.c_str(),0,0,"error",buf.size(),fname);
        } else {
            fprintf(stdout, "%s\t%d\t%d\t%s\t%zu\n", md5hex.c_str(),0,0,"error",buf.size());
        }
        return EXIT_FAILURE;
    }

    if (buf.size() < converter::MAGIC_SIZE || buf.size() < player::MAGIC_SIZE)  {
        fprintf(stderr, "Could not read magic for %s md5 %s\n", fname, md5hex.c_str());
        return EXIT_FAILURE;
    }

    if (player::is_our_file(buf.data(), buf.size())) {
        player::init();
        return player_songend(buf, fname, includefname, md5hex);
    } else {
        return uade_songend(buf, fname, includefname, md5hex);
    }
}
