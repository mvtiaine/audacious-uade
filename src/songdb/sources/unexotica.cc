// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <algorithm>
#include <cassert>
#include <numeric>
#include <set>
#include <string>

#include "common/common.h"
#include "common/logger.h"
#include "songdb/songdb.h"

using namespace std;
using namespace songdb;

namespace {

// TODO move logic to preprocessing

constexpr string_view UNKNOWN = "Unknown";
const set<string_view> pseudonyms ({
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

bool parse_tsv_row(const char *tuple, UnExoticaData &item) {
    string_view author, album;
    const auto cols = common::split_view_x<3>(tuple, '\t');
    const auto path = cols[0];
    const auto publisher = cols[1];
    int year = 0;
    if (cols.size() > 2 && cols[2].size()) {
        year = common::from_chars<int>(cols[2]);
    }

    const auto tokens = common::split_view<2>(path, '/');
    const int count = tokens.size();

    if (count < 2) {
        WARN("Skipping tuple: %s\n", tuple);
        return false;
    }

    author = tokens[0];
    album = tokens[1];

    auto author_tokens = common::split_view(author, '_');
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

    if (author == UNKNOWN) {
        item.author = UNKNOWN_AUTHOR;
    } else {
        if (author_tokens.size() == 1) {
            item.author = author_tokens[0];
        } else {
            common::mkString(author_tokens, " ", item.author);
        }
    }

    item.album = album;

    replace(item.album.begin(), item.album.end(), '_', ' ');

    item.publisher = publisher;
    item.year = year;

    return true;
}

string author_path(const string &author) {
    auto tokens = common::split_view(author, ' ');
    const auto candidate = common::mkString(tokens, "_");
    if (tokens.size() < 2 || pseudonyms.contains(candidate) || tokens[0] == "The") {
        return candidate;
    }
    if (tokens[1] == "Da" || tokens[1] == "de" || tokens[1] == "van" || tokens[1] == "Pieket") {
        const auto first = tokens.front();
        tokens.erase(tokens.begin());
        tokens.push_back(first);
    } else {
        const auto last = tokens.back();
        tokens.erase(tokens.end() - 1);
        tokens.insert(tokens.begin(), last);
    }
    return common::mkString(tokens, "_");
}

} // namespace songdb::unexotica