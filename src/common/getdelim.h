/*
 * Copyright 2021 arnavyc.
 * SPDX-License-Identifier: 0BSD
 *
 * Licensed under the Zero Clause BSD License. See LICENSE.md file in the
 * project root, or https://opensource.org/licenses/0BSD for full license
 * information.
 */

/*
 * To use this single-file library, create a file getdelim.c with the following
 * content: (or just copy src/ay/getdelim.c)
 *
 * #define AY_GETDELIM_IMPLEMENTATION
 * #include "<location of header>"
 */

// fixed restrict->__restrict, file unlocking after EOF etc. -mvtiaine

#ifndef AY_GETDELIM_H
#define AY_GETDELIM_H

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#define IS_POSIX
#endif

#ifdef IS_POSIX
#define _POSIX_C_SOURCE 200809L
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(IS_POSIX)
#include <sys/types.h>
#elif defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
typedef ptrdiff_t ssize_t;
#endif

ssize_t getdelim(char **__restrict lineptr, size_t *__restrict n, int delimiter,
                 FILE *__restrict stream);

ssize_t getline(char **__restrict lineptr, size_t *__restrict n,
                FILE *__restrict stream);

#ifdef AY_GETDELIM_IMPLEMENTATION
#include <errno.h>

#define INITIAL_BUFFSZ 128

static void _lockfile(FILE *__restrict stream) {
#if defined(IS_POSIX)
  flockfile(stream);
#elif defined(_WIN32)
  _lock_file(stream);
#endif
}
static void _unlockfile(FILE *__restrict stream) {
#if defined(IS_POSIX)
    funlockfile(stream);
#elif defined(_WIN32)
    _unlock_file(stream);
#endif
}
static int _getc(FILE *__restrict stream) {
#if defined(IS_POSIX)
  return getc_unlocked(stream);
#elif defined(_WIN32)
  return _getc_nolock(stream);
#else
  return getc(stream);
#endif
}

ssize_t getdelim(char **__restrict lineptr, size_t *__restrict n, int delimiter,
                 FILE *__restrict stream) {
  if (lineptr == NULL || stream == NULL || n == NULL) {
    errno = EINVAL;
    return -1;
  }

  _lockfile(stream);

  int c = _getc(stream);

  if (c == EOF) {
    _unlockfile(stream);
    return -1;
  }

  if (*lineptr == NULL) {
    *lineptr = malloc(INITIAL_BUFFSZ);
    if (*lineptr == NULL) {
      _unlockfile(stream);
      return -1;
    }
    *n = INITIAL_BUFFSZ;
  }

  size_t pos = 0;
  while (c != EOF) {
    if (pos + 1 >= *n) {
      size_t new_size = *n + (*n >> 2);
      if (new_size < INITIAL_BUFFSZ) {
        new_size = INITIAL_BUFFSZ;
      }
      char *new_ptr = realloc(*lineptr, new_size);
      if (new_ptr == NULL) {
        _unlockfile(stream);
        return -1;
      }
      *n = new_size;
      *lineptr = new_ptr;
    }

    ((unsigned char *)(*lineptr))[pos++] = c;
    if (c == delimiter) {
      break;
    }

    c = _getc(stream);
  }

  _unlockfile(stream);

  (*lineptr)[pos] = '\0';
  return pos;
}

ssize_t getline(char **__restrict lineptr, size_t *__restrict n,
                FILE *__restrict stream) {
  return getdelim(lineptr, n, '\n', stream);
}
#endif /* AY_GETDELIM_IMPLEMENTATION */

#endif /* AY_GETDELIM_H */
