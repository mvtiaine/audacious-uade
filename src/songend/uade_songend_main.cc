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

#include "songend.h"
#include "../uade_common.h"
#include "../hacks.h"
#include "../converter/converter.h"
#include "../3rdparty/md5.h"

using namespace std;

constexpr int freq = songend::PRECALC_FREQ;

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

    if (is_blacklisted_md5(md5hex)) {
        fprintf(stderr, "Blacklisted md5 for %s\n", fname);
        if (includefname) {
            fprintf(stdout, "%s\t%d\t%d\t%s\t%s\n", md5hex.c_str(),0,0,"error",fname);
        } else {
            fprintf(stdout, "%s\t%d\t%d\t%s\n", md5hex.c_str(),0,0,"error");
        }
        return EXIT_FAILURE;
    }

    if (buf.size() < converter::MAGIC_SIZE)  {
        fprintf(stderr, "Could not read magic for %s\n", fname);
        return EXIT_FAILURE;
    }

    bool converted = false;
    if (converter::needs_conversion(buf.data(), converter::MAGIC_SIZE)) {
        auto res = converter::convert(buf.data(), buf.size());
        if (res.success) {
            buf = res.data;
            converted = true;
        } else {
            fprintf(stderr, "Could not convert %s\n", fname);
            return EXIT_FAILURE;
        }
    }

    uade_state *state = create_uade_probe_state(freq);

    const auto play_uade = [&](const int subsong) {
        if (converted) {
            return uade_play_from_buffer(fname, buf.data(), buf.size(), subsong, state);
        } else {
            return uade_play(fname, subsong, state);
        }
    };

    if (play_uade(-1) != 1) {
        fprintf(stderr, "Could not play %s\n", fname);
        uade_cleanup_state(state);
        return EXIT_FAILURE;
    }

    const auto *info = uade_get_song_info(state);

    const int minsubsong = info->subsongs.min;
    // vaoid uade_subsong_control: Assertion `subsong >= 0 && subsong < 256' failed.
    const int maxsubsong = min(info->subsongs.max, 255);

    if (uade_stop(state) != 0) {
        fprintf(stderr, "Could not play (stop) %s\n", fname);
        uade_cleanup_state(state);
        return EXIT_FAILURE;
    }

    for (int subsong = minsubsong; subsong <= maxsubsong; subsong++) {
        int res = play_uade(subsong);
        if (res == 0) {
            fprintf(stderr, "Could not play %s subsong %d\n", fname, subsong);
            uade_stop(state);
            continue;
        } else if (res != 1) {
            fprintf(stderr, "Could not play %s subsong %d\n", fname, subsong);
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
            fprintf(stderr, "Could not stop %s subsong %d\n", fname, subsong);
            uade_cleanup_state(state);
            state = create_uade_probe_state(freq);
        }

        const auto reason = songend.status_string();
        if (subsong == minsubsong && includefname) {
            fprintf(stdout, "%s\t%d\t%d\t%s\t%s\n", md5hex.c_str(),subsong,songend.length,reason.c_str(),fname);
        } else {
            fprintf(stdout, "%s\t%d\t%d\t%s\n", md5hex.c_str(),subsong,songend.length,reason.c_str());
        }
    }

    return EXIT_SUCCESS;
}
