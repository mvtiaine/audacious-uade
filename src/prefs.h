// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#ifndef PREFS_H_
#define PREFS_H_

#include <libaudcore/preferences.h>

#include "common.h"
#include "uade_config.h"

// pref keys
constexpr const char *MODLAND_ALLMODS_MD5_FILE = "modland_allmods_md5_file";
constexpr const char *PRECALC_SONGLENGTHS = "precalc_songlengths";
constexpr const char *PRECALC_TIMEOUT = "precalc_timeout";

const char * const plugin_defaults[] = {
    MODLAND_ALLMODS_MD5_FILE, "",
    PRECALC_SONGLENGTHS, "FALSE",
    PRECALC_TIMEOUT, "3600",
    nullptr
};

const PreferencesWidget plugin_widgets[] = {
    WidgetLabel("Modland <i>allmods-md5.txt</i> path"),
    WidgetFileEntry("",
        WidgetString(PLUGIN_NAME, MODLAND_ALLMODS_MD5_FILE), WidgetVFileEntry()),
    WidgetLabel("<i>If empty a bundled one is used</i>", WIDGET_CHILD),
    WidgetSeparator(),
    WidgetCheck("Precalc missing song lengths",
        WidgetBool(PLUGIN_NAME, PRECALC_SONGLENGTHS)),
    WidgetSpin("Timeout", WidgetInt(PLUGIN_NAME, "precalc_timeout"), {0, 3600, 5, "<i>seconds</i>"}, WIDGET_CHILD),
    WidgetLabel("<i>Affects (Pro)tracker formats</i>", WIDGET_CHILD),
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

#endif /* PREFS_H_ */
