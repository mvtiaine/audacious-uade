// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2023-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#if defined(__AROS__)
// XXX avoid conflict with <exec/lists.h>
#define _GLIBCXX_ATOMICITY_H 1
#endif

#include <cassert>
#include <utility>

#include "common/logger.h"
#include "player/player.h"

extern "C"
{
#include "3rdparty/replay/libdigibooster3/libdigibooster3.h"
#include "3rdparty/replay/libdigibooster3/player.h"
// expose method from loader.c
struct DB3Module *DB3_LoadFromHandle(struct AbstractHandle *ah, int *errptr);
}

using namespace std;
using namespace common;
using namespace player;

namespace {

constexpr size_t mixBufSize(const int frequency) noexcept {
    return 4 * (frequency / 50 + (frequency % 50 != 0 ? 1 : 0));
}

struct DB3Context {
    void *engine;
    DB3Module *module;
    bool songend = false;
};

struct DB3Handle {
    const char *buf;
    ssize_t size;
    ssize_t pos;
};

const char* ErrorReasons[] = {
    "no error",
    "can't open file",
    "out of memory",
    "module corrupted",
    "unsupported format version",
    "data read error",
    "wrong chunk order in the module"
};

struct DB3Module *my_DB3_Load(const char *buf, ssize_t size, int *errptr) noexcept {
    struct AbstractHandle ah;
    DB3Handle ah_Handle = {buf, size, 0};

    const auto ah_Read = [](struct AbstractHandle *ah, void *db3buf, int db3bytes) {
        errno = 0;
        DB3Handle *handle = static_cast<DB3Handle*>(ah->ah_Handle);
        assert(handle);
        ssize_t bytes = min((ssize_t)db3bytes, handle->size - handle->pos);
        if (bytes <= 0) return 0;
        assert(db3buf);
        assert(handle->buf);
        assert(bytes > 0);
        assert(handle->pos >= 0 && handle->pos <= handle->size);
        assert(handle->pos + bytes <= handle->size);
        memcpy(db3buf, handle->buf + handle->pos, bytes);
        handle->pos += bytes;
        return 1;
    };

    ah.ah_Handle = &ah_Handle;
    ah.ah_Read = ah_Read;

    return DB3_LoadFromHandle(&ah, errptr);
}

constexpr_f2 ModuleInfo get_info(const string &path, struct DB3Module *mod) noexcept  {
    string format = (mod->CreatorVer == CREATOR_DIGIBOOSTER_2) ? "DigiBooster Pro 2" : "DigiBooster 3";
    format += "." + to_string(mod->CreatorRev);
    
    const int maxsubsong = mod->NumSongs - 1;
    const int channels = mod->NumTracks;

    return {Player::libdigibooster3, format, path, 0, maxsubsong, 0, channels};
}

}

namespace player::libdigibooster3 {

void init() noexcept {}
void shutdown() noexcept {}

bool is_our_file(const char */*path*/, const char *buf, size_t size) noexcept {
    assert(size >= 4);
    return buf[0] == 'D' && buf[1] == 'B' && buf[2] == 'M' && buf[3] == '0';
}

optional<ModuleInfo> parse(const char *path, const char *buf, size_t size) noexcept {
    int error = 0;
    struct DB3Module *mod = my_DB3_Load(buf, size, &error);
    if (!mod || error) {
        if (mod) {
            DB3_Unload(mod);
        }
        DEBUG("player_libdigibooster3::parse parsing failed for %s reason %s\n", path, ErrorReasons[error]);
        return {};
    }

    const auto &info = get_info(path, mod);
    DB3_Unload(mod);

    return info;
}

optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &config) noexcept {
    assert(config.player == Player::libdigibooster3 || config.player == Player::NONE);
    assert(config.tag == Player::libdigibooster3 || config.tag == Player::NONE);
    assert(subsong >= 0);
    int error = 0;
    struct DB3Module *mod = my_DB3_Load(buf, size, &error);
    if (!mod || error) {
        if (mod) {
            DB3_Unload(mod);
        }
        ERR("player_libdigibooster3::play parsing failed for %s reason %s\n", path, ErrorReasons[error]);
        return {};
    }

    int frames = mixBufSize(config.frequency) / 4;
    void *engine = DB3_NewEngine(mod, config.frequency, frames);
    if (!engine) {
        DB3_Unload(mod);
        ERR("player_libdigibooster3::play creating engine failed for %s\n", path);
        return {};
    }

    DB3Context *context = new DB3Context;
    context->engine = engine;
    context->module = mod;

    assert(subsong <= mod->NumSongs - 1);
    DB3_SetPos(engine, subsong, 0, 0);
    DB3_SetVolume(engine, 4);
    DB3_SetCallback(engine, [](void *db3context, struct UpdateEvent *event) {
        if (event->ue_Order == -1 && event->ue_Pattern == -1 && event->ue_Row == -1) {
            auto context = static_cast<DB3Context*>(db3context);
            assert(context);
            context->songend = true;
        }
    }, context);

    return PlayerState {Player::libdigibooster3, subsong, config.frequency, config.endian != endian::native, context, true, mixBufSize(config.frequency), 0};
}

bool stop(PlayerState &state) noexcept {
    assert(state.player == Player::libdigibooster3);
    if (state.context) {
        const auto context = static_cast<DB3Context*>(state.context);
        assert(context);
        DB3_DisposeEngine(context->engine);
        DB3_Unload(context->module);
        delete context;
    }
    return true;
}

pair<SongEnd::Status,size_t> render(PlayerState &state, char *buf, size_t size) noexcept {
    assert(state.player == Player::libdigibooster3);
    assert(size >= state.buffer_size);
    const auto context = static_cast<DB3Context*>(state.context);
    assert(context);
    size_t totalbytes = DB3_Mix(context->engine, state.buffer_size / 4, (int16_t*)buf) * 4;
    bool songend = context->songend || totalbytes < state.buffer_size;

    return pair<SongEnd::Status,size_t>(songend ? SongEnd::PLAYER : SongEnd::NONE, totalbytes);
}

bool restart(PlayerState &state) noexcept {
    assert(state.player == Player::libdigibooster3);
    const auto context = static_cast<DB3Context*>(state.context);
    assert(context);
    msynth_reset((struct ModSynth *)context->engine, true);
    DB3_SetPos(context->engine, state.subsong, 0, 0);

    return true;
}

} // namespace player::libdigibooster3