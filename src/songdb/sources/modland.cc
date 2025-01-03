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
constexpr_v string_view COOP1 = "coop-";
constexpr_v string_view COOP2 = "coop - ";
constexpr_v string_view COOP3 = "coop ";
constexpr_v string_view UNKNOWN = "- unknown";
constexpr_v string_view NOTBY = "not by ";
constexpr_v string_view UNNAMED = "unnamed";
} // namespace

namespace songdb::modland {

bool parse_tsv_row(const char *tuple, _ModlandData &item, const _ModlandData &prev_item, vector<string> &authors, vector<string> &albums) noexcept {
    const auto tokens = common::split_view_x(tuple, '/');
    assert(tokens.size() >= 1);

    const auto add_author = [&authors, &item](const string_view &author) {
        item.author = authors.size();
        authors.push_back(string(author));
    };

    const auto add_author_coop = [&add_author](const string_view &author1, const string_view &author2) {
        vector<string_view> coop = {author1, author2};
        sort(coop.begin(), coop.end());
        string author;
        common::mkString(coop, AUTHOR_JOIN, author);
        add_author(author);
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
            } else if (author[0] == 0x7f) {
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
            if (common::starts_with(token1, COOP1)) {
                add_author_coop(author, token1.substr(COOP1.length()));
            } else if (common::starts_with(token1, COOP2)) {
                add_author_coop(author, token1.substr(COOP2.length()));
            } else if (common::starts_with(token1, COOP3)) {
                add_author_coop(author, token1.substr(COOP3.length()));
            } else {
                if (author == UNKNOWN) {
                    item.author = UNKNOWN_AUTHOR_T;
                } else if (author[0] == 0x7f) {
                    item.author = prev_item.author;
                } else {
                    add_author(author);
                }
                if (!common::starts_with(token1, NOTBY)) {
                    if (token1[0] == 0x7f) {
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
            const auto add_album_ = [&item, &prev_item, &add_album](const string_view &album) {
                if (album[0] == 0x7f) {
                    item.album = prev_item.album;
                } else {
                    add_album(album);
                }
            };
            if (common::starts_with(token1, COOP1)) {
                add_author_coop(author, token1.substr(COOP1.length()));
                add_album_(tokens[2]);
            } else if (common::starts_with(token1, COOP2)) {
                add_author_coop(author, token1.substr(COOP2.length()));
                add_album_(tokens[2]);
            } else if (common::starts_with(token1, COOP3)) {
                add_author_coop(author, token1.substr(COOP3.length()));
                add_album_(tokens[2]);
            } else {
                if (author == UNKNOWN) {
                    item.author = UNKNOWN_AUTHOR_T;
                    item.album = STRING_NOT_FOUND;
                    if (token1 == UNNAMED)
                        break;
                } else if (author[0] == 0x7f) {
                    item.author = prev_item.author;
                } else {
                    add_author(author);
                }
                const string album = string(token1) + " (" + string(tokens[2]) + ")";
                add_album_(album);
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