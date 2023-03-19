// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#ifndef HACKS_H_
#define HACKS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>

bool is_blacklisted_extension(const char *ext);

bool is_blacklisted_title(const struct uade_song_info *info);

bool is_blacklisted_filename(const char *name);

struct uade_file *amiga_loader_wrapper(const char *name, const char *playerdir, void *context, struct uade_state *state);

#ifdef __cplusplus
}
#endif
#endif /* HACKS_H_ */
