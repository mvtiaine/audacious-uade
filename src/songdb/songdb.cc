// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#include "common/std/string_view.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <memory>
#include <mutex>
#include <numeric>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "common/compat.h"
#include "common/logger.h"
#include "common/strings.h"
#include "songdb/internal.h"
#include "songdb/songdb.h"

#include <string.h>
#include <time.h>

using namespace std;
using namespace songdb;
using namespace songdb::internal;


namespace {

constexpr size_t BUF_SIZE = 2048;

constexpr_f1 md5_t hex2md5(const char *hex) noexcept {
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

constexpr_f1 uint24_t b64d24(const string_view &b64) noexcept {
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

constexpr_f1 uint24_t b64d24(const char *b64, int &len) noexcept {
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

constexpr_f1 md5_t b64diff2md5(const md5_t prev, const char *b64) noexcept {
    if (b64[2] == '\n') {
        return prev + ((b64[0] - 45) << 6 |
               (b64[1] - 45));
    } else if (b64[3] == '\n') {
        return prev + ((b64[0] - 45) << 12 |
               (b64[1] - 45) << 6 |
               (b64[2] - 45));
    } else if (b64[4] == '\n') {
        return prev + ((b64[0] - 45) << 18 |
               (b64[1] - 45) << 12 |
               (b64[2] - 45) << 6 |
               (b64[3] - 45));
    } else if (b64[5] == '\n') {
        return prev + ((b64[0] - 45) << 24 |
               (b64[1] - 45) << 18 |
               (b64[2] - 45) << 12 |
               (b64[3] - 45) << 6 |
               (b64[4] - 45));
    } else {
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
vector<string> author_pool = {""}; // unknown author
vector<string> publisher_pool;
vector<string> album_pool;

constexpr_f1 md5_idx_t _md5idx(const md5_t hash) noexcept  {
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

constexpr_f1 md5_idx_t _md5hex(const string_view &md5) noexcept {
    assert(md5.size() >= 12);
    return _md5idx(hex2md5(md5.data()));
}

constexpr_f songend_t parse_songend(const string_view &songend) noexcept {
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
    return common::SongEnd::NONE;
}

const vector<string> tsvfiles ({
    "md5idx.tsv",
    "songlengths.tsv",
    "modinfos.tsv",
    "combined.tsv",
    "modland.tsv",
    "amp.tsv",
    "unexotica.tsv",
    "demozoo.tsv"
});

_SongInfo db_songinfos[SONGLENGTHS_SIZE+1]; // md5_idx_t -> first subsong info
unordered_map<md5_idx_t, vector<_SubSongInfo>> db_subsongs; // infos for extra subsongs
_ModInfo db_modinfos[MODINFOS_SIZE+1];

_MetaData db_combined[COMBINED_SIZE];
_MetaData db_modland[MODLAND_SIZE];
_MetaData db_amp[AMP_SIZE];
_MetaData db_unexotica[UNEXOTICA_SIZE];
_MetaData db_demozoo[DEMOZOO_SIZE];

unordered_map<md5_t, vector<SubSongInfo>> extra_subsongs; // runtime only
unordered_map<md5_t, ModInfo> extra_modinfos; // runtime only
mutex extra_mutex; // for extra_subsongs/modinfos thread safe access/update

constexpr Source SOURCE_MD5 = static_cast<Source>(0); // internal
bool initialized[Demozoo+1] = {};

constexpr_f2 string make_format(const string_t s) noexcept {
    if (s == STRING_NOT_FOUND) return "";
    return format_pool[s];
}

constexpr_f2 string make_author(const string_t s) noexcept {
    if (s == STRING_NOT_FOUND) return "";
    return author_pool[s];
}

constexpr_f2 string make_publisher(const string_t s) noexcept {
    if (s == STRING_NOT_FOUND) return "";
    return publisher_pool[s];
}

constexpr_f2 string make_album(const string_t s) noexcept {
    if (s == STRING_NOT_FOUND) return "";
    return album_pool[s];
}

template<size_t N>
constexpr_f1 optional<_MetaData> _find(const _MetaData db[N], const md5_idx_t md5) noexcept {
    if (md5 >= MD5_IDX_SIZE) return {};
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
        return cmp == md5 ? db[idx] : optional<_MetaData>();
    } else {
        while (cmp < md5 && idx < N - 1) {
            idx++;
            cmp = db[idx].md5;
        }
        return cmp == md5 ? db[idx] : optional<_MetaData>();
    }
}

constexpr_f2 optional<ModInfo> make_modinfo(const md5_idx_t md5) noexcept {
    const auto &data = db_modinfos[md5];
    if (data.format != STRING_NOT_FOUND || data.channels > 0) {
        return ModInfo {
            make_format(data.format),
            data.channels,
        };
    }
    return {};
}

template<size_t N>
constexpr_f2 optional<MetaData> make_meta(const md5_idx_t md5, const _MetaData db[N]) noexcept {
    const auto data = _find<N>(db, md5);
    if (data) {
        return MetaData {
            make_author(data->author),
            make_album(data->album),
            make_publisher(data->publisher),
            static_cast<uint16_t>(data->year > 0 ? (1900u + data->year) : 0),
        };
    }
    return {};
}

constexpr_f1 SubSongInfo make_subsonginfo(const uint8_t subsong, const _SubSongInfo &info) noexcept {
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
        const md5_t hash = (i == 0) ? static_cast<md5_t>(0) : b64diff2md5(prevhash, line);
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
        _SubSongInfo subsong_info(0,common::SongEnd::Status::ERROR);
        for (const auto &col : subsongs) {
            if (!col.empty()) {
                const auto e = common::split_view<2>(col, ',');
                const songlength_t songlength = e[0].empty() ? static_cast<songlength_t>(0) : b64d24(e[0]);
                const songend_t songend = e[1].empty() ? common::SongEnd::PLAYER : parse_songend(e[1]);
                subsong_info = _SubSongInfo(songlength, songend);
            }
            subsong_infos.push_back(subsong_info);
        }
        assert(!subsong_infos.empty());
        const uint8_t minsubsong = cols[1].empty() ? 1 : common::from_chars<uint8_t>(cols[1]);
        const _SongInfo song_info(subsongs.size() > 1, minsubsong, subsong_infos.front());
        assert(md5_idx < MD5_IDX_SIZE);
        db_songinfos[md5_idx] = song_info;
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

void parse_tsv_row(const char *tuple, _MetaData &item, const _MetaData &prev_item, vector<string> &authors, vector<string> &albums, vector<string> &publishers) noexcept {
    const auto cols = common::split_view_x<4>(tuple, '\t');
    const auto authors_ = common::split_view(cols[0], SEPARATOR);
    const auto publishers_ = common::split_view(cols[1], SEPARATOR);
    const auto album = cols[2];
    const auto date = cols[3];

    if (date.empty()) {
        item.year = 0;
    } else if (date.length() >= 4) {
        int year = common::from_chars<int>(date.substr(0,4));
        item.year = year != 0 ? year - 1900u : 0;
    } else if (date[0] == REPEAT) {
        item.year = prev_item.year;
    }

    const auto add_author = [&authors, &item](const string_view &author) {
        item.author = authors.size();
        authors.push_back(string(author));
    };

    const auto add_album = [&albums, &item](const string_view &album) {
        item.album = albums.size();
        albums.push_back(string(album));
    };

    const auto add_publisher = [&publishers, &item](const string_view &publisher) {
        item.publisher = publishers.size();
        publishers.push_back(string(publisher));
    };

    if (cols[0].empty()) {
        item.author = UNKNOWN_AUTHOR_T;
    } else if (cols[0][0] == REPEAT) {
        item.author = prev_item.author;
    } else if (authors_.size() == 1) {
        const auto &author = authors_[0];
        if (author.empty()) {
            item.author = UNKNOWN_AUTHOR_T;
        } else {
            add_author(author);
        }
    } else {
        string author;
        common::mkString(authors_, AUTHOR_JOIN, author);
        add_author(author);
    }

    if (cols[1].empty()) {
        item.publisher = STRING_NOT_FOUND;
    } else if (cols[1][0] == REPEAT) {
        item.publisher = prev_item.publisher;
    } else if (publishers_.size() == 1) {
        add_publisher(publishers_[0]);
    } else {
        string publisher;
        common::mkString(publishers_, AUTHOR_JOIN, publisher);
        add_publisher(publisher);
    }

    if (album.empty()) {
        item.album = STRING_NOT_FOUND;
    } else if (album[0] == REPEAT) {
        item.album = prev_item.album;
    } else {
        add_album(album);
    }
}

template<size_t N>
void parse_tsv(const string &tsv, _MetaData db[N]) noexcept {
    FILE *f = fopen(tsv.c_str(), "r"); 
    if (!f) {
        ERR("Could not open songdb file %s\n", tsv.c_str());
        return;
    }
    char line[BUF_SIZE];
    _MetaData tmp[MD5_IDX_SIZE];
    memset((void *)tmp, 0, sizeof tmp);
    md5_idx_t idx = 0;
    while (fgets(line, sizeof line, f)) {
        int len;
        const md5_idx_t next_idx = b64d24(line, len);
        assert(next_idx > static_cast<md5_idx_t>(0));
        assert(next_idx < static_cast<md5_idx_t>(MD5_IDX_SIZE));
        char *tuple = line + len; // skip md5
        const auto &prev = tmp[idx];
        if (*tuple == '\n') {
            idx = prev.md5 + next_idx;
            assert(idx > static_cast<md5_idx_t>(0));
            assert(idx < static_cast<md5_idx_t>(MD5_IDX_SIZE));
            tmp[idx] = {
                { idx },
                prev.author,
                prev.album,
                prev.publisher,
                prev.year,
            };
        } else {
            idx = next_idx;
            _MetaData &item = tmp[idx];
            parse_tsv_row(tuple + 1, item, prev, author_pool, album_pool, publisher_pool);
            item.md5 = idx;
        }
    }
    fclose(f);
    
    int i = 0;
    for (int j = 0; j < MD5_IDX_SIZE; ++j) {
        if (tmp[j].md5 > static_cast<md5_idx_t>(0)) {
            db[i++] = tmp[j];
        }
    }
    assert(i == N);
}

void parse_modinfos(const string &tsv) noexcept {
    FILE *f = fopen(tsv.c_str(), "r"); 
    if (!f) {
        ERR("Could not open songdb file %s\n", tsv.c_str());
        return;
    }

    md5_idx_t idx = 0;
    char line[BUF_SIZE];
    string_t format_t = STRING_NOT_FOUND;
    uint8_t channels = 0;
    while (fgets(line, sizeof line, f)) {
        int len;
        const md5_idx_t next_idx = b64d24(line, len);
        assert(next_idx < static_cast<md5_idx_t>(MD5_IDX_SIZE));
        char *tuple = line + len; // skip md5
        if (*tuple == '\n') {
            idx = idx + next_idx;
            assert(idx < static_cast<md5_idx_t>(MD5_IDX_SIZE));
            db_modinfos[idx] = { format_t, channels };
            continue;
        }
        idx = next_idx;
        const auto cols = common::split_view_x<2>(tuple + 1, '\t');
        if (cols[0][0] != REPEAT) {
            string fmt = string(cols[0]);
            format_t = format_pool.size();
            format_pool.push_back(fmt);
        }
        channels = cols[1].empty() ? 0
            : cols[1][0] == REPEAT ? channels
            : common::from_chars<uint8_t>(cols[1]);
        db_modinfos[idx] = { format_t, channels };
    }
    fclose(f);
}

} // namespace {}

namespace songdb {

optional<SubSongInfo> lookup(const string &md5, int subsong) noexcept {
    const auto md5_idx = _md5hex(md5);
    if (md5_idx != MD5_NOT_FOUND && initialized[Songlengths]) {
        const auto &info = db_songinfos[md5_idx];
        if (subsong < info.min_subsong()) return {};
        if (info.min_subsong() == subsong) return make_subsonginfo(subsong, info.info());
        if (info.has_subsongs()) {
            assert(db_subsongs.count(md5_idx));
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
    const lock_guard<mutex> lock(extra_mutex);
    if (extra_subsongs.count(hash)) {
        const auto &infos = extra_subsongs[hash];
        for (const auto &info : infos) {
            if (info.subsong == subsong) {
                return info.songend.status != common::SongEnd::NONE ? info : optional<SubSongInfo>();
            }
        }
    }
    return {};
}

optional<Info> lookup(const string &md5) noexcept {
    const auto md5_idx = _md5hex(md5);
    if (md5_idx != MD5_NOT_FOUND) {
        Info res;
        res.modinfo = initialized[ModInfos] ? make_modinfo(md5_idx) : optional<ModInfo>();
        res.combined = initialized[Combined] ? make_meta<COMBINED_SIZE>(md5_idx, db_combined) : optional<MetaData>();
        res.modland = initialized[Modland] ? make_meta<MODLAND_SIZE>(md5_idx, db_modland) : optional<MetaData>();
        res.amp = initialized[AMP] ? make_meta<AMP_SIZE>(md5_idx, db_amp) : optional<MetaData>();
        res.unexotica = initialized[UnExotica] ? make_meta<UNEXOTICA_SIZE>(md5_idx, db_unexotica) : optional<MetaData>();
        res.demozoo = initialized[Demozoo] ? make_meta<DEMOZOO_SIZE>(md5_idx, db_demozoo) : optional<MetaData>();
        if (!initialized[Songlengths]) {
            return res;
        }
        const auto &info = db_songinfos[md5_idx];
        res.subsongs.push_back(make_subsonginfo(info.min_subsong(), info.info()));
        if (info.has_subsongs()) {
            assert(db_subsongs.count(md5_idx));
            const auto &subsongs = db_subsongs[md5_idx];
            uint8_t sub = info.min_subsong() + 1;
            for (const auto &subsong : subsongs) {
                res.subsongs.push_back(make_subsonginfo(sub++, subsong));
            }
        }
        return res;
    }
    const md5_t hash = hex2md5(md5.c_str());
    const lock_guard<mutex> lock(extra_mutex);
    if (!extra_subsongs.count(hash) && !extra_modinfos.count(hash)) {
        return {};
    }
    Info res;
    if (extra_subsongs.count(hash)) {
        const auto &infos = extra_subsongs[hash];
        for (const auto &info : infos) {
            res.subsongs.push_back(info);
        }
    }
    if (extra_modinfos.count(hash)) {
        res.modinfo = extra_modinfos[hash];
    }
    return res;
}

void init(const string &songdb_path, const initializer_list<Source> &sources/*= {}*/) noexcept {
    TRACE("INIT %lu\n", clock() * 1000 / CLOCKS_PER_SEC);

    const auto sources_ = sources.size() ? sources : std::initializer_list<Source>{
#if MODINFOS_SIZE > 1
        ModInfos,
#endif
#if SONGLENGTHS_SIZE > 1
        Songlengths,
#endif
#if COMBINED_SIZE > 1
        Combined,
#endif
#if MODLAND_SIZE > 1
        Modland,
#endif
#if AMP_SIZE > 1
        AMP,
#endif
#if UNEXOTICA_SIZE > 1
        UnExotica,
#endif
#if DEMOZOO_SIZE > 1
        Demozoo,
#endif
    };
    assert(sources_.size());

    const auto shouldInit = [&sources_](const Source source) {
        return !initialized[source] && (source == SOURCE_MD5 ||
            any_of(sources_.begin(), sources_.end(), [source](const Source s) { return s == source; }));
    };

    const auto path = common::ends_with(songdb_path, "/") ? songdb_path : songdb_path + "/";

    if (shouldInit(SOURCE_MD5)) {
        assert(size(md5_idx) > 0);
        parse_md5idx(path + tsvfiles[SOURCE_MD5]);
        initialized[SOURCE_MD5] = true;
        TRACE("PARSE_MD5IDX %lu\n", clock() * 1000 / CLOCKS_PER_SEC);
    }

    if (shouldInit(Songlengths)) {
        assert(size(db_songinfos) > 1);
        parse_songlengths(path + tsvfiles[Songlengths]);
        initialized[Songlengths] = true;
        TRACE("PARSE_SONGLENGTHS %lu\n", clock() * 1000 / CLOCKS_PER_SEC);
    }

    if (shouldInit(ModInfos)) {
        assert(size(db_modinfos) > 1);
        parse_modinfos(path + tsvfiles[ModInfos]);
        initialized[ModInfos] = true;
        TRACE("PARSE_MOD_INFOS %lu\n", clock() * 1000 / CLOCKS_PER_SEC);
    }

    for (int source = Combined; source <= Demozoo; source++) {
        if (shouldInit(static_cast<Source>(source))) {
            const auto file = path + tsvfiles[source];
            switch (source) {
                case Combined:
                    assert(size(db_combined) > 1);
                    parse_tsv<COMBINED_SIZE>(file, db_combined);
                    break;
                case Modland:
                    assert(size(db_modland) > 1);
                    parse_tsv<MODLAND_SIZE>(file, db_modland);
                    break;
                 case AMP:
                    assert(size(db_amp) > 1);
                    parse_tsv<AMP_SIZE>(file, db_amp);
                    break;
                case UnExotica:
                    assert(size(db_unexotica) > 1);
                    parse_tsv<UNEXOTICA_SIZE>(file, db_unexotica);
                    break;
                case Demozoo:
                    assert(size(db_demozoo) > 1);
                    parse_tsv<DEMOZOO_SIZE>(file, db_demozoo);
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

    TRACE("MD5_IDX:%zu/%zu/%zu SONGINFOS:%zu/%zu/%zu SUBSONGS:%zu/%zu/%zu/%zu MODINFOS:%zu/%zu/%zu "
          "COMBINED:%zu/%zu/%zu MODLAND:%zu/%zu/%zu AMP:%zu/%zu/%zu UNEXOTICA:%zu/%zu/%zu DEMOZOO:%zu/%zu/%zu\n",
        sizeof(md5_idx), sizeof(md5_idx) / sizeof(md5_t), sizeof(md5_t),
        sizeof(db_songinfos), sizeof(db_songinfos) / sizeof(_SongInfo), sizeof(_SongInfo),
        size, db_subsongs.size(), sizeof(_SubSongInfo), count,
        sizeof(db_modinfos), sizeof(db_modinfos) / sizeof(_ModInfo), sizeof(_ModInfo),
        sizeof(db_combined), sizeof(db_combined) / sizeof(_MetaData), sizeof(_MetaData),
        sizeof(db_modland), sizeof(db_modland) / sizeof(_MetaData), sizeof(_MetaData),
        sizeof(db_amp), sizeof(db_amp) / sizeof(_MetaData), sizeof(_MetaData),
        sizeof(db_unexotica), sizeof(db_unexotica) / sizeof(_MetaData), sizeof(_MetaData),
        sizeof(db_demozoo), sizeof(db_demozoo) / sizeof(_MetaData), sizeof(_MetaData));

#endif // DEBUG_TRACE

    TRACE("DONE %lu\n", clock() * 1000 / CLOCKS_PER_SEC);
}

void update(const string &md5, const SubSongInfo &info, const int minsubsong, const int maxsubsong) noexcept {
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
    const lock_guard<mutex> lock(extra_mutex);
    if (extra_subsongs.count(hash)) {
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

void update(const string &md5, const ModInfo &info) noexcept {
    if (md5.empty()) {
        WARN("Invalid songdb update entry md5:%s\n", md5.c_str());
        return;
    }
    if (blacklist::is_blacklisted_songdb_key(md5)) {
        INFO("Blacklisted songdb key md5:%s\n", md5.c_str());
        return;
    }
    const md5_t hash = hex2md5(md5.c_str());
    const lock_guard<mutex> lock(extra_mutex);
    if (extra_modinfos.count(hash)) {
        DEBUG("Skipped modinfo update for %s, already exists\n", md5.c_str());
        return;
    }
    extra_modinfos.insert({hash, info});
}

optional<pair<int,int>> subsong_range(const string &md5) noexcept {
    const auto md5_idx = _md5hex(md5);
    if (md5_idx != MD5_NOT_FOUND) {
        const auto &info = db_songinfos[md5_idx];
        if (info.has_subsongs()) {
            assert(db_subsongs.count(md5_idx));
            return pair<int,int>(info.min_subsong(), info.min_subsong() + db_subsongs[md5_idx].size());
        } else {
            return pair<int,int>(info.min_subsong(), info.min_subsong());
        }
    }
    return {};
}

} // namespace songdb
