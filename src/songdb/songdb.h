// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <optional>
#include <string>
#include <utility>

namespace songdb {

constexpr std::string_view UNKNOWN_AUTHOR = "<Unknown>";

struct ModlandData {
    std::string path;
    std::string format;
    std::string author;
    std::string album;
    std::string filename;
};

struct UnExoticaData {
    std::string path;
    std::string author;
    std::string album;
    std::string note;
    std::string filename;
};

struct AMPData {
    std::string path;
    std::string author;
    std::string filename;
};

struct SongInfo {
    std::string md5;
    int subsong;
    int length;
    std::string status;
    ssize_t size;
    std::optional<ModlandData> modland_data;
    std::optional<AMPData> amp_data;
    std::optional<UnExoticaData> unexotica_data;

    SongInfo(std::string md5, int subsong, int length, std::string status, ssize_t size)
        : md5(md5), subsong(subsong), length(length), status(status), size(size) {}
    SongInfo(std::string md5, int subsong, int length, std::string status, ssize_t size, std::optional<ModlandData> modland_data)
        : md5(md5), subsong(subsong), length(length), status(status), size(size), modland_data(modland_data) {}
    SongInfo(std::string md5, int subsong, int length, std::string status, ssize_t size, std::optional<AMPData> amp_data)
        : md5(md5), subsong(subsong), length(length), status(status), size(size), amp_data(amp_data) {}
    SongInfo(std::string md5, int subsong, int length, std::string status, ssize_t size, std::optional<UnExoticaData> unexotica_data)
        : md5(md5), subsong(subsong), length(length), status(status), size(size), unexotica_data(unexotica_data) {}
};

void init(const std::string &songdb_path);
std::optional<SongInfo> lookup(const std::string &md5, int subsong, const std::string &path);
std::vector<SongInfo> lookup_all(const std::string &md5, int subsong);
void update(const SongInfo &songinfo);
std::optional<std::pair<int,int>> subsong_range(const std::string &md5);
bool exists(const std::string &path, const ssize_t size);


namespace modland {
    bool parse_path(const std::string &path, songdb::ModlandData &item, bool incoming);
} // namespace songdb::modland

namespace amp {
    bool parse_path(const std::string &path, songdb::AMPData &item);
} // namespace songdb::amp

namespace unexotica {
    bool parse_path(const std::string &path, songdb::UnExoticaData &item);
} // namespace songdb::unexotica

namespace blacklist {
    bool is_blacklisted_extension(const std::string &path, const std::string &ext);
    bool is_blacklisted_md5(const std::string &md5hex);
    bool is_blacklisted_songdb_key(const std::string &md5hex);
} // namespace songdb::blacklist

} // namespace songdb
