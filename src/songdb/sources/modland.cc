// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <algorithm>
#include <cassert>
#include <map>
#include <set>
#include <string>

#include "common/common.h"
#include "common/logger.h"
#include "songdb/songdb.h"

// XXX amigaos4/clib4 issue
#ifdef UNUSED
#undef UNUSED
#endif

using namespace std;

namespace {

constexpr string_view COOP = "coop-";
constexpr string_view UNKNOWN = "- unknown";
constexpr string_view NOTBY = "not by ";
constexpr string_view UNUSED = "Unused";

} // namespace

namespace songdb::modland {

bool parse_tsv_row(const vector<string> &cols, ModlandData &item) {
    assert(cols.size() >= 2);

    string author, album;
    
    const string path = cols[1];
    vector<string> tokens = common::split(path, "/");
    const int count = tokens.size();

    if (count < 1) {
        WARN("Skipping path %s, token count %d\n", path.c_str(), count);
        return false;
    }

    // TODO move logic to preprocessing
    switch (count) {
        case 1:
            author = tokens[0];
            break;
        case 2: {
            author = tokens[0];
            string token = tokens[1];
            if (token.starts_with(COOP)) {
                vector<string> authors = {author, token.substr(COOP.length())};
                sort(authors.begin(), authors.end());
                author = common::mkString(authors, " & ");
            } else if (!token.starts_with(NOTBY)) {
                album = token;
            }
            break;
        }
        case 3: {
            string token = tokens[1];
            if (token.starts_with(COOP)) {
                vector<string> authors = {author, token.substr(COOP.length())};
                sort(authors.begin(), authors.end());
                author = common::mkString(authors, " & ");
                album = tokens[2];
                break;
            } else if (tokens[2] == UNUSED) {
                author = tokens[0];
                album = tokens[1];
                break;
            }
            author = tokens[0];
            album = tokens[1] + " (" + tokens[2] + ")";
            break;
        }
        case 4:
            author = tokens[0];
            if (author == UNKNOWN) {
                break;
            } else {
                WARN("Skipping path %s, token count %d\n", path.c_str(), count);
                return false;
            }
        default:
            WARN("Skipping path %s, token count %d\n", path.c_str(), count);
            return false;
    }

    if (author == UNKNOWN) {
        item.author = UNKNOWN_AUTHOR;
    } else {
        item.author = author;
    }
    if (album.size()) {
        item.album = album;
    }

    return true;
}

} // namespace songdb::modland