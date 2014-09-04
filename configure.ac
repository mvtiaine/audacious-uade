AC_PREREQ(2.59)
AC_INIT([audacious-uade], [0.1])
AC_PROG_CC

AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE

LT_INIT([disable-static])

PKG_CHECK_MODULES([UADE],[libuade >= 2.13])
AC_SUBST(UADE_CFLAGS)
AC_SUBST(UADE_LIBS)

PKG_CHECK_MODULES([AUDACIOUS],[audacious >= 3.5 audacious < 3.6],
  [ac_audacious_plugindir=`$PKG_CONFIG --variable=plugin_dir audacious`],
  [ac_audacious_plugindir=;])
AC_SUBST(AUDACIOUS_CFLAGS)
AC_SUBST(AUDACIOUS_LIBS)
AC_SUBST(AUDACIOUS_PLUGINDIR, $ac_audacious_plugindir)

AC_CONFIG_FILES(Makefile
                src/Makefile)
AC_OUTPUT