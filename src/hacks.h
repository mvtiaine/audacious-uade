#ifndef HACKS_H_
#define HACKS_H_

#include <libaudcore/core.h>

bool_t is_blacklisted_extension(const char *ext);

bool_t is_blacklisted_title(const struct uade_song_info *info);

struct uade_file *amiga_loader_wrapper(const char *name, const char *playerdir, void *context, struct uade_state *state);

#endif /* HACKS_H_ */
