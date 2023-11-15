// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <bit>
#include <fstream>
#include <iostream>
#include <vector>

#include "player/player.h"

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

    const player::support::PlayerScope p;

    const auto player = player::check(fname, buffer.data(), buffer.size());
    if (player == player::Player::NONE) {
        fprintf(stderr, "Could not recognize %s\n", fname);
        return EXIT_FAILURE;
    }

    auto info = player::parse(fname, buffer.data(), buffer.size());
    if (!info) {
        fprintf(stderr, "Could not parse %s\n", fname);
        return EXIT_FAILURE;
    }

    player::PlayerConfig player_config = { frequency };
    player::uade::UADEConfig uade_config = {{ frequency }};
    auto &config = player == player::Player::uade ? uade_config : player_config;

    const char *endian_ = getenv("PLAYER_ENDIAN");
    if (endian_ && string(endian_) == "big") {
        config.endian = endian::big;
    } else if (endian_ && string(endian_) == "little") {
        config.endian = endian::little;
    }

    if (subsong < 0) {
        subsong = info->defsubsong;
    }

    auto state = player::play(fname, buffer.data(), buffer.size(), subsong, config);
    if (!state) {
        fprintf(stderr, "Could not play %s\n", fname);
        return EXIT_FAILURE;
    }

    const auto check_stop = []() { return false; };
    const auto check_seek = []() { return -1; };
    const auto write_audio = [](char *buf, int bytes) {
        fwrite(buf, bytes, 1, stdout);
    };

    const auto res = player::support::playback_loop(state.value(), config, check_stop, check_seek, write_audio);

    if (!player::stop(state.value())) {
        fprintf(stderr, "Could not stop %s\n", fname);
    }

    if (res.songend.status == player::SongEnd::ERROR) {
        fprintf(stderr, "Error playing %s\n", fname);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
