// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2025 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

#include "config.h"

#if PLAYER_uade
#include "../uade/src/frontends/include/uade/options.h"
#else
#define UADE_VERSION "N/A"
#endif

#if PLAYER_libopenmpt
#include <libopenmpt/libopenmpt_version.h>
#endif

#if PLAYER_libxmp
#include <xmp.h>
#endif

static constexpr const char *plugin_copyright =
"audacious-uade " PACKAGE_VERSION " (GPL-2.0-or-later)\n"
"Copyright (c) 2014-2025, Matti Tiainen\n"
"\n"
#if PLAYER_uade
"UADE " UADE_VERSION " (GPL-2.0-or-later)\n"
"https://zakalwe.fi/uade/\n"
"\n"
#endif
#if PLAYER_hivelytracker
"HivelyTracker 1.9 (BSD-3-Clause)\n"
"Copyright (c) 2006-2018, Pete Gordon\n"
"\n"
#endif
#if PLAYER_libdigibooster3
"libdigibooster3 1.2 (BSD-2-Clause)\n"
"Copyright (c) 2014, Grzegorz Kraszewski\n"
"\n"
#endif
#if PLAYER_ft2play || PLAYER_it2play || PLAYER_st3play || PLAYER_st23play
"ft2play, it2play, st3play v1.0.1,\n"
"st23play v0.35 (BSD-3-Clause)\n"
"Copyright (c) 2016-2024, Olav Sørensen\n"
"\n"
#endif
#if PLAYER_protrekkr1 || PLAYER_protrekkr2
"ProTrekkr v1.99e, v2.8.1 (BSD-2-Clause)\n"
"Copyright (C) 2008-2024, Franck Charlet\n"
"\n"
#endif
#if PLAYER_noisetrekker2
"NoiseTrekker2 final by Arguru\n"
"\n"
#endif
#if PLAYER_libopenmpt
"libopenmpt " OPENMPT_API_VERSION_STRING " (BSD-3-Clause)\n"
"https://lib.openmpt.org/libopenmpt/\n"
"\n"
#endif
#if PLAYER_libxmp
"libxmp " XMP_VERSION " (MIT)\n"
"https://xmp.sourceforge.net/\n"
"\n"
#endif
"See README for more information\n";
