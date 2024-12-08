// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <libaudcore/preferences.h>

#include "uade_prefs.h"

// pref keys
constexpr const char *PRECALC_SONGLENGTHS = "precalc_songlengths";

const char * const plugin_defaults[] = {
    "frequency",             "48000",
    PRECALC_SONGLENGTHS,     "TRUE",
    "it2play_driver",        "0", // HQ
    nullptr
};

const PreferencesWidget plugin_widgets[] = {
    WidgetLabel("<b>Sample rate</b>"),
    WidgetSpin("", WidgetInt(PLUGIN_NAME, "frequency"), {8000, 96000, 25, "Hz  "}),
    WidgetSeparator (),
    WidgetCheck("Precalc missing song lengths",
        WidgetBool(PLUGIN_NAME, PRECALC_SONGLENGTHS)),
    WidgetLabel("<b>it2play audio driver</b>"),
    WidgetRadio("HQ", WidgetInt(PLUGIN_NAME, "it2play_driver"), {0}, WIDGET_CHILD),
    WidgetRadio("SB16MMX", WidgetInt(PLUGIN_NAME, "it2play_driver"), {1}, WIDGET_CHILD),
    WidgetRadio("SB16", WidgetInt(PLUGIN_NAME, "it2play_driver"), {2}, WIDGET_CHILD),
    WidgetRadio("WAVWriter", WidgetInt(PLUGIN_NAME, "it2play_driver"), {3}, WIDGET_CHILD),
    WidgetLabel("<b>IMPORTANT!</b>"),
    WidgetLabel(
        "<b>Probe content of files with no recognized<br/>"
        "file name extension</b> setting must be <br/>"
        "<i>enabled</i> and <b>Guess missing metadata<br/>"
        "from filepath</b> setting <i>disabled</i> in Audacious<br/>"
        "prefs, in order for subsongs and meta data<br/>"
        "to work. The playlist must also have been<br/>"
        "created <i>after</i> the settings are applied."
    ),
};

const PreferencesWidget middle_column[] = {
    WidgetLabel("<b>UADE config</b>"),
    WidgetSeparator (),
    WidgetBox({{uade_audio_widgets1}}),
};

const PreferencesWidget right_column[] = {
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
