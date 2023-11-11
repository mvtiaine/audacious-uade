// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <string>
#include <vector>

namespace common {

inline std::vector<std::string> split(const std::string &str, const std::string &delimiter) {
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string s = str;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        if (pos != 0) {
            tokens.push_back(s.substr(0, pos));
        }
        s.erase(0, pos + delimiter.length());
    }
    tokens.push_back(s); // add last part

    return tokens;
}

} // namespace common