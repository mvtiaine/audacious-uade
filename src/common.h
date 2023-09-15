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
# ifdef AUDDBG
#  define DEBUG AUDDBG
# else
#  define DEBUG(fmt,...) fprintf(stderr, fmt, ## __VA_ARGS__)
# endif
#endif
#define WARN(fmt,...) fprintf(stderr, fmt, ## __VA_ARGS__)
#define ERROR(fmt,...) fprintf(stderr, fmt, ## __VA_ARGS__)

#if DEBUG_TRACE
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

inline bool starts_with(const string_view &str, const string_view &prefix) {
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

inline bool ends_with(const string_view &str, const string_view &suffix) {
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

#endif /* COMMON_H_ */
