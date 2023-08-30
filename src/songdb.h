// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#ifndef SONGDB_H_
#define SONGDB_H_

#include <optional>
#include <string>

using namespace std;

struct ModlandData {
    string format;
    string author;
    string album;
    string filename;
};

struct SongInfo {
    string md5;
    int subsong;
    int length;
    string status;
    optional<ModlandData> modland_data;
};

void songdb_init();
optional<SongInfo> songdb_lookup(const string &md5, int subsong, const string &filename);
void songdb_update(const SongInfo &songinfo);
optional<pair<int,int>> songdb_subsong_range(const string &md5);

#endif /* SONGDB_H_ */
