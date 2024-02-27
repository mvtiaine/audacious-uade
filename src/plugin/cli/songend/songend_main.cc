// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

// Outputs song length in milliseconds to stdout based on the raw input audio to stdin.
// Input must be raw S16LE mono at 8062Hz and have at least 3600s of audio.

// Example usage with uade:
// uade123 --stderr -s 0 -1 --subsong-timeout=3600 --silence-timeout=3600 --frequency=8062 --filter=none --resampler=none --panning=1 -e raw -f /dev/stdout <input-file> 2>/dev/null | ./songend

#include <cstdio>
#include <cstdlib>

#include "player/player.h"
#include "songend/detector.h"

#include <unistd.h>

using namespace songend;
using namespace std;

constexpr ssize_t MINLENGTH = player::PRECALC_FREQ * 2 * 2 * (player::PRECALC_TIMEOUT - 1);

int main(int argc, char *argv[]) {
    char buf[4096];
    ssize_t total = 0;
    ssize_t count;

    detector::SongEndDetector detector(player::PRECALC_FREQ, false, endian::native);

#ifndef NO_FREOPEN
    if (!freopen(NULL, "rb", stdin)) {
        fprintf(stderr, "Failed to freopen(rb) stdin\n");
        return EXIT_FAILURE;
    }
#endif
#ifdef __MINGW32__
    setmode (fileno (stdin), 0x8000);
#endif
    const string fname = argc < 2 ? "-" : argv[1];
    bool is_stdin = (fname == "-");

    int fd = STDIN_FILENO;
    if (!is_stdin) {
        FILE *f = fopen(fname.c_str(), "rb"); 
        if (!f) {
            fprintf(stderr, "File not found: %s\n", fname.c_str());
            return EXIT_FAILURE;
        }
        fd = fileno(f);
    }
    while ((count = read(fd, buf, sizeof buf)) > 0) {
        detector.update(buf, count);
        total += count;
    }
    if (!is_stdin) {
        close(fd);
    }

    if (total < MINLENGTH) {
        fprintf(stderr, "Not enough data. got %zu required %zu\n", total, MINLENGTH);
        return EXIT_FAILURE;
    }

    int res = detector.detect_silence(player::SILENCE_TIMEOUT);
    if (res) {
        fprintf(stdout, "%d\n", res + player::MAX_SILENCE);
        return EXIT_SUCCESS;
    }
    res = detector.detect_volume(player::SILENCE_TIMEOUT);
    if (res) {
        fprintf(stdout, "%d\n", res + player::MAX_SILENCE);
        return EXIT_SUCCESS;
    }
    res = detector.detect_repeat();
    if (res) {
        fprintf(stdout, "%d\n", res);
        return EXIT_SUCCESS;
    }
    res = detector.detect_loop();
    if (res) {
        fprintf(stdout, "%d\n", res);
        return EXIT_SUCCESS;
    } else {
        fprintf(stderr, "Could not determine song length\n");
        return EXIT_FAILURE;
    }

    return res ? EXIT_SUCCESS : EXIT_FAILURE;
}
