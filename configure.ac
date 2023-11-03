AC_PREREQ(2.59)
AC_INIT([audacious-uade], m4_normalize(m4_include([VERSION])))
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_17

AM_INIT_AUTOMAKE([tar-ustar dist-bzip2 no-dist-gzip])
AC_CONFIG_MACRO_DIRS([m4])

LT_INIT([disable-static])

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

AC_ARG_WITH([system-libuade],
  AS_HELP_STRING([--with-system-libuade],[Use system libuade instead of bundled one]),
  [system_libuade="yes"])
AS_IF([test "$system_libuade" = "yes"], [AC_DEFINE([SYSTEM_LIBUADE], [1], [Use system libuade])])
AM_CONDITIONAL([SYSTEM_LIBUADE], [test "$system_libuade" = "yes"])

PKG_CHECK_MODULES([AUDACIOUS],[audacious >= 3.8],
  [ac_audacious_plugindir=`$PKG_CONFIG --variable=plugin_dir audacious`],
  [ac_audacious_plugindir=;])
AC_SUBST(AUDACIOUS_CFLAGS)
AC_SUBST(AUDACIOUS_LIBS)

AC_MSG_CHECKING([for audacious plugindir])
AC_ARG_WITH([audacious-plugindir],
  [--with-audacious-plugindir=<plugindir> installs uade plugin under <plugindir>/Input],
  [if test x"$withval" != x ; then
     AUDACIOUS_PLUGINDIR="$withval"
   fi],
   [AUDACIOUS_PLUGINDIR=$ac_audacious_plugindir])
AC_SUBST(AUDACIOUS_PLUGINDIR)
AC_MSG_RESULT($AUDACIOUS_PLUGINDIR)

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES(Makefile
                src/Makefile
                src/converter/Makefile
                src/songend/Makefile)

AC_MSG_CHECKING([whether to use system libuade])
if test "$system_libuade" = "yes"
then
  AC_MSG_RESULT(yes)
  PKG_CHECK_MODULES([UADE],[libuade >= 3.0],
    [ac_uade_datadir=`$PKG_CONFIG --variable=data_directory libuade`],
    [ac_uade_datadir=;])
  AC_SUBST(UADE_CFLAGS)
  AC_SUBST(UADE_LIBS)
  AC_SUBST(UADE_SYSTEM_DATADIR, $ac_uade_datadir)
else
  AC_MSG_RESULT(no)
  if ! test -d "$srcdir/uade"; then
    AC_MSG_ERROR([Could not find uade submodule. Please retry after running: git submodule update --init --recursive])
  fi
  if test "$build_linux" = "yes"
  then
    PKG_CHECK_MODULES([LIBBSD],[libbsd],
      [ac_libbsd_libs=`$PKG_CONFIG --libs libbsd`],
      [ac_libbsd_libs=;])
    export LDFLAGS="$ac_libbsd_libs ${LDFLAGS}"
  fi

  export CFLAGS="-DDISABLE_BENCODETOOLS -DDISABLE_ZAKALWE -fvisibility=hidden -fdata-sections -ffunction-sections ${CFLAGS} -Wno-format -Wno-format-security"
  uade_prefix=${prefix}
  if test "$uade_prefix" = "NONE"
  then
    uade_prefix=/usr/local
  fi
  AX_SUBDIRS_CONFIGURE([uade], [
    [--without-uade123], [--without-uadesimple], [--without-write-audio],[--without-uadefs],
    [--prefix=\${uade_prefix}/share/audacious-uade],
    [--bindir=/dev/null], [--mandir=/dev/null], [--sharedir=/dev/null],
    [--bencode-tools-prefix=/dev/null], [--libzakalwe-prefix=/dev/null],
  ], [])
  UADE_CFLAGS="-I\${top_srcdir}/uade/src/frontends/include -I\${top_builddir}/uade/src/frontends/include"
  # TODO shutup libtool warning:
  # *** Warning: Linking the shared library uade.la against the
  # *** static library uade/src/frontends/libuade/libuade.a is not portable!
  UADE_LIBS="\${top_builddir}/uade/src/frontends/libuade/libuade.a"
fi

AC_OUTPUT
