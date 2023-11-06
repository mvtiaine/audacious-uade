// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#ifndef HACKS_H_
#define HACKS_H_

#include <string>

using namespace std;

bool is_blacklisted_extension(const string &filename, const string &ext);

bool is_blacklisted_title(const struct uade_song_info *info);

bool is_blacklisted_md5(const string &md5hex);

bool is_blacklisted_songdb(const string &md5hex);

bool is_octamed(const struct uade_song_info *info);

bool is_vss(const struct uade_song_info *info);

bool allow_songend_error(const struct uade_song_info *info);

#if __cplusplus
extern "C"
{
#endif

struct uade_file *amiga_loader_wrapper(const char *name, const char *playerdir, void *context, struct uade_state *state);

#if __cplusplus
}
#endif

#endif /* HACKS_H_ */
