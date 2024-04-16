// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

#include "common/songend.h"

namespace songdb {

const std::string UNKNOWN_AUTHOR = "<Unknown>";
constexpr std::string_view AUTHOR_JOIN = " & ";

enum Source {
    //MD5 = 0, // internal
    Songlengths = 1,
    ModInfos,
    Modland,
    AMP,
    UnExotica,
    Demozoo,
};

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

struct ModInfo {
    std::string format;
    uint8_t channels;
};

struct SubSongInfo {
    uint8_t subsong;
    common::SongEnd songend;
};

struct Info {
    std::vector<SubSongInfo> subsongs;
    std::optional<ModInfo> modinfo;
    std::optional<ModlandData> modland;
    std::optional<AMPData> amp;
    std::optional<UnExoticaData> unexotica;
    std::optional<DemozooData> demozoo;
};

// if extra_sources is empty, all sources are initialized
void init(const std::string &songdb_path, const std::initializer_list<Source> &sources = {});

std::optional<SubSongInfo> lookup(const std::string &md5, int subsong);
std::optional<Info> lookup(const std::string &md5);

void update(const std::string &md5, const SubSongInfo &);
void update(const std::string &md5, const ModInfo &modinfo);

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
