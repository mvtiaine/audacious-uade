// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024-2025 Matti Tiainen <mvtiaine@cc.hut.fi>


#ifdef WARPUP
extern "C" {
#include <bits/gthr.h>
// XXX latest mos2wos (1.4) seems broken, but only one with gcc9
// undefined reference to `__gthr_morphos_mutex_lock' etc.
int __gthr_morphos_mutex_lock (__gthread_mutex_t *__mutex) {}
int __gthr_morphos_mutex_unlock (__gthread_mutex_t *__mutex) {}
int __gthr_morphos_active_p (void) { return 0; }
} // extern "C"
#endif // WARPUP
