// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <fstream>
#include <iostream>
#include <vector>

#include "common/md5.h"
#include "3rdparty/SimpleBinStream.h"

using namespace std;

int main(int argc, char *argv[]) {
    const string fname = argc < 2 ? "-" : argv[1];
    bool is_stdin = (fname == "-");

    ifstream file;
    if (is_stdin) {
// TODO configure.ac check for freopen
#if !defined(__HAIKU__) && !defined(__OpenBSD__) && !defined(__NetBSD__)
        if (!freopen(NULL, "rb", stdin)) {
            fprintf(stderr, "Failed to freopen(rb) stdin\n");
            return EXIT_FAILURE;
        }
#endif
    } else {
        file = ifstream(fname, ios::in | ios::binary);
        if (!file.is_open()) {
            fprintf(stderr, "File not found: %s\n", fname.c_str());
            return EXIT_FAILURE;
        }
    }
    
    istream& input = is_stdin ? cin : file;
    simple::mem_ostream<true_type> out;

    while (input.good()) {
        vector<char> buf(4096);
        input.read(buf.data(), buf.capacity());
        buf.resize(input.gcount());
        out << buf;
    }

    if (input.fail() && !input.eof()) {
        fprintf(stderr, "Could not read: %s\n", fname.c_str());
        return EXIT_FAILURE;
    }

    vector<char> buffer = out.get_internal_vec();
    MD5 md5; md5.update((const unsigned char *)buffer.data(), buffer.size()); md5.finalize();
    string md5hex = md5.hexdigest();

    fprintf(stdout, "%s\n", md5hex.c_str());

    return EXIT_SUCCESS;
}
