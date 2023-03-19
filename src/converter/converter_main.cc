// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <fstream>
#include <iostream>
#include <vector>
#include "converter.h"

using namespace std;

int main(int argc, char *argv[]) {
    // TODO stdin support, filename out support
    if (argc < 2) {
        fprintf(stderr, "File not given\n");
        return -1;
    }
    const char* fname = argv[1];
    ifstream input(fname, ios::in | ios::binary | ios::ate );
    if (!input.is_open()) {
        fprintf(stderr, "File not found: %s\n", fname);
        return -1;
    }
    vector<char> buffer(input.tellg());
    input.seekg(0, std::ios::beg);
    input.read(buffer.data(), buffer.size());

    auto res = converter::convert(buffer.data(), buffer.size());
    if (!res.success) {
        fprintf(stderr, "Conversion failed: %s\n", res.reason_failed.c_str());
        return -1;
    }
    fwrite(res.data.data(), res.data.size(), 1, stdout);

    return 0;
}

