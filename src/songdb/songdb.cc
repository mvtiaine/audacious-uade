// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <algorithm>
#include <array>
#include <cassert>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <numeric>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "common/common.h"
#include "common/logger.h"
#include "songdb/songdb.h"

#include <string.h>

using namespace std;
using namespace songdb;

namespace songdb::modland {
    bool parse_tsv_row(const char *tuple, songdb::ModlandData &item);
}

namespace songdb::amp {
    bool parse_tsv_row(const char *tuple, songdb::AMPData &item);
}

namespace songdb::unexotica {
    bool parse_tsv_row(const char *tuple, songdb::UnExoticaData &item);
}

namespace songdb::demozoo {
    bool parse_tsv_row(const char *tuple, songdb::DemozooData &item);
}

namespace {

constexpr size_t BUF_SIZE = 2048;

#if 0
struct uint24_t {
    // 16 million md5s should be enough for everyone
    uint8_t data[3];
    constexpr uint24_t(const uint24_t &other) {
        data[0] = other.data[0];
        data[1] = other.data[1];
        data[2] = other.data[2];
    }
    constexpr uint24_t(const uint32_t val) {
        assert(val <= (1u << 24) - 1);
        data[0] = (val >> 16) & 0xff;
        data[1] = (val >> 8) & 0xff;
        data[2] = val & 0xff;
    }
    constexpr uint32_t value() const {
        return (data[0] << 16) | (data[1] << 8) | data[2];
    }
    constexpr operator uint32_t() const {
        return value();
    }
    constexpr bool operator==(const uint24_t &other) const {
        return value() == other.value();
    }
    constexpr bool operator<(const uint24_t &other) const {
        return value() < other.value();
    }
    constexpr bool operator>(const uint24_t &other) const {
        return value() > other.value();
    }
} __attribute__((packed));

struct uint48_t {
    uint16_t data[3];
    constexpr uint48_t(const uint48_t &other) {
        data[0] = other.data[0];
        data[1] = other.data[1];
        data[2] = other.data[2];
    }
    constexpr uint48_t(const uint64_t val) {
        assert(val <= (1ull << 48) - 1);
        data[0] = (val >> 32) & 0xffff;
        data[1] = (val >> 16) & 0xffff;
        data[2] = val & 0xffff;
    }
    constexpr uint64_t value() const {
        return
            (static_cast<uint64_t>(data[0]) << 32) |
            (static_cast<uint64_t>(data[1]) << 16) |
            data[2];
    }
    constexpr operator uint64_t() const {
        return value();
    }
    constexpr bool operator==(const uint48_t &other) const {
        return value() == other.value();
    }
    constexpr bool operator<(const uint48_t &other) const {
        return value() < other.value();
    }
    constexpr bool operator>(const uint48_t &other) const {
        return value() > other.value();
    }
} __attribute__((packed));

#else

typedef uint32_t uint24_t;
typedef uint64_t uint48_t;

#endif

constexpr int32_t UINT24_T_MAX = (1u << 24) - 1;
constexpr int64_t UINT48_T_MAX = (1ull << 48) - 1;

typedef uint8_t subsong_t;
typedef uint8_t year_t;
typedef uint24_t songlength_t;
typedef uint48_t md5_t;
constexpr md5_t MD5_T_MAX = UINT48_T_MAX;
// indexed types
typedef uint24_t string_t;
constexpr string_t STRING_NOT_FOUND = UINT24_T_MAX;
typedef uint24_t md5_idx_t;
constexpr md5_idx_t MD5_NOT_FOUND = UINT24_T_MAX;

typedef common::SongEnd::Status songend_t;

struct _Data {
    md5_idx_t md5;
} __attribute__((packed));
template <typename T>
concept _Data_ = is_base_of<_Data, T>::value;

struct _ModInfo : _Data {
    string_t format;
    uint8_t channels;
} __attribute__((packed));

struct _ModlandData : _Data {
    string_t author;
    string_t album;
} __attribute__((packed));

struct _UnExoticaData : _Data {
    string_t author;
    string_t album;
    string_t publisher;
    year_t year;
} __attribute__((packed));

struct _AMPData : _Data {
    string_t author;
} __attribute__((packed));;

struct _DemozooData : _Data {
    string_t author;
    string_t album;
    string_t publisher;
    year_t year;
} __attribute__((packed));

struct _SongInfo {
    subsong_t subsong;
    songlength_t songlength;
    songend_t songend;
} __attribute__((packed));

md5_t hex2md5(const char *hex) {
    md5_t ret = 0; 
    for (int i = 0; i < 12; ++i) {
        const char c = *hex++;
        ret = (ret << 4);
        if (c > 58) {
            ret |= (c - 87);
        } else {
            ret |= (c - 48);
        }
   }
   return ret; 
}

vector<md5_t> md5_idx;
void create_string_pool(vector<string> &strings) {
    assert(string_pool.empty());
    for (auto s : strings) {
        s.shrink_to_fit();
        string_pool.push_back(s);
    }
    assert(string_pool.size() < STRING_NOT_FOUND);
}

md5_idx_t dedup_md5(const string_view &md5) {
    if (md5_idx.empty()) return MD5_NOT_FOUND;
    assert(md5.size() >= 12);
    const md5_t hash = hex2md5(md5.data());
    unsigned int idx = ((double)hash / MD5_T_MAX) * md5_idx.size();
    assert(idx < md5_idx.size());
    md5_t cmp = md5_idx[idx];
    if (cmp == hash) {
         return md5_idx_t { idx };
    } else if (cmp > hash) {
        while (cmp > hash && idx > 0) {
            idx--;
            cmp = md5_idx[idx];
        }
        return (cmp == hash) ? md5_idx_t { idx } : MD5_NOT_FOUND;
    } else {
        while (cmp < hash && idx < md5_idx.size() - 1) {
            idx++;
            cmp = md5_idx[idx];
        }
        return (cmp == hash) ? md5_idx_t { idx } : MD5_NOT_FOUND;
    }
}

songend_t parse_songend(const string_view &songend) {
    if (songend == "e") return common::SongEnd::ERROR;
    if (songend == "p") return common::SongEnd::PLAYER;
    if (songend == "t") return common::SongEnd::TIMEOUT;
    if (songend == "s") return common::SongEnd::DETECT_SILENCE;
    if (songend == "l") return common::SongEnd::DETECT_LOOP;
    if (songend == "v") return common::SongEnd::DETECT_VOLUME;
    if (songend == "r") return common::SongEnd::DETECT_REPEAT;
    if (songend == "p+s") return common::SongEnd::PLAYER_PLUS_SILENCE;
    if (songend == "p+v") return common::SongEnd::PLAYER_PLUS_VOLUME;
    if (songend == "l+s") return common::SongEnd::LOOP_PLUS_SILENCE;
    if (songend == "l+v") return common::SongEnd::LOOP_PLUS_VOLUME;
    if (songend == "n") return common::SongEnd::NOSOUND;
    assert(false);
    return common::SongEnd::NONE;
}

enum Source {
    ModInfos,
    Modland,
    AMP,
    UnExotica,
    Demozoo,
};

const vector<pair<string, Source>> tsvfiles ({
    {"modland.tsv", Modland},
    {"amp.tsv", AMP},
    {"unexotica.tsv", UnExotica},
    {"demozoo.tsv", Demozoo},
});

vector<vector<_SongInfo>> db_songlengths; // md5_idx_t ->
unordered_map<md5_t, tuple<string, uint8_t, vector<_SongInfo>>> extra_songlengths; // runtime only
vector<_ModInfo> db_modinfos;
vector<_ModlandData> db_modland;
vector<_AMPData> db_amp;
vector<_UnExoticaData> db_unexotica;
vector<_DemozooData> db_demozoo;

bool initialized = false;

const string make_string(const string_t s) {
    if (s == STRING_NOT_FOUND) return "";
    return string_pool[s];
}

template <_Data_ T>
optional<T> find(const vector<T> &db, const md5_idx_t md5) {
    if (md5 >= md5_idx.size()) return optional<T>();
    unsigned int idx = ((double)md5 / md5_idx.size()) * db.size();
    assert(idx < db.size());
    T val = db[idx];
    md5_idx_t cmp = val.md5;
    if (cmp == md5) {
        return val;
    } else if (cmp > md5) {
        while (cmp > md5 && idx > 0) {
            idx--;
            cmp = db[idx].md5;
        }
        return cmp == md5 ? db[idx] : optional<T>();
    } else {
        while (cmp < md5 && idx < db.size() - 1) {
            idx++;
            cmp = db[idx].md5;
        }
        return cmp == md5 ? db[idx] : optional<T>();
    }
}

optional<ModlandData> make_modland(const md5_idx_t md5) {
    const auto data = find(db_modland, md5);
    if (data) {
        return ModlandData {
            make_string(data->author),
            make_string(data->album),
        };
    }
    return {};
}

optional<AMPData> make_amp(const md5_idx_t md5) {
    const auto data = find(db_amp, md5);
    if (data) {
        return AMPData {
           make_string(data->author),
        };
    }
    return {};
}

optional<UnExoticaData> make_unexotica(const md5_idx_t md5) {
    const auto data = find(db_unexotica, md5);
    if (data) {
        return UnExoticaData {
            make_string(data->author),
            make_string(data->album),
            make_string(data->publisher),
            static_cast<uint16_t>(data->year > 0 ? (1900u + data->year) : 0),
        };
    };
    return {};
}

optional<DemozooData> make_demozoo(const md5_idx_t md5) {
    const auto data = find(db_demozoo, md5);
    if (data) {
        return DemozooData {
            make_string(data->author),
            make_string(data->album),
            make_string(data->publisher),
            static_cast<uint16_t>(data->year > 0 ? (1900u + data->year) : 0),
        };
    };
    return {};
}

SongInfo make_info(const md5_idx_t md5, const string &format, const uint8_t channels, const _SongInfo &info) {
    return {
        info.subsong,
        info.songlength,
        common::SongEnd::status_string(info.songend),
        format,
        channels,
        make_modland(md5),
        make_amp(md5),
        make_unexotica(md5),
        make_demozoo(md5),
    };
}

void parse_songlengths(const string &tsv) {
    FILE *f = fopen(tsv.c_str(), "r"); 
    if (!f) {
        ERR("Could not open songdb file %s\n", tsv.c_str());
        return;
    }
    assert(md5_idx.empty());

    md5_t prevhash = 0;
    char line[BUF_SIZE];
    while (fgets(line, sizeof line, f)) {
        const md5_t hash = hex2md5(line);
        assert(hash > prevhash);
        md5_idx.push_back(hash);
        prevhash = hash;
        const auto cols = common::split_view_x<2>(line + 13, '\t');
        const uint8_t minsubsong = common::from_chars<uint8_t>(cols[0]);
        const auto subsongs = common::split_view(cols[1], ' ');
        uint8_t subsong = minsubsong;
        vector<_SongInfo> infos;
        for (const auto &col : subsongs) {
            const auto e = common::split_view<2>(col, ',');
            const uint24_t songlength = common::from_chars<uint24_t>(e[0]);
            const auto songend = parse_songend(e[1]);
            infos.push_back({ subsong++, songlength, songend});
        }
        db_songlengths.push_back(infos);
    }

    fclose(f);
}

void parse_tsv(const string &tsv, const Source source, vector<string> &strings) {
    FILE *f = fopen(tsv.c_str(), "r"); 
    if (!f) {
        ERR("Could not open songdb file %s\n", tsv.c_str());
        return;
    }
    char line[BUF_SIZE];
    string_t prev_author = STRING_NOT_FOUND;
    string_t prev_album = STRING_NOT_FOUND;
    string_t prev_publisher = STRING_NOT_FOUND;
    year_t prev_year = 0;
    string prev_tuple;
    while (fgets(line, sizeof line, f)) {
        const auto md5 = dedup_md5(line);
        assert(md5 != MD5_NOT_FOUND);
        char *tuple = line + 13;
        switch (source) {
            case Modland: {
                if (prev_tuple == tuple) {
                    const _ModlandData _item = {
                        { md5 },
                        prev_author,
                        prev_album,
                    };
                    db_modland.push_back(_item);
                    continue;
                }
                ModlandData item {};
                if (modland::parse_tsv_row(tuple, item)) {
                    string_t sc = strings.size();
                    string_t author = item.author.empty() ? STRING_NOT_FOUND : sc++;
                    string_t album = item.album.empty() ? STRING_NOT_FOUND : sc++;
                    if (author != STRING_NOT_FOUND) strings.push_back(item.author);
                    if (album != STRING_NOT_FOUND) strings.push_back(item.album);
                    assert(strings.size() < STRING_NOT_FOUND);
                    const _ModlandData _item = {
                        { md5 },
                        author,
                        album,
                    };
                    db_modland.push_back(_item);
                    prev_author = author;
                    prev_album = album;
                }
                prev_tuple = tuple;
                break;
            }
            case AMP: {
                if (prev_tuple == tuple) {
                    const _AMPData _item = {
                        { md5 },
                        prev_author,
                    };
                    db_amp.push_back(_item);
                    continue;
                }
                AMPData item {};
                if (amp::parse_tsv_row(tuple, item)) {
                    string_t sc = strings.size();
                    string_t author = item.author.empty() ? STRING_NOT_FOUND : sc++;
                    if (author != STRING_NOT_FOUND) strings.push_back(item.author);
                    assert(strings.size() < STRING_NOT_FOUND);
                    const _AMPData _item = {
                        { md5 },
                        author,
                    };
                    db_amp.push_back(_item);
                    prev_author = author;
                }
                prev_tuple = tuple;
                break;
            }
            case UnExotica: {
                if (prev_tuple == tuple) {
                    const _UnExoticaData _item = {
                        { md5 },
                        prev_author,
                        prev_album,
                        prev_publisher,
                        prev_year,
                    };
                    db_unexotica.push_back(_item);
                    continue;
                }
                UnExoticaData item {};
                if (unexotica::parse_tsv_row(tuple, item)) {
                    string_t sc = strings.size();
                    string_t author = item.author.empty() ? STRING_NOT_FOUND : sc++;
                    string_t album = item.album.empty() ? STRING_NOT_FOUND : sc++;
                    string_t publisher = item.publisher.empty() ? STRING_NOT_FOUND : sc++;
                    if (author != STRING_NOT_FOUND) strings.push_back(item.author);
                    if (album != STRING_NOT_FOUND) strings.push_back(item.album);
                    if (publisher != STRING_NOT_FOUND) strings.push_back(item.publisher);
                    assert(strings.size() < STRING_NOT_FOUND);
                    year_t year = item.year != 0 ? item.year - 1900u : 0;
                    assert(year <= UINT8_MAX);
                    const _UnExoticaData _item = {
                        { md5 },
                        author,
                        album,
                        publisher,
                        year,
                    };
                    db_unexotica.push_back(_item);
                    prev_author = author;
                    prev_album = album;
                    prev_publisher = publisher;
                    prev_year = year;
                }
                prev_tuple = tuple;
                break;
            }
            case Demozoo: {
                if (prev_tuple == tuple) {
                    const _DemozooData _item = {
                        { md5 },
                        prev_author,
                        prev_album,
                        prev_publisher,
                        prev_year,
                    };
                    db_demozoo.push_back(_item);
                    continue;
                }
                DemozooData item {};
                if (demozoo::parse_tsv_row(tuple, item)) {
                    string_t sc = strings.size();
                    string_t author = item.author.empty() ? STRING_NOT_FOUND : sc++;
                    string_t album = item.album.empty() ? STRING_NOT_FOUND : sc++;
                    string_t publisher = item.publisher.empty() ? STRING_NOT_FOUND : sc++;
                    if (author != STRING_NOT_FOUND) strings.push_back(item.author);
                    if (album != STRING_NOT_FOUND) strings.push_back(item.album);
                    if (publisher != STRING_NOT_FOUND) strings.push_back(item.publisher);
                    assert(strings.size() < STRING_NOT_FOUND);
                    year_t year = item.year != 0 ? item.year - 1900u : 0;
                    assert(year <= UINT8_MAX);
                    const _DemozooData _item = {
                        { md5 },
                        author,
                        album,
                        publisher,
                        year,
                    };
                    db_demozoo.push_back(_item);
                    prev_author = author;
                    prev_album = album;
                    prev_publisher = publisher;
                    prev_year = year;
                }
                prev_tuple = tuple;
                break;
            }
            default: assert(false); break;
        }
    };
    fclose(f);
}

void parse_modinfos(const string &tsv, vector<string> &strings) {
    FILE *f = fopen(tsv.c_str(), "r"); 
    if (!f) {
        ERR("Could not open songdb file %s\n", tsv.c_str());
        return;
    }
    char line[BUF_SIZE];
    string prev_tuple;
    string_t prev_format = STRING_NOT_FOUND;
    uint8_t prev_channels = 0;
    while (fgets(line, sizeof line, f)) {
        const auto md5 = dedup_md5(line);
        assert(md5 != MD5_NOT_FOUND);
        char *tuple = line + 13; // skip md5
        if (prev_tuple == tuple) {
            const _ModInfo _info {{ md5 }, prev_format, prev_channels};
            db_modinfos.push_back(_info);
            continue;
        }
        const auto cols = common::split_view_x<2>(tuple, '\t');
        string_t format;
        if (cols[0].empty()) {
            format = STRING_NOT_FOUND;
        } else {
            format = strings.size();
            strings.push_back(string(cols[0]));
            assert(strings.size() < STRING_NOT_FOUND);
        }
        const uint8_t channels = common::from_chars<uint8_t>(cols[1]);
        const _ModInfo _info = {{ md5 }, format, channels};
        db_modinfos.push_back(_info);
        prev_format = format;
        prev_channels = channels;
        prev_tuple = tuple;
    }
    fclose(f);
}

} // namespace {}

