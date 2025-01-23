// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include "common/std/optional.h"
#include "common/std/string_view.h"

#include <cstdint>
#include <string>
#include <utility>

#include "common/constexpr.h"
#include "common/songend.h"

namespace songdb {

enum Source {
    //MD5 = 0, // internal
    Songlengths = 1,
    ModInfos,
    Combined,
    // these are by default not included in build
    Modland,
    AMP,
    UnExotica,
    Demozoo,
};

struct MetaData {
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
    // combined metadata
    std::optional<MetaData> combined;
    // these are by default not included in build
    std::optional<MetaData> modland;
    std::optional<MetaData> amp;
    std::optional<MetaData> unexotica;
    std::optional<MetaData> demozoo;
};

// if sources is empty, all builtin sources are used
void init(const std::string &songdb_path, const std::initializer_list<Source> &sources = {}) noexcept;

std::optional<SubSongInfo> lookup(const std::string &md5, int subsong) noexcept;
std::optional<Info> lookup(const std::string &md5) noexcept;

void update(const std::string &md5, const SubSongInfo &info, const int minsubsong, const int maxsubsong) noexcept;
void update(const std::string &md5, const ModInfo &modinfo) noexcept;

std::optional<std::pair<int,int>> subsong_range(const std::string &md5) noexcept;

namespace blacklist {
    bool is_blacklisted_extension(const std::string &path, const std::string &ext) noexcept;
    bool is_blacklisted_md5(const std::string &md5hex) noexcept;
    bool is_blacklisted_songdb_key(const std::string &md5hex) noexcept;
} // namespace songdb::blacklist

} // namespace songdb
