// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2023-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include "common/std/optional.h"
#include "common/std/string_view.h"

#include <cassert>
#include <cstddef>
#include <functional>
#include <set>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "common/compat.h"
#include "common/constexpr.h"
#include "common/foreach.h"
#include "common/endian.h"
#include "common/songend.h"
#include "config.h"

namespace player {

constexpr int MAGIC_SIZE = 4;

constexpr int PRECALC_FREQ = 8062;
constexpr int PRECALC_TIMEOUT = 3600;

constexpr int SILENCE_TIMEOUT = 30;
constexpr int MAX_SILENCE = 3000;

// uade has heaviest detection, but must be before ft2play for MOD detection
#define PLAYERS VA_LIST( \
    hivelytracker, \
    libdigibooster3, \
    noisetrekker2, \
    protrekkr1, \
    protrekkr2, \
    st23play, \
    st3play, \
    it2play, \
    uade, \
    ft2play, \
    libopenmpt, \
    libxmp \
)

#define STRINGIFY(x) #x,
constexpr char const *player_names[] = {
    "NONE",
    FOREACH(STRINGIFY, PLAYERS)
};

enum class Player {
    NONE,
    PLAYERS
};

constexpr std::string_view name(Player player) noexcept {
    return player_names[static_cast<int>(player)];
}

constexpr_f Player player(const std::string_view &name) noexcept {
    for (size_t i = 0; i < std::size(player_names); ++i) {
        if (name == player_names[i]) {
            return static_cast<Player>(i);
        }
    }
    return Player::NONE;
}

struct MetaData {
    std::string author;
    std::string album;
    std::string publisher;
    uint16_t year;
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
    Player tag = Player::NONE; // internal
    Player player = Player::NONE;
    // frequency and known_timeout are ignored if probing
    int frequency;
    int known_timeout = 0;
    std::endian endian = std::endian::native;
    bool probe = false;

    constexpr_f1 PlayerConfig() noexcept {}
    constexpr_f1 PlayerConfig(const Player player, const int frequency) noexcept
    : player(player), frequency(frequency) {}
    constexpr_f1 PlayerConfig(const Player player, const int frequency, const int known_timeout) noexcept
    : player(player), frequency(frequency), known_timeout(known_timeout) {}
    constexpr_f1 PlayerConfig(const Player player, const int frequency, const int known_timeout, const std::endian endian, const bool probe) noexcept
    : player(player), frequency(frequency), known_timeout(known_timeout), endian(endian), probe(probe) {}
};

struct PlayerState {
    Player player;
    int subsong;
    int frequency;
    bool swap_endian;
    void *context;
    bool stereo;
    size_t buffer_size;
    int pos_millis = 0;
};

enum class Filter {
    NONE,
    A500,
    A1200
};

void init() noexcept;
void shutdown() noexcept;

std::vector<Player> check(const char *path, const char *buf, size_t size, bool check_all = true) noexcept;
std::optional<ModuleInfo> parse(const char *path, const char *buf, size_t size, Player player = Player::NONE) noexcept;
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

struct PlaybackStepResult {
    common::SongEnd songend;
    std::vector<char> buffer;
};

PlaybackStepResult playback_step(
    PlayerState &state,
    int known_timeout = 0,
    int silence_timeout = 0) noexcept;

} // namespace player::support

namespace player::uade {

constexpr const char* UNKNOWN_CODEC = "UADE";

enum class Resampler {
    NONE,
    DEFAULT,
    SINC
};

// TODO set default audacious config based on this
struct UADEConfig : PlayerConfig {
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

    constexpr_f1 UADEConfig() noexcept { player = tag = Player::uade; }
    constexpr_f1 UADEConfig(const int frequency) noexcept
    : PlayerConfig(Player::uade, frequency) { tag = Player::uade; }
    constexpr_f1 UADEConfig(const int frequency, const int known_timeout, const std::endian endian, const bool probe) noexcept
    : PlayerConfig(Player::uade, frequency, known_timeout, endian, probe) { tag = Player::uade; }
    constexpr_f1 UADEConfig(const PlayerConfig &config) noexcept
    : PlayerConfig(Player::uade, config.frequency, config.known_timeout, config.endian, config.probe) { tag = Player::uade; }
};

bool seek(PlayerState &state, int millis) noexcept;

} // namespace player::uade

namespace player::it2play {

// note that HQ driver "better tempo (BPM) precision" results in slightly different song lengths than others
enum class Driver {
    HQ = 0,
    SB16MMX = 1,
    SB16 = 2,
    WAVWRITER = 3
};

// TODO set default audacious config based on this
struct IT2PlayConfig : PlayerConfig {
    Driver driver = Driver::HQ;

    constexpr_f1 IT2PlayConfig() noexcept { player = tag = Player::it2play; }
    constexpr_f1 IT2PlayConfig(const int frequency) noexcept
    : PlayerConfig(Player::it2play, frequency) { tag = Player::it2play; }
    constexpr_f1 IT2PlayConfig(const int frequency, const int known_timeout, const std::endian endian, const bool probe) noexcept
    : PlayerConfig(Player::it2play, frequency, known_timeout, endian, probe) { tag = Player::it2play; }
    constexpr_f1 IT2PlayConfig(const PlayerConfig &config) noexcept
    : PlayerConfig(Player::it2play, config.frequency, config.known_timeout, config.endian, config.probe) { tag = Player::it2play; }
};

} // namespace player::it2play

namespace player::libopenmpt {

struct LibOpenMPTConfig : PlayerConfig {
    Filter filter = Filter::A1200;
    float panning = 0.7;
    LibOpenMPTConfig() noexcept { player = tag = Player::libopenmpt; }
    LibOpenMPTConfig(const int frequency) noexcept
    : PlayerConfig(Player::libopenmpt, frequency) { tag = Player::libopenmpt; }
    LibOpenMPTConfig(const int frequency, const int known_timeout, const std::endian endian, const bool probe) noexcept
    : PlayerConfig(Player::libopenmpt, frequency, known_timeout, endian, probe) { tag = Player::libopenmpt; }
    LibOpenMPTConfig(const PlayerConfig &config) noexcept
    : PlayerConfig(Player::libopenmpt, config.frequency, config.known_timeout, config.endian, config.probe) { tag = Player::libopenmpt; }
};

} // namespace player::libopenmpt

namespace player::libxmp {

struct LibXMPConfig : PlayerConfig {
    Filter filter = Filter::NONE;
    float panning = 0.7;
    LibXMPConfig() noexcept { player = tag = Player::libxmp; }
    LibXMPConfig(const int frequency) noexcept
    : PlayerConfig(Player::libxmp, frequency) { tag = Player::libxmp; }
    LibXMPConfig(const int frequency, const int known_timeout, const std::endian endian, const bool probe) noexcept
    : PlayerConfig(Player::libxmp, frequency, known_timeout, endian, probe) { tag = Player::libxmp; }
    LibXMPConfig(const PlayerConfig &config) noexcept
    : PlayerConfig(Player::libxmp, config.frequency, config.known_timeout, config.endian, config.probe) { tag = Player::libxmp; }
};

} // namespace player::libxmp
