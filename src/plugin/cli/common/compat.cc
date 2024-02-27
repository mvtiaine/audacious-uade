// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#ifdef __AMIGA__

#if defined(__CLIB2__)

extern "C" {
// XXX workaround undefined references
#include <assert.h>
#include <stdlib.h>
double __subdf3 (double a, double b) {
    assert(0);
    return 0;
}
long double strtold(const char *restrict nptr, char **restrict endptr) {
    assert(0);
    return 0;
}
}

#endif

#endif // __AMIGA__

#ifdef __AROS__
// https://github.com/sahlberg/libsmb2/blob/master/lib/compat.c
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2020 by Ronnie Sahlberg <ronniesahlberg@gmail.com>
extern "C" {
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
ssize_t writev(int fd, const struct iovec *vector, int count)
{
        /* Find the total number of bytes to be written.  */
        size_t bytes = 0;
        int i;
        char *buffer;
        size_t to_copy;
        char *bp;
		ssize_t bytes_written;

        for (i = 0; i < count; ++i) {
                /* Check for ssize_t overflow.  */
                if (((ssize_t)-1) - bytes < vector[i].iov_len) {
                        errno = EINVAL;
                        return -1;
                }
                bytes += vector[i].iov_len;
        }

        buffer = (char *)malloc(bytes);
        if (buffer == NULL)
                /* XXX I don't know whether it is acceptable to try writing
                the data in chunks.  Probably not so we just fail here.  */
                return -1;

        /* Copy the data into BUFFER.  */
        to_copy = bytes;
        bp = buffer;
        for (i = 0; i < count; ++i) {
                size_t copy = (vector[i].iov_len < to_copy) ? vector[i].iov_len : to_copy;

                memcpy((void *)bp, (void *)vector[i].iov_base, copy);

                bp += copy;
                to_copy -= copy;
                if (to_copy == 0)
                        break;
        }

        bytes_written = write(fd, buffer, bytes);

        free(buffer);
        return bytes_written;
}
}
#endif // __AROS