// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#include "common/logger.h"
#include "common/songend.h"
#include "player/player.h"
#include "songend/detector.h"
#include "songend/precalc.h"

using namespace common;
using namespace player;
using namespace songend;
using namespace songend::detector;
using namespace std;

namespace {

constexpr_f2 void apply_detector(SongEndDetector &detector, SongEnd &songend) noexcept {
    uint32_t silence = detector.detect_silence(SILENCE_TIMEOUT);
    if (silence > 0) {
        if (songend.length <= silence) {
            songend.length = 0;
            songend.status = SongEnd::NOSOUND;
        } else if (songend.length > silence + MAX_SILENCE) {
            songend.length = silence + MAX_SILENCE;
            songend.status = SongEnd::DETECT_SILENCE;
        }
    } else {
        uint32_t volume = detector.detect_volume(SILENCE_TIMEOUT);
        if (volume > 0 && songend.length > volume + MAX_SILENCE) {
            songend.length = volume + MAX_SILENCE;
            songend.status = SongEnd::DETECT_VOLUME;
        } else {
            uint32_t repeat = detector.detect_repeat();
            if (repeat > 0) {
                assert(repeat < songend.length);
                songend.length = repeat;
                songend.status = SongEnd::DETECT_REPEAT;
            } else {
                uint32_t loop = detector.detect_loop();
                if (loop > 0) {
                    assert(loop <= songend.length / 2);
                    songend.length = loop;
                    songend.status = SongEnd::DETECT_LOOP;
                    uint32_t silence = detector.trim_silence(songend.length);
                    if (silence > MAX_SILENCE) {
                        assert(songend.length > MAX_SILENCE);
                        assert(songend.length > silence);
                        songend.length = songend.length - silence + MAX_SILENCE;
                        songend.status = SongEnd::LOOP_PLUS_SILENCE;
                    } else {
                        uint32_t volume = detector.trim_volume(songend.length);
                        if (volume > MAX_SILENCE) {
                            assert(songend.length > MAX_SILENCE);
                            assert(songend.length > volume);
                            songend.length = songend.length - volume + MAX_SILENCE;
                            songend.status = SongEnd::LOOP_PLUS_VOLUME;
                        }
                    }
                }
            }
        }
    }
}

constexpr void apply_trimmer(SongEndDetector &detector, SongEnd &songend) noexcept {
    if (songend.status == SongEnd::PLAYER) {
        uint32_t silence = detector.trim_silence(songend.length);
        if (silence >= songend.length) {
            songend.status = SongEnd::NOSOUND;
            songend.length = 0;
        } else if (songend.length > MAX_SILENCE) {
            if (silence > MAX_SILENCE) {
                assert(silence < songend.length);
                songend.length = songend.length - silence + MAX_SILENCE;
                songend.status = SongEnd::PLAYER_PLUS_SILENCE;
            } else {
                uint32_t volume = detector.trim_volume(songend.length);
                if (volume > MAX_SILENCE) {
                    assert(volume < songend.length);
                    songend.length = songend.length - volume + MAX_SILENCE;
                    songend.status = SongEnd::PLAYER_PLUS_VOLUME;
                }
            }
        }
    } else if (songend.status == SongEnd::DETECT_SILENCE) {
        uint32_t silence = detector.trim_silence(songend.length);
        if (silence >= songend.length) {
            songend.status = SongEnd::NOSOUND;
            songend.length = 0;
        } else if (songend.length > MAX_SILENCE && silence > MAX_SILENCE) {
            songend.length = songend.length - silence + MAX_SILENCE;
        }
    }
}

} // namespace {}

namespace songend::precalc {

SongEnd precalc_song_end(const ModuleInfo &info, const char *buf, size_t size, int subsong, const string &md5hex) noexcept {
    const auto check_stop = []() { return false; };
    const auto check_seek = []() { return -1; };
    const int frequency = PRECALC_FREQ;
    SongEndDetector detector(frequency, info.player != Player::uade, endian::native);
    const auto write_audio = [&detector](char *mixbuf, int size) {
         detector.update(mixbuf, size);
    };

    const player::PlayerConfig player_config = { frequency, 0, endian::native, true };
    player::uade::UADEConfig uade_config = { frequency, 0, endian::native, true };
    uade_config.silence_timeout = SILENCE_TIMEOUT;
    const auto &config = info.player == player::Player::uade ? uade_config : player_config;

    auto state = player::play(info.path.c_str(), buf, size, subsong, config);
    if (!state) {
        WARN("Could not play %s subsong %d md5 %s\n", info.path.c_str(), subsong, md5hex.c_str());
        return { SongEnd::ERROR, 0 };
    }

    auto res = support::playback_loop(state.value(), config, check_stop, check_seek, write_audio);
    assert(!res.seeked);
    assert(!res.stopped);
    if (!player::stop(state.value())) {
        WARN("Could not stop %s subsong %d md5 %s\n", info.path.c_str(), subsong, md5hex.c_str());
    }

    if (res.songend.status == SongEnd::TIMEOUT) {
        apply_detector(detector, res.songend);
    }

    if (res.songend.status != SongEnd::ERROR || allow_songend_error(info.format)) {
        apply_trimmer(detector, res.songend);
        TRACE("precalc_song_length %s - status: %d length: %d\n", info.path.c_str(), res.songend.status, res.songend.length);
    } else {
        ERR("Error precalcing %s\n", info.path.c_str());
    }

    return res.songend;
}

} // namespace songend::precalc