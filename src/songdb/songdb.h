// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2014-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include "common/std/optional.h"
#include "common/std/string_view.h"

#include <cstdint>
#include <set>
#include <string>
#include <utility>

#include "common/constexpr.h"
#include "common/songend.h"

namespace songdb {

constexpr size_t XXH_MAX_BYTES = 256 * 1024; // 256 KiB

enum Source {
    //Hash = 0, // internal
    Songlengths = 1,
    ModInfos,
    Metadata,
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
    bool is_duplicate;
};

struct Info {
    std::vector<SubSongInfo> subsongs;
    std::optional<ModInfo> modinfo;
    std::optional<MetaData> metadata;
};

// if sources is empty, all builtin sources are used
void init(const std::string &songdb_path, const std::initializer_list<Source> &sources = {}) noexcept;

std::optional<SubSongInfo> lookup(const std::string &hash, int subsong) noexcept;
std::optional<Info> lookup(const std::string &hash) noexcept;

void update(const std::string &hash, const SubSongInfo &info, const int minsubsong, const int maxsubsong) noexcept;
void update(const std::string &hash, const ModInfo &modinfo) noexcept;

std::optional<std::pair<int,int>> subsong_range(const std::string &hash) noexcept;

namespace blacklist {
    bool is_blacklisted_extension(const std::string &path, const std::string &ext, const std::set<std::string> &whitelist) noexcept;
    bool is_blacklisted_hash(const std::string &hash) noexcept;
    bool is_blacklisted_songdb_hash(const std::string &hash) noexcept;
} // namespace songdb::blacklist

} // namespace songdb
