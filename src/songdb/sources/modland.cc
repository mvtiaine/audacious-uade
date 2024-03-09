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

bool parse_tsv_row(const char *tuple, ModlandData &item) {
    const auto tokens = common::split_view_x(tuple, '/');
    assert(tokens.size() >= 1);

    // TODO move logic to preprocessing
    switch (tokens.size()) {
        case 1:
            item.author = tokens[0];
            break;
        case 2: {
            item.author = tokens[0];
            const auto token = tokens[1];
            if (token.starts_with(COOP)) {
                vector<string_view> authors = {item.author, token.substr(COOP.length())};
                sort(authors.begin(), authors.end());
                common::mkString(authors, AUTHOR_JOIN, item.author);
            } else if (!token.starts_with(NOTBY)) {
                item.album = token;
            }
            break;
        }
        case 3: {
            const auto token = tokens[1];
            if (token.starts_with(COOP)) {
                vector<string_view> authors = {item.author, token.substr(COOP.length())};
                sort(authors.begin(), authors.end());
                common::mkString(authors, AUTHOR_JOIN, item.author);
                item.album = tokens[2];
                break;
            } else if (tokens[2] == UNUSED) {
                item.author = tokens[0];
                item.album = tokens[1];
                break;
            }
            item.author = tokens[0];
            item.album = string(tokens[1]) + " (" + string(tokens[2]) + ")";
            break;
        }
        case 4:
            item.author = tokens[0];
            if (item.author == UNKNOWN) {
                break;
            } else {
                WARN("Skipping tuple %s\n", tuple);
                return false;
            }
        default:
            WARN("Skipping tuple %s\n", tuple);
            return false;
    }

    if (item.author == UNKNOWN) {
        item.author = UNKNOWN_AUTHOR;
    }

    return true;
}

} // namespace songdb::modland