// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <algorithm>
#include <cassert>
#include <climits>
#include <fstream>
#include <map>
#include <memory>
#include <numeric>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "common/common.h"
#include "common/logger.h"
#include "songdb/songdb.h"

using namespace std;
using namespace songdb;

namespace songdb::modland {
    bool parse_tsv_row(const std::vector<std::string> &cols, songdb::ModlandData &item);
}

namespace songdb::amp {
    bool parse_tsv_row(const std::vector<std::string> &cols, songdb::AMPData &item);
}

namespace songdb::unexotica {
    bool parse_tsv_row(const std::vector<std::string> &cols, songdb::UnExoticaData &item);
}

namespace songdb::demozoo {
    bool parse_tsv_row(const std::vector<std::string> &cols, songdb::DemozooData &item);
}

namespace {

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
constexpr int32_t UINT24_T_MAX = (1u << 24) - 1;

struct uint48_t {
    uint16_t data[3];
    constexpr uint48_t(const uint48_t &other) {
        data[0] = other.data[0];
        data[1] = other.data[1];
        data[2] = other.data[2];
    }
    constexpr uint48_t(const uint64_t val) {
        assert(val <= (1ul << 48) - 1);
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
constexpr int64_t UINT48_T_MAX = (1ul << 48) - 1;

typedef uint8_t subsong_t;
typedef uint8_t year_t;
typedef uint24_t songlength_t;
typedef uint48_t hash_t;
constexpr hash_t HASH_T_MAX = UINT48_T_MAX;
// indexed types
typedef uint16_t string_t;
constexpr string_t STRING_NOT_FOUND = UINT16_MAX;
typedef uint24_t md5_t;
constexpr md5_t MD5_NOT_FOUND = UINT24_T_MAX;

typedef common::SongEnd::Status songend_t;

struct _Data {
    md5_t md5;
} __attribute__((packed));
template <typename T>
concept _Data_ = is_base_of<_Data, T>::value;

struct _ModInfo {
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

vector<hash_t> md5_idx;
vector<string> string_pool;

hash_t _hash(const string &str) {
    const uint64_t hash = ::hash<string>{}(str);
    return { hash >> 16 };
}

string_t dedup_string(const string &str) {
    const auto hash = _hash(str);
    unsigned int idx = ((double)hash / HASH_T_MAX) * string_pool.size();
    assert(idx < string_pool.size());
    hash_t cmp = _hash(string_pool[idx]);
    if (cmp == hash) {
        return idx;
    } else if (cmp > hash) {
        while (cmp > hash && idx > 0) {
            idx--;
            cmp = _hash(string_pool[idx]);
        }
        return (cmp == hash) ? idx : STRING_NOT_FOUND;
    } else {
        while (cmp < hash && idx < string_pool.size() - 1) {
            idx++;
            cmp = _hash(string_pool[idx]);
        }
        return (cmp == hash) ? idx : STRING_NOT_FOUND;
    }
}

void create_string_pool(const set<string> &strings) {
    map<hash_t, string> hashes;
    for (const auto &str : strings) {
        string tmp = str;
        tmp.shrink_to_fit();
        hashes.insert({_hash(tmp), tmp});
    }
    assert(string_pool.empty());
    for (const auto &hash : hashes) {
        string_pool.push_back(hash.second);
    }
}

md5_t dedup_md5(const string &md5) {
    if (md5_idx.size() == 0) return MD5_NOT_FOUND;
    assert(md5.size() == 12 || md5.size() == 32);
    const hash_t hash = stoul(md5.substr(0, 12), 0, 16);
    unsigned int idx = ((double)hash / HASH_T_MAX) * md5_idx.size();
    assert(idx < md5_idx.size());
    hash_t cmp = md5_idx[idx];
    if (cmp == hash) {
         return md5_t { idx };
    } else if (cmp > hash) {
        while (cmp > hash && idx > 0) {
            idx--;
            cmp = md5_idx[idx];
        }
        return (cmp == hash) ? md5_t { idx } : MD5_NOT_FOUND;
    } else {
        while (cmp < hash && idx < md5_idx.size() - 1) {
            idx++;
            cmp = md5_idx[idx];
        }
        return (cmp == hash) ? md5_t { idx } : MD5_NOT_FOUND;
    }
}

void create_md5_idx(const set<hash_t> &md5s) {
    assert(md5_idx.empty());
    for (const auto &md5 : md5s) {
        md5_idx.push_back(md5);
    }
}

constexpr songend_t parse_songend(const string &songend) {
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

vector<vector<_SongInfo>> db_songlengths; // md5_t ->
vector<_ModInfo> db_modinfos; // md5_t ->
map<hash_t, tuple<string, uint8_t, vector<_SongInfo>>> extra_songlengths; // runtime only
vector<_ModlandData> db_modland;
vector<_AMPData> db_amp;
vector<_UnExoticaData> db_unexotica;
vector<_DemozooData> db_demozoo;

bool initialized = false;

const string make_string(const string_t s) {
    assert(string_pool.size() > s);
    return string_pool[s];
}

template <_Data_ T>
optional<T> find(const vector<T> &db, const md5_t md5) {
    if (md5 >= md5_idx.size()) return optional<T>();
    unsigned int idx = ((double)md5 / md5_idx.size()) * db.size();
    assert(idx < db.size());
    T val = db[idx];
    md5_t cmp = val.md5;
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

optional<ModlandData> make_modland(const md5_t md5) {
    const auto data = find(db_modland, md5);
    if (data.has_value()) {
        return ModlandData {
            make_string(data->author),
            make_string(data->album),
        };
    }
    return {};
}

optional<AMPData> make_amp(const md5_t md5) {
    const auto data = find(db_amp, md5);
    if (data.has_value()) {
        return AMPData {
           make_string(data->author),
        };
    }
    return {};
}

optional<UnExoticaData> make_unexotica(const md5_t md5) {
    const auto data = find(db_unexotica, md5);
    if (data.has_value()) {
        return UnExoticaData {
            make_string(data->author),
            make_string(data->album),
            make_string(data->publisher),
            static_cast<uint16_t>(data->year > 0 ? (1900u + data->year) : 0),
        };
    };
    return {};
}

optional<DemozooData> make_demozoo(const md5_t md5) {
    const auto data = find(db_demozoo, md5);
    if (data.has_value()) {
        return DemozooData {
            make_string(data->author),
            make_string(data->album),
            make_string(data->publisher),
            static_cast<uint16_t>(data->year > 0 ? (1900u + data->year) : 0),
        };
    };
    return {};
}

SongInfo make_info(const md5_t md5, const string &format, const uint8_t channels, const _SongInfo &info) {
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

void parse_songlengths(const string &tsv, set<string> &strings) {
    ifstream songdbtsv(tsv, ios::in);
    if (!songdbtsv.is_open()) {
        ERR("Could not open songdb file %s\n", tsv.c_str());
        return;
    }
    set<hash_t> md5s;
    string line;
    hash_t prevhash = 0;
    while (getline(songdbtsv, line)) {
        const auto cols = common::split(line, "\t");
        assert(cols.size() == 5);
        const auto &md5 = cols[0];
        assert(md5.size() == 12);
        const hash_t hash = stoul(md5.substr(0, 12), 0, 16);
        assert(hash > prevhash);
        md5s.insert(hash);
        prevhash = hash;
        const auto &format = cols[1];
        strings.insert(format);
    }

    create_md5_idx(md5s);
    md5s.clear();

    songdbtsv.clear();
    songdbtsv.seekg(0);

    while (getline(songdbtsv, line)) {
        const auto cols = common::split(line, "\t");
        const auto md5 = cols[0];
        const uint8_t minsubsong = stoi(cols[3]);
        const auto subsongs = common::split(cols[4], " ");
        uint8_t subsong = minsubsong;
        vector<_SongInfo> infos;
        for (const auto &col : subsongs) {
            const auto e = common::split(col, ",");
            const uint24_t songlength = stoi(e[0]);
            const auto songend = parse_songend(e[1]);
            infos.push_back({ subsong++, songlength, songend});
        }
        db_songlengths.push_back(infos);
    }
}

void parse_strings(const string &songdb_path, const vector<pair<string, Source>> &tsvfiles, set<string> &strings) {
    for (const auto &tsv : tsvfiles) {
        ifstream songdbtsv(songdb_path + "/" + tsv.first, ios::in);
        if (!songdbtsv.is_open()) {
            ERR("Could not open songdb file %s\n", tsv.first.c_str());
            return;
        }
        string line;
        while (getline(songdbtsv, line)) {
            const auto cols = common::split(line, "\t");
            switch (tsv.second) {
                case Modland: {
                    ModlandData item {};
                    if (modland::parse_tsv_row(cols, item)) {
                        strings.insert(item.author);
                        strings.insert(item.album);
                    }
                    break;
                }
                case AMP: {
                    AMPData item {};
                    if (amp::parse_tsv_row(cols, item)) {
                        strings.insert(item.author);
                    }
                    break;
                }
                case UnExotica: {
                    UnExoticaData item {};
                    if (unexotica::parse_tsv_row(cols, item)) {
                        strings.insert(item.author);
                        strings.insert(item.album);
                        strings.insert(item.publisher);
                    }
                    break;
                }
                case Demozoo: {
                    DemozooData item {};
                    if (demozoo::parse_tsv_row(cols, item)) {
                        strings.insert(item.author);
                        strings.insert(item.album);
                        strings.insert(item.publisher);
                    }
                    break;
                }
                default: assert(false); break;
            }
        }
    }
}

void parse_tsv(const string &tsv, const Source source) {
    ifstream songdbtsv(tsv, ios::in);
    if (!songdbtsv.is_open()) {
        ERR("Could not open songdb file %s\n", tsv.c_str());
        return;
    }
    string line;
    while (getline(songdbtsv, line)) {
        const auto cols = common::split(line, "\t");
        const auto md5 = dedup_md5(cols[0]);
        assert(md5 != MD5_NOT_FOUND);

        switch (source) {
            case Modland: {
                ModlandData item {};
                if (modland::parse_tsv_row(cols, item)) {
                    const _ModlandData _item = {
                        { md5 },
                        dedup_string(item.author),
                        dedup_string(item.album),
                    };
                    db_modland.push_back(_item);
                }
                break;
            }
            case AMP: {
                AMPData item {};
                if (amp::parse_tsv_row(cols, item)) {
                    const _AMPData _item = {
                        { md5 },
                        dedup_string(item.author),
                    };
                    db_amp.push_back(_item);
                }
                break;
            }
            case UnExotica: {
                UnExoticaData item {};
                if (unexotica::parse_tsv_row(cols, item)) {
                    uint8_t year = item.year != 0 ? item.year - 1900u : 0;
                    assert(year <= UINT8_MAX);
                    const _UnExoticaData _item = {
                        { md5 },
                        dedup_string(item.author),
                        dedup_string(item.album),
                        dedup_string(item.publisher),
                        year,
                    };
                    db_unexotica.push_back(_item);
                }
                break;
            }
            case Demozoo: {
                DemozooData item {};
                if (demozoo::parse_tsv_row(cols, item)) {
                    uint8_t year = item.year != 0 ? item.year - 1900u : 0;
                    assert(year <= UINT8_MAX);
                    const _DemozooData _item = {
                        { md5 },
                        dedup_string(item.author),
                        dedup_string(item.album),
                        dedup_string(item.publisher),
                        year,
                    };
                    db_demozoo.push_back(_item);
                }
                break;
            }
            default: assert(false); break;
        }
    };
}

void parse_modinfos(const string &tsv) {
    ifstream songdbtsv(tsv, ios::in);
    if (!songdbtsv.is_open()) {
        ERR("Could not open songdb file %s\n", tsv.c_str());
        return;
    }
    string line;
    uint32_t i = 0;
    while (getline(songdbtsv, line)) {
        const auto cols = common::split(line, "\t");
        assert(cols.size() == 5);
        const auto &md5 = cols[0];
        assert(dedup_md5(md5) == md5_t {i++});
        const auto &format = cols[1];
        const auto fmt = dedup_string(format);
        const uint8_t channels = stoi(cols[2]);
        const _ModInfo info = {fmt, channels};
        db_modinfos.push_back(info);
    }
}

} // namespace {}

namespace songdb {

optional<SongInfo> lookup(const string &md5, int subsong) {
    const auto md5_t = dedup_md5(md5);
    if (md5_t != MD5_NOT_FOUND) { 
        for (const auto &info : db_songlengths[md5_t]) {
            if (info.subsong == subsong) {
                const auto &modinfo = db_modinfos[md5_t];
                return make_info(md5_t, make_string(modinfo.format), modinfo.channels, info);
            }
        }
    }
    const hash_t hash = stoul(md5.substr(0, 12), 0, 16);
    if (extra_songlengths.contains(hash)) {
        const auto &tuple = extra_songlengths[hash];
        const auto &format = get<0>(tuple);
        const auto channels = get<1>(tuple);
        for (const auto &info : get<2>(tuple)) {
            if (info.subsong == subsong) {
                return make_info(md5_t, format, channels, info);
            }
        }
    }
    return {};
}

vector<SongInfo> lookup_all(const string &md5) {
    const auto md5_t = dedup_md5(md5);
    if (md5_t != MD5_NOT_FOUND) {        
        const auto &modinfo = db_modinfos[md5_t];
        vector<SongInfo> res;
        for (const auto &i : db_songlengths[md5_t]) {
            res.push_back(make_info(md5_t, make_string(modinfo.format), modinfo.channels, i));
        }
        return res;
    }
    return {};
}

void init(const string &songdb_path) {
    if (initialized) {
        return;
    }
    set<string> strings;
    parse_songlengths(songdb_path + "/songlengths.tsv", strings);
    parse_strings(songdb_path, tsvfiles, strings);
    create_string_pool(strings);

    parse_modinfos(songdb_path + "/songlengths.tsv");

    for (const auto &tsv : tsvfiles) {
        parse_tsv(songdb_path + "/" + tsv.first, tsv.second);
    }

    sort(db_modland.begin(), db_modland.end(), [](const _ModlandData &a, const _ModlandData &b) { return a.md5 < b.md5; });
    sort(db_amp.begin(), db_amp.end(), [](const _AMPData &a, const _AMPData &b) { return a.md5 < b.md5; });
    sort(db_unexotica.begin(), db_unexotica.end(), [](const _UnExoticaData &a, const _UnExoticaData &b) { return a.md5 < b.md5; });
    sort(db_demozoo.begin(), db_demozoo.end(), [](const _DemozooData &a, const _DemozooData &b) { return a.md5 < b.md5; });

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
    const hash_t hash = stoul(md5.substr(0, 12), 0, 16);
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
    const auto md5_t = dedup_md5(md5);
    if (md5_t != MD5_NOT_FOUND) {
        const auto &infos = db_songlengths[md5_t];
        return pair(infos.front().subsong, infos.back().subsong);
    }
    return {};
}

} // namespace songdb