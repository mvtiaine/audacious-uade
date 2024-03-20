// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <algorithm>
#include <cassert>
#include <numeric>
#include <set>
#include <string>

#include "common/common.h"
#include "common/logger.h"
#include "songdb/internal.h"
#include "songdb/songdb.h"

using namespace std;
using namespace songdb;
using namespace songdb::internal;

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

bool parse_tsv_row(const char *tuple, _UnExoticaData &item, vector<string> &strings, const _UnExoticaData &prev_item,
                   string_view &prev_author, string_view &prev_publisher, string_view &prev_album) {
    const auto cols = common::split_view_x<3>(tuple, '\t');
    const auto path = cols[0];

    const auto tokens = common::split_view<2>(path, '/');
    const int count = tokens.size();

    if (count < 2) {
        WARN("Skipping tuple: %s\n", tuple);
        return false;
    }

    const auto publisher = cols[1];
    if (cols.size() > 2 && cols[2].size()) {
        int year = common::from_chars<int>(cols[2]);
        item.year = year != 0 ? year - 1900u : 0;
    }

    string_view author = tokens[0];
    string_view album = tokens[1];

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

    string_t sc = strings.size();
    if (author == UNKNOWN) {
        item.author = UNKNOWN_AUTHOR_T;
    } else if (author == prev_author) {
        item.author = prev_item.author;
    } else {
        string author_;
        if (author_tokens.size() == 1) {
            author_ = string(author_tokens[0]);
        } else {
            common::mkString(author_tokens, " ", author_);
        }
        item.author = sc++;
        strings.push_back(author_);
        prev_author = strings.back();
    }

    if (album.empty()) {
        item.album = STRING_NOT_FOUND;
    } else if (album == prev_album) {
        item.album = prev_item.album;
    } else {
        string _album = string(album);
        replace(_album.begin(), _album.end(), '_', ' ');
        item.album = sc++;
        strings.push_back(_album);
        prev_album = strings.back();
    }

    if (publisher.empty()) {
        item.publisher = STRING_NOT_FOUND;
    } else if (publisher == prev_publisher) {
        item.publisher = prev_item.publisher;
    } else {
        item.publisher = sc++;
        strings.push_back(string(publisher));
        prev_publisher = strings.back();
    }

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