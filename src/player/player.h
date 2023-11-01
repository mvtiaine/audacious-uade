// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#ifndef PLAYER_H_
#define PLAYER_H_

#include <optional>
#include <string>

using namespace std;

namespace player {

constexpr int MIXBUFSIZE = 8192;

enum Player {
    NONE,
    //UADE,
    HIVELY,
    DIGIBOOSTERPRO
};

struct ModuleInfo {
    string format;
    string fname;
    int maxsubsong;
    int channels;
};

struct PlayerState {
    Player player;
    int subsong;
    int frequency;
    void *context;
    int pos_millis;
    bool songend;
};

constexpr int MAGIC_SIZE = 4;

void init();

bool is_our_file(const char *buf, size_t size);
optional<ModuleInfo> parse(const char *fname, const char *buf, size_t size);
optional<PlayerState> play(const char *fname, const char *buf, size_t size, int subsong, int frequency);
pair<bool,size_t> render(PlayerState &state, char *buf, size_t size);
void stop(PlayerState &state);
bool seek(PlayerState &state, int millis);
bool restart(PlayerState &state);

} // namespace player

#endif // PLAYER_H_
