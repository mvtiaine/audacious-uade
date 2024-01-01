// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

#include "common/common.h"
#include "common/logger.h"
#include "songdb/songdb.h"

using namespace std;
using namespace songdb;

namespace songdb::demozoo {

bool parse_tsv_row(const std::vector<std::string> &cols, songdb::DemozooData &item) {
    assert(cols.size() >= 5);

    const string date = cols[1];
    vector<string> authors = common::split(cols[2], ",");
    sort(authors.begin(), authors.end());
    vector<string> publishers = common::split(cols[3], ",");
    sort(publishers.begin(), publishers.end());
    const string album = cols[4];

    if (date.length() >= 4) {
        item.year = stoi(date.substr(0, 4));
    }
    if (authors.size()) {
        const auto author = common::mkString(authors, " & ");
        if (author == "?") {
            item.author = UNKNOWN_AUTHOR;
        } else {
            item.author = author;
        }
    } else {
        item.author = UNKNOWN_AUTHOR;
    }
    if(publishers.size()) {
        item.publisher = common::mkString(publishers, " & ");
    }
    item.album = album;

    return true;
}

} // namespace songdb::demozoo