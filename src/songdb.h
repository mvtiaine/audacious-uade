// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#ifndef SONGDB_H_
#define SONGDB_H_

#include <optional>
#include <string>

#include "modland.h"

using namespace std;

struct UnExoticaData {
    string path;
    string author;
    string album;
    string note;
    string filename;
};

struct AMPData {
    string path;
    string author;
    string filename;
};

struct SongInfo {
    string md5;
    int subsong;
    int length;
    string status;
    ssize_t size;
    optional<ModlandData> modland_data;
    optional<AMPData> amp_data;
    optional<UnExoticaData> unexotica_data;

    SongInfo(string md5, int subsong, int length, string status, ssize_t size)
        : md5(md5), subsong(subsong), length(length), status(status), size(size) {}
    SongInfo(string md5, int subsong, int length, string status, ssize_t size, optional<ModlandData> modland_data)
        : md5(md5), subsong(subsong), length(length), status(status), size(size), modland_data(modland_data) {}
    SongInfo(string md5, int subsong, int length, string status, ssize_t size, optional<AMPData> amp_data)
        : md5(md5), subsong(subsong), length(length), status(status), size(size), amp_data(amp_data) {}
    SongInfo(string md5, int subsong, int length, string status, ssize_t size, optional<UnExoticaData> unexotica_data)
        : md5(md5), subsong(subsong), length(length), status(status), size(size), unexotica_data(unexotica_data) {}
};

void songdb_init();
optional<SongInfo> songdb_lookup(const string &md5, int subsong, const string &path);
void songdb_update(const SongInfo &songinfo);
optional<pair<int,int>> songdb_subsong_range(const string &md5);

#endif /* SONGDB_H_ */
