// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <bit>
#include <fstream>
#include <iostream>
#include <vector>

#include "player/player.h"

using namespace player;
using namespace std;

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "File or frequency not given\n");
        return EXIT_FAILURE;
    }

    int frequency = stoi(argv[1]);
    const char* fname = argv[2];
    int subsong = -1;
    if (argc >= 4) {
        subsong = stoi(argv[3]);
    }

    ifstream input(fname, ios::in | ios::binary | ios::ate);
    if (!input.is_open()) {
        fprintf(stderr, "File not found: %s\n", fname);
        return EXIT_FAILURE;
    }
    vector<char> buffer(input.tellg());
    input.seekg(0, ios::beg);
    input.read(buffer.data(), buffer.size());

    const support::PlayerScope p;

    const auto player = check(fname, buffer.data(), buffer.size());
    if (player == Player::NONE) {
        fprintf(stderr, "Could not recognize %s\n", fname);
        return EXIT_FAILURE;
    }

    auto info = parse(fname, buffer.data(), buffer.size());
    if (!info) {
        fprintf(stderr, "Could not parse %s\n", fname);
        return EXIT_FAILURE;
    }

    PlayerConfig player_config = { frequency };
    uade::UADEConfig uade_config = {{ frequency }};
    if (getenv("SONGEND_MODE")) {
        uade_config.subsong_timeout = player::PRECALC_TIMEOUT;
        uade_config.silence_timeout = player::PRECALC_TIMEOUT;
        uade_config.filter = uade::Filter::NONE;
        uade_config.resampler = uade::Resampler::NONE;
        uade_config.panning = 1;
    }
    auto &config = player == Player::uade ? uade_config : player_config;

    const char *endian_ = getenv("PLAYER_ENDIAN");
    if (endian_ && string(endian_) == "big") {
        config.endian = endian::big;
    } else if (endian_ && string(endian_) == "little") {
        config.endian = endian::little;
    }

    if (subsong < 0) {
        subsong = info->defsubsong;
    }

    auto state = play(fname, buffer.data(), buffer.size(), subsong, config);
    if (!state) {
        fprintf(stderr, "Could not play %s\n", fname);
        return EXIT_FAILURE;
    }

    const auto check_stop = []() { return false; };
    const auto check_seek = []() { return -1; };
    const auto write_audio = [](char *buf, int bytes) {
        fwrite(buf, bytes, 1, stdout);
    };

    const auto res = support::playback_loop(state.value(), config, check_stop, check_seek, write_audio);
    fflush(stdout);

    if (!stop(state.value())) {
        fprintf(stderr, "Could not stop %s\n", fname);
    }

    if (res.songend.status == SongEnd::ERROR) {
        fprintf(stderr, "Error playing %s\n", fname);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
