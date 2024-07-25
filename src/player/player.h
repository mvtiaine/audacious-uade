// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include "common/std/optional.h"

#include <cassert>
#include <cstddef>
#include <functional>
#include <string>
#include <type_traits>
#include <utility>

#include "common/compat.h"
#include "common/constexpr.h"
#include "common/endian.h"
#include "common/songend.h"

namespace player {

constexpr int MAGIC_SIZE = 4;

constexpr int PRECALC_FREQ = 8062;
constexpr int PRECALC_TIMEOUT = 3600;

constexpr int SILENCE_TIMEOUT = 30;
constexpr int MAX_SILENCE = 3000;

// TODO make externally configurable
// uade has heaviest detection, but must be before ft2play for MOD detection
#define PLAYERS VA_LIST( \
    hivelytracker, \
    libdigibooster3, \
    uade, \
    ft2play \
 )

enum class Player {
    NONE,
    PLAYERS
};

_CONSTEXPR_F2 std::string name(Player player) noexcept {
    switch(player) {
        case Player::hivelytracker: return "hivelytracker";
        case Player::libdigibooster3: return "libdigibooster3";
        case Player::uade: return "uade";
        case Player::ft2play: return "ft2play";
        default: assert(false); return "";
    }
    assert(false);
    return "";
}

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
    
    _CONSTEXPR_F1 PlayerConfig() noexcept {}
    _CONSTEXPR_F1 PlayerConfig(const int frequency) noexcept : frequency(frequency) {}
    _CONSTEXPR_F1 PlayerConfig(const int frequency, const int known_timeout) noexcept
    : frequency(frequency), known_timeout(known_timeout) {}
    _CONSTEXPR_F1 PlayerConfig(const int frequency, const int known_timeout, const std::endian endian, const bool probe) noexcept
    : frequency(frequency), known_timeout(known_timeout), endian(endian), probe(probe) {}
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

void init() noexcept;
void shutdown() noexcept;

Player check(const char *path, const char *buf, size_t size) noexcept;
std::optional<ModuleInfo> parse(const char *path, const char *buf, size_t size) noexcept;
std::optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &config) noexcept;
std::pair<common::SongEnd::Status, size_t> render(PlayerState &state, char *buf, size_t size) noexcept;
bool stop(PlayerState &state) noexcept;
bool seek(PlayerState &state, int millis) noexcept;

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
    const std::function<void(char *, int)> write_audio) noexcept;

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

    _CONSTEXPR_F1 UADEConfig() noexcept {}
    _CONSTEXPR_F1 UADEConfig(const int frequency) noexcept
    : PlayerConfig(frequency) {}
    _CONSTEXPR_F1 UADEConfig(const int frequency, const int known_timeout, const std::endian endian, const bool probe) noexcept
    : PlayerConfig(frequency, known_timeout, endian, probe) {}
};

bool seek(PlayerState &state, int millis) noexcept;

} // namespace player::uade
