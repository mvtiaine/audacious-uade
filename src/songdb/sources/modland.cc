// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <algorithm>
#include <cassert>
#include <map>
#include <set>
#include <string>

#include "common/common.h"
#include "common/logger.h"
#include "songdb/internal.h"
#include "songdb/songdb.h"

// XXX amigaos4/clib4 issue
#ifdef UNUSED
#undef UNUSED
#endif

using namespace std;
using namespace songdb::internal;

namespace {
// TODO move logic to preprocessing
constexpr string_view COOP = "coop-";
constexpr string_view UNKNOWN = "- unknown";
constexpr string_view NOTBY = "not by ";
constexpr string_view UNNAMED = "unnamed";
} // namespace

namespace songdb::modland {

bool parse_tsv_row(const char *tuple, _ModlandData &item, vector<string> &strings, const _ModlandData &prev_item,
                   string_view &prev_author, string_view &prev_album) {
    const auto tokens = common::split_view_x(tuple, '/');
    assert(tokens.size() >= 1);

    string_t sc = strings.size();
    switch (tokens.size()) {
        case 1:
        case 4: {
            const auto author = tokens[0];
            if (author == UNKNOWN) {
                item.author = UNKNOWN_AUTHOR_T;
            } else if (author == prev_author) {
                item.author = prev_item.author;
            } else {
                item.author = sc++;
                strings.push_back(string(author));
                prev_author = strings.back();
            }
            item.album = STRING_NOT_FOUND;
            break;
        }
        case 2: {
            const auto author = tokens[0];
            const auto token1 = tokens[1];
            if (token1.starts_with(COOP)) {
                vector<string_view> authors = {author, token1.substr(COOP.length())};
                sort(authors.begin(), authors.end());
                string author;
                common::mkString(authors, AUTHOR_JOIN, author);
                item.author = sc++;
                strings.push_back(author);
                prev_author = strings.back();
            } else {
                if (author == UNKNOWN) {
                    item.author = UNKNOWN_AUTHOR_T;
                } else if (author == prev_author) {
                    item.author = prev_item.author;
                } else {
                    item.author = sc++;
                    strings.push_back(string(author));
                    prev_author = strings.back();
                }
                if (!token1.starts_with(NOTBY)) {
                    if (token1 == prev_album) {
                        item.album = prev_item.album;
                    } else {
                        item.album = sc++;
                        strings.push_back(string(token1));
                        prev_album = strings.back();
                    }
                } else {
                    item.album = STRING_NOT_FOUND;
                }
            }
            break;
        }
        case 3: {
            const auto author = tokens[0];
            const auto token1 = tokens[1];
            if (token1.starts_with(COOP)) {
                vector<string_view> authors = {author, token1.substr(COOP.length())};
                sort(authors.begin(), authors.end());
                string author;
                common::mkString(authors, AUTHOR_JOIN, author);
                item.author = sc++;
                strings.push_back(author);
                prev_author = author;
                if (tokens[2] == prev_album) {
                    item.album = prev_item.album;
                } else {
                    item.album = sc++;
                    strings.push_back(string(tokens[2]));
                    prev_album = strings.back();
                }
            } else {
                if (author == UNKNOWN) {
                    item.author = UNKNOWN_AUTHOR_T;
                    item.album = STRING_NOT_FOUND;
                    if (token1 == UNNAMED)
                        break;
                } else if (author == prev_author) {
                    item.author = prev_item.author;
                } else {
                    item.author = sc++;
                    strings.push_back(string(author));
                    prev_author = strings.back();
                }
                const string album = string(token1) + " (" + string(tokens[2]) + ")";
                if (album == prev_album) {
                    item.album = prev_item.album;
                } else {
                    item.album = sc++;
                    strings.push_back(album);
                    prev_album = strings.back();
                }
            }
            break;
        }
        default:
            WARN("Skipping tuple %s\n", tuple);
            return false;
    }

    return true;
}

} // namespace songdb::modland