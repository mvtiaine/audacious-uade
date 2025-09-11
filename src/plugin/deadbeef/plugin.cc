// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#define DDB_API_LEVEL 10
extern "C" {
#include <deadbeef/deadbeef.h>
}

#include <cstring>
#include <string>
#include <vector>

#include "config.h"
#include "common/logger.h"
#include "common/strings.h"
#include "player/extensions.h"
#include "player/player.h"
#include "songend/precalc.h"
#include "songdb/songdb.h"
#include "plugin/common/copyright.h"
#include "3rdparty/xxhash/xxhash.h"

using namespace std;

namespace plugin::deadbeef {
DB_decoder_t *uade_plugin;
DB_functions_t *deadbeef;
}

using namespace plugin::deadbeef;

namespace {

constexpr const char *PLUGINID = "audacious-uade";

constexpr const char settings_dlg[] =
"property \"Sample rate (Hz)\" entry uade.frequency 48000;\n" // 8000..96000
"property \"Precalc missing song lengths\" checkbox uade.precalc_songlengths 1;\n"
"property \"Skip broken and duplicate subsongs\" checkbox uade.skip_broken_subsongs 1;\n"
"property \"Minimum song length (seconds)\" entry uade.min_songlength 0;\n"
"property \"it2play driver\" select[4] uade.it2play_driver 0 \"HQ\" \"SB16MMX\" \"SB16\" \"WAVWRITER\";\n" 
"property \"Filter\" select[3] uade.filter 1 \"A500\" \"A1200\" \"None\";\n" // Auto not implemented
"property \"Resampler\" select[3] uade.resampler 1 \"Default\" \"Sinc\" \"None\";\n"
"property \"Force LED\" checkbox uade.force_led_enabled 0;\n"
"property \"LED state\" select[2] uade.force_led 0 \"OFF\" \"ON\";\n" // when Force LED enabled
"property \"Panning\" entry uade.panning 0.7;\n"
"property \"Gain\" entry uade.gain 1.0;\n"
"property \"Headphones effect\" checkbox uade.headphones 0;\n"
"property \"Headphones 2 effect\" checkbox uade.headphones2 0;\n"
"property \"Subsong timeout (seconds)\" entry uade.subsong_timeout 600;\n" // when songlength N/A
"property \"Silence timeout (seconds)\" entry uade.silence_timeout 10;\n" // when songlength N/A
;

player::Filter player_filter(const int filter) {
    switch(filter) {
        case 1: return player::Filter::A500;
        case 2: return player::Filter::A1200;
        case 3: return player::Filter::NONE;
        default: return player::Filter::A1200;
    }
}
    
player::uade::Resampler uade_resampler(const int resampler) {
    switch(resampler) {
        case 0: return player::uade::Resampler::DEFAULT;
        case 1: return player::uade::Resampler::SINC;
        case 2: return player::uade::Resampler::NONE;
        default: return player::uade::Resampler::SINC;
    }
}
    
player::uade::UADEConfig get_uade_config(const player::PlayerConfig &config) {
    const int filter = deadbeef->conf_get_int("uade.filter", 2);
    const bool force_led_enabled = deadbeef->conf_get_int("uade.force_led_enabled", 0);
    const int force_led = deadbeef->conf_get_int("uade.force_led", 0);
    const int resampler = deadbeef->conf_get_int("uade.resampler", 1);
    const float panning = deadbeef->conf_get_float("uade.panning", 0.7);
    const bool headphones = deadbeef->conf_get_int("uade.headphones", 0);
    const bool headphones2 = deadbeef->conf_get_int("uade.headphones2", 0);
    const float gain = deadbeef->conf_get_float("uade.gain", 1.0);
    const int subsong_timeout = deadbeef->conf_get_int("uade.subsong_timeout", 600);
    const int silence_timeout = deadbeef->conf_get_int("uade.silence_timeout", 10);

    player::uade::UADEConfig conf(config);

    conf.filter = player_filter(filter);
    conf.resampler = uade_resampler(resampler);

    if (force_led_enabled)
        conf.force_led = force_led ? true : false;
    else
        conf.force_led = {};

    conf.panning = panning;
    conf.gain = gain;

    conf.headphones = headphones;
    conf.headphones2 = headphones2;

    conf.subsong_timeout = subsong_timeout;
    conf.silence_timeout = silence_timeout;

    return conf;
}
    
player::it2play::Driver it2play_driver(const int driver) {
    switch(driver) {
        case 0: return player::it2play::Driver::HQ;
        case 1: return player::it2play::Driver::SB16MMX;
        case 2: return player::it2play::Driver::SB16;
        case 3: return player::it2play::Driver::WAVWRITER;
        default: return player::it2play::Driver::HQ;
    }
}

player::it2play::IT2PlayConfig get_it2play_config(const player::PlayerConfig &config) {
    const int driver = deadbeef->conf_get_int("uade.it2play_driver", 0);
    player::it2play::IT2PlayConfig conf(config);
    conf.driver = it2play_driver(driver);
    return conf;
}

player::libopenmpt::LibOpenMPTConfig get_libopenmpt_config(const player::PlayerConfig &config) {
    const int filter = deadbeef->conf_get_int("uade.filter", 2);
    const float panning = deadbeef->conf_get_float("uade.panning", 0.7);

    player::libopenmpt::LibOpenMPTConfig conf(config);
    conf.filter = player_filter(filter);
    conf.panning = panning;

    return conf;
}
    
player::libxmp::LibXMPConfig get_libxmp_config(const player::PlayerConfig &config) {
    const int filter = deadbeef->conf_get_int("uade.filter", 2);
    const float panning = deadbeef->conf_get_float("uade.panning", 0.7);

    player::libxmp::LibXMPConfig conf(config);
    conf.filter = player_filter(filter);
    conf.panning = panning;

    return conf;
}

string xxh32hash(const vector<char> &buf) {
    const auto xxh32 = XXH32(buf.data(), min(songdb::XXH_MAX_BYTES, buf.size()), 0);
    return common::to_hex(xxh32) + common::to_hex((uint16_t)(buf.size() & 0xFFFF));
}

struct uade_info_t {
    DB_fileinfo_t info;
    player::PlayerState state;
    int known_timeout = 0;
    int silence_timeout = 0; // uade only
    vector<char> tail; // track leftover bytes from previous read
};

int uade_start(void) {
    //DEBUG("uade_start\n");
    player::init();
    songdb::init(UADEDIR "/songdb");

    return 0;
}
    
int uade_stop(void) {
    //DEBUG("uade_stop\n");
    player::shutdown();
    return 0;
}

DB_fileinfo_t *uade_open(uint32_t hints) {
    TRACE("uade_open - hints %u\n", hints);
    uade_info_t *info = new uade_info_t;
    memset(&info->info, 0, sizeof(DB_fileinfo_t));
    return &info->info;
}

int uade_init(DB_fileinfo_t *_info, DB_playItem_t *it) {
    deadbeef->pl_lock();
    const string fname = deadbeef->pl_find_meta(it, ":URI");
    deadbeef->pl_unlock();
    TRACE("uade_init %s\n", fname.c_str());

    uade_info_t *info = (uade_info_t *)_info;

    DB_FILE *fp = deadbeef->fopen(fname.c_str());
    if (!fp) {
        ERR("uade_init fopen failed for %s\n", fname.c_str());
        return -1;
    }

    const size_t size = deadbeef->fgetlength(fp);
    vector<char> buf(size);
    size_t read = deadbeef->fread(buf.data(), 1, size, fp);
    deadbeef->fclose(fp);

    if (read != size) {
        ERR("uade_init read failed for %s\n", fname.c_str());
        return -1;
    }

    const auto playername = deadbeef->pl_find_meta(it, "player");
    player::Player player = player::Player::NONE;
    if (playername) {
        player = player::player(playername);
    } else {
        const auto players = player::check(fname.c_str(), buf.data(), size, size);
        for (const auto &p : players) {
            if (player::parse(fname.c_str(), buf.data(), size, p)) {
                player = p;
                break;
            }
        }
    }

    if (player == player::Player::NONE) {
        DEBUG("uade_init could not parse %s\n", fname.c_str());
        return -1;
    }

    int subsong = deadbeef->pl_find_meta_int(it, ":TRACKNUM", -1);
    assert(subsong >= 0);
    const int frequency = deadbeef->conf_get_int("uade.frequency", 48000);

    int known_timeout = deadbeef->pl_get_item_duration(it) * 1000;
    if (known_timeout <= 0 && deadbeef->conf_get_int("uade.precalc_songlengths", 1) &&
        !deadbeef->pl_find_meta(it, "songend")) {
        ddb_playlist_t *plt = deadbeef->plt_get_curr ();
        if (!plt) {
            WARN("uade_init could not get current playlist\n");
        } else {
            const auto modinfo = player::parse(fname.c_str(), buf.data(), size, player);
            if (!modinfo) {
                ERR("uade_init could not parse %s\n", fname.c_str());
                return -1;
            }
            const auto hash = xxh32hash(buf);
            const auto songend = songend::precalc::precalc_song_end(modinfo.value(), buf.data(), size, subsong, hash);
            deadbeef->pl_add_meta(it, "songend", songend.status_string().c_str());
            deadbeef->plt_set_item_duration(plt, it, songend.length / 1000.0f);
            deadbeef->plt_modified (plt);
            deadbeef->plt_unref(plt);
            deadbeef->sendmessage (DB_EV_PLAYLISTCHANGED, 0, DDB_PLAYLIST_CHANGE_CONTENT, 0);
        }
    }
    const player::PlayerConfig player_config = {player, frequency, known_timeout};
    const auto uade_config = get_uade_config(player_config);
    const auto it2play_config = get_it2play_config(player_config);
    const auto libopenmpt_config = get_libopenmpt_config(player_config);
    const auto libxmp_config = get_libxmp_config(player_config);
    const auto &config =
        player == player::Player::uade ? uade_config :
        player == player::Player::it2play ? it2play_config :
        player == player::Player::libopenmpt ? libopenmpt_config :
        player == player::Player::libxmp ? libxmp_config :
        player_config;

    const auto state = player::play(fname.c_str(), buf.data(), size, subsong, config);
    if (!state) {
        return -1;
    }
    info->state = state.value();
    info->known_timeout = known_timeout;
    info->silence_timeout = player == player::Player::uade ? uade_config.silence_timeout : 0;
    _info->plugin = uade_plugin;
    _info->fmt.channels = 2;
    _info->fmt.bps = 16;
    _info->fmt.samplerate = state->frequency;
    _info->fmt.channelmask = DDB_SPEAKER_FRONT_LEFT | DDB_SPEAKER_FRONT_RIGHT;
    _info->readpos = 0;

    return 0;
}

void uade_free(DB_fileinfo_t *_info) {
    //TRACE("uade_free\n");
    uade_info_t *info = (uade_info_t *)_info;
    if (info) {
        player::stop(info->state);
        delete info;
    }
}

int uade_read(DB_fileinfo_t *_info, char *bytes, int _size) {
    //TRACE("uade_read size %d\n", size);
    
    uade_info_t *info = (uade_info_t *)_info;
    if (!info) {
        ERR("uade_read info is null\n");
        return 0;
    }
    if (info->known_timeout == 0) {
        DEBUG("uade_read known_timeout is 0\n");
        return 0;
    }
    if (_info->readpos == 0)
        info->tail.clear();
    assert(_size > 0);
    const size_t size = _size;
    size_t bytes_read = info->tail.size();
    if (bytes_read) {
        memcpy(bytes, info->tail.data(), bytes_read);
        info->tail.clear();
    }
    while (bytes_read < size) {
        const auto res = player::support::playback_step(
            info->state, info->known_timeout, info->silence_timeout);

        if (bytes_read + res.buffer.size() > size) {
            memcpy(bytes + bytes_read, res.buffer.data(), size - bytes_read);
            info->tail = {res.buffer.begin() + size - bytes_read, res.buffer.end()};
        } else {
            memcpy(bytes + bytes_read, res.buffer.data(), res.buffer.size());
        }
        bytes_read += res.buffer.size();

        if (res.songend.status != common::SongEnd::NONE)
            break;
    }
    const int res = min(bytes_read, size);
    const int samplesize = (_info->fmt.bps/8) * _info->fmt.channels;
    assert(res % samplesize == 0);
    _info->readpos += res / samplesize / (float)_info->fmt.samplerate;

    return res;
}

int uade_seek_sample (DB_fileinfo_t *_info, int sample) {
    //TRACE("uade_seek_sample to %d \n", sample);
    uade_info_t *info = (uade_info_t *)_info;

    if (!info) {
        ERR("uade_seek_sample info is null\n");
        return -1;
    }

    int millis = (int64_t)sample * 1000 / info->info.fmt.samplerate;
    if (!player::seek(info->state, millis)) {
        ERR("uade_seek_sample seek failed\n");
        return -1;
    }

    _info->readpos = millis / 1000.0;
    info->tail.clear();
    return 0;
}

int uade_seek(DB_fileinfo_t *info, float time) {
    return uade_seek_sample(info, time * info->fmt.samplerate);
}

DB_playItem_t *uade_insert(ddb_playlist_t *plt, DB_playItem_t *after, const char *fname) {
    //TRACE("uade_insert %s\n", fname);

    DB_FILE *fp = deadbeef->fopen(fname);
    if (!fp) {
        ERR("uade_insert fopen failed for %s\n", fname);
        return nullptr;
    }

    const size_t size = deadbeef->fgetlength(fp);
    vector<char> buf(size);
    size_t read = deadbeef->fread(buf.data(), 1, size, fp);
    deadbeef->fclose(fp);

    if (read != size) {
        ERR("uade_insert read failed for %s\n", fname);
        return nullptr;
    }

    auto modinfo = optional<player::ModuleInfo>();
    const auto players = player::check(fname, buf.data(), size, size);
    if (players.empty()) {
        DEBUG("uade_insert no player found for %s\n", fname);
        return nullptr;
    }

    const auto hash = xxh32hash(buf);

#if PLAYER_uade
    if (songdb::blacklist::is_blacklisted_hash(hash)) {
        // may hang UADE completely
        WARN("uade_insert blacklisted hash %s (%s)\n", fname, hash.c_str());
        return nullptr;
    }
#endif

    for (const auto &p : players) {
        modinfo = player::parse(fname, buf.data(), size, p);
        if (modinfo) break;
    }

    if (!modinfo) {
        DEBUG("uade_insert could not parse %s\n", fname);
        return nullptr;
    }

    const int minsubsong = modinfo->minsubsong;
    const int maxsubsong = modinfo->maxsubsong;
    const auto metadata = songdb::lookup(hash);
    int i = 0;
    const bool skip_broken = deadbeef->conf_get_int("uade.skip_broken_subsongs", 1);
    const int min_length = deadbeef->conf_get_int("uade.min_songlength", 0);
    for (int s = minsubsong; s <= maxsubsong; s++) {
        const optional<songdb::SubSongInfo> ss = metadata ? metadata->subsongs[i++] : optional<songdb::SubSongInfo>();
        assert(!ss || ss->subsong == s);
        if (ss && skip_broken && (!ss->songend.length || ss->is_duplicate || ss->songend.length >= player::PRECALC_TIMEOUT * 1000)) {
            DEBUG("uade_insert skipping broken/duplicate subsong %s #%d\n", fname, s);
            continue;
        }
        if (ss && min_length && ss->songend.length < min_length * 1000) {
            DEBUG("uade_insert skipping short subsong %s #%d length %d\n", fname, s, ss->songend.length);
            continue;
        }
        DB_playItem_t *it = deadbeef->pl_item_alloc_init(fname, PLUGINID);
        deadbeef->pl_set_meta_int(it, ":TRACKNUM", s);
        int numtracks = maxsubsong - minsubsong + 1;
        if (numtracks > 1) {
            deadbeef->pl_set_meta_int(it, "track", s);
            deadbeef->pl_set_meta_int(it, "numtracks", maxsubsong - minsubsong + 1);
        }
        deadbeef->pl_add_meta(it, "title", common::split(fname, "/").back().c_str());
        deadbeef->pl_add_meta(it, "player", player::name(modinfo->player).data());
        deadbeef->pl_replace_meta(it, ":FILETYPE", modinfo->format.c_str());
        if (modinfo->channels > 0)
            deadbeef->pl_set_meta_int(it, ":CHANNELS", modinfo->channels);
        if (metadata && metadata->metadata) {
            if (!metadata->metadata->author.empty())
                deadbeef->pl_add_meta(it, "artist", metadata->metadata->author.c_str());
            if (!metadata->metadata->album.empty())
                deadbeef->pl_add_meta(it, "album", metadata->metadata->album.c_str());
            if (!metadata->metadata->publisher.empty())
                deadbeef->pl_add_meta(it, "publisher", metadata->metadata->publisher.c_str());
            if (metadata->metadata->year)
                deadbeef->pl_add_meta(it, "year", to_string(metadata->metadata->year).c_str());
        }
        common::SongEnd songend = {common::SongEnd::NONE, 0};
        const auto songinfo = metadata ? songdb::lookup(hash, s) : optional<songdb::SubSongInfo>();
        bool precalc = deadbeef->conf_get_int("uade.precalc_songlengths", 1);
        if (songinfo) {
            songend = songinfo->songend;
        } else if (precalc) {
            songend = songend::precalc::precalc_song_end(modinfo.value(), buf.data(), size, s, hash);
        }
        if (songend.status != common::SongEnd::NONE) {
            auto songend_str = songend.status_string();
            if (ss && ss->is_duplicate) {
                songend_str += " (!)";
            }
            deadbeef->pl_add_meta(it, "songend", songend_str.c_str());
            deadbeef->plt_set_item_duration(plt, it, songend.length / 1000.0f);
        }
        after = deadbeef->plt_insert_item(plt, after, it);
        deadbeef->pl_item_unref(it);
    }

    return after;
}

DB_decoder_t _uade_plugin = {
    .plugin = {
        .type = DB_PLUGIN_DECODER,
        .api_vmajor = DB_API_VERSION_MAJOR,
        .api_vminor = DB_API_VERSION_MINOR,
        .version_major = 0,
        .version_minor = 1,
        .flags = DDB_PLUGIN_FLAG_LOGGING,
        .id = PLUGINID,
        .name = "UADE Plugin",
        .descr = "Decoder plugin for UADE and other retro music replays",
        .copyright = plugin_copyright,
        .website = PACKAGE_URL,
        .start = uade_start,
        .stop = uade_stop,
        .configdialog = settings_dlg,
    },
    .open = uade_open,
    .init = uade_init,
    .free = uade_free,
    .read = uade_read,
    .seek = uade_seek,
    .seek_sample = uade_seek_sample,
    .insert = uade_insert,
    .exts = const_cast<const char **>(player::exts),
    .prefixes = const_cast<const char **>(player::exts),
};

} // namespace {}

extern "C" __attribute__ ((visibility ("default")))
// XXX use aaa_ prefix to make it highest prio plugin
DB_plugin_t * aaa_uade_load (DB_functions_t *api) {
    deadbeef = api;
    uade_plugin = &_uade_plugin;
    return DB_PLUGIN (uade_plugin);
}
