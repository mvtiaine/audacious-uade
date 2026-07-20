// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2025-2026 Matti Tiainen <mvtiaine@cc.hut.fi>

#include "common/std/optional.h"

#include "common/endian.h"
#include "common/logger.h"
#include "player/player.h"
#include "player/players/internal.h"

#include "config.h"

extern "C" {
#include <xmp.h>
}

using namespace std;
using namespace common;
using namespace player;
using namespace player::internal;

namespace {

constexpr size_t mixBufSize(const int frequency) noexcept {
    return 4 * (frequency / 50 + (frequency % 50 != 0 ? 1 : 0));
}

struct XMPContext {
    xmp_context context;
    xmp_module_info info;
};

} // namespace {}

namespace player::libxmp {

void init() noexcept {}

void shutdown() noexcept {}

bool is_our_file(const char *path, const char *buf, size_t bufsize, size_t filesize) noexcept {
    // XXX ASMA .sap files get detected as SoundFX 1.3
    if (ends_with(path, ".sap") && bufsize >= 3 && buf[0] == 'S' && buf[1] == 'A' && buf[2] == 'P') {
        return false;
    }
#if PLAYER_libopenmpt
    // do not accept it or xm when libopenmpt is available as xmp does not detect missing openmpt plugins/extensions
    if (is_xm(path, buf, bufsize) || is_it(path, buf, bufsize))
        return false;
#endif
	return xmp_test_module_from_memory(buf, bufsize, nullptr) == 0;
}

optional<ModuleInfo> parse(const char *path, const char *buf, size_t size) noexcept {
    xmp_context c = xmp_create_context();
    if (!c) {
        ERR("player_libxmp::parse xmp_create_context failed for %s\n", path);
        return {};
    }
    // XXX must use path based loading to support StarTrekker/AudioSculpture (and external sample loading)
    int ret = xmp_load_module(c, path);
    if (ret) {
        DEBUG("player_libxmp::parse xmp_load_module failed for %s (%d)\n", path, ret);
        xmp_free_context(c);
        return {};
    }
    xmp_module_info info;
    xmp_get_module_info(c, &info);
    const auto channels = info.mod->chn;
    const auto subsongs = info.num_sequences;
    const auto format = string(info.mod->type);
    xmp_release_module(c);
    xmp_free_context(c);
    return ModuleInfo {Player::libxmp, format, path, 0, subsongs - 1, 0, channels};
}

optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &_config) noexcept {
    assert(_config.player == Player::libxmp || _config.player == Player::NONE);
    assert(_config.tag == Player::libxmp || _config.tag == Player::NONE);
    assert(subsong >= 0);
    const auto &__config = static_cast<const LibXMPConfig&>(_config);
    const auto config = __config.tag == Player::libxmp ? __config : LibXMPConfig(_config);
    XMPContext *context = nullptr;
    xmp_context c = xmp_create_context();
    int frequency = min(_config.frequency, 48000);
    if (!c) {
        ERR("player_libxmp::play xmp_create_context failed for %s\n", path);
        return {};
    }
    // XXX must use path based loading to support StarTrekker/AudioSculpture (and external sample loading)
    int ret = xmp_load_module(c, path);
    if (ret) {
        ERR("player_libxmp::play xmp_load_module failed for %s (%d)\n", path, ret);
        goto error;
    }
    xmp_module_info info;
    xmp_get_module_info(c, &info);
    if (subsong >= info.num_sequences) {
        ERR("player_libxmp::play invalid subsong %d for %s\n", subsong, path);
        goto error;
    }
    ret = xmp_start_player(c, frequency, 0);
    if (ret) {
        ERR("player_libxmp::play xmp_start_player failed for %s (%d)\n", path, ret);
        goto error;
    }
    if (subsong > 0) {
        int pos = info.seq_data[subsong].entry_point;
        ret = xmp_set_position(c, pos);
        if (ret != pos) {
            ERR("player_libxmp::play xmp_set_position failed for %s (%d)\n", path, ret);
            goto error;
        }
    }
    if (config.probe) {
        xmp_set_player(c, XMP_PLAYER_MIX, 0);
        xmp_set_player(c, XMP_PLAYER_INTERP, 0);
        xmp_set_player(c, XMP_PLAYER_DSP, 0);
    } else {
        xmp_set_player(c, XMP_PLAYER_INTERP, XMP_INTERP_SPLINE);
        xmp_set_player(c, XMP_PLAYER_DSP, XMP_DSP_ALL);
        xmp_set_player(c, XMP_PLAYER_MIX, (1.0f - config.panning) * 100);
        if (config.filter == Filter::A500) {
            xmp_set_player(c, XMP_PLAYER_FLAGS, XMP_FLAGS_A500);
        }
    }

    context = new XMPContext;
    context->context = c;
    context->info = info;

    return PlayerState {Player::libxmp, subsong, frequency, config.endian != endian::native, context, !config.probe, mixBufSize(frequency), 0, 0};
error:
    xmp_release_module(c);
    xmp_free_context(c);
    delete context;
    return {};
}

pair<SongEnd::Status,size_t> render(PlayerState &state, char *buf, size_t size) noexcept {
    assert(state.player == Player::libxmp);
    assert(size >= state.buffer_size);
    XMPContext *context = static_cast<XMPContext*>(state.context);
    assert(context);
    assert(context->context);
    // xmp_play_buffer() songend is buggy, so use moduleinfo duration as well
    // also avoid rendering past end of song (loop)
    bool songend = false;
    size_t bytestorender = state.buffer_size;
    const int64_t bytespersec = 4 * state.frequency;
    const int64_t totalbytes = context->info.seq_data[state.subsong].duration * bytespersec / 1000;
    // rendered bytes must be multiple of 4
    const auto modulo = totalbytes % 4 > 0 ? 4 - (totalbytes % 4) : 0;
    const int64_t bytesleft = (totalbytes + modulo) - state.total_bytes;
    if (bytesleft <= bytestorender) {
        bytestorender = bytesleft;
        songend = true;
    }
    int ret = xmp_play_buffer(context->context, buf, bytestorender, 0);
    songend |= ret == -XMP_END;
    if (ret && !songend) {
        ERR("player_libxmp::render xmp_play_buffer failed (%d)\n", ret);
        return {SongEnd::ERROR, 0};
    }
    return {songend ? SongEnd::PLAYER : SongEnd::NONE, bytestorender};
}

bool stop(PlayerState &state) noexcept {
    assert(state.player == Player::libxmp);
    XMPContext *context = static_cast<XMPContext*>(state.context);
    assert(context);
    assert(context->context);
    xmp_end_player(context->context);
    xmp_release_module(context->context);
    xmp_free_context(context->context);
    delete context;
    return true;
}

bool restart(PlayerState &state) noexcept {
    assert(state.player == Player::libxmp);
    XMPContext *context = static_cast<XMPContext*>(state.context);
    assert(context);
    assert(context->context);
    xmp_restart_module(context->context);
    return true;
}

} // namespace player::libxmp
