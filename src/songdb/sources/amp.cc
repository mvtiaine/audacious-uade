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

namespace {

constexpr string_view UNKNOWN_COMPOSERS = "UnknownComposers";

} // namespace {}

namespace songdb::amp {

bool parse_tsv_row(const char *tuple, AMPData &item) {
    const auto cols = common::split_view(tuple, '\t');
    assert(cols.size() > 0);
    const auto author = cols[0];
    const auto authors = cols.size() > 1 && !cols[1].empty() ? common::split_view(cols[1], ',') : vector<string_view>();
    
    if (author == UNKNOWN_COMPOSERS) {
        item.author = UNKNOWN_AUTHOR;
    } else {
        if (authors.size()) {
            item.author = common::mkString(authors, " & ");
        } else {
            item.author = author;
        }
    }

    return true;
}

} // namespace songdb::amp