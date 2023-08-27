// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

// Outputs song length in milliseconds to stdout based on the raw input audio to stdin.
// Input must be raw S16LE mono at 8062Hz and have at least 3600s of audio.

// Example usage with uade:
// make songend
// uade123 --stderr -s 0 -1 --subsong-timeout=3600 --silence-timeout=3600 --frequency=8062 --filter=none --resampler=none --panning=1 -e raw -f /dev/stdout <input-file> 2>/dev/null | ./songend

#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#include "songend.h"

using namespace std;

constexpr size_t MINLENGTH = songend::PRECALC_FREQ * 2 * 2 * (songend::PRECALC_TIMEOUT - 1);

int main(int argc, char *argv[]) {
    char buf[4096];
    ssize_t total = 0;
    ssize_t count;

    songend::SongEndDetector detector(songend::PRECALC_FREQ);

    if (!freopen(NULL, "rb", stdin)) {
        fprintf(stderr, "Failed to freopen(rb) stdin\n");
        return EXIT_FAILURE;
    }
    
    while ((count = read(STDIN_FILENO, buf, sizeof buf)) > 0) {
        detector.update(buf, count);
        total += count;
    }

    if (total < MINLENGTH) {
        fprintf(stderr, "Not enough data. got %zu required %zu\n", total, MINLENGTH);
        return EXIT_FAILURE;
    }

    fprintf(stderr, "got %zu required %zu\n", total, MINLENGTH);
 
    int res = detector.detect_silence(songend::SILENCE_TIMEOUT);
    if (res) {
        fprintf(stdout, "%d\n", res + songend::MAX_SILENCE);
        return EXIT_SUCCESS;
    }
    res = detector.detect_volume();
    if (res) {
        fprintf(stdout, "%d\n", res + songend::MAX_SILENCE);
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
