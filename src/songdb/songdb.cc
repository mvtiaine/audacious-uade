// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <algorithm>
#include <fstream>
#include <map>
#include <numeric>
#include <sstream>
#include <set>
#include <vector>

#include "common.h"
#include "uade/hacks.h"
#include "songdb/songdb.h"

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
    {UADEDIR "/songdb/modland.tsv", Modland},
    {UADEDIR "/songdb/amp.tsv", AMP},
    {UADEDIR "/songdb/unexotica.tsv", UnExotica},
    {UADEDIR "/songdb/modsanthology.tsv", Mods_Anthology},
    {UADEDIR "/songdb/wantedteam.tsv", Wanted_Team},
    {UADEDIR "/songdb/zakalwe.tsv", Zakalwe},
    {UADEDIR "/songdb/aminet.tsv", Aminet},
    {UADEDIR "/songdb/modland_incoming.tsv", Modland_Incoming},
});

bool initialized = false;

map<pair<string,int>, vector<SongInfo>> db;
map<string,pair<int,int>> db_subsongs;
set<pair<string,ssize_t>> db_filenames;

constexpr string_view UNKNOWN = "Unknown";
const set<string> pseudonyms ({
    "Creative_Thought",
    "DJ_Braincrack",
    "Digital_Masters",
    "MC_Slack",
    "Mad_Jack",
    "Mad_Phantom",
    "McMullan_and_Low",
    "Pipe_Smokers_Cough",
    "Private_Affair",
    "Radio_Poland",
    "Sonic_Boom_Boy",
    "Sonicom_Music",
    "Ten_Pin_Alley",
    "Urban_Shakedown",
});

bool parse_unexotica_path(const string &path, UnExoticaData &item) {
    string author, album, note, filename;

    vector<string> tokens = split(path, "/");
    const int count = tokens.size();

    if (count < 4) {
        WARN("Skipping path: %s\n", path.c_str());
        return false;
    }

    author = tokens[1];
    album = tokens[2];

    if (count > 4) {
        note = tokens[3];
        filename = tokens[4];
    } else {
        filename = tokens[3];
    }

    vector<string> author_tokens = split(author, "_");
    const auto first = author_tokens[0];
    const bool pseudonym = pseudonyms.count(author);

    if (!pseudonym && (first == "Da" || first == "de" || first == "van" || first == "Pieket")) {
        const auto last = author_tokens.back();
        author_tokens.erase(author_tokens.end() - 1);
        author_tokens.insert(author_tokens.begin(), last);
    } else if (!pseudonym && first != "The") {
        author_tokens.erase(author_tokens.begin());
        author_tokens.push_back(first);
    }
    item.path = path;

    if (author == UNKNOWN) {
        item.author = UNKNOWN_AUTHOR;
    } else {
        item.author = accumulate(begin(author_tokens), end(author_tokens), string(),[](string &ss, string &s) {
            return ss.empty() ? s : ss + " " + s;
        });
    }

    item.album = album;
    item.note = note;
    item.filename = filename;

    replace(item.album.begin(), item.album.end(), '_', ' ');
    replace(item.note.begin(), item.note.end(), '_', ' ');

    return true;
}

constexpr string_view UNKNOWN_COMPOSERS = "UnknownComposers";

bool parse_amp_path(const string &path, AMPData &item) {
    string author, filename;
    
    vector<string> tokens = split(path, "/");
    const int count = tokens.size();

    if (count < 3) {
        WARN("Skipping path: %s\n", path.c_str());
        return false;
    }

    author = tokens[1];
    filename = tokens[2];

    item.path = path;
    if (author == UNKNOWN_COMPOSERS) {
        item.author = UNKNOWN_AUTHOR;
    } else {
        item.author = author;
    }
    item.filename = filename;

    return true;
}

} // namespace

void songdb_init(void) {
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
                    if (parse_modland_path(path, item, source == Modland_Incoming)) {
                        modland_items.push_back(item);
                    }
                    break;
                }
                case AMP: {
                    AMPData item {};
                    if (parse_amp_path(path, item)) {
                        amp_items.push_back(item);
                    }
                    break;
                }
                case UnExotica: {
                    UnExoticaData item {};
                    if (parse_unexotica_path(path, item)) {
                        unexotica_items.push_back(item);
                    }
                    break;
                }
                default:
                    break;
            }
        };

        while (getline(songdbtsv, line)) {
            const auto cols = split(line, "\t");
            if (cols.size() < 4) {
                ERR("Invalid line %s\n", line.c_str());
                return;
            }
            string md5 = cols[0];
            int subsong = atoi(cols[1].c_str());
            int length = atoi(cols[2].c_str());
            string reason = cols[3];
            string path = cols.size() > 5 ? cols[5] : "";

            if (prevmd5 != md5) {
                if (path.empty()) {
                    INFO("No path for MD5 %s\n", md5.c_str());
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
                    songdb_update(info);
                }
            } else if (amp_items.size()) {
               for (const auto &item : amp_items) {
                    const SongInfo info = { md5, subsong, length, reason, size, item };
                    songdb_update(info);
                }
            } else if (unexotica_items.size()) {
                for (const auto &item : unexotica_items) {
                    const SongInfo info = { md5, subsong, length, reason, size, item };
                    songdb_update(info);
                }
            } else {
                const SongInfo info = { md5, subsong, length, reason, size };
                if (!db.count(pair(info.md5, info.subsong))) {
                    songdb_update(info);
                }
            }

            if (!prevmd5.empty() && prevmd5 != md5) {
                if (!db_subsongs.count(prevmd5)) {
                    db_subsongs[prevmd5] = pair(minsubsong, maxsubsong);
                }
                size = cols.size() > 4 ? atoi(cols[4].c_str()) : -1;

                minsubsong = subsong;
                maxsubsong = subsong;

            } else {
                minsubsong = min(minsubsong, subsong);
                maxsubsong = max(maxsubsong, subsong);
            }

            if (path.size()) {
                string filename = split(path,"/").back();
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
        parsetsv(tsv.first, tsv.second);
    }

    initialized = true;

    return;
}

