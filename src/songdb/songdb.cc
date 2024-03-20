// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <numeric>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "common/common.h"
#include "common/logger.h"
#include "songdb/internal.h"
#include "songdb/songdb.h"

#include <string.h>

using namespace std;
using namespace songdb;
using namespace songdb::internal;

namespace songdb::modland {
    bool parse_tsv_row(const char *tuple, _ModlandData &item, vector<string> &strings, const _ModlandData &prev_item,
                       string_view &prev_author, string_view &prev_album);
}

namespace songdb::unexotica {
    bool parse_tsv_row(const char *tuple, _UnExoticaData &item, vector<string> &strings, const _UnExoticaData &prev_item,
                       string_view &prev_author, string_view &prev_publisher, string_view &prev_album);
}

namespace songdb::demozoo {
    bool parse_tsv_row(const char *tuple, _DemozooData &item, vector<string> &strings, const _DemozooData &prev_item,
                       string_view &prev_author, string_view &prev_publisher, string_view &prev_album);
}

namespace {

constexpr size_t BUF_SIZE = 2048;

md5_t hex2md5(const char *hex) {
    md5_t ret = 0; 
    for (int i = 0; i < 12; ++i) {
        const char c = *hex++;
        ret = (ret << 4);
        if (c > 58) {
            ret = ret | (c - 87);
        } else {
            ret = ret | (c - 48);
        }
   }
   return ret; 
}

vector<md5_t> md5_idx;
vector<string> string_pool; // can contain duplicates


md5_idx_t _md5(const string_view &md5) {
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
unordered_map<md5_t, pair<ModInfo, vector<_SongInfo>>> extra_songlengths; // runtime only
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

optional<ModInfo> make_modinfo(const md5_idx_t md5) {
    const auto data = find(db_modinfos, md5);
    if (data) {
        return ModInfo {
            make_string(data->format),
            data->channels,
        };
    }
    return {};
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

SongInfo make_info(const md5_idx_t md5, const _SongInfo &info) {
    return {
        info.subsong,
        info.songlength,
        common::SongEnd::status_string(info.songend),
        make_modinfo(md5),
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
    string_view prev_author = UNKNOWN_AUTHOR;
    string_view prev_publisher = "";
    string_view prev_album = "";
    string_t prev_author_t = STRING_NOT_FOUND;
    _ModlandData prev_modland;
    _UnExoticaData prev_unexotica;
    _DemozooData prev_demozoo;
    while (fgets(line, sizeof line, f)) {
        const auto md5 = _md5(line);
        assert(md5 != MD5_NOT_FOUND);
        assert(strings.size() < STRING_NOT_FOUND);
        char *tuple = line + 12; // skip md5
        switch (source) {
            case Modland: {
                if (*tuple == '\n') {
                     db_modland.push_back({
                        { md5 },
                        prev_modland.author,
                        prev_modland.album,
                    });
                    continue;
                }
                _ModlandData _item;
                if (modland::parse_tsv_row(tuple + 1, _item, strings, prev_modland, prev_author, prev_album)) {
                    assert(strings.size() < STRING_NOT_FOUND);
                    _item.md5 = md5;
                    db_modland.push_back(_item);
                    prev_modland = _item;
                }
                break;
            }
            case AMP: {
                if (*tuple == '\n') {
                    db_amp.push_back({{ md5 }, prev_author_t});
                    continue;
                }
                string author = tuple + 1;
                author.pop_back();
                string_t sc = strings.size();
                strings.push_back(author);
                db_amp.push_back({{ md5 }, sc});
                prev_author_t = sc;
                break;
            }
            case UnExotica: {
                if (*tuple == '\n') {
                    db_unexotica.push_back({
                        { md5 },
                        prev_unexotica.author,
                        prev_unexotica.album,
                        prev_unexotica.publisher,
                        prev_unexotica.year,
                    });
                    continue;
                }
                _UnExoticaData _item;
                if (unexotica::parse_tsv_row(tuple + 1, _item, strings, prev_unexotica, prev_author, prev_publisher, prev_album)) {
                    assert(strings.size() < STRING_NOT_FOUND);
                    _item.md5 = md5;
                    db_unexotica.push_back(_item);
                    prev_unexotica = _item;
                }
                break;
            }
            case Demozoo: {
                if (*tuple == '\n') {
                    db_demozoo.push_back({
                        { md5 },
                        prev_demozoo.author,
                        prev_demozoo.album,
                        prev_demozoo.publisher,
                        prev_demozoo.year,
                    });
                    continue;
                }
                _DemozooData _item;
                if (demozoo::parse_tsv_row(tuple + 1, _item, strings, prev_demozoo, prev_author, prev_publisher, prev_album)) {
                    assert(strings.size() < STRING_NOT_FOUND);
                    _item.md5 = md5;
                    db_demozoo.push_back(_item);
                    prev_demozoo = _item;
                }
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
    string_t prev_format_t = STRING_NOT_FOUND;
    uint8_t prev_channels = 0;
    while (fgets(line, sizeof line, f)) {
        const auto md5 = _md5(line);
        assert(md5 != MD5_NOT_FOUND);
        assert(strings.size() < STRING_NOT_FOUND);
        char *tuple = line + 12; // skip md5
        if (*tuple == '\n') {
            db_modinfos.push_back({{ md5 }, prev_format_t, prev_channels});
            continue;
        }
        const auto cols = common::split_view_x<2>(tuple + 1, '\t');

        string fmt = string(cols[0]);
        string_t format_t = strings.size();
        strings.push_back(fmt);
        prev_format_t = format_t;

        const uint8_t channels = common::from_chars<uint8_t>(cols[1]);
        prev_channels = channels;

        db_modinfos.push_back({{ md5 }, format_t, channels});
    }
    fclose(f);
}

} // namespace {}

namespace songdb {

optional<SongInfo> lookup(const string &md5, int subsong) {
    const auto md5_idx = _md5(md5);
    if (md5_idx != MD5_NOT_FOUND) {
        for (const auto &info : db_songlengths[md5_idx]) {
            if (info.subsong == subsong) {
                return make_info(md5_idx, info);
            }
        }
    }
    const md5_t hash = hex2md5(md5.c_str());
    if (extra_songlengths.contains(hash)) {
        const auto &pair = extra_songlengths.at(hash);
        const auto &modinfo = pair.first;
        const auto &infos = pair.second;
        for (const auto &info : infos) {
            if (info.subsong == subsong) {
                return SongInfo {
                    info.subsong,
                    info.songlength,
                    common::SongEnd::status_string(info.songend),
                    modinfo
                };
            }
        }
    }
    return {};
}

vector<SongInfo> lookup_all(const string &md5) {
    const auto md5_idx = _md5(md5);
    if (md5_idx != MD5_NOT_FOUND) {
        vector<SongInfo> res;
        const auto modinfo = make_modinfo(md5_idx);
        const auto modland = make_modland(md5_idx);
        const auto amp = make_amp(md5_idx);
        const auto unexotica = make_unexotica(md5_idx);
        const auto demozoo = make_demozoo(md5_idx);
        for (const auto &info : db_songlengths[md5_idx]) {
            res.push_back({
                info.subsong,
                info.songlength,
                common::SongEnd::status_string(info.songend),
                modinfo, modland, amp, unexotica, demozoo
            });
        }
        return res;
    }
    const md5_t hash = hex2md5(md5.c_str());
    if (extra_songlengths.contains(hash)) {
        vector<SongInfo> res;
        const auto &pair = extra_songlengths.at(hash);
        const auto &modinfo = pair.first;
        const auto &infos = pair.second;
        for (const auto &info : infos) {
            res.push_back({
                info.subsong,
                info.songlength,
                common::SongEnd::status_string(info.songend),
                modinfo
            });
        }
        return res;
    }
    return {};
}

void init(const string &songdb_path) {
    if (initialized) {
        return;
    }
    assert(string_pool.empty());
    string_pool.push_back(UNKNOWN_AUTHOR);

    parse_songlengths(songdb_path + "/songlengths.tsv");

    parse_modinfos(songdb_path + "/modinfos.tsv", string_pool);

    for (const auto &tsv : tsvfiles) {
        parse_tsv(songdb_path + "/" + tsv.first, tsv.second, string_pool);
    }

    assert(string_pool.size() < STRING_NOT_FOUND);

    const auto sorter = [](const _Data &a, const _Data &b) { return a.md5 < b.md5; };
    sort(db_modinfos.begin(), db_modinfos.end(), sorter);
    sort(db_modland.begin(), db_modland.end(), sorter);
    sort(db_amp.begin(), db_amp.end(), sorter);
    sort(db_unexotica.begin(), db_unexotica.end(), sorter);
    sort(db_demozoo.begin(), db_demozoo.end(), sorter);

    db_songlengths.shrink_to_fit();
    db_modinfos.shrink_to_fit();
    db_modland.shrink_to_fit();
    db_amp.shrink_to_fit();
    db_unexotica.shrink_to_fit();
    db_demozoo.shrink_to_fit();

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
    assert(subsong >= 0);
    assert(subsong <= 255);
    assert(songlength >= 0);
    assert(songlength <= UINT24_T_MAX);

    const md5_t hash = hex2md5(md5.c_str());
    _SongInfo info = { static_cast<subsong_t>(subsong), static_cast<songlength_t>(songlength), songend };
    if (extra_songlengths.contains(hash)) {
        auto &infos = extra_songlengths.at(hash).second;
        for (const auto &info : infos) {
            if (info.subsong == subsong) {
                WARN("Skipped songdb update for %s:%d, already exists\n", md5.c_str(), subsong);
                return;
            }
        }
        infos.push_back(info);
        sort(infos.begin(), infos.end(), [](const _SongInfo &a, const _SongInfo &b) { return a.subsong < b.subsong; });
    } else {
        vector<_SongInfo> infos;
        infos.push_back(info);
        ModInfo modinfo = {format, static_cast<uint8_t>(channels)};
        extra_songlengths.insert({hash, {modinfo, infos}});
    }
}

optional<pair<int,int>> subsong_range(const string &md5) {
    const auto md5_idx = _md5(md5);
    if (md5_idx != MD5_NOT_FOUND) {
        const auto &infos = db_songlengths[md5_idx];
        return pair(infos.front().subsong, infos.back().subsong);
    }
    return {};
}

} // namespace songdb