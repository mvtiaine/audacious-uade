// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2014-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#include "common/compat.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <map>
#include <mutex>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "common/logger.h"
#include "common/strings.h"
#include "player/player.h"
#include "player/players/internal.h"

#include <sys/stat.h>

extern "C" {
#include "../uade/src/frontends/include/uade/options.h"
#include "../uade/src/frontends/include/uade/uadeconfstructure.h"
#include "../uade/src/frontends/include/uade/uade.h"
}

using namespace std;
using namespace player;
using namespace player::internal;
using namespace player::uade;
using namespace common;

namespace {

constexpr size_t mixBufSize(const int frequency) noexcept {
    return 4 * (frequency / 50 + (frequency % 50 != 0 ? 1 : 0));
}

struct uade_context {
    uade_state *state;
    uade_config *config;
    bool initialized = false;
    bool available = true;
    int id;
    bool probe;
};
constexpr int MAX_PROBES = 8;
uade_context probes[MAX_PROBES];
mutex probe_mutex;

// Modland hack for missing WantedTeam.bin files
const map<pair<string,string>, string> bundled_extfiles ({
    {{"1ad8cd8a8e799fe7f1f27e00f925d9e0","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/GeorgGlaxo/WantedTeam.bin"},
    {{"81b641316c9af33154bea71eb97fa916","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/GeorgGlaxo/WantedTeam.bin"},
    {{"a33a4ef23f43543ae09ee4fecc4a0a61","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/GeorgGlaxo/WantedTeam.bin"},
    {{"153c555fdc2c72e5a62e21c131f4b635","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"159581910f064311d7fc72b8ccf29ab9","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"19ed56bde9033c6fb1c88731ce4fd2d1","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"23aa0bce4f4e6293ad6565eda2c78347","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"283a7612f6afc3a54666423bdfccb594","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"301a523cb5686a4226fe0a62f4820779","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"37006b2614276015f32f866565bf2ea2","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"3ecf6d705b44906eaecf7630c05751a0","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"3f897211dc0f17c2cc13db4c3698de78","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"43be812e51102ff8e76424b9c37614f9","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"478bf4b89695454d067e34e05a22cacc","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"4a08b49732bfb2f2f2acd01b2c15dde3","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"5254492b6ca356a8c81054befaa0b1db","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"53f7ebaa4edf178c31d214f928d75934","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"557550edf2c96732bc992785f9907a15","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"55f682366163f34b528f9b7c709b64a8","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"59aaf90869419c55ac9e26cc6c0a014e","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"6b13749109816261295ef3c08dee35f4","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"7187fca3ce63d75bc08a5fea917c3b6e","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"7c2a31d9c62154a7a85a4f7e0bf17b06","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"7d8082ab2b8bf21586712591ad8213d5","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"7dc142146e6946a6a0e61af40ee762b7","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"7f119629fc9a51d6e78e8ae7662d62b8","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"80b7a1e3039d808aaca66f78d49c1f41","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"854201a7ceb6e497ffd6c742dd8faf54","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"859c834231a3efa428ac5c49ceb42481","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"884f0124a90a1b7f805af8df61b47f5d","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"8de8e2c0e1d23edd6d84bfdfcf7efb26","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"998c371cecbb9b2592ec110298258295","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"a0b78c1b40738913f370019034090543","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"a29d573748bcdee158d462556ce1b79b","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"ac6ee85f36542340f2b0dc8300938439","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"b069ee530330bfde8346a98e7a7c2619","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"bbb1f74fbdcbb36be6a768251807086b","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"c61bfb3bc7e04c593dcd1b45689c7ceb","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"c9b430c12ba35598f8e51a569a1b510d","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"cd64ac2c1892666c85f65600ccfa181d","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"d84d3fe127fd9980f7a4c7a91db407c5","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"dc099b0c587c9e92c03e2ba93076c49d","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"e18c7366eadcf2e171b39816918408f0","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"ef9c33a9ecb0e14c68a09a44f08b6616","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"fe771473508f0efcb23dcacc1eb85fe9","WantedTeam.bin"}, UADEDIR "/ext/JesperOlsen/LollyPop/WantedTeam.bin"},
    {{"607fa8d7a49c0b373580fd629acd40e8","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/AncientArtOfWar/WantedTeam.bin"},
    {{"7795cf2a65a25a099f509640087dac15","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/AncientArtOfWar/WantedTeam.bin"},
    {{"19435767e2bf68b6c04941385d505745","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/AncientArtOfWar/WantedTeam.bin"},
    {{"12178bce42c9e8702881c14993789163","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/AncientArtOfWar/WantedTeam.bin"},
    {{"1e35824e46acb0f89e25ab8d01d73a30","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/AncientArtOfWar/WantedTeam.bin"},
    {{"5ef88969b20d52d9408c3d78d604cd1e","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/AncientArtOfWar/WantedTeam.bin"},
    {{"a52948cd3d62978a6b9570ae21b7401d","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/AncientArtOfWar/WantedTeam.bin"},
    {{"f9c3e244ada1a0ca701211a2cefa26b0","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/AncientArtOfWar/WantedTeam.bin"},
    {{"a717edf8c6257e3d2d0a0d27b9798616","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/AncientArtOfWar/WantedTeam.bin"},
    {{"2ec0c17d505d95da48fdf8a7728a46e2","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/Dogfight/WantedTeam.bin"},
    {{"61d746d9fa2766b803e58ac7766c3f6d","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/Dogfight/WantedTeam.bin"},
    {{"7ee7a0176b92e62ebc29cc1506b48434","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/Dogfight/WantedTeam.bin"},
    {{"cf8e54284a53a44f83bd28a17ecbe84e","WantedTeam.bin"}, UADEDIR "/ext/PaulTonge/Dogfight/WantedTeam.bin"},
});

struct uade_file *uade_load(const char *name, const char*playerdir, struct uade_state *state) noexcept {
    struct uade_file *amiga_file = uade_load_amiga_file(name, playerdir, state);
    if (!amiga_file) {
        // try with lower case
        string lcname = name;
        transform(lcname.begin(), lcname.end(), lcname.begin(), ::tolower);
        if (lcname != name) {
            amiga_file = uade_load_amiga_file(lcname.c_str(), playerdir, state);
        }
    }
    if (amiga_file) {
        TRACE("amiga_loader_wrapper found file: %s\n", name);
    } else {
        TRACE("amiga_loader_wrapper did NOT find file: %s\n", name);
    }
    return amiga_file;
}

struct uade_file *sample_loader_wrapper(const char *name, const char *playerdir, uade_state *state) noexcept {
    // hack for modland incoming/vault/Sonix (instruments named wrongly)
    const auto load = [&](const string &path) -> struct uade_file* {
        auto *amiga_file = uade_load(path.c_str(), playerdir, state);
        if (!amiga_file) {
            string newpath = path;
            replace(newpath.begin(), newpath.end(), '_', ' ');
            if (newpath != path) {
                amiga_file = uade_load(newpath.c_str(), playerdir, state);
            }
            if (!amiga_file) {
                newpath = path;
                replace(newpath.begin(), newpath.end(), ' ', '_');
                if (newpath != path) {
                    amiga_file = uade_load(newpath.c_str(), playerdir, state);
                }
            }
        }
        return amiga_file;
    };

    struct uade_file *amiga_file = load(name);
    if (!amiga_file) {
        vector<string> stokens = split(name, "/");
        bool absolute = string(name).rfind('/', 0) == 0;
        if (stokens.size() >= 3 && stokens[0] != ".") {
            int cnt = 0;
            const string sample = stokens[stokens.size() - 2] + "/" + stokens[stokens.size() - 1];
            stokens.erase(stokens.end() - 1);
            stokens.erase(stokens.end() - 1);
            string samplepath = sample;
            while (!amiga_file && cnt++ < 3) {
                if (stokens.size() > 0) {
                    stokens.erase(stokens.end() - 1);
                    const string parent = mkString(stokens, "/");
                    const string newpath = (absolute ? "/" : "") + (parent.empty() ? sample : parent + "/" + sample);
                    amiga_file = load(newpath.c_str());
                } else {
                    samplepath = "../"+samplepath;
                    amiga_file = load(samplepath);
                }
            }
        } else if ((stokens.size() == 3 && stokens[0] == ".") || stokens.size() == 2) {
            string samplepath = stokens[stokens.size() - 2] + "/" + stokens[stokens.size() - 1];
            int cnt = 0;
            while (!amiga_file && cnt++ < 3) {
                amiga_file = load(samplepath);
                samplepath = "../"+samplepath;
            }
        }
    }
    if (!amiga_file) {
        DEBUG("sample_loader_wrapper could NOT find file: %s\n", name);
    }
    return amiga_file;
}

} // namespace {}

extern "C" {

struct uade_file *amiga_loader_wrapper(const char *name, const char *playerdir, void */*context*/, uade_state *state) noexcept {
    const struct uade_song_info *info = uade_get_song_info(state);
    const string player = info->playerfname;
    TRACE("amiga_loader_wrapper name: %s player: %s\n", name, player.c_str());

    const string fname = split(name, "/").back();
    const auto key = pair<string,string>(string(info->modulemd5),fname);
    if (bundled_extfiles.count(key)) {
        const auto bundled = bundled_extfiles.at(key);
        TRACE("amiga_loader_wrapper loading bundled extfile for %s bundled: %s\n", name, bundled.c_str());
        return uade_load(bundled.c_str(), playerdir, state);
    }

    if (player.find("/ZoundMonitor") != player.npos || player.find("/SonixMusicDriver") != player.npos) {
        return sample_loader_wrapper(name, playerdir, state);
    }

    // for example cust.zoids (aka zoids.src) and other SunTronic customs will first request instr/ directory (for listing?)
    // just return empty file for directories to make them happy
    struct stat s;
    if (stat(name,&s) == 0 && s.st_mode & S_IFDIR) {
        DEBUG("amiga_loader_wrapper returning dummy file for directory %s\n", name);
        struct uade_file *dummyfile = static_cast<struct uade_file*>(malloc(sizeof (struct uade_file)));
        assert(dummyfile);
        dummyfile->name = nullptr;
        dummyfile->data = nullptr;
        dummyfile->size = 0;
        return dummyfile;
    }

    return uade_load(name, playerdir, state);
}

} // extern "C"

namespace {

constexpr const char *uade_filter(Filter filter) noexcept {
    switch(filter) {
        case Filter::NONE: return "none";
        case Filter::A500: return "a500";
        case Filter::A1200: return "a1200";
        default: assert(false); return "none";
    }
}
constexpr const char *uade_resampler(Resampler resampler) noexcept {
    switch(resampler) {
        case Resampler::NONE: return "none";
        case Resampler::DEFAULT: return "default";
        case Resampler::SINC: return "sinc";
        default: assert(false); return "none";
    }
}

void uade_common_options(struct uade_config *uc) noexcept {
#if DEBUG_TRACE
    uade_config_set_option(uc, UC_VERBOSE, nullptr);
#endif
#ifdef UADE_BASE_DIR
    uade_config_set_option(uc, UC_BASE_DIR, UADE_BASE_DIR);
#else
    // use our uade.conf, song.conf and contentdb
    if (getenv("UADE_BASE_DIR")) {
        // for unit tests
        uade_config_set_option(uc, UC_BASE_DIR, getenv("UADE_BASE_DIR"));
    } else {
        uade_config_set_option(uc, UC_BASE_DIR, UADEDATADIR);
    }
#endif
#ifdef UADE_CORE_FILE
    uade_config_set_option(uc, UC_UADECORE_FILE, UADE_CORE_FILE);
#else
    if (getenv("UADE_CORE_FILE")) {
        // for unit tests
        uade_config_set_option(uc, UC_UADECORE_FILE, getenv("UADE_CORE_FILE"));
    }
#endif
    uade_config_set_option(uc, UC_NO_CONTENT_DB, nullptr);
    uade_config_set_option(uc, UC_ONE_SUBSONG, nullptr);
    uade_config_set_option(uc, UC_TIMEOUT_VALUE, "-1");
    uade_config_set_option(uc, UC_DISABLE_TIMEOUTS, nullptr);
}

uade_state *create_uade_probe_state(uade_config *uc) noexcept {
    assert(uc);
    uade_common_options(uc);
    uade_config_set_option(uc, UC_SUBSONG_TIMEOUT_VALUE, to_string(PRECALC_TIMEOUT).c_str());
    uade_config_set_option(uc, UC_SILENCE_TIMEOUT_VALUE,to_string(SILENCE_TIMEOUT).c_str());
    uade_config_set_option(uc, UC_FREQUENCY, to_string(PRECALC_FREQ).c_str());
    uade_config_set_option(uc, UC_FILTER_TYPE, "none");
    uade_config_set_option(uc, UC_RESAMPLER, "none");
    uade_config_set_option(uc, UC_PANNING_VALUE, "1");
    uade_config_set_option(uc, UC_NO_FILTER, nullptr);
    uade_config_set_option(uc, UC_NO_HEADPHONES, nullptr);
    uc->gain_enable = 0;
 
    uade_state *state = uade_new_state(uc);
    assert(state);
    uade_set_amiga_loader(amiga_loader_wrapper, nullptr, state);
#if DEBUG_TRACE
    uade_enable_uadecore_log_collection(state);
#endif
    return state;
}
 
uade_context *get_probe_context() noexcept {
    uade_context *context = nullptr;
    {
        const lock_guard<mutex> lock(probe_mutex);
        for (int i = 0; i < MAX_PROBES; ++i) {
            if (probes[i].available) {
                probes[i].available = false;
                if (!probes[i].initialized) {
                    uade_config *uc = uade_new_config();
                    probes[i].config = uc;
                    probes[i].state = create_uade_probe_state(uc);
                    probes[i].initialized = true;
                    probes[i].id = i;
                }
                context = &probes[i];
                break;
            }
        }
    }
    assert(context);
    assert(context->state);
    return context;
}

void release_probe_context(uade_context *context) noexcept {
    TRACE("release_probe_context id %d\n", context->id);
    const lock_guard<mutex> lock(probe_mutex);
    context->available = true;
}

void cleanup_probe_context(uade_context *context) noexcept {
    TRACE("cleanup_probe_context id %d\n", context->id);
    uade_cleanup_state(context->state);
    context->state = create_uade_probe_state(context->config);
}

void stop_probe_context(uade_context *context) noexcept {
    TRACE("stop_probe_context id %d\n", context->id);
    if (uade_stop(context->state)) {
        WARN("uade_stop failed for id %d\n", context->id);
        cleanup_probe_context(context);
    }
}

struct probe_scope {
    uade_context *context;
    probe_scope() noexcept :
        context(get_probe_context()) {
            assert(context);
            assert(context->state);
        }
    ~probe_scope() noexcept {
        assert(context);
        assert(context->state);
        stop_probe_context(context);
        release_probe_context(context);
    }
};

struct uade_state *create_uade_state(const UADEConfig &config, struct uade_config *uc) noexcept {
    DEBUG("uade_config: frequency %d, filter %d, force_led_enabled %d, force_led %d, resampler %d, panning %f, "
          "headphones %d, headphones2 %d, gain %f, subsong_timeout %d, silence_timeout %d\n",
          config.frequency, static_cast<int>(config.filter), config.force_led ? true : false,
          config.force_led ? config.force_led.value() : -1, static_cast<int>(config.resampler), config.panning,
          config.headphones, config.headphones2, config.gain, config.subsong_timeout, config.silence_timeout);
    assert(uc);
    uade_common_options(uc);

    // UC_ENABLE_TIMEOUTS is also enabled in uade.conf, must not be enabled here by default as it otherwise
    // messes with always_ends setting in eagleplayer.conf (Protracker)
    if (config.known_timeout > 0) {
        const auto timeout = to_string(config.known_timeout / 1000 + 1);
        uade_config_set_option(uc, UC_SUBSONG_TIMEOUT_VALUE, timeout.c_str());
        uade_config_set_option(uc, UC_SILENCE_TIMEOUT_VALUE, timeout.c_str());
    } else {
        uade_config_set_option(uc, UC_SUBSONG_TIMEOUT_VALUE, to_string(config.subsong_timeout).c_str());
        uade_config_set_option(uc, UC_SILENCE_TIMEOUT_VALUE, to_string(config.silence_timeout).c_str());
    }
    uade_config_set_option(uc, UC_FREQUENCY, to_string(config.frequency).c_str());
    uade_config_set_option(uc, UC_FILTER_TYPE, uade_filter(config.filter));

    if (config.force_led)
        uade_config_set_option(uc, UC_FORCE_LED, config.force_led.value() ? "on" : "off");

    uade_config_set_option(uc, UC_RESAMPLER, uade_resampler(config.resampler));
    uade_config_set_option(uc, UC_PANNING_VALUE, to_string(config.panning).c_str());
    uade_config_set_option(uc, UC_GAIN, to_string(config.gain).c_str());

    if (config.headphones)
        uade_config_set_option(uc, UC_HEADPHONES, nullptr);
    if (config.headphones2)
        uade_config_set_option(uc, UC_HEADPHONES2, nullptr);
    if (!config.headphones && !config.headphones2)
        uade_config_set_option(uc, UC_NO_HEADPHONES, nullptr);

    struct uade_state *state = uade_new_state(uc);
    assert(state);
    uade_set_amiga_loader(amiga_loader_wrapper, nullptr, state);
#if DEBUG_TRACE
    uade_enable_uadecore_log_collection(state);
#endif
    return state;
}

const struct uade_song_info *get_song_info(const uade_state *state) noexcept {
    const struct uade_song_info *info = uade_get_song_info(state);
#if DEBUG_TRACE
    const struct uade_subsong_info &subsong = info->subsongs;
    const struct uade_detection_info &detection = info->detectioninfo;
    char infotext[16384];
    TRACE("uade_song_info " 
          " - subsong cur:%d, min:%d, def:%d, max:%d"
          " - detection custom:%d, content:%d, ext:%s"
          " - module bytes:%lu, md5:%s, fname:%s, name:%s"
          " - duration:%f, subsongbytes:%zd, songbytes:%zd"
          " - player fname:%s, name:%s, format:%s"
          "\n",
          subsong.cur, subsong.min, subsong.def, subsong.max,
          detection.custom, detection.content, detection.ext,
          info->modulebytes, info->modulemd5, info->modulefname, info->modulename,
          info->duration, (ssize_t)info->subsongbytes, (ssize_t)info->songbytes,
          info->playerfname, info->playername, info->formatname);
    if (!uade_song_info(infotext, size(infotext), info->modulefname, UADE_MODULE_INFO)) {
        TRACE("uade_song_info - module info:\n%s\n", infotext);
    } else {
        TRACE("uade_song_info - no module info\n");
    }
#endif
    return info;
}

void cleanup_context(uade_context *context) noexcept {
    assert(context);
    assert(context->state);
    if (context->probe) {
        stop_probe_context(context);
        release_probe_context(context);
    } else {
        uade_cleanup_state(context->state);
        free(context->config->resampler);
        free(context->config);
    }
}

constexpr_f2 string parse_codec(const struct uade_song_info *info) noexcept {
    constexpr_v string_view TYPE_PREFIX = "type: ";
    const string playername = info->playername;
    string formatname = info->formatname;
    if (!formatname.empty()) {
        // remove "type: " prefix included in some formats
        if (common::starts_with(formatname, TYPE_PREFIX)) formatname = formatname.substr(TYPE_PREFIX.length());
        if (formatname.length() == 4 && common::starts_with(formatname, "MMD")) formatname = "OctaMED " + formatname;
        return formatname;
    } else if (!playername.empty()) {
        return playername;
    } else {
        return UNKNOWN_CODEC;
    }
}

} // namespace {}

namespace player::uade {

void init() noexcept {
    for (int i = 0; i < MAX_PROBES; ++i) {
        probes[i] = {};
        probes[i].probe = true;
    }
}

void shutdown() noexcept {
    for (int i = 0; i < MAX_PROBES; ++i) {
        if (probes[i].initialized) {
            probes[i].initialized = false;
            assert(probes[i].state);
            uade_cleanup_state(probes[i].state);
            free(probes[i].config->resampler);
            free(probes[i].config);
            probes[i].state = nullptr;
            probes[i].config = nullptr;
        }
    }
}

bool is_our_file(const char *path, const char *buf, size_t size) noexcept {
    if (!is_xm(path,buf,size) && !is_fst(path,buf,size) && !is_s3m(path,buf,size) && !is_it(path,buf,size) && !is_sid(path,buf,size)) {
        const probe_scope probe;
        TRACE("uade::is_our_file using probe id %d - %s\n", probe.context->id, path);
        return uade_is_our_file_from_buffer(path, buf, size, probe.context->state) != 0;
    }
    return false;
}

optional<ModuleInfo> parse(const char *path, const char *buf, size_t size) noexcept {
    const probe_scope probe;
    TRACE("uade::parse using probe id %d - %s\n", probe.context->id, path);
    switch (uade_play_from_buffer(path, buf, size, -1, probe.context->state)) {
        case 1: {
            const struct uade_song_info* info = get_song_info(probe.context->state);
            const string format = parse_codec(info);
            // avoid uade_subsong_control: Assertion `subsong >= 0 && subsong < 256' failed.
            if (info->subsongs.min < 0) WARN("uade::parse invalid min subsong %d for %s\n", info->subsongs.min, path);
            if (info->subsongs.max > 255) WARN("uade::parse invalid max subsong %d for %s\n", info->subsongs.max, path);
            const int minsubsong = max(0, info->subsongs.min);
            const int maxsubsong = min(255, info->subsongs.max);
            assert(minsubsong <= maxsubsong);
            // TODO channels
            return ModuleInfo {Player::uade, format, path, minsubsong, maxsubsong, info->subsongs.def, 0};
        }
        case -1:
            WARN("uade::parse fatal error on %s \n", path);
            return {};
        default:
            DEBUG("uade::parse cannot play %s\n", path);
            return {};
    }
}

optional<PlayerState> play(const char *path, const char *buf, size_t size, int subsong, const PlayerConfig &config) noexcept {
    assert(subsong >= 0 && subsong <= 255);
    uade_context *context;
    if (config.probe) {
        context = get_probe_context();
    } else {
        // create new for playback
        context = new uade_context;
        context->id = -1;
        context->probe = false;
        struct uade_config *uc = uade_new_config();
        context->config = uc;
        const auto &uade_config = static_cast<const UADEConfig&>(config);
        context->state = create_uade_state(
            uade_config.player == Player::uade ? uade_config : UADEConfig(config),
            uc);
    }

    switch (uade_play_from_buffer(path, buf, size, subsong, context->state)) {
        case 1:
            return PlayerState {Player::uade, subsong, config.frequency, config.endian != endian::native, context, !config.probe, mixBufSize(config.frequency), 0};
        default:
            ERR("Could not play %s\n", path);
            cleanup_context(context);
            if (!config.probe) {
                assert(!context->probe);
                delete context;
            }
            return {};
    }
}

pair<SongEnd::Status,size_t> render(PlayerState &state, char *buf, size_t size) noexcept {
    assert(state.player == Player::uade);
    assert(size >= mixBufSize(state.frequency));
    const auto context = static_cast<uade_context*>(state.context);
    assert(context);
    assert(context->state);
    uade_notification n;
    SongEnd::Status status = SongEnd::NONE;
    ssize_t nbytes = uade_read(buf, mixBufSize(state.frequency), context->state);
    while (uade_read_notification(&n, context->state)) {
        switch (n.type) {
            case UADE_NOTIFICATION_MESSAGE:
                TRACE("Amiga message: %s\n", n.msg);
                break;
            case UADE_NOTIFICATION_SONG_END: {
                TRACE("%s: %s\n", n.song_end.happy ? "song end" : "bad song end", n.song_end.reason);
                constexpr_v string_view reason_timeout1 = "song timeout";
                constexpr_v string_view reason_timeout2 = "subsong timeout";
                constexpr_v string_view reason_silence = "silence";
                if (n.song_end.happy) {
                    string reason = n.song_end.reason;
                    if (reason == reason_timeout1 || reason == reason_timeout2)
                        status = SongEnd::TIMEOUT;
                    else if (reason == reason_silence)
                        status = SongEnd::DETECT_SILENCE;
                    else
                        status = SongEnd::PLAYER;
                } else {
                    status = SongEnd::ERROR;
                }
                break;
            }
            default:
                WARN("Unknown notification type from libuade\n");
                break;
        }
        uade_cleanup_notification(&n);
    }
    if (nbytes < 0) {
        status = SongEnd::ERROR;
        nbytes = 0;
    }
    return pair<SongEnd::Status,size_t>(status,nbytes);
}

bool stop(PlayerState &state) noexcept {
    assert(state.player == Player::uade);
    if (state.context) {
        auto context = static_cast<uade_context*>(state.context);
        cleanup_context(context);
        if (!context->probe)
            delete context;
        state.context = nullptr;
    }
    return true;
}

bool restart(PlayerState &/*state*/) noexcept {
    assert(false);
    return false;
}

bool seek(PlayerState &state, int millis) noexcept {
    assert(state.player == Player::uade);
    const auto context = static_cast<uade_context*>(state.context);
    assert(context);
    assert(context->state);
    bool res = !uade_seek(UADE_SEEK_SUBSONG_RELATIVE, millis / 1000.0, -1, context->state);
    if (res) {
        state.pos_millis = millis;
    }
    return res;
}

} // player::uade