optional<SongInfo> songdb_lookup(const string &md5, int subsong, const string &path) {
    const auto key = pair(md5, subsong);
    const auto filename = split(path, "/").back();
    if (db.count(key)) {
        bool foundPath = false;
        bool foundAuthor = false;
        bool foundFilename = false;
        optional<SongInfo> found;
        for (const auto& data : db[key]) {
            if (foundPath && foundAuthor && foundFilename) break;
            if (!found.has_value()) {
                found = data;
            }
            if (!foundPath) {
                const auto modland_path = data.modland_data.has_value() ?
                    data.modland_data.value().path : "";
                const auto amp_path = data.amp_data.has_value() ?
                    data.amp_data.value().path : "";
                const auto unexotica_path = data.unexotica_data.has_value() ?
                    data.unexotica_data.value().path : "";

                if (!foundPath && !modland_path.empty() && ends_with(path, modland_path)) {
                    foundPath = true;
                }
                if (!foundPath && !amp_path.empty() && ends_with(path, amp_path)) {
                    foundPath = true;
                }
                if (!foundPath && !unexotica_path.empty() && ends_with(path, unexotica_path)) {
                    foundPath = true;
                }
                if (foundPath) {
                    found = data;
                    foundFilename = true;
                    foundAuthor = true;
                }
            }
            if (!foundAuthor) {
                const auto modland_author = data.modland_data.has_value() ?
                    data.modland_data.value().author : "";
                const auto amp_author = data.amp_data.has_value() ?
                    data.amp_data.value().author : "";
                const auto unexotica_author = data.unexotica_data.has_value() ?
                    data.unexotica_data.value().author : "";

                if (!foundAuthor && !modland_author.empty() && modland_author != UNKNOWN_AUTHOR) {
                    found = data;
                    foundAuthor = true;
                    if (data.modland_data.value().filename == filename) {
                        foundFilename = true;
                    }
                }
                if (!foundAuthor && !amp_author.empty() && amp_author != UNKNOWN_AUTHOR) {
                    found = data;
                    foundAuthor = true;
                    if (data.amp_data.value().filename == filename) {
                        foundFilename = true;
                    }
                }
                if (!foundAuthor && !unexotica_author.empty() && unexotica_author != UNKNOWN_AUTHOR) {
                    found = data;
                    foundAuthor = true;
                    if (data.unexotica_data.value().filename == filename) {
                        foundFilename = true;
                    }
                }
            }
            if (!foundFilename) {
                const auto modland_filename = data.modland_data.has_value() ?
                    data.modland_data.value().filename : "";
                const auto amp_filename = data.amp_data.has_value() ?
                    data.amp_data.value().filename : "";
                const auto unexotica_filename = data.unexotica_data.has_value() ?
                    data.unexotica_data.value().filename : "";

                if (!foundFilename && modland_filename == filename) {
                    if (!foundAuthor || (foundAuthor && data.modland_data.value().author != UNKNOWN_AUTHOR)) {
                        found = data;
                        foundFilename = true;
                    }
                }
                if (!foundFilename && amp_filename == filename) {
                    if (!foundAuthor || (foundAuthor && data.amp_data.value().author != UNKNOWN_AUTHOR)) {
                        found = data;
                        foundFilename = true;
                    }
                }
                if (!foundFilename && unexotica_filename == filename) {
                    if (!foundAuthor || (foundAuthor && data.unexotica_data.value().author != UNKNOWN_AUTHOR)) {
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

optional<pair<int,int>> songdb_subsong_range(const string &md5) {
    if (db_subsongs.count(md5)) {
        return db_subsongs[md5];
    }
    return {};
}

bool songdb_exists(const string &filename, const ssize_t size) {
    auto lcname = filename;
    transform(lcname.begin(), lcname.end(), lcname.begin(), ::tolower);
    return db_filenames.count(pair(lcname,size));
}
