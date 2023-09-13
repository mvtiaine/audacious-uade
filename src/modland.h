// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#ifndef MODLAND_H_
#define MODLAND_H_

#include <string>

using namespace std;

constexpr string_view UNKNOWN_AUTHOR = "<Unknown>";

struct ModlandData {
    string format;
    string author;
    string album;
    string filename;
};

bool parse_modland_path(const string &path, ModlandData &item, bool incoming);

#endif // MODLAND_H_