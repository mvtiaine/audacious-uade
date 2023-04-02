// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#ifndef PREFS_H_
#define PREFS_H_

#include <libaudcore/preferences.h>

#include "common.h"

const char * const plugin_defaults[] = {
   MODLAND_ALLMODS_MD5_FILE, "",
    nullptr
};

const PreferencesWidget prefs_widgets[] = {
  { WidgetFileEntry("Modland allmods-md5.txt file path",
        WidgetString(PLUGIN_NAME, MODLAND_ALLMODS_MD5_FILE)) },
  { WidgetCheck("Precalc missing song lengths (heavy)",
        WidgetBool(PLUGIN_NAME, PRECALC_SONGLENGTHS)),
  }
};

const PluginPreferences plugin_prefs = {{ prefs_widgets }};

#endif /* PREFS_H_ */
