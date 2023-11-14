// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include "common/common.h"
#include "common/logger.h"
#include "player/player.h"
#include "songend/detector.h"
#include "songend/precalc.h"

using namespace player;
using namespace songend;
using namespace songend::detector;
using namespace std;

namespace {

void apply_detector(SongEndDetector &detector, SongEnd &songend) {
    int silence = detector.detect_silence(SILENCE_TIMEOUT);
    if (silence > 0) {
        if (songend.length == silence) {
            songend.length = 0;
            songend.status = SongEnd::NOSOUND;
        } else {
            songend.length = silence + MAX_SILENCE;
            songend.status = SongEnd::DETECT_SILENCE;
        }
    } else {
        int volume = detector.detect_volume(SILENCE_TIMEOUT);
        if (volume > 0) {
            songend.length = volume + MAX_SILENCE;
            songend.status = SongEnd::DETECT_VOLUME;
        } else {
            int repeat = detector.detect_repeat();
            if (repeat > 0) {
                songend.length = repeat;
                songend.status = SongEnd::DETECT_REPEAT;
            } else {
                int loop = detector.detect_loop();
                if (loop > 0) {
                    songend.length = loop;
                    songend.status = SongEnd::DETECT_LOOP;
                    int silence = detector.trim_silence(songend.length);
                    if (silence > MAX_SILENCE) {
                        songend.length = songend.length - silence + MAX_SILENCE;
                        songend.status = SongEnd::LOOP_PLUS_SILENCE;
                    } else {
                        int volume = detector.trim_volume(songend.length);
                        if (volume > MAX_SILENCE && volume < songend.length) {
                            songend.length = songend.length - volume + MAX_SILENCE;
                            songend.status = SongEnd::LOOP_PLUS_VOLUME;
                        }
                    }
                } else {
                    songend.length = PRECALC_TIMEOUT * 1000;
                    songend.status = SongEnd::TIMEOUT;
                }
            }
        }
    }
}

void apply_trimmer(SongEndDetector &detector, SongEnd &songend) {
    if (songend.status == SongEnd::PLAYER) {
        int silence = detector.trim_silence(songend.length);
        if (silence == songend.length) {
            songend.status = SongEnd::NOSOUND;
            songend.length = 0;
        } else if (silence > MAX_SILENCE) {
            songend.length = songend.length - silence + MAX_SILENCE;
            songend.status = SongEnd::PLAYER_PLUS_SILENCE;
        } else {
            int volume = detector.trim_volume(songend.length);
            if (volume > MAX_SILENCE && volume < songend.length) {
                songend.length = songend.length - volume + MAX_SILENCE;
                songend.status = SongEnd::PLAYER_PLUS_VOLUME;
            }
        }
    } else if (songend.status == SongEnd::DETECT_SILENCE) {
        int silence = detector.trim_silence(songend.length);
        if (silence == songend.length) {
            songend.status = SongEnd::NOSOUND;
            songend.length = 0;
        } else {
            songend.length = songend.length - SILENCE_TIMEOUT * 1000 + MAX_SILENCE;
        }
    } else if (songend.status == SongEnd::TIMEOUT) {
        songend.length = PRECALC_TIMEOUT * 1000;
    }
}

bool is_octamed(const ModuleInfo &info) {
    return info.format.starts_with("MMD0") ||
           info.format.starts_with("MMD1") ||
           info.format.starts_with("MMD2");
}

bool is_vss(const ModuleInfo &info) {
    return info.format.starts_with("VSS");
}

} // namespace {}

namespace songend::precalc {

bool allow_songend_error(const ModuleInfo &info) {
    return info.player == Player::uade && (
        is_octamed(info) || is_vss(info));
}


SongEnd precalc_song_end(const ModuleInfo &info, const char *buf, size_t size, int subsong, const string &md5hex) {
    const auto check_stop = []() { return false; };
    const auto check_seek = []() { return -1; };
    int frequency = info.player == player::Player::uade ? PRECALC_FREQ_UADE : PRECALC_FREQ;
    SongEndDetector detector(frequency, info.player != Player::uade, endian::native);
    const auto write_audio = [&detector](char *mixbuf, int size) {
         detector.update(mixbuf, size);
    };

    const player::PlayerConfig player_config = { frequency, 0, std::endian::native, true };
    player::uade::UADEConfig uade_config = {{ frequency, 0, std::endian::native, true }};
    uade_config.silence_timeout = SILENCE_TIMEOUT;
    const auto &config = info.player == player::Player::uade ? uade_config : player_config;

    auto state = player::play(info.path.c_str(), buf, size, subsong, config);
    if (!state) {
        WARN("Could not play %s subsong %d md5 %s\n", info.path.c_str(), subsong, md5hex.c_str());
        return {};
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

    if (res.songend.status != SongEnd::ERROR || allow_songend_error(info)) {
        apply_trimmer(detector, res.songend);
        TRACE("precalc_song_length %s - status: %d length: %d\n", info.path.c_str(), res.songend.status, res.songend.length);
    } else {
        ERR("Error precalcing %s\n", info.path.c_str());
    }

    return res.songend;
}

} // namespace songend::precalc