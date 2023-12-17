// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

#include "common/common.h"

namespace songdb {

const std::string UNKNOWN_AUTHOR = "<Unknown>";

struct ModlandData {
    std::string author;
    std::string album;
};

struct UnExoticaData {
    std::string author;
    std::string album;
    std::string publisher;
    uint16_t year;
};

struct AMPData {
    std::string author;
};

struct DemozooData {
    std::string author;
    std::string album;
    std::string publisher;
    uint16_t year;
};

struct SongInfo {
    const uint8_t subsong;
    const uint32_t songlength;
    const std::string songend;
    const std::string format;
    const uint8_t channels;
    const std::optional<ModlandData> modland_data;
    const std::optional<AMPData> amp_data;
    const std::optional<UnExoticaData> unexotica_data;
    const std::optional<DemozooData> demozoo_data;
};

void init(const std::string &songdb_path);
std::optional<SongInfo> lookup(const std::string &md5, int subsong);
std::vector<SongInfo> lookup_all(const std::string &md5);
void update(const std::string &md5, const int subsong, const int songlength, common::SongEnd::Status songend, const std::string &format, const int channels);
std::optional<std::pair<int,int>> subsong_range(const std::string &md5);

namespace blacklist {
    bool is_blacklisted_extension(const std::string &path, const std::string &ext);
    bool is_blacklisted_md5(const std::string &md5hex);
    bool is_blacklisted_songdb_key(const std::string &md5hex);
} // namespace songdb::blacklist

namespace unexotica {
    std::string author_path(const std::string &author);
} // namespace songdb::unexotica

} // namespace songdb
