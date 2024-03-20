// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

#include "common/common.h"
#include "common/logger.h"
#include "songdb/internal.h"
#include "songdb/songdb.h"

using namespace std;
using namespace songdb;
using namespace songdb::internal;

namespace songdb::demozoo {

bool parse_tsv_row(const char *tuple, _DemozooData &item, vector<string> &strings, const _DemozooData &prev_item,
                   string_view &prev_author, string_view &prev_publisher, string_view &prev_album) {
    const auto cols = common::split_view_x<4>(tuple, '\t');
    const auto authors = common::split_view(cols[0], ',');
    const auto publishers = common::split_view(cols[1], ',');
    const auto album = cols[2];
    const auto date = cols[3];

    if (date.length() >= 4) {
        int year = common::from_chars<uint16_t>(date.substr(0,4));
        item.year = year != 0 ? year - 1900u : 0;
    }
    string_t sc = strings.size();
    if (cols[0] == prev_author) {
        item.author = prev_item.author;
    } else if (authors.size() == 1) {
        const auto &author = authors[0];
        if (author.empty() || author == "?") {
            item.author = UNKNOWN_AUTHOR_T;
        } else {
            item.author = sc++;
            strings.push_back(string(author));
            prev_author = strings.back();
        }
    } else if (authors.size() > 1) {
        string author;
        common::mkString(authors, AUTHOR_JOIN, author);
        item.author = sc++;
        strings.push_back(author);
        prev_author = strings.back();
    } else {
        item.author = UNKNOWN_AUTHOR_T;
    }

    if (cols[1] == prev_publisher) {
        item.publisher = prev_item.publisher;
    } else if (publishers.size() == 1) {
        item.publisher = sc++;
        strings.push_back(string(publishers[0]));
        prev_publisher = strings.back();
    } else if (publishers.size() > 1) {
        string publisher;
        common::mkString(publishers, AUTHOR_JOIN, publisher);
        item.publisher = sc++;
        strings.push_back(publisher);
        prev_publisher = strings.back();
    } else {
        item.publisher = STRING_NOT_FOUND;
    }

    if (album.empty()) {
        item.album = STRING_NOT_FOUND;
    } else if (album == prev_album) {
        item.album = prev_item.album;
    } else {
        item.album = sc++;
        strings.push_back(string(album));
        prev_album = strings.back();
    }

    return true;
}

} // namespace songdb::demozoo