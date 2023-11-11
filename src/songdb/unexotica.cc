// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <algorithm>
#include <numeric>
#include <set>
#include <string>

#include "common/common.h"
#include "common/logger.h"
#include "songdb/songdb.h"

using namespace std;
using namespace songdb;

namespace {

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

} // namespace {}

namespace songdb::unexotica {

bool parse_path(const string &path, UnExoticaData &item) {
    string author, album, note, filename;

    vector<string> tokens = common::split(path, "/");
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

    vector<string> author_tokens = common::split(author, "_");
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
        item.author = accumulate(begin(author_tokens), end(author_tokens), string(),[](const string &ss, const string &s) {
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

} // namespace songdb::unexotica