// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <memory>
#include <numeric>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "common/logger.h"
#include "songdb/internal.h"
#include "songdb/songdb.h"
#include "common/strings.h"

#include <string.h>
#include <time.h>

using namespace std;
using namespace songdb;
using namespace songdb::internal;

namespace songdb::modland {
    bool parse_tsv_row(const char *tuple, _ModlandData &item, const _ModlandData &prev_item, vector<string> &authors, vector<string> &albums) noexcept;
}

namespace songdb::unexotica {
    bool parse_tsv_row(const char *tuple, _UnExoticaData &item, const _UnExoticaData &prev_item, vector<string> &authors, vector<string> &albums, vector<string> &publishers) noexcept;
}

namespace songdb::demozoo {
    bool parse_tsv_row(const char *tuple, _DemozooData &item, const _DemozooData &prev_item, vector<string> &authors, vector<string> &albums, vector<string> &publishers) noexcept;
}

namespace {

constexpr size_t BUF_SIZE = 2048;

md5_t hex2md5(const char *hex) noexcept {
    uint64_t ret = 0; 
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

inline uint24_t b64d24(const string_view &b64) noexcept {
    if (b64.size() == 3) {
        return (b64[0] - 45) << 12 |
               (b64[1] - 45) << 6 |
               (b64[2] - 45);
    } else if (b64.size() == 2) {
        return (b64[0] - 45) << 6 |
               (b64[1] - 45);
    } else if (b64.size() == 1) {
        return b64[0] - 45;
    } else {
        assert(b64.size() == 4);
        return (b64[0] - 45) << 18 |
               (b64[1] - 45) << 12 |
               (b64[2] - 45) << 6 |
               (b64[3] - 45);
    }
}

inline uint24_t b64d24(const char *b64, int &len) noexcept {
    if (b64[1] == '\n' || b64[1] == '\t') {
        len = 1;
        return b64[0] - 45;
    } else if (b64[2] == '\n' || b64[2] == '\t') {
        len = 2;
        return (b64[0] - 45) << 6 |
               (b64[1] - 45);
    } else if (b64[3] == '\n' || b64[3] == '\t') {
        len = 3;
        return (b64[0] - 45) << 12 |
               (b64[1] - 45) << 6 |
               (b64[2] - 45);
    } else {
        len = 4;
        return (b64[0] - 45) << 18 |
               (b64[1] - 45) << 12 |
               (b64[2] - 45) << 6 |
               (b64[3] - 45);
    }
}

inline md5_t b64diff2md5(const md5_t prev, const char *b64, int &len) noexcept {
    if (b64[3] == '\n') {
        len = 4;
        return prev + ((b64[0] - 45) << 12 |
               (b64[1] - 45) << 6 |
               (b64[2] - 45));
    } else if (b64[4] == '\n') {
        len = 5;
        return prev + ((b64[0] - 45) << 18 |
               (b64[1] - 45) << 12 |
               (b64[2] - 45) << 6 |
               (b64[3] - 45));
    } else if (b64[5] == '\n') {
        len = 6;
        return prev + ((b64[0] - 45) << 24 |
               (b64[1] - 45) << 18 |
               (b64[2] - 45) << 12 |
               (b64[3] - 45) << 6 |
               (b64[4] - 45));
    } else {
        len = 7;
        uint32_t part1 = (b64[0] - 45) << 12;
        part1 |= (b64[1] - 45) << 6;
        part1 |= (b64[2] - 45);
        uint32_t part2 = (b64[3] - 45) << 12;
        part2 |= (b64[4] - 45) << 6;
        part2 |= b64[5] - 45;
        return prev + ((static_cast<uint64_t>(part1) << 18) | part2); 
    }
}

md5_t md5_idx[MD5_IDX_SIZE];
// these may can contain duplicates
vector<string> format_pool;
vector<string> author_pool = {UNKNOWN_AUTHOR};
vector<string> publisher_pool;
vector<string> album_pool;

md5_idx_t _md5idx(const md5_t hash) noexcept  {
    uint32_t idx = ((double)hash / MD5_T_MAX) * MD5_IDX_SIZE;
    assert(idx < MD5_IDX_SIZE);
    md5_t cmp = md5_idx[idx];
    if (cmp == hash) {
         return idx;
    } else if (cmp > hash) {
        while (cmp > hash && idx > 0) {
            idx--;
            cmp = md5_idx[idx];
        }
        return (cmp == hash) ? static_cast<md5_idx_t>(idx) : MD5_NOT_FOUND;
    } else {
        while (cmp < hash && idx < MD5_IDX_SIZE - 1) {
            idx++;
            cmp = md5_idx[idx];
        }
        return (cmp == hash) ? static_cast<md5_idx_t>(idx) : MD5_NOT_FOUND;
    }
}

md5_idx_t _md5hex(const string_view &md5) noexcept {
    assert(md5.size() >= 12);
    return _md5idx(hex2md5(md5.data()));
}

songend_t parse_songend(const string_view &songend) noexcept {
    if (songend == "e") return common::SongEnd::ERROR;
    if (songend == "p") return common::SongEnd::PLAYER;
    if (songend == "t") return common::SongEnd::TIMEOUT;
    if (songend == "s") return common::SongEnd::DETECT_SILENCE;
    if (songend == "l") return common::SongEnd::DETECT_LOOP;
    if (songend == "v") return common::SongEnd::DETECT_VOLUME;
    if (songend == "r") return common::SongEnd::DETECT_REPEAT;
    if (songend == "b") return common::SongEnd::PLAYER_PLUS_SILENCE;
    if (songend == "P") return common::SongEnd::PLAYER_PLUS_VOLUME;
    if (songend == "i") return common::SongEnd::LOOP_PLUS_SILENCE;
    if (songend == "L") return common::SongEnd::LOOP_PLUS_VOLUME;
    if (songend == "n") return common::SongEnd::NOSOUND;
    assert(false);
    return common::SongEnd::NONE;
}

const vector<string> tsvfiles ({
    "md5idx.tsv",
    "songlengths.tsv",
    "modinfos.tsv",
    "modland.tsv",
    "amp.tsv",
    "unexotica.tsv",
    "demozoo.tsv"
});

_SongInfo db_songinfos[MD5_IDX_SIZE]; // md5_idx_t -> first subsong info
unordered_map<md5_idx_t, vector<_SubSongInfo>> db_subsongs; // infos for extra subsongs
_ModInfo db_modinfos[MD5_IDX_SIZE];
array<_ModlandData,MODLAND_SIZE> db_modland;
array<_AMPData,AMP_SIZE> db_amp;
array<_UnExoticaData,UNEXOTICA_SIZE> db_unexotica;
array<_DemozooData,DEMOZOO_SIZE> db_demozoo;

unordered_map<md5_t, vector<SubSongInfo>> extra_subsongs; // runtime only
unordered_map<md5_t, ModInfo> extra_modinfos; // runtime only
mutex extra_mutex; // for extra_subsongs/modinfos thread safe access/update

constexpr Source SOURCE_MD5 = static_cast<Source>(0); // internal
bool initialized[Demozoo+1] = {};

const string make_format(const string_t s) noexcept {
    if (s == STRING_NOT_FOUND) return "";
    return format_pool[s];
}

const string make_author(const string_t s) noexcept {
    if (s == STRING_NOT_FOUND) return "";
    return author_pool[s];
}

const string make_publisher(const string_t s) noexcept {
    if (s == STRING_NOT_FOUND) return "";
    return publisher_pool[s];
}

const string make_album(const string_t s) noexcept {
    if (s == STRING_NOT_FOUND) return "";
    return album_pool[s];
}

template <_Data_ T, size_t N>
optional<T> find(const array<T,N> &db, const md5_idx_t md5) noexcept {
    if (md5 >= MD5_IDX_SIZE) return optional<T>();
    unsigned int idx = ((double)md5 / MD5_IDX_SIZE) * N;
    assert(idx < N);
    md5_idx_t cmp = db[idx].md5;
    if (cmp == md5) {
        return db[idx];
    } else if (cmp > md5) {
        while (cmp > md5 && idx > 0) {
            idx--;
            cmp = db[idx].md5;
        }
        return cmp == md5 ? db[idx] : optional<T>();
    } else {
        while (cmp < md5 && idx < N - 1) {
            idx++;
            cmp = db[idx].md5;
        }
        return cmp == md5 ? db[idx] : optional<T>();
    }
}

optional<ModInfo> make_modinfo(const md5_idx_t md5) noexcept {
    if (!initialized[ModInfos]) return {};
    const auto &data = db_modinfos[md5];
    if (data.format != STRING_NOT_FOUND || data.channels > 0) {
        return ModInfo {
            make_format(data.format),
            data.channels,
        };
    }
    return {};
}

optional<ModlandData> make_modland(const md5_idx_t md5) noexcept {
    if (!initialized[Modland]) return {};
    const auto data = find<_ModlandData,MODLAND_SIZE>(db_modland, md5);
    if (data) {
        return ModlandData {
            make_author(data->author),
            make_album(data->album),
        };
    }
    return {};
}

optional<AMPData> make_amp(const md5_idx_t md5) noexcept {
    if (!initialized[AMP]) return {};
    const auto data = find<_AMPData,AMP_SIZE>(db_amp, md5);
    if (data) {
        return AMPData {
           make_author(data->author),
        };
    }
    return {};
}

optional<UnExoticaData> make_unexotica(const md5_idx_t md5) noexcept {
    if (!initialized[UnExotica]) return {};
    const auto data = find<_UnExoticaData,UNEXOTICA_SIZE>(db_unexotica, md5);
    if (data) {
        return UnExoticaData {
            make_author(data->author),
            make_album(data->album),
            make_publisher(data->publisher),
            static_cast<uint16_t>(data->year > 0 ? (1900u + data->year) : 0),
        };
    };
    return {};
}

optional<DemozooData> make_demozoo(const md5_idx_t md5) noexcept {
    if (!initialized[Demozoo]) return {};
    const auto data = find<_DemozooData,DEMOZOO_SIZE>(db_demozoo, md5);
    if (data) {
        return DemozooData {
            make_author(data->author),
            make_album(data->album),
            make_publisher(data->publisher),
            static_cast<uint16_t>(data->year > 0 ? (1900u + data->year) : 0),
        };
    };
    return {};
}

SubSongInfo make_subsonginfo(const uint8_t subsong, const _SubSongInfo &info) noexcept {
    return {
        subsong,
        {info.songend(), info.songlength_ms()}
    };
}

void parse_md5idx(const string &tsv) noexcept {
    FILE *f = fopen(tsv.c_str(), "r"); 
    if (!f) {
        ERR("Could not open songdb file %s\n", tsv.c_str());
        return;
    }

    int i = 0;
    md5_t prevhash = 0;
    char line[BUF_SIZE];
    while (fgets(line, sizeof line, f)) {
        int len = 9;
        const md5_t hash = (i == 0) ? static_cast<md5_t>(0) : b64diff2md5(prevhash, line, len);
        assert(i == 0 || hash > prevhash);
        md5_idx[i] = hash;
        prevhash = hash;
        i++;
    }
    fclose(f);
    assert(i == MD5_IDX_SIZE);
}

void parse_songlengths(const string &tsv) noexcept {
    FILE *f = fopen(tsv.c_str(), "r"); 
    if (!f) {
        ERR("Could not open songdb file %s\n", tsv.c_str());
        return;
    }
    int md5_idx = 1; // 0 entry is special
    char line[BUF_SIZE];
    while (fgets(line, sizeof line, f)) {
        const auto cols = common::split_view_x<2>(line, '\t');
        const auto subsongs = common::split_view(cols[0], ' ');
        vector<_SubSongInfo> subsong_infos;
        _SubSongInfo prev_info(0,common::SongEnd::Status::ERROR);
        for (const auto &col : subsongs) {
            if (col.empty()) {
                subsong_infos.push_back(prev_info);
            } else {
                const auto e = common::split_view<2>(col, ',');
                const songlength_t songlength = e[0].empty() ? static_cast<songlength_t>(0) : b64d24(e[0]);
                const songend_t songend = e[1].empty() ? common::SongEnd::PLAYER : parse_songend(e[1]);
                const auto info = _SubSongInfo(songlength, songend);
                subsong_infos.push_back(info);
                prev_info = info;
            }
        }
        assert(!subsong_infos.empty());
        const uint8_t minsubsong = cols[1].empty() ? 1 : common::from_chars<uint8_t>(cols[1]);
        const _SongInfo info(subsongs.size() > 1, minsubsong, subsong_infos.front());
        assert(md5_idx < MD5_IDX_SIZE);
        db_songinfos[md5_idx] = info;
        if (subsong_infos.size() > 1) {
            subsong_infos.erase(subsong_infos.begin());
            subsong_infos.shrink_to_fit();
            db_subsongs.insert({md5_idx, subsong_infos});
        }
        md5_idx++;
    }
    fclose(f);
    assert(md5_idx == MD5_IDX_SIZE);
}

template <_Data_ T, size_t N, Source S>
void parse_tsv(const string &tsv) noexcept {
    FILE *f = fopen(tsv.c_str(), "r"); 
    if (!f) {
        ERR("Could not open songdb file %s\n", tsv.c_str());
        return;
    }
    char line[BUF_SIZE];
    T *tmp = (T*)malloc(MD5_IDX_SIZE * sizeof(T));
    memset((void *)tmp, 0, MD5_IDX_SIZE * sizeof(T));
    md5_idx_t prev_idx = 0;
    while (fgets(line, sizeof line, f)) {
        int len;
        const md5_idx_t md5_idx = b64d24(line, len);
        assert(md5_idx > static_cast<md5_idx_t>(0));
        assert(md5_idx < static_cast<md5_idx_t>(MD5_IDX_SIZE));
        char *tuple = line + len; // skip md5
        switch (S) {
            case Modland: {
                const auto &prev = ((_ModlandData*)tmp)[prev_idx];
                if (*tuple == '\n') {
                    const md5_idx_t idx = prev.md5 + md5_idx;
                    ((_ModlandData*)tmp)[idx] = {
                        { idx },
                        prev.author,
                        prev.album,
                    };
                    prev_idx = idx;
                    continue;
                }
                _ModlandData _item;
                if (modland::parse_tsv_row(tuple + 1, _item, prev, author_pool, album_pool)) {
                    _item.md5 = md5_idx;
                    ((_ModlandData*)tmp)[md5_idx] = _item;
                    prev_idx = md5_idx;
                }
                break;
            }
            case AMP: {
                const auto &prev = ((_AMPData*)tmp)[prev_idx];
                if (*tuple == '\n') {
                    const md5_idx_t idx = prev.md5 + md5_idx;
                    ((_AMPData*)tmp)[idx] = {{ idx }, prev.author};
                    prev_idx = idx;
                    continue;
                }
                string author = tuple + 1;
                author.pop_back();
                string_t sc = author_pool.size();
                author_pool.push_back(author);
                ((_AMPData*)tmp)[md5_idx] = {{ md5_idx }, sc};
                prev_idx = md5_idx;
                break;
            }
            case UnExotica: {
                const auto &prev = ((_UnExoticaData*)tmp)[prev_idx];
                if (*tuple == '\n') {
                    const md5_idx_t idx = prev.md5 + md5_idx;
                    ((_UnExoticaData*)tmp)[idx] = {
                        { idx },
                        prev.author,
                        prev.album,
                        prev.publisher,
                        prev.year,
                    };
                    prev_idx = idx;
                    continue;
                }
                _UnExoticaData _item;
                if (unexotica::parse_tsv_row(tuple + 1, _item, prev, author_pool, album_pool, publisher_pool)) {
                    _item.md5 = md5_idx;
                    ((_UnExoticaData*)tmp)[md5_idx] = _item;
                    prev_idx = md5_idx;
                }
                break;
            }
            case Demozoo: {
                const auto &prev = ((_DemozooData*)tmp)[prev_idx];
                if (*tuple == '\n') {
                    const md5_idx_t idx = prev.md5 + md5_idx;
                    ((_DemozooData*)tmp)[idx] = {
                        { idx },
                        prev.author,
                        prev.album,
                        prev.publisher,
                        prev.year,
                    };
                    prev_idx = idx;
                    continue;
                }
                _DemozooData _item;
                if (demozoo::parse_tsv_row(tuple + 1, _item, prev, author_pool, album_pool, publisher_pool)) {
                    _item.md5 = md5_idx;
                    ((_DemozooData*)tmp)[md5_idx] = _item;
                    prev_idx = md5_idx;
                }
                break;
            }
            default: assert(false); break;
        }
        assert(prev_idx > static_cast<md5_idx_t>(0));
        assert(prev_idx < static_cast<md5_idx_t>(MD5_IDX_SIZE));
    };
    fclose(f);
    int i = 0;
    for (int j = 0; j < MD5_IDX_SIZE; ++j) {
        if (tmp[j].md5 > static_cast<md5_idx_t>(0)) {
            switch (S) {
                case Modland: db_modland[i++] = ((_ModlandData*)tmp)[j]; break;
                case AMP: db_amp[i++] = ((_AMPData*)tmp)[j]; break;
                case UnExotica: db_unexotica[i++] = ((_UnExoticaData*)tmp)[j]; break;
                case Demozoo: db_demozoo[i++] = ((_DemozooData*)tmp)[j]; break;
                default: assert(false); break;
            }
        }
    }
    free(tmp);
    assert(i == N);
}

void parse_modinfos(const string &tsv) noexcept {
    FILE *f = fopen(tsv.c_str(), "r"); 
    if (!f) {
        ERR("Could not open songdb file %s\n", tsv.c_str());
        return;
    }

    md5_idx_t prev_idx = 0;
    char line[BUF_SIZE];
    string_t prev_format_t = STRING_NOT_FOUND;
    uint8_t prev_channels = 0;
    while (fgets(line, sizeof line, f)) {
        int len;
        const md5_idx_t md5_idx = b64d24(line, len);
        assert(md5_idx < static_cast<md5_idx_t>(MD5_IDX_SIZE));
        char *tuple = line + len; // skip md5
        if (*tuple == '\n') {
            md5_idx_t idx = prev_idx + md5_idx;
            assert(idx < static_cast<md5_idx_t>(MD5_IDX_SIZE));
            db_modinfos[idx] = { prev_format_t, prev_channels };
            prev_idx = idx;
            continue;
        }
        const auto cols = common::split_view_x<2>(tuple + 1, '\t');
        string_t format_t;
        if (cols[0].at(0) == 0x7f) {
            format_t = prev_format_t;
        } else {
            string fmt = string(cols[0]);
            format_t = format_pool.size();
            prev_format_t = format_t;
            format_pool.push_back(fmt);
        }
        const uint8_t channels = cols[1].empty() ? 0 : common::from_chars<uint8_t>(cols[1]);
        prev_channels = channels;
        db_modinfos[md5_idx] = { format_t, channels };
        prev_idx = md5_idx;
    }
    fclose(f);
}

} // namespace {}

namespace songdb {

optional<SubSongInfo> lookup(const string &md5, int subsong) {
    const auto md5_idx = _md5hex(md5);
    if (md5_idx != MD5_NOT_FOUND && initialized[Songlengths]) {
        const auto &info = db_songinfos[md5_idx];
        if (subsong < info.min_subsong()) return {};
        if (info.min_subsong() == subsong) return make_subsonginfo(subsong, info.info());
        if (info.has_subsongs()) {
            assert(db_subsongs.contains(md5_idx));
            const auto &subsongs = db_subsongs[md5_idx];
            if (info.min_subsong() + subsongs.size() < static_cast<unsigned>(subsong)) return {};
            int sub = info.min_subsong() + 1;
            for (const auto &info: subsongs) {
                if (sub++ == subsong) return make_subsonginfo(subsong, info);
            }
            assert(false);
        } else return {};
    }
    const md5_t hash = hex2md5(md5.c_str());
    const lock_guard lock(extra_mutex);
    if (extra_subsongs.contains(hash)) {
        const auto &infos = extra_subsongs[hash];
        for (const auto &info : infos) {
            if (info.subsong == subsong) {
                return info.songend.status != common::SongEnd::NONE ? info : optional<SubSongInfo>();
            }
        }
    }
    return {};
}

optional<Info> lookup(const string &md5) {
    const auto md5_idx = _md5hex(md5);
    if (md5_idx != MD5_NOT_FOUND) {
        Info res;
        res.modinfo = make_modinfo(md5_idx);
        res.modland = make_modland(md5_idx);
        res.amp = make_amp(md5_idx);
        res.unexotica = make_unexotica(md5_idx);
        res.demozoo = make_demozoo(md5_idx);
        if (!initialized[Songlengths]) {
            return res;
        }
        const auto &info = db_songinfos[md5_idx];
        res.subsongs.push_back(make_subsonginfo(info.min_subsong(), info.info()));
        if (info.has_subsongs()) {
            assert(db_subsongs.contains(md5_idx));
            const auto &subsongs = db_subsongs[md5_idx];
            uint8_t sub = info.min_subsong() + 1;
            for (const auto &subsong : subsongs) {
                res.subsongs.push_back(make_subsonginfo(sub++, subsong));
            }
        }
        return res;
    }
    const md5_t hash = hex2md5(md5.c_str());
    const lock_guard lock(extra_mutex);
    if (!extra_subsongs.contains(hash) && !extra_modinfos.contains(hash)) {
        return {};
    }
    Info res;
    if (extra_subsongs.contains(hash)) {
        const auto &infos = extra_subsongs[hash];
        for (const auto &info : infos) {
            res.subsongs.push_back(info);
        }
    }
    if (extra_modinfos.contains(hash)) {
        res.modinfo = extra_modinfos[hash];
    }
    return res;
}

void init(const string &songdb_path, const initializer_list<Source> &sources/*= {}*/) {
    TRACE("INIT %lu\n", clock() * 1000 / CLOCKS_PER_SEC);

    const auto shouldInit = [&sources](const Source source) {
        return !initialized[source] &&
            (source == SOURCE_MD5 || !sources.size() ||
            any_of(sources.begin(), sources.end(), [source](const Source s) { return s == source; }));
    };

    const auto path = songdb_path.ends_with("/") ? songdb_path : songdb_path + "/";

    if (shouldInit(SOURCE_MD5)) {
        parse_md5idx(path + tsvfiles[SOURCE_MD5]);
        initialized[SOURCE_MD5] = true;
        TRACE("PARSE_MD5IDX %lu\n", clock() * 1000 / CLOCKS_PER_SEC);
    }

    if (shouldInit(Songlengths)) {
        parse_songlengths(path + tsvfiles[Songlengths]);
        initialized[Songlengths] = true;
        TRACE("PARSE_SONGLENGTHS %lu\n", clock() * 1000 / CLOCKS_PER_SEC);
    }

    if (shouldInit(ModInfos)) {
        parse_modinfos(path + tsvfiles[ModInfos]);
        initialized[ModInfos] = true;
        TRACE("PARSE_MOD_INFOS %lu\n", clock() * 1000 / CLOCKS_PER_SEC);
    }

    for (int source = Modland; source <= Demozoo; source++) {
        if (shouldInit(static_cast<Source>(source))) {
            const auto file = path + tsvfiles[source];
            switch (source) {
                case Modland:
                    parse_tsv<_ModlandData,MODLAND_SIZE,Modland>(file);
                    break;
                 case AMP:
                    parse_tsv<_AMPData,AMP_SIZE,AMP>(file);
                    break;
                case UnExotica:
                    parse_tsv<_UnExoticaData,UNEXOTICA_SIZE,UnExotica>(file);
                    break;
                case Demozoo:
                    parse_tsv<_DemozooData,DEMOZOO_SIZE,Demozoo>(file);
                    break;
                default: assert(false); break;
            }
            initialized[source] = true;
            TRACE("PARSE_TSV %s - %lu\n", tsvfiles[source].c_str(), clock() * 1000 / CLOCKS_PER_SEC);
        }
    }

    assert(format_pool.size() < STRING_NOT_FOUND);
    assert(author_pool.size() < STRING_NOT_FOUND);
    assert(album_pool.size() < STRING_NOT_FOUND);
    assert(publisher_pool.size() < STRING_NOT_FOUND);

    format_pool.shrink_to_fit();
    format_pool.shrink_to_fit();
    album_pool.shrink_to_fit();
    publisher_pool.shrink_to_fit();

#ifdef DEBUG_TRACE

    size_t size = 0;
    size_t count = 0;
    for (const auto &v : db_subsongs) {
        size += sizeof(v.first) + sizeof(v.second) + v.second.capacity() * sizeof(_SubSongInfo);
        count += v.second.size();
    }

    size_t fsize = 0;
    for (const auto &s : format_pool) {
        fsize += sizeof(s);
    }
    size_t ausize = 0;
    for (const auto &s : author_pool) {
        ausize += sizeof(s);
    }
    size_t alsize = 0;
    for (const auto &s : album_pool) {
        alsize += sizeof(s);
    }
    size_t psize = 0;
    for (const auto &s : publisher_pool) {
        psize += sizeof(s);
    }

    TRACE("FORMATS:%zu/%zu AUTHORS:%zu/%zu ALBUMS:%zu/%zu PUBLISHERS:%zu/%zu\n",
          fsize, format_pool.capacity(), ausize, author_pool.capacity(), alsize, album_pool.capacity(), psize, publisher_pool.capacity());

    TRACE("MD5_IDX:%zu/%zu/%zu SONGINFOS:%zu/%zu/%zu SUBSONGS:%zu/%zu/%zu/%zu MODINFOS:%zu/%zu/%zu MODLAND:%zu/%zu/%zu AMP:%zu/%zu/%zu UNEXOTICA:%zu/%zu/%zu DEMOZOO:%zu/%zu/%zu\n",
        sizeof(md5_idx), sizeof(md5_idx) / sizeof(md5_t), sizeof(md5_t),
        sizeof(db_songinfos), sizeof(db_songinfos) / sizeof(_SongInfo), sizeof(_SongInfo),
        size, db_subsongs.size(), sizeof(_SubSongInfo), count,
        sizeof(db_modinfos), sizeof(db_modinfos) / sizeof(_ModInfo), sizeof(_ModInfo),
        sizeof(db_modland), sizeof(db_modland) / sizeof(_ModlandData), sizeof(_ModlandData),
        sizeof(db_amp), sizeof(db_amp) / sizeof(_AMPData), sizeof(_AMPData),
        sizeof(db_unexotica), sizeof(db_unexotica) / sizeof(_UnExoticaData), sizeof(_UnExoticaData),
        sizeof(db_demozoo), sizeof(db_demozoo) / sizeof(_DemozooData), sizeof(_DemozooData));

#endif // DEBUG_TRACE

    TRACE("DONE %lu\n", clock() * 1000 / CLOCKS_PER_SEC);
}

void update(const string &md5, const SubSongInfo &info, const int minsubsong, const int maxsubsong) {
    if (md5.empty() || info.subsong < 0 || info.subsong > 255) {
        WARN("Invalid songdb update entry md5:%s subsong:%d\n", md5.c_str(), info.subsong);
        return;
    }
    if (blacklist::is_blacklisted_songdb_key(md5)) {
        INFO("Blacklisted songdb key md5:%s\n", md5.c_str());
        return;
    }
    assert(minsubsong >= 0);
    assert(minsubsong <= 255);
    assert(maxsubsong >= 0);
    assert(maxsubsong <= 255);
    assert(minsubsong <= maxsubsong);
    assert(info.subsong >= 0);
    assert(info.subsong <= 255);
    assert(info.songend.length >= 0);
    assert(info.songend.length <= UINT24_T_MAX);

    const md5_t hash = hex2md5(md5.c_str());
    const lock_guard lock(extra_mutex);
    if (extra_subsongs.contains(hash)) {
        auto &infos = extra_subsongs[hash];
        for (auto &oldinfo : infos) {
            if (oldinfo.subsong == info.subsong) {
                if (oldinfo.songend.status != common::SongEnd::NONE) {
                    DEBUG("Skipped songlength update for %s:%d, already exists\n", md5.c_str(), info.subsong);
                    return;
                }
                oldinfo = info;
                return;
            }
        }
        assert(false);
    } else {
        vector<SubSongInfo> infos(maxsubsong - minsubsong + 1);
        int ss = minsubsong;
        for (auto &i : infos) {
            if (ss == info.subsong) {
                i = info;
            } else {
                i.subsong = ss;
                i.songend = {common::SongEnd::NONE, 0};
            }
            ss++;
        }
        extra_subsongs.insert({hash, infos});
    }
}

void update(const string &md5, const ModInfo &info) {
    if (md5.empty()) {
        WARN("Invalid songdb update entry md5:%s\n", md5.c_str());
        return;
    }
    if (blacklist::is_blacklisted_songdb_key(md5)) {
        INFO("Blacklisted songdb key md5:%s\n", md5.c_str());
        return;
    }
    const md5_t hash = hex2md5(md5.c_str());
    const lock_guard lock(extra_mutex);
    if (extra_modinfos.contains(hash)) {
        DEBUG("Skipped modinfo update for %s, already exists\n", md5.c_str());
        return;
    }
    extra_modinfos.insert({hash, info});
}

optional<pair<int,int>> subsong_range(const string &md5) {
    const auto md5_idx = _md5hex(md5);
    if (md5_idx != MD5_NOT_FOUND) {
        const auto &info = db_songinfos[md5_idx];
        if (info.has_subsongs()) {
            assert(db_subsongs.contains(md5_idx));
            return pair(info.min_subsong(), info.min_subsong() + db_subsongs[md5_idx].size());
        } else {
            return pair(info.min_subsong(), info.min_subsong());
        }
    }
    return {};
}

} // namespace songdb