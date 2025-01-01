// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <string>
#include <vector>

#include "common/endian.h"
#include "player/player.h"

#include <sys/stat.h>
#include <unistd.h>

using namespace common;
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

    FILE *f = fopen(fname, "rb"); 
    if (!f) {
        fprintf(stderr, "File not found: %s\n", fname);
        return EXIT_FAILURE;
    }
    int fd = fileno(f);

    struct stat st;
    if (fstat(fd, &st)) {
        close(fd);
        fprintf(stderr, "Failed to read file size for %s\n", fname);
        return EXIT_FAILURE;
    }

    char buf[4096];
    vector<char> buffer;
    buffer.reserve(st.st_size);

    ssize_t count;
    while ((count = read(fd, buf, sizeof buf)) > 0) {
        buffer.insert(buffer.end(), buf, buf + count);
    }
    close(fd);

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
    auto uade_config = uade::UADEConfig(player_config);
    if (getenv("SONGEND_MODE")) {
        uade_config.subsong_timeout = player::PRECALC_TIMEOUT;
        uade_config.silence_timeout = player::PRECALC_TIMEOUT;
        uade_config.filter = uade::Filter::NONE;
        uade_config.resampler = uade::Resampler::NONE;
        uade_config.panning = 1;
    }
    auto it2play_config = it2play::IT2PlayConfig(player_config);
    if (getenv("IT2PLAY_DRIVER")) {
        const auto mixer = string(getenv("IT2PLAY_DRIVER"));
        if (mixer == "hq") {
            it2play_config.driver = it2play::Driver::HQ;
        } else if (mixer == "sb16mmx") {
            it2play_config.driver = it2play::Driver::SB16MMX;
        } else if (mixer == "sb16") {
            it2play_config.driver = it2play::Driver::SB16;
        } else if (mixer == "wavwriter") {
            it2play_config.driver = it2play::Driver::WAVWRITER;
        }
    }
    auto &config =
        player == Player::uade ? uade_config :
        player == Player::it2play ? it2play_config :
        player_config;

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

#ifdef __MINGW32__
    _setmode(_fileno(stdout), 0x8000);
#endif

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
