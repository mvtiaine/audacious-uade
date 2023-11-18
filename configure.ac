AC_PREREQ(2.59)
AC_INIT([audacious-uade], m4_normalize(m4_include([VERSION])), [], [], [https://github.com/mvtiaine/audacious-uade])
AC_PROG_CXX

AM_INIT_AUTOMAKE([tar-ustar dist-bzip2 no-dist-gzip])
AC_CONFIG_MACRO_DIRS([m4])

LT_INIT([disable-static])

# AC_CANONICAL_HOST is needed to access the 'host_os' variable
AC_CANONICAL_HOST

build_linux=no
build_windows=no
build_macos=no
build_bsd=no

# Detect the target system
case "${host_os}" in
    linux*)
        build_linux=yes
        ;;
    cygwin*|mingw*)
        AC_MSG_WARN(["OS $host_os is not supported"])
        build_windows=yes
        ;;
    darwin*)
        build_macos=yes
        ;;
    *bsd*)
        build_bsd=yes
        ;;
    *)
        AC_MSG_WARN(["OS $host_os is not supported"])
        ;;
esac

AM_CONDITIONAL([BUILD_LINUX], [test "$build_linux" = "yes"])
AM_CONDITIONAL([BUILD_WIN32], [test "$build_windows" = "yes"])
AM_CONDITIONAL([BUILD_MACOS], [test "$build_macos" = "yes"])
AM_CONDITIONAL([BUILD_BSD], [test "$build_bsd" = "yes"])

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

if test "$build_linux" = "yes"
then
  PKG_CHECK_MODULES([LIBBSD],[libbsd],
    [ac_libbsd_libs=`$PKG_CONFIG --libs libbsd`],
    [ac_libbsd_libs=;])
  AC_SUBST(LIBBSD_LIBS)
fi

CFLAGS="-g -Os -fPIC -fvisibility=hidden -fdata-sections -ffunction-sections ${CFLAGS}"
CXXFLAGS="-std=c++20 -fvisibility-inlines-hidden -fno-rtti -Wall ${CFLAGS} ${CXXFLAGS}"
if test "$build_macos" = "yes"
then
  LDFLAGS="-Wl,-dead_strip -Wl,-undefined,error ${LDFLAGS}"
  SHREXT="-shrext .dylib"
  AC_SUBST([SHREXT])
else
  LDFLAGS="-Wl,--gc-sections -Wl,--as-needed -Wl,--exclude-libs=ALL -Wl,--discard-all -Wl,--no-undefined -Wl,-O1 ${LDFLAGS}"
fi

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
  [CFLAGS=${CFLAGS} -DDISABLE_BENCODETOOLS -DDISABLE_ZAKALWE -Wno-format -Wno-format-security -Wno-unused-command-line-argument],
  [LDFLAGS=${LDFLAGS} ${LIBBSD_LIBS}],
],[])
UADE_CPPFLAGS="-I\${top_srcdir}/uade/src/frontends/include -I\${top_builddir}/uade/src/frontends/include"
AC_SUBST([UADE_CPPFLAGS])
# TODO shutup libtool warning:
# *** Warning: Linking the shared library uade.la against the
# *** static library uade/src/frontends/libuade/libuade.a is not portable!
UADE_LIBS="\${top_builddir}/uade/src/frontends/libuade/libuade.a"
AC_SUBST([UADE_LIBS])

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES(Makefile
                src/Makefile)

AC_OUTPUT
