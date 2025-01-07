// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <algorithm>
#include <cassert>
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

namespace songdb::modland {

bool parse_tsv_row(const char *tuple, _ModlandData &item, const _ModlandData &prev_item, vector<string> &authors, vector<string> &albums) noexcept {
    const auto cols = common::split_view_x<2>(tuple, '\t');

    const auto add_author = [&authors, &item](const string_view &author) {
        item.author = authors.size();
        authors.push_back(string(author));
    };

    const auto add_album = [&albums, &item](const string_view &album) {
        item.album = albums.size();
        albums.push_back(string(album));
    };

    assert(!cols[0].empty() || !cols[1].empty());

    if (cols[0].empty()) {
        item.author = UNKNOWN_AUTHOR_T;
    } else if (cols[0][0] == REPEAT) {
        item.author = prev_item.author;
    } else {
        auto author_tokens = common::split_view(cols[0], SEPARATOR);
        if (author_tokens.size() == 1) {
            add_author(author_tokens[0]);
        } else {
            string author;
            common::mkString(author_tokens, AUTHOR_JOIN, author);
            add_author(author);
        }
    }

    if (cols[1].empty()) {
        item.album = STRING_NOT_FOUND;
    } else if (cols[1][0] == REPEAT) {
        item.album = prev_item.album;
    } else {
        add_album(cols[1]);
    }

    return true;
}

} // namespace songdb::modland