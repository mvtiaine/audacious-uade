// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#include "common/std/optional.h"

#include <mutex>
#include <set>

#include "common/endian.h"
#include "common/logger.h"
#include "player/player.h"

// use C-interface to avoid exceptions
extern "C" {
#include <libopenmpt/libopenmpt.h>
}

using namespace std;
using namespace common;
using namespace player;

namespace {

constexpr size_t mixBufSize(const int frequency) noexcept {
    return 4 * (frequency / 50 + (frequency % 50 != 0 ? 1 : 0));
}
// XXX libopenmpt seems to have some thread safety issue, so limit concurrent access:
/*
Thread 17 Crashed:: pool
0   libsystem_platform.dylib      	       0x191340f10 setjmp + 36
1   libmpg123.0.dylib             	       0x1030af970 INT123_getcpuflags + 80
2   libopenmpt.0.dylib            	       0x104029de4 OpenMPT::CSoundFile::ReadMO3(OpenMPT::detail::FileReader<mpt::mpt_libopenmpt::IO::FileCursorTraitsFileData, mpt::mpt_libopenmpt::IO::FileCursorFilenameTraits<mpt::mpt_libopenmpt::BasicPathString<mpt::mpt_libopenmpt::Utf8PathTraits, false>>>&, OpenMPT::CSoundFile::ModLoadingFlags) + 10120
*/
mutex guard;
openmpt_module *create_module(const char *path, const char *buf, size_t size, bool play) noexcept  {
    const auto logfail = [&play, &path](const char * reason, int err = -1) {
        if (play) {
            ERR("player_libopenmpt::play parsing failed for %s - reason: %s (%d)\n", path, reason, err);
        } else {
            DEBUG("player_libopenmpt::parse parsing failed for %s - reason: %s (%d)\n", path, reason, err);
        }
    };
    std::string warning;
    const auto logfunc = [](const char * message, std::string *warning) {
        TRACE("libopenmpt: %s\n", message);
        if (warning->empty()) warning->append(message);
    };
    int err = 0;
    const auto errfunc = [](int error, void *) -> int {
        TRACE("libopenmpt: ERROR %d\n", error);
        return OPENMPT_ERROR_FUNC_RESULT_STORE;
    };
    guard.lock();
    auto *mod = openmpt_module_create_from_memory2(buf, size,
        (openmpt_log_func)*logfunc, &warning, (openmpt_error_func)*errfunc, nullptr, &err,
        nullptr, nullptr);
    guard.unlock();
    if (!mod || err || !warning.empty()) {
        if (err) {
            auto msg = mod ? openmpt_module_error_get_last_message(mod) : "-";
            logfail(msg, err);
            if (mod) openmpt_free_string(msg);
        } else if (!warning.empty()) {
            logfail(warning.c_str());
        } else {
            logfail("-");
        }
        if (mod) openmpt_module_destroy(mod);
        return nullptr;
    }
    return mod;
}

} // namespace {}

namespace player::libopenmpt {

void init() noexcept {}

void shutdown() noexcept {}

bool is_our_file(const char *path, const char *buf, size_t size) noexcept {
	int res = openmpt_probe_file_header_without_filesize(OPENMPT_PROBE_FILE_HEADER_FLAGS_DEFAULT, buf, size,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    return res == OPENMPT_PROBE_FILE_HEADER_RESULT_SUCCESS;
}

optional<ModuleInfo> parse(const char *path, const char *buf, size_t size) noexcept {
    auto *mod = create_module(path, buf, size, false);
    if (!mod) {
        return {};
    }
    auto format_ = openmpt_module_get_metadata(mod, "tracker");
    if (!format_ || !format_[0]) {
        if (format_) openmpt_free_string(format_);
        format_ = openmpt_module_get_metadata(mod, "type_long");
    }
    const auto channels = openmpt_module_get_num_channels(mod);
    const auto subsongs = openmpt_module_get_num_subsongs(mod);
    const string format = format_ && format_[0] ? format_ : "";
    if (format_) openmpt_free_string(format_);
    openmpt_module_destroy(mod);
    return ModuleInfo {Player::libopenmpt, format, path, 0, subsongs - 1, 0, channels};
}

optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &_config) noexcept {
    assert(subsong >= 0);
    auto *mod = create_module(path, buf, size, true);
    if (!mod) {
        return {};
    }
    openmpt_module_ctl_set_boolean(mod, "load.skip_subsongs_init", 1);
    if (!openmpt_module_select_subsong(mod, subsong)) {
        ERR("player_libopenmpt::play invalid subsong %d for %s\n", subsong, path);
        openmpt_module_destroy(mod);
        return {};
    }
    openmpt_module_ctl_set_text(mod, "play.at_end", "stop");
    const auto &__config = static_cast<const LibOpenMPTConfig&>(_config);
    const auto config = __config.player == Player::libopenmpt ? __config : LibOpenMPTConfig(_config);
    if (config.probe) {
        openmpt_module_ctl_set_integer(mod, "dither", 0);
        openmpt_module_ctl_set_boolean(mod, "render.resampler.emulate_amiga", 0);
        openmpt_module_set_render_param(mod, OPENMPT_MODULE_RENDER_STEREOSEPARATION_PERCENT, 0);
        openmpt_module_set_render_param(mod, OPENMPT_MODULE_RENDER_INTERPOLATIONFILTER_LENGTH, 1);
    } else {
        openmpt_module_ctl_set_boolean(mod, "render.resampler.emulate_amiga", 1);
        openmpt_module_ctl_set_text(mod, "render.resampler.emulate_amiga_type",
            config.filter == Filter::A500 ? "a500" :
            config.filter == Filter::A1200 ? "a1200" :
            "none");
        openmpt_module_set_render_param(mod, OPENMPT_MODULE_RENDER_STEREOSEPARATION_PERCENT, (1.0f - config.panning) * 200);
    }

    return PlayerState {Player::libopenmpt, subsong, config.frequency, config.endian != endian::native, mod, !config.probe, mixBufSize(config.frequency), 0};
}

pair<SongEnd::Status,size_t> render(PlayerState &state, char *buf, size_t size) noexcept {
    assert(state.player == Player::libopenmpt);
    assert(size >= mixBufSize(state.frequency));
    auto *mod = static_cast<openmpt_module*>(state.context);
    assert(mod);
    const auto bytes = openmpt_module_read_interleaved_stereo(mod, state.frequency, size / 4, (int16_t*)buf) * 4;
    return {bytes == 0 ? SongEnd::PLAYER : SongEnd::NONE, bytes};
}

bool stop(PlayerState &state) noexcept {
    assert(state.player == Player::libopenmpt);
    if (state.context) {
        auto *mod = static_cast<openmpt_module*>(state.context);
        assert(mod);
        openmpt_module_destroy(mod);
    }
    return true;
}

bool restart(PlayerState &state) noexcept {
    assert(state.player == Player::libopenmpt);
    auto *mod = static_cast<openmpt_module*>(state.context);
    assert(mod);
    openmpt_module_set_position_seconds(mod, 0);
    return true;
}

} // namespace player::libopenmpt
