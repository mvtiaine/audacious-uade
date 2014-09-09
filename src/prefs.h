#ifndef PREFS_H_
#define PREFS_H_

#include <audacious/preferences.h>

#include "common.h"

const char * const plugin_defaults[] = {
   MODLAND_ALLMODS_MD5_FILE, "",
   NULL
};

// TODO should have a file selector
const PreferencesWidget prefs_widgets[] = {
  { WIDGET_ENTRY, "Modland allmods_md5.txt file path",
    .cfg_type = VALUE_STRING, .csect = PLUGIN_NAME, .cname = MODLAND_ALLMODS_MD5_FILE }
};

const PluginPreferences plugin_prefs = {
   .widgets = prefs_widgets,
   .n_widgets = ARRAY_LEN (prefs_widgets)
};


#endif /* PREFS_H_ */
