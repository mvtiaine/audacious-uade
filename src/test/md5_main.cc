// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#include "common/md5.h"

#include <cassert>
#include <unistd.h>

using namespace std;

// Note, to support pipe + stdin with AmigaOS you need: Pipe-1.5 from Aminet, set _pchar | and using IN: as the file parameter, e.g.
// set _pchar |
// cat foo | md5 IN:

int main(int argc, char *argv[]) {
    const string fname = argc < 2 ? "-" : argv[1];
    bool is_stdin = (fname == "-");

    int fd = -1;
    if (is_stdin) {
        fd = STDIN_FILENO;
#ifndef NO_FREOPEN
        if (!freopen(NULL, "rb", stdin)) {
            fprintf(stderr, "Failed to freopen(rb) stdin\n");
            return EXIT_FAILURE;
        }
#endif
#ifdef __MINGW32__
        setmode (fileno (stdin), 0x8000);
#endif
    } else {
        FILE *f = fopen(fname.c_str(), "rb"); 
        if (!f) {
            fprintf(stderr, "File not found: %s\n", fname.c_str());
            return EXIT_FAILURE;
        }
        fd = fileno(f);
    }
    assert(fd >= 0);

    uint8_t buf[4096];
    ssize_t count;
    MD5 md5;
    while ((count = read(fd, buf, sizeof buf)) > 0) {
        md5.update(buf, count);
    }
    if (!is_stdin) {
        close(fd);
    }
    md5.finalize();
    string md5hex = md5.hexdigest();
    fprintf(stdout, "%s\n", md5hex.c_str());

    return EXIT_SUCCESS;
}
