// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <algorithm>
#include <climits>
#include <fstream>
#include <map>
#include <numeric>
#include <sstream>
#include <set>
#include <vector>

#include "common/common.h"
#include "common/logger.h"
#include "songdb/songdb.h"

using namespace std;
using namespace songdb;

namespace {

enum Source {
    Modland,
    AMP,
    UnExotica,
    Mods_Anthology,
    Wanted_Team,
    Zakalwe,
    Aminet,
    Modland_Incoming,
};

const vector<pair<string, Source>> tsvfiles ({
    {"modland.tsv", Modland},
    {"amp.tsv", AMP},
    {"unexotica.tsv", UnExotica},
    {"modsanthology.tsv", Mods_Anthology},
    {"wantedteam.tsv", Wanted_Team},
    {"zakalwe.tsv", Zakalwe},
    {"aminet.tsv", Aminet},
    {"modland_incoming.tsv", Modland_Incoming},
});

bool initialized = false;

map<pair<string,int>, vector<SongInfo>> db;
map<string,pair<int,int>> db_subsongs;
set<pair<string,ssize_t>> db_filenames;

} // namespace {}

namespace songdb {

void init(const string &songdb_path) {
    if (initialized) {
        return;
    }
    const auto parsetsv = [&](const string &tsv, const Source source) {
        ifstream songdbtsv(tsv, ios::in);
        if (!songdbtsv.is_open()) {
            ERR("Could not open songdb file %s\n", tsv.c_str());
            return;
        }
        string line;
        string prevmd5;
        ssize_t size = -1;
        vector<ModlandData> modland_items;
        vector<AMPData> amp_items;
        vector<UnExoticaData> unexotica_items;
        int minsubsong = INT_MAX;
        int maxsubsong = INT_MIN;

        const auto parse_path = [&](const string &path) {
            if (path.empty()) return;

            switch (source) {
                case Modland:
                case Modland_Incoming: {
                    ModlandData item {};
                    if (modland::parse_path(path, item, source == Modland_Incoming)) {
                        modland_items.push_back(item);
                    }
                    break;
                }
                case AMP: {
                    AMPData item {};
                    if (amp::parse_path(path, item)) {
                        amp_items.push_back(item);
                    }
                    break;
                }
                case UnExotica: {
                    UnExoticaData item {};
                    if (unexotica::parse_path(path, item)) {
                        unexotica_items.push_back(item);
                    }
                    break;
                }
                default:
                    break;
            }
        };

        while (getline(songdbtsv, line)) {
            const auto cols = common::split(line, "\t");
            if (cols.size() < 4) {
                ERR("Invalid line %s\n", line.c_str());
                return;
            }
            string md5 = cols[0];
            int subsong = stoi(cols[1]);
            int length = stoi(cols[2]);
            string reason = cols[3];
            size = cols.size() > 4 ? stoi(cols[4]) : size;
            string path = cols.size() > 5 ? cols[5] : "";

            if (prevmd5 != md5) {
                if (path.empty()) {
                    TRACE("No path for MD5 %s\n", md5.c_str());
                }
                modland_items.clear();
                amp_items.clear();
                unexotica_items.clear();

                parse_path(path);

            } else if (prevmd5 == md5 && !path.empty()) {
                TRACE("Duplicate MD5 %s for %s\n", md5.c_str(), path.c_str());
                parse_path(path);
            }

            if (modland_items.size()) {
                for (const auto &item : modland_items) {
                    const SongInfo info = { md5, subsong, length, reason, size, item };
                    update(info);
                }
            } else if (amp_items.size()) {
               for (const auto &item : amp_items) {
                    const SongInfo info = { md5, subsong, length, reason, size, item };
                    update(info);
                }
            } else if (unexotica_items.size()) {
                for (const auto &item : unexotica_items) {
                    const SongInfo info = { md5, subsong, length, reason, size, item };
                    update(info);
                }
            } else {
                const SongInfo info = { md5, subsong, length, reason, size };
                if (!db.count(pair(info.md5, info.subsong))) {
                    update(info);
                }
            }

            if (!prevmd5.empty() && prevmd5 != md5) {
                if (!db_subsongs.count(prevmd5)) {
                    db_subsongs[prevmd5] = pair(minsubsong, maxsubsong);
                }
                minsubsong = subsong;
                maxsubsong = subsong;

            } else {
                minsubsong = min(minsubsong, subsong);
                maxsubsong = max(maxsubsong, subsong);
            }

            if (path.size()) {
                string filename = common::split(path,"/").back();
                transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
                const auto key = pair(filename, size);
                if (!db_filenames.count(key)) {
                    db_filenames.insert(key);
                }
            }
    
            prevmd5 = md5;
            //TRACE("%s -> format = %s, author = %s, album = %s, filename = %s\n", line.c_str(), item.format, item.author, item.album, item.filename);
        }
    };

    db.clear();
    db_subsongs.clear();

    for (const auto &tsv : tsvfiles) {
        parsetsv(songdb_path + "/" + tsv.first, tsv.second);
    }

    initialized = true;

    return;
}

optional<SongInfo> lookup(const string &md5, int subsong, const string &path) {
    const auto key = pair(md5, subsong);
    const auto filename = common::split(path, "/").back();
    if (db.count(key)) {
        bool foundPath = false;
        bool foundAuthor = false;
        bool foundFilename = false;
        optional<SongInfo> found;
        for (const auto& data : db[key]) {
            if (foundPath && foundAuthor && foundFilename) break;
            if (!found) {
                found = data;
            }
            if (!foundPath) {
                const auto modland_path = data.modland_data ?
                    data.modland_data->path : "";
                const auto amp_path = data.amp_data ?
                    data.amp_data->path : "";
                const auto unexotica_path = data.unexotica_data ?
                    data.unexotica_data->path : "";

                if (!foundPath && !modland_path.empty() && path.ends_with(modland_path)) {
                    foundPath = true;
                }
                if (!foundPath && !amp_path.empty() && path.ends_with(amp_path)) {
                    foundPath = true;
                }
                if (!foundPath && !unexotica_path.empty() && path.ends_with(unexotica_path)) {
                    foundPath = true;
                }
                if (foundPath) {
                    found = data;
                    foundFilename = true;
                    foundAuthor = true;
                }
            }
            if (!foundAuthor) {
                const auto modland_author = data.modland_data ?
                    data.modland_data->author : "";
                const auto amp_author = data.amp_data ?
                    data.amp_data->author : "";
                const auto unexotica_author = data.unexotica_data ?
                    data.unexotica_data->author : "";

                if (!foundAuthor && !modland_author.empty() && modland_author != UNKNOWN_AUTHOR) {
                    found = data;
                    foundAuthor = true;
                    if (data.modland_data->filename == filename) {
                        foundFilename = true;
                    }
                }
                if (!foundAuthor && !amp_author.empty() && amp_author != UNKNOWN_AUTHOR) {
                    found = data;
                    foundAuthor = true;
                    if (data.amp_data->filename == filename) {
                        foundFilename = true;
                    }
                }
                if (!foundAuthor && !unexotica_author.empty() && unexotica_author != UNKNOWN_AUTHOR) {
                    found = data;
                    foundAuthor = true;
                    if (data.unexotica_data->filename == filename) {
                        foundFilename = true;
                    }
                }
            }
            if (!foundFilename) {
                const auto modland_filename = data.modland_data ?
                    data.modland_data->filename : "";
                const auto amp_filename = data.amp_data ?
                    data.amp_data->filename : "";
                const auto unexotica_filename = data.unexotica_data ?
                    data.unexotica_data->filename : "";

                if (!foundFilename && modland_filename == filename) {
                    if (!foundAuthor || (foundAuthor && data.modland_data->author != UNKNOWN_AUTHOR)) {
                        found = data;
                        foundFilename = true;
                    }
                }
                if (!foundFilename && amp_filename == filename) {
                    if (!foundAuthor || (foundAuthor && data.amp_data->author != UNKNOWN_AUTHOR)) {
                        found = data;
                        foundFilename = true;
                    }
                }
                if (!foundFilename && unexotica_filename == filename) {
                    if (!foundAuthor || (foundAuthor && data.unexotica_data->author != UNKNOWN_AUTHOR)) {
                        found = data;
                        foundFilename = true;
                    }
                }
            }
        }
        return found;
    }

    return {};
}

vector<SongInfo> lookup_all(const string &md5, int subsong) {
    vector<SongInfo> infos;
    const auto key = pair(md5, subsong);
    if (db.count(key)) {
        for (const auto &data : db[key]) {
            infos.push_back(data);
        }
    }
    return infos;
}

void update(const SongInfo &info) {
    if (info.md5.empty() || info.subsong < 0) {
        WARN("Invalid songdb key md5:%s subsong:%d\n", info.md5.c_str(), info.subsong);
        return;
    }
    if (blacklist::is_blacklisted_songdb_key(info.md5)) {
        INFO("Blacklisted songdb key md5:%s\n", info.md5.c_str());
        return;
    }
    const auto key = pair(info.md5, info.subsong);
    if (db.count(key)) {
         //TRACE("Duplicate md5 for: %s:%d\n", key.first.c_str(), key.second);
         auto& data = db[key];
         data.push_back(info);
    } else {
         vector<SongInfo> data;
         data.push_back(info);
         db[key] = data;
    }

    return;
}

optional<pair<int,int>> subsong_range(const string &md5) {
    if (db_subsongs.count(md5)) {
        return db_subsongs[md5];
    }
    return {};
}

bool exists(const string &path, const ssize_t size) {
    auto lcname = common::split(path, "/").back();
    transform(lcname.begin(), lcname.end(), lcname.begin(), ::tolower);
    return db_filenames.count(pair(lcname,size));
}

} // namespace songdb
