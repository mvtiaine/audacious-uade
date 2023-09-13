// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <fstream>
#include <map>
#include <sstream>
#include <set>
#include <vector>

#include "common.h"
#include "hacks.h"
#include "songdb.h"
#include "prefs.h"

namespace {

constexpr auto MODLAND_TSV_FILE = UADEDIR "/modland.tsv";
constexpr auto MODLAND_INCOMING_TSV_FILE = UADEDIR "/modland_incoming.tsv";
const vector<string> tsvfiles ({
    MODLAND_TSV_FILE,
    UADEDIR "/unexotica.tsv",
    UADEDIR "/amp.tsv",
    UADEDIR "/modsanthology.tsv",
    UADEDIR "/wantedteam.tsv",
    UADEDIR "/zakalwe.tsv",
    UADEDIR "/aminet.tsv",
    MODLAND_INCOMING_TSV_FILE,
});

bool initialized = false;

map<pair<string,int>, vector<SongInfo>> db;
map<string,pair<int,int>> db_subsongs;

} // namespace

void songdb_init(void) {
    if (initialized) {
        return;
    }
    const auto parsetsv = [&](const string &tsv, const bool modland, const bool modland_incoming) {
        ifstream songdbtsv(tsv, ios::in);
        if (!songdbtsv.is_open()) {
            ERROR("Could not open songdb file %s\n", tsv.c_str());
            return;
        }
        string line;
        string prevmd5;
        vector<ModlandData> modland_items;
        int minsubsong = INT_MAX;
        int maxsubsong = INT_MIN;
        while (getline(songdbtsv, line)) {
            const auto cols = split(line, "\t");
            if (cols.size() < 4) {
                ERROR("Invalid line %s\n", line.c_str());
                return;
            }
            string md5 = cols[0];
            int subsong = atoi(cols[1].c_str());
            int length = atoi(cols[2].c_str());
            string reason = cols[3];
            if (prevmd5 != md5) {
                modland_items.clear();
                if (cols.size() > 4 && modland) {
                    const auto modland_path = cols[4];
                    ModlandData item {};
                    if (parse_modland_path(modland_path, item, modland_incoming)) {
                        modland_items.push_back(item);
                    }
                }
            } else if (prevmd5 == md5 && cols.size() > 4) {
                const auto path = cols[4];
                TRACE("Duplicate MD5 %s for %s\n", md5.c_str(), path.c_str());
                ModlandData item {};
                if (modland && parse_modland_path(path, item, modland_incoming)) {
                    modland_items.push_back(item);
                }
            }

            if (modland_items.size()) {
                for (const auto &item : modland_items) {
                    const SongInfo info = { md5, subsong, length, reason, item };
                    songdb_update(info);
                }
            } else {
                const SongInfo info = { md5, subsong, length, reason };
                if (!db.count(pair(info.md5, info.subsong))) {
                    songdb_update(info);
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
            prevmd5 = md5;
            //TRACE("%s -> format = %s, author = %s, album = %s, filename = %s\n", line.c_str(), item.format, item.author, item.album, item.filename);
        }
    };

    db.clear();
    db_subsongs.clear();

    for (const auto &tsv : tsvfiles) {
        const bool modland = tsv == MODLAND_TSV_FILE || tsv == MODLAND_INCOMING_TSV_FILE;
        const bool modland_incoming = tsv == MODLAND_INCOMING_TSV_FILE;
        parsetsv(tsv, modland, modland_incoming);
    }

    initialized = true;

    return;
}

optional<SongInfo> songdb_lookup(const string &md5, int subsong, const string &filename) {
    const auto key = pair(md5, subsong);
    if (db.count(key)) {
        bool foundAuthor = false;
        bool foundFilename = false;
        optional<SongInfo> found;
        for (const auto& data : db[key]) {
            if (foundAuthor && foundFilename) break;
            if (!found.has_value()) {
                found = data;
            }
            if (!foundAuthor) {
                if (data.modland_data.has_value() && data.modland_data.value().author != UNKNOWN_AUTHOR) {
                    found = data;
                    foundAuthor = true;
                    if (data.modland_data.value().filename == filename) {
                        foundFilename = true;
                    }
                }
            }
            if (!foundFilename) {
                if (data.modland_data.has_value() && data.modland_data.value().filename == filename) {
                    if (!foundAuthor || (foundAuthor && data.modland_data.value().author != UNKNOWN_AUTHOR)) {
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

void songdb_update(const SongInfo &info) {
    if (info.md5.empty() || info.subsong < 0) {
        WARN("Invalid songdb key md5:%s subsong:%d\n", info.md5.c_str(), info.subsong);
        return;
    }
    if (is_blacklisted_songdb(info.md5)) {
        WARN("Blacklisted songdb key md5:%s\n", info.md5.c_str());
        return;
    }
    const auto key = pair(info.md5, info.subsong);
    if (db.count(key)) {
         TRACE("Duplicate md5 for: %s:%d\n", key.first.c_str(), key.second);
         auto& data = db[key];
         data.push_back(info);
    } else {
         vector<SongInfo> data;
         data.push_back(info);
         db[key] = data;
    }

    return;
}

optional<pair<int,int>> songdb_subsong_range(const string &md5) {
    if (db_subsongs.count(md5)) {
        return db_subsongs[md5];
    }
    return {};
}