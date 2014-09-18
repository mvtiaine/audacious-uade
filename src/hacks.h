/*
    Copyright (C) 2014  Matti Tiainen <mvtiaine@cc.hut.fi>

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

#ifndef HACKS_H_
#define HACKS_H_

#include <libaudcore/core.h>

bool_t is_blacklisted_extension(const char *ext);

bool_t is_blacklisted_title(const struct uade_song_info *info);

struct uade_file *amiga_loader_wrapper(const char *name, const char *playerdir, void *context, struct uade_state *state);

#endif /* HACKS_H_ */
