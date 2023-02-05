/*
    Copyright (C) 2014-2023  Matti Tiainen <mvtiaine@cc.hut.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef PREFS_H_
#define PREFS_H_

#include <libaudcore/preferences.h>

#include "common.h"

const char * const plugin_defaults[] = {
   MODLAND_ALLMODS_MD5_FILE, "",
   NULL
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
