// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

#include "common/logger.h"
#include "common/strings.h"
#include "songdb/internal.h"
#include "songdb/songdb.h"

using namespace std;
using namespace songdb;

namespace songdb::internal {

bool parse_tsv_row(const char *tuple, _FullData &item, const _FullData &prev_item, vector<string> &authors, vector<string> &albums, vector<string> &publishers) noexcept {
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

    return true;
}

} // namespace songdb::internal
