// SPDX-License-Identifier: LGPL-2.1-or-later
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

constexpr_f1 hash_t hex2hash(const char *hex) noexcept {
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

constexpr_f1 hash_t b64diff2hash(const hash_t prev, const char *b64) noexcept {
    if (b64[1] == '\n') {
        return prev + (b64[0] - 45);
    } else if (b64[2] == '\n') {
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

hash_t hash_idx[HASH_IDX_SIZE];
// these may can contain duplicates
vector<string> format_pool;
vector<string> author_pool = {""}; // unknown author
vector<string> publisher_pool;
vector<string> album_pool;

constexpr_f1 hash_idx_t _hashidx(const hash_t hash) noexcept  {
    uint32_t idx = ((double)hash / HASH_T_MAX) * HASH_IDX_SIZE;
    assert(idx < HASH_IDX_SIZE);
    hash_t cmp = hash_idx[idx];
    if (cmp == hash) {
         return idx;
    } else if (cmp > hash) {
        while (cmp > hash && idx > 0) {
            idx--;
            cmp = hash_idx[idx];
        }
        return (cmp == hash) ? static_cast<hash_idx_t>(idx) : HASH_NOT_FOUND;
    } else {
        while (cmp < hash && idx < HASH_IDX_SIZE - 1) {
            idx++;
            cmp = hash_idx[idx];
        }
        return (cmp == hash) ? static_cast<hash_idx_t>(idx) : HASH_NOT_FOUND;
    }
}

constexpr_f1 hash_idx_t _hashhex(const string_view &hash) noexcept {
    assert(hash.size() >= 12);
    return _hashidx(hex2hash(hash.data()));
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
    "xxh32idx.tsv",
    "songlengths.tsv",
    "modinfos.tsv",
    "metadata.tsv",
});

_SongInfo db_songinfos[SONGLENGTHS_SIZE+1]; // md5_idx_t -> first subsong info
unordered_map<hash_idx_t, vector<_SubSongInfo>> db_subsongs; // infos for extra subsongs
_ModInfo db_modinfos[MODINFOS_SIZE+1];
_MetaData db_metadata[METADATA_SIZE];

unordered_map<hash_t, vector<SubSongInfo>> extra_subsongs; // runtime only
unordered_map<hash_t, ModInfo> extra_modinfos; // runtime only
mutex extra_mutex; // for extra_subsongs/modinfos thread safe access/update

constexpr Source SOURCE_HASH = static_cast<Source>(0); // internal
bool initialized[Metadata+1] = {};

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

constexpr_f1 optional<_MetaData> _find(const _MetaData *db, const size_t N, const hash_idx_t hash) noexcept {
    if (hash >= HASH_IDX_SIZE) return {};
    unsigned int idx = ((double)hash / HASH_IDX_SIZE) * N;
    assert(idx < N);
    hash_idx_t cmp = db[idx].hash;
    if (cmp == hash) {
        return db[idx];
    } else if (cmp > hash) {
        while (cmp > hash && idx > 0) {
            idx--;
            cmp = db[idx].hash;
        }
        return cmp == hash ? db[idx] : optional<_MetaData>();
    } else {
        while (cmp < hash && idx < N - 1) {
            idx++;
            cmp = db[idx].hash;
        }
        return cmp == hash ? db[idx] : optional<_MetaData>();
    }
}

constexpr_f2 optional<ModInfo> make_modinfo(const hash_idx_t hash) noexcept {
    const auto &data = db_modinfos[hash];
    if (data.format != STRING_NOT_FOUND || data.channels > 0) {
        return ModInfo {
            make_format(data.format),
            data.channels,
        };
    }
    return {};
}

constexpr_f2 optional<MetaData> make_meta(const hash_idx_t hash, const _MetaData *db, const size_t N) noexcept {
    const auto data = _find(db, N, hash);
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
        {info.songend(), info.songlength_ms()},
        info.is_duplicate()
    };
}

void parse_hashidx(const string &tsv) noexcept {
    FILE *f = fopen(tsv.c_str(), "r"); 
    if (!f) {
        ERR("Could not open songdb file %s (%d)\n", tsv.c_str(), errno);
        return;
    }

    int i = 0;
    hash_t prevhash = 0;
    char line[BUF_SIZE];
    while (fgets(line, sizeof line, f)) {
        const hash_t hash = (i == 0) ? static_cast<hash_t>(0) : b64diff2hash(prevhash, line);
        assert(i == 0 || hash > prevhash);
        hash_idx[i] = hash;
        prevhash = hash;
        i++;
    }
    fclose(f);
    assert(i == HASH_IDX_SIZE);
}

void parse_songlengths(const string &tsv) noexcept {
    FILE *f = fopen(tsv.c_str(), "r"); 
    if (!f) {
        ERR("Could not open songdb file %s (%d)\n", tsv.c_str(), errno);
        return;
    }
    int hash_idx = 1; // 0 entry is special
    char line[BUF_SIZE];
    while (fgets(line, sizeof line, f)) {
        const auto cols = common::split_view_x<2>(line, '\t');
        const auto subsongs = common::split_view(cols[0], ' ');
        vector<_SubSongInfo> subsong_infos;
        _SubSongInfo subsong_info(0,common::SongEnd::Status::ERROR,false);
        for (const auto &col : subsongs) {
            if (col.empty()) {
                subsong_info = _SubSongInfo(subsong_info, false);
            } else if (col == "!") {
                subsong_info = _SubSongInfo(subsong_info, true);
            } else {
                const auto e = common::split_view<2>(col, ',');
                const songlength_t songlength = e[0].empty() ? static_cast<songlength_t>(0) : b64d24(e[0]);
                const songend_t songend = e[1].empty() || e[1] == "!" ? common::SongEnd::PLAYER : parse_songend(e[1]);
                const bool is_duplicate = (e.size() == 2 && e[1] == "!") || (e.size() > 2 && e[2] == "!");
                subsong_info = _SubSongInfo(songlength, songend, is_duplicate);
            }
            subsong_infos.push_back(subsong_info);
        }
        assert(!subsong_infos.empty());
        const uint8_t minsubsong = cols[1].empty() ? 1 : common::from_chars<uint8_t>(cols[1]);
        const _SongInfo song_info(subsongs.size() > 1, minsubsong, subsong_infos.front());
        assert(hash_idx < HASH_IDX_SIZE);
        db_songinfos[hash_idx] = song_info;
        if (subsong_infos.size() > 1) {
            subsong_infos.erase(subsong_infos.begin());
            subsong_infos.shrink_to_fit();
            db_subsongs.insert({hash_idx, subsong_infos});
        }
        hash_idx++;
    }
    fclose(f);
    assert(hash_idx == HASH_IDX_SIZE);
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

void parse_tsv(const string &tsv, _MetaData *db, const size_t N) noexcept {
    FILE *f = fopen(tsv.c_str(), "r"); 
    if (!f) {
        ERR("Could not open songdb file %s (%d)\n", tsv.c_str(), errno);
        return;
    }
    char line[BUF_SIZE];
    // use malloc to avoid excessive stack usage
    _MetaData *tmp = (_MetaData *)malloc(HASH_IDX_SIZE * sizeof(_MetaData));
    assert(tmp);
    memset((void *)tmp, 0, HASH_IDX_SIZE * sizeof(_MetaData));
    hash_idx_t idx = 0;
    while (fgets(line, sizeof line, f)) {
        int len;
        const hash_idx_t next_idx = b64d24(line, len);
        assert(next_idx > static_cast<hash_idx_t>(0));
        assert(next_idx < static_cast<hash_idx_t>(HASH_IDX_SIZE));
        char *tuple = line + len; // skip hash
        const auto &prev = tmp[idx];
        if (*tuple == '\n') {
            idx = prev.hash + next_idx;
            assert(idx > static_cast<hash_idx_t>(0));
            assert(idx < static_cast<hash_idx_t>(HASH_IDX_SIZE));
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
            item.hash = idx;
        }
    }
    fclose(f);
    
    size_t i = 0;
    for (int j = 0; j < HASH_IDX_SIZE; ++j) {
        if (tmp[j].hash > static_cast<hash_idx_t>(0)) {
            db[i++] = tmp[j];
        }
    }
    free(tmp);
    assert(i == N);
}

void parse_modinfos(const string &tsv) noexcept {
    FILE *f = fopen(tsv.c_str(), "r"); 
    if (!f) {
        ERR("Could not open songdb file %s (%d)\n", tsv.c_str(), errno);
        return;
    }

    hash_idx_t idx = 0;
    char line[BUF_SIZE];
    string_t format_t = STRING_NOT_FOUND;
    uint8_t channels = 0;
    while (fgets(line, sizeof line, f)) {
        int len;
        const hash_idx_t next_idx = b64d24(line, len);
        assert(next_idx < static_cast<hash_idx_t>(HASH_IDX_SIZE));
        char *tuple = line + len; // skip hash
        if (*tuple == '\n') {
            idx = idx + next_idx;
            assert(idx < static_cast<hash_idx_t>(HASH_IDX_SIZE));
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

optional<SubSongInfo> lookup(const string &hash, int subsong) noexcept {
    const auto hash_idx = _hashhex(hash);
    if (hash_idx != HASH_NOT_FOUND && initialized[Songlengths]) {
        const auto &info = db_songinfos[hash_idx];
        if (subsong < info.min_subsong()) return {};
        if (info.min_subsong() == subsong) return make_subsonginfo(subsong, info.info());
        if (info.has_subsongs()) {
            assert(db_subsongs.count(hash_idx));
            const auto &subsongs = db_subsongs[hash_idx];
            if (info.min_subsong() + subsongs.size() < static_cast<unsigned>(subsong)) return {};
            int sub = info.min_subsong() + 1;
            for (const auto &info: subsongs) {
                if (sub++ == subsong) return make_subsonginfo(subsong, info);
            }
            assert(false);
        } else return {};
    }
    const hash_t _hash = hex2hash(hash.c_str());
    const lock_guard<mutex> lock(extra_mutex);
    if (extra_subsongs.count(_hash)) {
        const auto &infos = extra_subsongs[_hash];
        for (const auto &info : infos) {
            if (info.subsong == subsong) {
                return info.songend.status != common::SongEnd::NONE ? info : optional<SubSongInfo>();
            }
        }
    }
    return {};
}

optional<Info> lookup(const string &hash) noexcept {
    const auto hash_idx = _hashhex(hash);
    if (hash_idx != HASH_NOT_FOUND) {
        Info res;
        res.modinfo = initialized[ModInfos] ? make_modinfo(hash_idx) : optional<ModInfo>();
        res.metadata = initialized[Metadata] ? make_meta(hash_idx, db_metadata, METADATA_SIZE) : optional<MetaData>();
        if (!initialized[Songlengths]) {
            return res;
        }
        const auto &info = db_songinfos[hash_idx];
        res.subsongs.push_back(make_subsonginfo(info.min_subsong(), info.info()));
        if (info.has_subsongs()) {
            assert(db_subsongs.count(hash_idx));
            const auto &subsongs = db_subsongs[hash_idx];
            uint8_t sub = info.min_subsong() + 1;
            for (const auto &subsong : subsongs) {
                res.subsongs.push_back(make_subsonginfo(sub++, subsong));
            }
        }
        return res;
    }
    const hash_t _hash = hex2hash(hash.c_str());
    const lock_guard<mutex> lock(extra_mutex);
    if (!extra_subsongs.count(_hash) && !extra_modinfos.count(_hash)) {
        return {};
    }
    Info res;
    if (extra_subsongs.count(_hash)) {
        const auto &infos = extra_subsongs[_hash];
        for (const auto &info : infos) {
            res.subsongs.push_back(info);
        }
    }
    if (extra_modinfos.count(_hash)) {
        res.modinfo = extra_modinfos[_hash];
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
#if METADATA_SIZE > 1
        Metadata,
#endif
    };
    assert(sources_.size());

    const auto shouldInit = [&sources_](const Source source) {
        return !initialized[source] && (source == SOURCE_HASH ||
            any_of(sources_.begin(), sources_.end(), [source](const Source s) { return s == source; }));
    };

    const auto path = common::ends_with(songdb_path, "/") ? songdb_path : songdb_path + "/";

    if (shouldInit(SOURCE_HASH)) {
        assert(size(hash_idx) > 0);
        parse_hashidx(path + tsvfiles[SOURCE_HASH]);
        initialized[SOURCE_HASH] = true;
        TRACE("PARSE_HASHIDX %lu\n", clock() * 1000 / CLOCKS_PER_SEC);
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

    if (shouldInit(Metadata)) {
        assert(size(db_metadata) > 1);
        parse_tsv(path + tsvfiles[Metadata], db_metadata, METADATA_SIZE);
        initialized[Metadata] = true;
        TRACE("PARSE_METADATA %lu\n", clock() * 1000 / CLOCKS_PER_SEC);
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

    TRACE("HASH_IDX:%zu/%zu/%zu SONGINFOS:%zu/%zu/%zu SUBSONGS:%zu/%zu/%zu/%zu MODINFOS:%zu/%zu/%zu METADATA:%zu/%zu/%zu\n",
        sizeof(hash_idx), sizeof(hash_idx) / sizeof(hash_t), sizeof(hash_t),
        sizeof(db_songinfos), sizeof(db_songinfos) / sizeof(_SongInfo), sizeof(_SongInfo),
        size, db_subsongs.size(), sizeof(_SubSongInfo), count,
        sizeof(db_modinfos), sizeof(db_modinfos) / sizeof(_ModInfo), sizeof(_ModInfo),
        sizeof(db_metadata), sizeof(db_metadata) / sizeof(_MetaData), sizeof(_MetaData));

#endif // DEBUG_TRACE

    TRACE("DONE %lu\n", clock() * 1000 / CLOCKS_PER_SEC);
}

void update(const string &hash, const SubSongInfo &info, const int minsubsong, const int maxsubsong) noexcept {
    assert(hash.size() >= 12);
    if (blacklist::is_blacklisted_songdb_hash(hash)) {
        INFO("Blacklisted songdb key hash:%s\n", hash.c_str());
        return;
    }
    assert(minsubsong >= 0);
    assert(minsubsong <= 255);
    assert(maxsubsong >= 0);
    assert(maxsubsong <= 255);
    assert(minsubsong <= maxsubsong);
    assert(info.subsong >= 0);
    assert(info.subsong <= 255);
    assert(info.songend.length <= UINT24_T_MAX);

    const hash_t _hash = hex2hash(hash.c_str());
    const lock_guard<mutex> lock(extra_mutex);
    if (extra_subsongs.count(_hash)) {
        auto &infos = extra_subsongs[_hash];
        for (auto &oldinfo : infos) {
            if (oldinfo.subsong == info.subsong) {
                if (oldinfo.songend.status != common::SongEnd::NONE) {
                    DEBUG("Skipped songlength update for %s:%d, already exists\n", hash.c_str(), info.subsong);
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
        extra_subsongs.insert({_hash, infos});
    }
}

void update(const string &hash, const ModInfo &info) noexcept {
    if (hash.empty()) {
        WARN("Invalid songdb update entry hash:%s\n", hash.c_str());
        return;
    }
    if (blacklist::is_blacklisted_songdb_hash(hash)) {
        INFO("Blacklisted songdb key hash:%s\n", hash.c_str());
        return;
    }
    const hash_t _hash = hex2hash(hash.c_str());
    const lock_guard<mutex> lock(extra_mutex);
    if (extra_modinfos.count(_hash)) {
        DEBUG("Skipped modinfo update for %s, already exists\n", hash.c_str());
        return;
    }
    extra_modinfos.insert({_hash, info});
}

} // namespace songdb
