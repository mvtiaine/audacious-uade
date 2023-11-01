// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <cassert>

#include "../common.h"
#include "player.h"

extern "C"
{
#include "../3rdparty/dbm/libdigibooster3.h"
// expose method from loader.c
struct DB3Module *DB3_LoadFromHandle(struct AbstractHandle *ah, int *errptr);
}

using namespace std;
using namespace player;

namespace {

struct DB3Context {
    void *engine;
    DB3Module *module;
};

struct DB3Handle {
    const char *buf;
    size_t size;
    size_t pos;
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

struct DB3Module *my_DB3_Load(const char *buf, size_t size, int *errptr) {
	struct AbstractHandle ah;
    DB3Handle ah_Handle = {buf, size, 0};

    const auto ah_Read = [](struct AbstractHandle *ah, void *db3buf, int db3bytes) {
        DB3Handle *handle = static_cast<DB3Handle*>(ah->ah_Handle);
        size_t bytes = min((size_t)db3bytes, handle->size - handle->pos);
        if (!bytes) return 0;
        memcpy(db3buf, handle->buf + handle->pos, bytes);
        handle->pos += bytes;
        return 1;
    };

    ah.ah_Handle = &ah_Handle;
    ah.ah_Read = ah_Read;

	return DB3_LoadFromHandle(&ah, errptr);
}

}

namespace player::dbm {

void init() {
}

optional<ModuleInfo> parse(const char *fname, const char *buf, size_t size) {
    int error;
    struct DB3Module *mod = my_DB3_Load(buf, size, &error);
    if (!mod) {
        ERR("player_dbm::parse parsing failed for %s reason %s\n", fname, ErrorReasons[error]);
        return {};
    }

    string format = (mod->CreatorVer == CREATOR_DIGIBOOSTER_2) ? "DigiBooster Pro 2" : "DigiBooster 3";
    format += "." + to_string(mod->CreatorRev);
    
    const int maxsubsong = mod->NumSongs - 1;
    const int channels = mod->NumTracks;

    DB3_Unload(mod);

    const ModuleInfo info = {format, fname, maxsubsong, channels};
    return info;
}

optional<PlayerState> play(const char *fname, const char *buf, size_t size, int subsong, int frequency) {
    int error;
    struct DB3Module *mod = my_DB3_Load(buf, size, &error);
    if (!mod) {
        ERR("player_dbm::play parsing failed for %s reason %s\n", fname, ErrorReasons[error]);
        return {};
    }

    int frames = MIXBUFSIZE / 4;
    void *engine = DB3_NewEngine(mod, frequency, frames);
    if (!engine) {
        DB3_Unload(mod);
        ERR("player_dbm::play creating engine failed for %s\n", fname);
        return {};
    }

    DB3Context *context = new DB3Context;
    context->engine = engine;
    context->module = mod;

    PlayerState state = {DIGIBOOSTERPRO, subsong, frequency, context, 0, false};

    DB3_SetPos(engine, subsong, 0, 0);
    DB3_SetCallback(engine, [](void *context, struct UpdateEvent *event) {
        if (event->ue_Order == -1 && event->ue_Pattern == -1 && event->ue_Row == -1) {
            ((PlayerState&)context).songend = true;
        }
    }, &state);

    return state;
}

void stop(PlayerState &state) {
    assert(state.player == DIGIBOOSTERPRO);
    if (state.context) {
        const auto context = (DB3Context*)(state.context);
        DB3_DisposeEngine(context->engine);
        DB3_Unload(context->module);
        delete context;
    }
}

pair<bool,size_t> render(PlayerState &state, char *buf, size_t size) {
    assert(state.player == DIGIBOOSTERPRO);

    const auto context = (DB3Context*)(state.context);
    size_t totalbytes = DB3_Mix(context->engine, size / 4, (int16_t*)buf) * 4;
    bool songend = state.songend || totalbytes < size;

    return pair(songend, totalbytes);
}

bool restart(PlayerState &state) {
    assert(state.player == DIGIBOOSTERPRO);

    const auto context = (DB3Context*)(state.context);
    DB3_SetPos(context->engine, state.subsong, 0, 0);

    return true;
}

} // namespace player::dbm