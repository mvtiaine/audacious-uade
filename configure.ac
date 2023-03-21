AC_PREREQ(2.59)
AC_INIT([audacious-uade], [0.5-dev])
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_17

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
                src/Makefile
                src/converter/Makefile)

# AC_CANONICAL_HOST is needed to access the 'host_os' variable
AC_CANONICAL_HOST

build_linux=no
build_windows=no
build_macos=no

# Detect the target system
case "${host_os}" in
    linux*)
        build_linux=yes
        ;;
    cygwin*|mingw*)
        build_windows=yes
        ;;
    darwin*)
        build_macos=yes
        ;;
    *)
        AC_MSG_ERROR(["OS $host_os is not supported"])
        ;;
esac

AM_CONDITIONAL([BUILD_LINUX], [test "$build_linux" = "yes"])
AM_CONDITIONAL([BUILD_WIN32], [test "$build_windows" = "yes"])
AM_CONDITIONAL([BUILD_MACOS], [test "$build_macos" = "yes"])

AC_OUTPUT