namespace songdb {

optional<SongInfo> lookup(const string &md5, int subsong) {
    const auto md5_idx = dedup_md5(md5);
    const md5_t hash = hex2md5(md5.c_str());
    if (md5_idx != MD5_NOT_FOUND) {
        for (const auto &info : db_songlengths[md5_idx]) {
            if (info.subsong == subsong) {
                const auto modinfo = find(db_modinfos, md5_idx);
                assert(modinfo.has_value());
                return make_info(md5_idx, make_string(modinfo->format), modinfo->channels, info);
            }
        }
    }
    if (extra_songlengths.contains(hash)) {
        const auto &tuple = extra_songlengths[hash];
        const auto &format = get<0>(tuple);
        const auto channels = get<1>(tuple);
        for (const auto &info : get<2>(tuple)) {
            if (info.subsong == subsong) {
                return make_info(md5_idx, format, channels, info);
            }
        }
    }
    return {};
}

vector<SongInfo> lookup_all(const string &md5) {
    const auto md5_idx = dedup_md5(md5);
    if (md5_idx != MD5_NOT_FOUND) {
        const auto modinfo = find(db_modinfos, md5_idx);
        assert(modinfo.has_value());
        vector<SongInfo> res;
        for (const auto &i : db_songlengths[md5_idx]) {
            res.push_back(make_info(md5_idx, make_string(modinfo->format), modinfo->channels, i));
        }
        return res;
    }
    return {};
}

void init(const string &songdb_path) {
    if (initialized) {
        return;
    }
    vector<string> strings;

    parse_songlengths(songdb_path + "/songlengths.tsv");

    parse_modinfos(songdb_path + "/modinfos.tsv", strings);

    for (const auto &tsv : tsvfiles) {
        parse_tsv(songdb_path + "/" + tsv.first, tsv.second, strings);
    }
    
    create_string_pool(strings);

    const auto sorter = [](const _Data &a, const _Data &b) { return a.md5 < b.md5; };
    sort(db_modinfos.begin(), db_modinfos.end(), sorter);
    sort(db_modland.begin(), db_modland.end(), sorter);
    sort(db_amp.begin(), db_amp.end(), sorter);
    sort(db_unexotica.begin(), db_unexotica.end(), sorter);
    sort(db_demozoo.begin(), db_demozoo.end(), sorter);

    initialized = true;
}

void update(const string &md5, const int subsong, const int songlength, common::SongEnd::Status songend, const string &format, const int channels) {
    if (md5.empty() || subsong < 0 || subsong > 255) {
        WARN("Invalid songdb key md5:%s subsong:%d\n", md5.c_str(), subsong);
        return;
    }
    if (blacklist::is_blacklisted_songdb_key(md5)) {
        INFO("Blacklisted songdb key md5:%s\n", md5.c_str());
        return;
    }
    const md5_t hash = hex2md5(md5.c_str());
    assert(subsong >= 0);
    assert(subsong <= 255);
    assert(songlength >= 0);
    assert(songlength <= UINT24_T_MAX);

    _SongInfo info = { static_cast<subsong_t>(subsong), static_cast<songlength_t>(songlength), songend };
    if (extra_songlengths.contains(hash)) {
        auto &entry = extra_songlengths[hash];
        auto &infos = get<2>(entry);
        for (const auto &info : infos) {
            if (info.subsong == subsong) {
                WARN("Skipped songdb update for %s:%d, already exists\n", md5.c_str(), subsong);
            }
        }
        infos.push_back(info);
        sort(infos.begin(), infos.end(), []( _SongInfo &a, _SongInfo &b) { return a.subsong < b.subsong; });
    } else {
        vector<_SongInfo> infos;
        infos.push_back(info);
        extra_songlengths.insert({hash, {format, channels, infos}});
    }
}

optional<pair<int,int>> subsong_range(const string &md5) {
    const auto md5_idx = dedup_md5(md5);
    if (md5_idx != MD5_NOT_FOUND) {
        const auto &infos = db_songlengths[md5_idx];
        return pair(infos.front().subsong, infos.back().subsong);
    }
    return {};
}

} // namespace songdb