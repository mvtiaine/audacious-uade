AC_PREREQ(2.59)
AC_INIT([audacious-uade], [0.2])
AC_PROG_CC
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_17

AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE

LT_INIT([disable-static])

PKG_CHECK_MODULES([UADE],[libuade >= 3.0])
AC_SUBST(UADE_CFLAGS)
AC_SUBST(UADE_LIBS)

PKG_CHECK_MODULES([AUDACIOUS],[audacious >= 3.8],
  [ac_audacious_plugindir=`$PKG_CONFIG --variable=plugin_dir audacious`],
  [ac_audacious_plugindir=;])
AC_SUBST(AUDACIOUS_CFLAGS)
AC_SUBST(AUDACIOUS_LIBS)
AC_SUBST(AUDACIOUS_PLUGINDIR, $ac_audacious_plugindir)

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES(Makefile
                src/Makefile)
AC_OUTPUT
