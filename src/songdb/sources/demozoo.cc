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

bool parse_tsv_row(const char *tuple, songdb::DemozooData &item) {
    const auto cols = common::split_view(tuple, '\t');
    assert(cols.size() >= 4);

    const auto authors = common::split_view(cols[0], ',');
    const auto publishers = common::split_view(cols[1], ',');
    const auto album = cols[2];
    const auto date = cols[3];

    if (date.length() >= 4) {
        item.year = common::from_chars<uint16_t>(date.substr(0,4));
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
    if (publishers.size()) {
        item.publisher = common::mkString(publishers, " & ");
    }
    item.album = album;

    return true;
}

} // namespace songdb::demozoo