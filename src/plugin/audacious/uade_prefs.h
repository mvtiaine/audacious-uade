// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <libaudcore/preferences.h>

constexpr const char *PLUGIN_NAME = "uade";

const char * const uade_defaults[] = {
   "filter",                "2", // A1200
   "force_led_enabled",     "FALSE",
   "force_led",             "0", // OFF
   "resampler",             "1", // Sinc
   "panning",               "0.7",
   "headphones",            "FALSE",
   "headphones2",           "FALSE",
   "gain",                  "1.0",
   "subsong_timeout",       "600",
   "silence_timeout",       "10",
   nullptr
};

const PreferencesWidget uade_audio_widgets1[] = {
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
    WidgetSeparator(),
    WidgetSeparator(),
    WidgetSpin("Panning", WidgetFloat(PLUGIN_NAME, "panning"), {0, 1, 0.1}),
    WidgetLabel("<i>0 stereo 1 mono</i>"),
    WidgetSeparator(),
    WidgetSpin("Volume gain", WidgetFloat(PLUGIN_NAME, "gain"), {0, 128.0, 0.1}),

    WidgetLabel("<b>Headphones effect</b>"),
    WidgetCheck("Enable", WidgetBool(PLUGIN_NAME, "headphones"), WIDGET_CHILD),
    WidgetLabel("<b>Headphones 2 effect</b>"),
    WidgetCheck("Enable", WidgetBool(PLUGIN_NAME, "headphones2"), WIDGET_CHILD),
};

const PreferencesWidget uade_timeout_widgets[] = {
    WidgetLabel("<b>Timeouts (seconds)</b>"),
    WidgetLabel("<i>Applied when songlength N/A</i>", WIDGET_CHILD),
    WidgetSpin("Song timeout", WidgetInt(PLUGIN_NAME, "subsong_timeout"), {1, 3600, 5}, WIDGET_CHILD),
    WidgetSpin("Silence timeout", WidgetInt(PLUGIN_NAME, "silence_timeout"), {1, 3600, 1}, WIDGET_CHILD),
};
