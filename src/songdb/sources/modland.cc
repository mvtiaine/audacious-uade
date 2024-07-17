// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <algorithm>
#include <cassert>
#include <map>
#include <set>
#include <string>

#include "common/logger.h"
#include "common/strings.h"
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

bool parse_tsv_row(const char *tuple, _ModlandData &item, const _ModlandData &prev_item, vector<string> &authors, vector<string> &albums) noexcept {
    const auto tokens = common::split_view_x(tuple, '/');
    assert(tokens.size() >= 1);

    const auto add_author = [&authors, &item](const string_view &author) {
        item.author = authors.size();
        authors.push_back(string(author));
    };

    const auto add_album = [&albums, &item](const string_view &album) {
        item.album = albums.size();
        albums.push_back(string(album));
    };

    switch (tokens.size()) {
        case 1:
        case 4: {
            const auto author = tokens[0];
            if (author == UNKNOWN) {
                item.author = UNKNOWN_AUTHOR_T;
            } else if (author.at(0) == 0x7f) {
                item.author = prev_item.author;
            } else {
                add_author(author);
            }
            item.album = STRING_NOT_FOUND;
            break;
        }
        case 2: {
            const auto author = tokens[0];
            const auto token1 = tokens[1];
            if (common::starts_with(token1, COOP)) {
                vector<string_view> authors = {author, token1.substr(COOP.length())};
                sort(authors.begin(), authors.end());
                string author;
                common::mkString(authors, AUTHOR_JOIN, author);
                add_author(author);
            } else {
                if (author == UNKNOWN) {
                    item.author = UNKNOWN_AUTHOR_T;
                } else if (author.at(0) == 0x7f) {
                    item.author = prev_item.author;
                } else {
                    add_author(author);
                }
                if (!common::starts_with(token1, NOTBY)) {
                    if (token1.at(0) == 0x7f) {
                        item.album = prev_item.album;
                    } else {
                        add_album(token1);
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
            if (common::starts_with(token1, COOP)) {
                vector<string_view> authors = {author, token1.substr(COOP.length())};
                sort(authors.begin(), authors.end());
                string author;
                common::mkString(authors, AUTHOR_JOIN, author);
                add_author(author);
                if (tokens[2].at(0) == 0x7f) {
                    item.album = prev_item.album;
                } else {
                    add_album(tokens[2]);
                }
            } else {
                if (author == UNKNOWN) {
                    item.author = UNKNOWN_AUTHOR_T;
                    item.album = STRING_NOT_FOUND;
                    if (token1 == UNNAMED)
                        break;
                } else if (author.at(0) == 0x7f) {
                    item.author = prev_item.author;
                } else {
                    add_author(author);
                }
                const string album = string(token1) + " (" + string(tokens[2]) + ")";
                if (album.at(0) == 0x7f) {
                    item.album = prev_item.album;
                } else {
                    add_album(album);
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