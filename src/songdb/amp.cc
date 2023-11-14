// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

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

bool parse_path(const string &path, AMPData &item) {
    string author, filename;
    
    vector<string> tokens = common::split(path, "/");
    const int count = tokens.size();

    if (count < 3) {
        WARN("Skipping path: %s\n", path.c_str());
        return false;
    }

    author = tokens[1];
    filename = tokens[2];

    item.path = path;
    if (author == UNKNOWN_COMPOSERS) {
        item.author = UNKNOWN_AUTHOR;
    } else {
        item.author = author;
    }
    item.filename = filename;

    return true;
}

} // namespace songdb::amp