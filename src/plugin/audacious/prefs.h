// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include <libaudcore/preferences.h>

#include "uade_prefs.h"

const char * const plugin_defaults[] = {
    "frequency",             "48000",
    "precalc_songlengths",     "TRUE",
    "skip_broken_subsongs",    "TRUE",
    "min_songlength", "0",
    "it2play_driver",        "0", // HQ
    nullptr
};

const PreferencesWidget plugin_widgets[] = {
    WidgetLabel("<b>Sample rate</b>"),
    WidgetSpin("", WidgetInt(PLUGIN_NAME, "frequency"), {8000, 96000, 25, "Hz  "}),
    WidgetSeparator (),
    WidgetCheck("Precalc missing song lengths",
        WidgetBool(PLUGIN_NAME, "precalc_songlengths")),
    WidgetLabel("<b>Filter subsongs</b>"),
    WidgetLabel("<i>Subsong filters only work for known files</i>", WIDGET_CHILD),
    WidgetLabel("<i>with more than one subsong</i>", WIDGET_CHILD),
    WidgetCheck("Skip broken and duplicate subsongs",
        WidgetBool(PLUGIN_NAME, "skip_broken_subsongs"), WIDGET_CHILD),
    WidgetSpin("Minimum length", WidgetInt(PLUGIN_NAME, "min_songlength"), {0, 3600, 5, "seconds"}, WIDGET_CHILD),
    WidgetLabel("<b>it2play driver</b>"),
    WidgetRadio("HQ", WidgetInt(PLUGIN_NAME, "it2play_driver"), {0}, WIDGET_CHILD),
    WidgetRadio("SB16MMX", WidgetInt(PLUGIN_NAME, "it2play_driver"), {1}, WIDGET_CHILD),
    WidgetRadio("SB16", WidgetInt(PLUGIN_NAME, "it2play_driver"), {2}, WIDGET_CHILD),
    WidgetRadio("WAVWriter", WidgetInt(PLUGIN_NAME, "it2play_driver"), {3}, WIDGET_CHILD),
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
    WidgetLabel("<b>IMPORTANT!</b>"),
    WidgetLabel(
        "<b>Probe content of files with no recognized file name extension</b> setting must be <i>enabled</i> and<br/>"
        "<b>Guess missing metadata from filepath</b> setting <i>disabled</i> in Audacious prefs, in order for subsongs<br/>"
        "and meta data to work. The playlist must also have been created <i>after</i> the settings are applied."
    ),
};

const PluginPreferences plugin_prefs = {{ prefs_widget }};
