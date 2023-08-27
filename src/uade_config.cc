// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <algorithm>

#include <libaudcore/runtime.h>

#include "uade_config.h"
#include "uade_common.h"
#include "hacks.h"
#include "songend/songend.h"

using namespace std;

namespace {

static pthread_mutex_t probe_mutex = PTHREAD_MUTEX_INITIALIZER;

const char *uade_filter(const int filter) {
    switch(filter) {
        case 2: return "a1200";
        case 3: return "none";
        default: return "a500";
    }
}
const char *uade_resampler(const int resampler) {
    switch(resampler) {
        case 1: return "sinc";
        case 2: return "none";
        default: return "default";
    }
}

void uade_common_options(struct uade_config *uc, const int known_timeout) {
#if DEBUG_TRACE
    uade_config_set_option(uc, UC_VERBOSE, nullptr);
#endif
    // use our uade.conf, song.conf and contentdb even with system libuade
    uade_config_set_option(uc, UC_BASE_DIR, UADEDATADIR);
#if SYSTEM_LIBUADE
    // make sure to use system version for these
    uade_config_set_option(uc, UC_UAE_CONFIG_FILE, UADE_CONFIG_BASE_DIR "/uaerc");
    uade_config_set_option(uc, UC_SCORE_FILE, UADE_CONFIG_BASE_DIR "/score");
    uade_config_set_option(uc, UC_UADECORE_FILE, UADE_CONFIG_UADE_CORE);
#else
    uade_config_set_option(uc, UC_NO_CONTENTDB, nullptr);
#endif
    uade_config_set_option(uc, UC_ONE_SUBSONG, nullptr);
    // use just subsong timeout
    uade_config_set_option(uc, UC_TIMEOUT_VALUE, "-1");
    // UC_ENABLE_TIMEOUTS is also enabled in uade.conf, must not be enabled here by default as it otherwise
    // messes with always_ends setting in eagleplayer.conf (Protracker)
    if (known_timeout > 0) {
        const auto timeout = to_string(known_timeout / 1000 + 1);
        uade_config_set_option(uc, UC_SUBSONG_TIMEOUT_VALUE, timeout.c_str());
        uade_config_set_option(uc, UC_SILENCE_TIMEOUT_VALUE, timeout.c_str());
    } else {
        const char *subsong_timeout = aud_get_str(PLUGIN_NAME, "subsong_timeout");
        const char *silence_timeout = aud_get_str(PLUGIN_NAME, "silence_timeout");
        uade_config_set_option(uc, UC_SUBSONG_TIMEOUT_VALUE, subsong_timeout);
        uade_config_set_option(uc, UC_SILENCE_TIMEOUT_VALUE, silence_timeout);
    }
}

} // namespace {}

struct uade_state *create_uade_state(const int known_timeout) {
    struct uade_config *uc = uade_new_config();

    const char *frequency = aud_get_str(PLUGIN_NAME, "frequency");
    const int filter = aud_get_int(PLUGIN_NAME, "filter");
    const bool force_led_enabled = aud_get_bool(PLUGIN_NAME, "force_led_enabled");
    const int force_led = aud_get_int(PLUGIN_NAME, "force_led");
    const int resampler = aud_get_int(PLUGIN_NAME, "resampler");
    const char *panning = aud_get_str(PLUGIN_NAME, "panning");
    const bool headphones = aud_get_bool(PLUGIN_NAME, "headphones");
    const bool headphones2 = aud_get_bool(PLUGIN_NAME, "headphones2");
    const char *gain = aud_get_str(PLUGIN_NAME, "gain");

    DEBUG("uade_config: frequency %s, filter %s, force_led_enabled %d, force_led %d, resampler %s, panning %s, "
          "headphones %d, headphones2 %d, gain %s, subsong_timeout %s, silence_timeout %s\n",
          frequency, uade_filter(filter), force_led_enabled, force_led, uade_resampler(resampler), panning,
          headphones, headphones2, gain, (const char *)aud_get_str(PLUGIN_NAME, "subsong_timeout"),
          (const char *)aud_get_str(PLUGIN_NAME, "silence_timeout"));

    uade_common_options(uc, known_timeout);

    uade_config_set_option(uc, UC_FREQUENCY, frequency);
    uade_config_set_option(uc, UC_FILTER_TYPE, uade_filter(filter));

    if (force_led_enabled)
        uade_config_set_option(uc, UC_FORCE_LED, force_led ? "on" : "off");

    uade_config_set_option(uc, UC_RESAMPLER, uade_resampler(resampler));
    uade_config_set_option(uc, UC_PANNING_VALUE, panning);

    if (headphones)
        uade_config_set_option(uc, UC_HEADPHONES, nullptr);
    if (headphones2)
        uade_config_set_option(uc, UC_HEADPHONES, nullptr);
    if (!headphones && !headphones2)
        uade_config_set_option(uc, UC_NO_HEADPHONES, nullptr);

    struct uade_state *state = uade_new_state(uc);
    uade_set_amiga_loader(amiga_loader_wrapper, nullptr, state);
    return state;
}

void cleanup_uade_state(uade_state *state, int id, const char *uri) {
    // avoid concurrent contentdb updates
    pthread_mutex_lock (&probe_mutex);
    TRACE("cleanup_uade_state state id %d - %s\n", id, uri);
    uade_cleanup_state(state);
    pthread_mutex_unlock(&probe_mutex);
}

probe_state *get_probe_state() {
    pthread_mutex_lock (&probe_mutex);
    probe_state *state = nullptr;
    for (int i = 0; i < MAX_PROBES; ++i) {
        if (probes[i].available) {
            probes[i].available = false;
            if (!probes[i].initialized) {
                probes[i].state = create_uade_probe_state(songend::PRECALC_FREQ);
                probes[i].initialized = true;
                probes[i].id = i;
            }
            state = &probes[i];
            break;
        }
    }
    pthread_mutex_unlock(&probe_mutex);
    assert(state);
    assert(state->state);
    return state;
}

void release_probe_state(probe_state *probe_state) {
    pthread_mutex_lock (&probe_mutex);
    probe_state->available = true;
    pthread_mutex_unlock(&probe_mutex);
}
