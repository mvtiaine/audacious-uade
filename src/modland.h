// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#ifndef MODLAND_H_
#define MODLAND_H_

#include <string>
#include <set>

using namespace std;

struct ModlandData {
    string format;
    string author;
    string album;
};

optional<ModlandData> modland_lookup(const char *md5);

#endif /* MODLAND_H_ */
