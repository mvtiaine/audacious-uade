// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <vector>

#include "converter/converter.h"

#include <sys/stat.h>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[]) {
    // TODO stdin support, filename out support
    if (argc < 2) {
        fprintf(stderr, "File not given\n");
        return EXIT_FAILURE;
    }
    const char *fname = argv[1];
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

    auto res = converter::convert(buffer.data(), buffer.size());
    if (!res.success) {
        fprintf(stderr, "Conversion failed: %s\n", res.reason_failed.c_str());
        return EXIT_FAILURE;
    }
#ifdef __MINGW32__
    _setmode(_fileno(stdout), 0x8000);
#endif
    fwrite(res.data.data(), res.data.size(), 1, stdout);

    return EXIT_SUCCESS;
}

