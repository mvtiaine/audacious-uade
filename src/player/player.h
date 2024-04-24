// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <bit>
#include <cassert>
#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

#include "common/compat.h"
#include "common/songend.h"

namespace player {

constexpr int MAGIC_SIZE = 4;

constexpr int PRECALC_FREQ = 8062;
constexpr int PRECALC_TIMEOUT = 3600;

constexpr int SILENCE_TIMEOUT = 30;
constexpr int MAX_SILENCE = 3000;

// TODO make externally configurable
// uade should be last as it has heaviest detection
#define PLAYERS \
    hivelytracker, \
    libdigibooster3, \
    uade, \
    ft2play

enum class Player {
    NONE,
    PLAYERS
};

struct ModuleInfo {
    Player player;
    std::string format;
    std::string path;
    int minsubsong;
    int maxsubsong;
    int defsubsong;
    int channels;
};

struct PlayerConfig {
    // frequency and known_timeout are ignored if probing
    int frequency;
    int known_timeout = 0;
    std::endian endian = std::endian::native;
    bool probe = false;
};

struct PlayerState {
    ModuleInfo info;
    int subsong;
    int frequency;
    bool swap_endian;
    void *context;
    bool stereo;
    size_t buffer_size;
    int pos_millis = 0;
};


void init();
void shutdown();

Player check(const char *path, const char *buf, size_t size);
std::optional<ModuleInfo> parse(const char *path, const char *buf, size_t size);
std::optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &config);
std::pair<common::SongEnd::Status, size_t> render(PlayerState &state, char *buf, size_t size);
bool stop(PlayerState &state);
bool seek(PlayerState &state, int millis);

} // namespace player

namespace player::support {

struct PlayerScope {
    PlayerScope() {
        player::init();
    }
    ~PlayerScope() {
        player::shutdown();
    }
};

struct PlaybackResult {
    common::SongEnd songend;
    bool seeked;
    bool stopped;
};

PlaybackResult playback_loop(
    PlayerState &state,
    const PlayerConfig &config,
    const std::function<bool(void)> check_stop,
    const std::function<int(void)> check_seek,
    const std::function<void(char *, int)> write_audio);

} // namespace player::support

namespace player::uade {

constexpr const char* UNKNOWN_CODEC = "UADE";

enum class Filter {
    NONE,
    A500,
    A1200
};

enum class Resampler {
    NONE,
    DEFAULT,
    SINC
};

// TODO set default audacious config based on this
struct UADEConfig : PlayerConfig {
    Player player = Player::uade;
    Filter filter = Filter::A1200;
    Resampler resampler = Resampler::SINC;
    std::optional<bool> force_led = {};
    bool headphones = false;
    bool headphones2 = false;
    float panning = 0.7;
    float gain = 1.0;
    // TODO support timeouts with other players
    int subsong_timeout = 600;
    int silence_timeout = 10;
};

bool seek(PlayerState &state, int millis);

} // namespace player::uade
