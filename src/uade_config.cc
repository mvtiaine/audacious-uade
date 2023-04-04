// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>
#include "config.h"
#ifdef SYSTEM_LIBUADE
#include <uade/uade.h>
#else
#include "../uade/src/frontends/include/uade/options.h"
#include "../uade/src/frontends/include/uade/uadeconfstructure.h"
#include "../uade/src/frontends/include/uade/uade.h"
#endif

#include "uade_config.h"
#include "hacks.h"

namespace {
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

    void uade_common_options(struct uade_config *uc) {
        // use our uade.conf, song.conf and contentdb even with system libuade
        uade_config_set_option(uc, UC_BASE_DIR, UADEDATADIR);
#ifdef SYSTEM_LIBUADE
        // make sure to use system version for these
        uade_config_set_option(uc, UC_UAE_CONFIG_FILE, UADE_CONFIG_BASE_DIR "/uaerc");
        uade_config_set_option(uc, UC_SCORE_FILE, UADE_CONFIG_BASE_DIR "/score");
        uade_config_set_option(uc, UC_UADECORE_FILE, UADE_CONFIG_UADE_CORE);
#endif
        uade_config_set_option(uc, UC_ONE_SUBSONG, nullptr);
        // UC_ENABLE_TIMEOUTS is enabled in uade.conf, must not be enabled here as it otherwise
        // messes with always_ends setting in eagleplayer.conf (Protracker)       
        const char *subsong_timeout = aud_get_str(PLUGIN_NAME, "subsong_timeout");
        const char *silence_timeout = aud_get_str(PLUGIN_NAME, "silence_timeout");
        uade_config_set_option(uc, UC_SUBSONG_TIMEOUT_VALUE, subsong_timeout);
        uade_config_set_option(uc, UC_SILENCE_TIMEOUT_VALUE, silence_timeout);

#ifdef DEBUG_TRACE
        uade_config_set_option(uc, UC_VERBOSE, nullptr);
#endif
    }
}

struct uade_state *create_uade_state() {
    struct uade_config *uc = uade_new_config();
    uade_common_options(uc);

    const char *frequency = aud_get_str(PLUGIN_NAME, "frequency");
    int filter = aud_get_int(PLUGIN_NAME, "filter");
    bool force_led_enabled = aud_get_bool(PLUGIN_NAME, "force_led_enabled");
    int force_led = aud_get_int(PLUGIN_NAME, "force_led");
    int resampler = aud_get_int(PLUGIN_NAME, "resampler");
    const char *panning = aud_get_str(PLUGIN_NAME, "panning");
    bool headphones = aud_get_bool(PLUGIN_NAME, "headphones");
    bool headphones2 = aud_get_bool(PLUGIN_NAME, "headphones2");
    const char *gain = aud_get_str(PLUGIN_NAME, "gain");

    DEBUG("uade_config: frequency %s, filter %s, force_led_enabled %d, force_led %d, resampler %s, panning %s, "
          "headphones %d, headphones2 %d, gain %s, subsong_timeout %s, silence_timeout %s\n",
          frequency, uade_filter(filter), force_led_enabled, force_led, uade_resampler(resampler), panning,
          headphones, headphones2, gain, (const char *)aud_get_str(PLUGIN_NAME, "subsong_timeout"),
          (const char *)aud_get_str(PLUGIN_NAME, "silence_timeout"));

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

struct uade_state *create_uade_probe_state() {
    struct uade_config *uc = uade_new_config();
    uade_common_options(uc);

    uade_config_set_option(uc, UC_FREQUENCY, "8000");
    uade_config_set_option(uc, UC_FILTER_TYPE, "none");
    uade_config_set_option(uc, UC_RESAMPLER, "none");
    uade_config_set_option(uc, UC_PANNING_VALUE, "0");
    uade_config_set_option(uc, UC_NO_FILTER, nullptr);
    uade_config_set_option(uc, UC_NO_HEADPHONES, nullptr);
    uade_config_set_option(uc, UC_NO_PANNING, nullptr);
    uade_config_set_option(uc, UC_NO_POSTPROCESSING, nullptr);
 
    struct uade_state *state = uade_new_state(uc);
    uade_set_amiga_loader(amiga_loader_wrapper, nullptr, state);
    return state;
}
