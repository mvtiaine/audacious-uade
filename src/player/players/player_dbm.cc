// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <cassert>

#include "common/common.h"
#include "common/logger.h"
#include "player/player.h"

extern "C"
{
#include "3rdparty/replay/dbm/libdigibooster3.h"
#include "3rdparty/replay/dbm/player.h"
// expose method from loader.c
struct DB3Module *DB3_LoadFromHandle(struct AbstractHandle *ah, int *errptr);
}

using namespace std;
using namespace common;
using namespace player;

namespace {

constexpr int mixBufSize(int frequency) {
    if (frequency > 96000) return 8192;
    else if (frequency > 48000) return 4096;
    else if (frequency > 24000) return 2048;
    else if (frequency > 12000) return 1024;
    else return 512;
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

struct DB3Module *my_DB3_Load(const char *buf, ssize_t size, int *errptr) {
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

ModuleInfo get_info(const string &path, struct DB3Module *mod) {
    string format = (mod->CreatorVer == CREATOR_DIGIBOOSTER_2) ? "DigiBooster Pro 2" : "DigiBooster 3";
    format += "." + to_string(mod->CreatorRev);
    
    const int maxsubsong = mod->NumSongs - 1;
    const int channels = mod->NumTracks;

    return {Player::dbm, format, path, 0, maxsubsong, 0, channels};
}

}

namespace player::dbm {

void init() {}
void shutdown() {}

bool is_our_file(const char */*path*/, const char *buf, size_t size) {
    assert(size >= 4);
    return buf[0] == 'D' && buf[1] == 'B' && buf[2] == 'M' && buf[3] == '0';
}

optional<ModuleInfo> parse(const char *path, const char *buf, size_t size) {
    int error = 0;
    struct DB3Module *mod = my_DB3_Load(buf, size, &error);
    if (!mod || error) {
        if (mod) {
            DB3_Unload(mod);
        }
        WARN("player_dbm::parse parsing failed for %s reason %s\n", path, ErrorReasons[error]);
        return {};
    }

    const auto &info = get_info(path, mod);
    DB3_Unload(mod);

    return info;
}

optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &config) {
    int error = 0;
    struct DB3Module *mod = my_DB3_Load(buf, size, &error);
    if (!mod || error) {
        if (mod) {
            DB3_Unload(mod);
        }
        ERR("player_dbm::play parsing failed for %s reason %s\n", path, ErrorReasons[error]);
        return {};
    }

    if (subsong == -1) {
        subsong = 0;
    }

    int frames = mixBufSize(config.frequency) / 4;
    void *engine = DB3_NewEngine(mod, config.frequency, frames);
    if (!engine) {
        DB3_Unload(mod);
        ERR("player_dbm::play creating engine failed for %s\n", path);
        return {};
    }

    DB3Context *context = new DB3Context;
    context->engine = engine;
    context->module = mod;

    const auto &info = get_info(path, mod);
    PlayerState state = {info, subsong, config.frequency, config.endian != endian::native, context, true, 0};

    assert(subsong >= 0 && subsong <= mod->NumSongs - 1);
    DB3_SetPos(engine, subsong, 0, 0);
    DB3_SetVolume(engine, 4);
    DB3_SetCallback(engine, [](void *db3context, struct UpdateEvent *event) {
        if (event->ue_Order == -1 && event->ue_Pattern == -1 && event->ue_Row == -1) {
            auto context = static_cast<DB3Context*>(db3context);
            assert(context);
            context->songend = true;
        }
    }, context);

    return state;
}

bool stop(PlayerState &state) {
    assert(state.info.player == Player::dbm);
    if (state.context) {
        const auto context = static_cast<DB3Context*>(state.context);
        assert(context);
        DB3_DisposeEngine(context->engine);
        DB3_Unload(context->module);
        delete context;
    }
    return true;
}

pair<SongEnd::Status,size_t> render(PlayerState &state, char *buf, size_t size) {
    assert(size >= mixBufSize(state.frequency));
    assert(state.info.player == Player::dbm);
    const auto context = static_cast<DB3Context*>(state.context);
    assert(context);
    size_t totalbytes = DB3_Mix(context->engine, mixBufSize(state.frequency) / 4, (int16_t*)buf) * 4;
    bool songend = context->songend || totalbytes < mixBufSize(state.frequency);

    return pair(songend ? SongEnd::PLAYER : SongEnd::NONE, totalbytes);
}

bool restart(PlayerState &state) {
    assert(state.info.player == Player::dbm);
    const auto context = static_cast<DB3Context*>(state.context);
    assert(context);
    msynth_reset((struct ModSynth *)context->engine, true);
    DB3_SetPos(context->engine, state.subsong, 0, 0);

    return true;
}

} // namespace player::dbm