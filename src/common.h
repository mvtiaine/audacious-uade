// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#ifndef COMMON_H_
#define COMMON_H_

#include <cstdio>
#include <string>
#include <vector>

using namespace std;

constexpr const char *PLUGIN_NAME = "uade";

//#define DEBUG_TRACE 1

#ifndef __cplusplus
# define DEBUG(fmt,...) fprintf(stderr, fmt, ## __VA_ARGS__)
#else
# include <libaudcore/runtime.h>
# define DEBUG AUDDBG
#endif
#define WARN(fmt,...) fprintf(stderr, fmt, ## __VA_ARGS__)
#define ERROR(fmt,...) fprintf(stderr, fmt, ## __VA_ARGS__)

#ifdef DEBUG_TRACE
# define TRACE DEBUG
#else
# define TRACE(fmt,...) while (0)
#endif

inline vector<string> split(const string &str, const string &delimiter) {
    vector<string> tokens;
    size_t pos = 0;
    string s = str;
    while ((pos = s.find(delimiter)) != string::npos) {
        if (pos != 0) {
            tokens.push_back(s.substr(0, pos));
        }
        s.erase(0, pos + delimiter.length());
    }
    tokens.push_back(s); // add last part
    
    return tokens;
}

#endif /* COMMON_H_ */
