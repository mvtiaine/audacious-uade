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

bool parse_tsv_row(const vector<string> &cols, AMPData &item) {
    assert(cols.size() >= 2);

    const string path = cols[1];
    vector<string> authors = cols.size() > 2 && !cols[2].empty() ? common::split(cols[2], ",") : vector<string>();
    sort(authors.begin(), authors.end());
    
    vector<string> tokens = common::split(path, "/");
    const int count = tokens.size();

    if (count < 1) {
        WARN("Skipping path: %s\n", path.c_str());
        return false;
    }

    string author = tokens[0];

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