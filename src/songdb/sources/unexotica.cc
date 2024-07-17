// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <algorithm>
#include <cassert>
#include <numeric>
#include <set>
#include <string>

#include "common/logger.h"
#include "common/strings.h"
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

bool parse_tsv_row(const char *tuple, _UnExoticaData &item, const _UnExoticaData &prev_item, vector<string> &authors, vector<string> &albums, vector<string> &publishers) noexcept {
    const auto cols = common::split_view_x<4>(tuple, '\t');
    const auto author = cols[0];
    const auto publisher = cols[1];
    const auto album = cols[2];

    if (cols[3].empty()) {
        item.year = 0;
    } else if (cols[3].at(0) == 0x7f) {
        item.year = prev_item.year;
    } else {
        int year = common::from_chars<int>(cols[3]);
        item.year = year != 0 ? year - 1900u : 0;
    }

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

    if (author == UNKNOWN) {
        item.author = UNKNOWN_AUTHOR_T;
    } else if (author.at(0) == 0x7f) {
        item.author = prev_item.author;
    } else {
        string author_;
        if (author_tokens.size() == 1) {
            author_ = string(author_tokens[0]);
        } else {
            common::mkString(author_tokens, " ", author_);
        }
        add_author(author_);
    }

    if (album.empty()) {
        item.album = STRING_NOT_FOUND;
    } else if (album.at(0) == 0x7f) {
        item.album = prev_item.album;
    } else {
        string _album = string(album);
        replace(_album.begin(), _album.end(), '_', ' ');
        add_album(_album);
    }

    if (publisher.empty()) {
        item.publisher = STRING_NOT_FOUND;
    } else if (publisher.at(0) == 0x7f) {
        item.publisher = prev_item.publisher;
    } else {
        add_publisher(publisher);
    }

    return true;
}

string author_path(const string &author) {
    auto tokens = common::split_view(author, ' ');
    const auto candidate = common::mkString(tokens, "_");
    if (tokens.size() < 2 || pseudonyms.count(candidate) || tokens[0] == "The") {
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