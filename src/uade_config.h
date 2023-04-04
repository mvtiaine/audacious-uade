// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#ifndef UADE_CONFIG_H_
#define UADE_CONFIG_H_

#include <libaudcore/preferences.h>

#include "config.h"
#ifdef SYSTEM_LIBUADE
#include <uade/uade.h>
#else
#include "../uade/src/frontends/include/uade/options.h"
#include "../uade/src/frontends/include/uade/uadeconfstructure.h"
#include "../uade/src/frontends/include/uade/uade.h"
#endif

#include "common.h"

const char * const uade_defaults[] = {
   "frequency",         "44100",
   "filter",            "1", // A500
   "force_led_enabled", "FALSE",
   "force_led",         "0", // OFF
   "resampler",         "0", // Default
   "panning",           "0.7",
   "headphones",        "FALSE",
   "headphones2",       "FALSE",
   "gain",              "1.0",
   "subsong_timeout",   "600",
   "silence_timeout",   "20",
   nullptr
};

const PreferencesWidget uade_audio_widgets1[] = {
    WidgetLabel("<b>Sample rate</b>"),
    WidgetSpin("", WidgetInt(PLUGIN_NAME, "frequency"), {8000, 96000, 25, "<i>Hz  </i>"}),

    WidgetLabel("<b>Filter</b>"),
    // 0 reserved for "Auto"
    WidgetRadio("A500", WidgetInt(PLUGIN_NAME, "filter"), {1}, WIDGET_CHILD),
    WidgetRadio("A1200", WidgetInt(PLUGIN_NAME, "filter"), {2}, WIDGET_CHILD),
    WidgetRadio("None", WidgetInt(PLUGIN_NAME, "filter"), {3}, WIDGET_CHILD),

    WidgetLabel("<b>Resampler</b>"),
    WidgetRadio("Default", WidgetInt(PLUGIN_NAME, "resampler"), {0}, WIDGET_CHILD),
    WidgetRadio("Sinc", WidgetInt(PLUGIN_NAME, "resampler"), {1}, WIDGET_CHILD),
    WidgetRadio("None", WidgetInt(PLUGIN_NAME, "resampler"), {2}, WIDGET_CHILD),

    WidgetLabel("<b>Force LED</b>"),
    WidgetCheck("Enable", WidgetBool(PLUGIN_NAME, "force_led_enabled")),
    WidgetRadio("OFF", WidgetInt(PLUGIN_NAME, "force_led"), {0}, WIDGET_CHILD),
    WidgetRadio("ON", WidgetInt(PLUGIN_NAME, "force_led"), {1}, WIDGET_CHILD),
};

const PreferencesWidget uade_audio_widgets2[] = {
    WidgetSpin("<b>Panning</b>", WidgetFloat(PLUGIN_NAME, "panning"), {0, 2, 0.1}),
    WidgetLabel("<i>0 stereo 1 mono 2 inverse stereo</i>"),
    WidgetSeparator(),
    WidgetSpin("<b>Volume gain</b>", WidgetFloat(PLUGIN_NAME, "gain"), {0, 128.0, 0.1}),

    WidgetLabel("<b>Headphones effect</b>"),
    WidgetCheck("Enable", WidgetBool(PLUGIN_NAME, "headphones"), WIDGET_CHILD),
    WidgetLabel("<b>Headphones 2 effect</b>"),
    WidgetCheck("Enable", WidgetBool(PLUGIN_NAME, "headphones2"), WIDGET_CHILD),
};

const PreferencesWidget uade_timeout_widgets[] = {
    WidgetLabel("<b>Timeouts (seconds)</b>"),
    WidgetSpin("Song timeout", WidgetInt(PLUGIN_NAME, "subsong_timeout"), {1, 3600, 5}, WIDGET_CHILD),
    WidgetSpin("Silence timeout", WidgetInt(PLUGIN_NAME, "silence_timeout"), {1, 3600, 1}, WIDGET_CHILD),
};

struct uade_state *create_uade_state();
struct uade_state *create_uade_probe_state();

#endif /* UADE_CONFIG_H_ */
