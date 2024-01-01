// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <libaudcore/preferences.h>

#include "uade_prefs.h"

// pref keys
constexpr const char *PRECALC_SONGLENGTHS = "precalc_songlengths";

const char * const plugin_defaults[] = {
    PRECALC_SONGLENGTHS, "TRUE",
    nullptr
};

const PreferencesWidget plugin_widgets[] = {
    WidgetCheck("Precalc missing song lengths",
        WidgetBool(PLUGIN_NAME, PRECALC_SONGLENGTHS)),
};

const PreferencesWidget middle_column[] = {
    WidgetLabel("<b>UADE config</b>"),
    WidgetSeparator (),
    WidgetBox({{uade_audio_widgets1}}),
};

const PreferencesWidget right_column[] = {
    WidgetLabel(""),
    WidgetSeparator (),
    WidgetBox({{uade_audio_widgets2}}),
    WidgetSeparator (),
    WidgetBox({{uade_timeout_widgets}}),
};

const PreferencesWidget left_column[] = {
    WidgetLabel("<b>Plugin config</b>"),
    WidgetSeparator (),
    WidgetBox({{plugin_widgets}}),
};

const PreferencesWidget widget_columns [] = {
    WidgetBox({{left_column}}),
    WidgetSeparator (),
    WidgetBox({{middle_column}}),
    WidgetSeparator (),
    WidgetBox({{right_column}}),
};

const PreferencesWidget prefs_widget[] = {
    WidgetBox ({{ widget_columns }, true}),
};

const PluginPreferences plugin_prefs = {{ prefs_widget }};
