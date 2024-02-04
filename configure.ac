AC_PREREQ(2.59)
AC_INIT(
  [audacious-uade],
  m4_normalize(m4_include([VERSION])),
  [https://github.com/mvtiaine/audacious-uade/issues],
  [],
  [https://github.com/mvtiaine/audacious-uade])
AC_PROG_CC
AC_PROG_CXX

AM_INIT_AUTOMAKE([tar-ustar dist-bzip2 no-dist-gzip])
AC_CONFIG_MACRO_DIR([m4])

LT_INIT

# AC_CANONICAL_HOST is needed to access the 'host_os' variable
AC_CANONICAL_HOST

build_linux=no
build_windows=no
build_macos=no
build_bsd=no
build_openbsd=no
build_netbsd=no
build_haiku=no
build_cygwin=no
build_mingw=no

# Detect the target system
case "${host_os}" in
    linux*)
        build_linux=yes
        ;;
    mingw*|msys*)
        build_mingw=yes
        build_windows=yes
        ;;
    darwin*)
        build_macos=yes
        ;;
    openbsd*)
        build_openbsd=yes
        build_bsd=yes
        ;;
    netbsd*)
        build_netbsd=yes
        build_bsd=yes
        ;;
    *bsd*|dragonfly*)
        build_bsd=yes
        ;;
    haiku*)
        build_haiku=yes
        ;;
    cygwin*)
        build_cygwin=yes
        build_windows=yes
        ;;
    *)
        AC_MSG_WARN(["OS $host_os is not supported"])
        ;;
esac

AM_CONDITIONAL([BUILD_LINUX], [test "$build_linux" = "yes"])
AM_CONDITIONAL([BUILD_WINDOWS], [test "$build_windows" = "yes"])
AM_CONDITIONAL([BUILD_MACOS], [test "$build_macos" = "yes"])
AM_CONDITIONAL([BUILD_BSD], [test "$build_bsd" = "yes"])
AM_CONDITIONAL([BUILD_OPENBSD], [test "$build_openbsd" = "yes"])
AM_CONDITIONAL([BUILD_NETBSD], [test "$build_netbsd" = "yes"])
AM_CONDITIONAL([BUILD_HAIKU], [test "$build_haiku" = "yes"])
AM_CONDITIONAL([BUILD_CYGWIN], [test "$build_cygwin" = "yes"])
AM_CONDITIONAL([BUILD_MINGW], [test "$build_mingw" = "yes"])

ld_apple=no
ld_lld=no
ld_gnu=no

linker_version=$($LD -v 2>&1)
case "$linker_version" in
    GNU*|mold*)
	ld_gnu=yes
        ;;
    LLD*)
        ld_lld=yes
        ;;
    *dyld*)
        ld_apple=yes
        ;;
    *)
        AC_MSG_WARN(["Linker $LD ($linker_version) is not supported"])
        ;;
esac

AM_CONDITIONAL([LD_APPLE], [test "$ld_apple" = "yes"])
AM_CONDITIONAL([LD_LLD], [test "$ld_lld" = "yes"])
AM_CONDITIONAL([LD_GNU], [test "$ld_gnu" = "yes"])

# defines WORDS_BIGENDIAN
AC_C_BIGENDIAN

AC_COMPILE_IFELSE(
  [AC_LANG_PROGRAM([
    #if defined __FAST_MATH__
    #error Building with -ffast-math and/or -Ofast not supported!
    #endif
  ])],
  [],
  [AC_MSG_ERROR([Building with -ffast-math and/or -Ofast not supported!])])

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

saved_cxxflags="$CXXFLAGS"
CXXFLAGS="-std=c++20 $AUDACIOUS_CFLAGS"
AC_LANG_PUSH(C++)
AC_COMPILE_IFELSE(
  [AC_LANG_PROGRAM([
    #include <libaudcore/tuple.h>
    int test() {
      return Tuple::Publisher;
    }
  ])],
  [AC_DEFINE([AUDACIOUS_HAS_PUBLISHER], [1], [Defined if Audacious version supports Publisher tuple.])],
  []
)
AC_COMPILE_IFELSE(
  [AC_LANG_PROGRAM([
    #include <libaudcore/tuple.h>
    int test() {
      return Tuple::Channels;
    }
  ])],
  [AC_DEFINE([AUDACIOUS_HAS_CHANNELS], [1], [Defined if Audacious version supports Channels tuple.])],
  []
)
AC_COMPILE_IFELSE(
  [AC_LANG_PROGRAM([
    #include <libaudcore/plugin.h>
    int test() {
      return _AUD_PLUGIN_DEFAULT_PRIO;
    }
  ])],
  [AC_DEFINE([AUDACIOUS_HAS_DEFAULT_PRIO], [1], [Defined if Audacious version supports _AUD_PLUGIN_DEFAULT_PRIO.])],
  []
)
AC_LANG_POP
CXXFLAGS="$saved_cxxflags"

EXTRA_LIBS=""
if test "$build_haiku" = "yes"
then
  EXTRA_LIBS="-lbsd -lnetwork"
elif test "$build_cygwin" = "yes"
then
  EXTRA_LIBS="-lcygwin"
elif test "$build_mingw" = "yes"
then
  CPPFLAGS="-I/usr/include ${CPPFLAGS}"
  CFLAGS="-I/usr/include ${CFLAGS}"
  LIBS="$LIBS -lws2_32"
fi

CPPFLAGS="-D_DEFAULT_SOURCE -D_GNU_SOURCE ${CPPFLAGS}"
# TODO proper freopen() test
if test "$build_haiku" = "yes" || test "$build_openbsd" = "yes" || test "$build_netbsd" = "yes" || test "$build_mingw" = "yes"
then
  CPPFLAGS="-DNO_FREOPEN ${CPPFLAGS}"
fi

CFLAGS="-fPIC -fvisibility=hidden -fdata-sections -ffunction-sections ${CFLAGS}"
CXXFLAGS="-std=c++20 -fvisibility-inlines-hidden -fno-rtti -Wall ${CFLAGS} ${CXXFLAGS}"

if test "$ld_apple" = "yes"
then
  LDFLAGS="-Wl,-dead_strip -Wl,-undefined,error ${LDFLAGS}"
else
  # assume GNU compatible linker
  if test "$build_openbsd" = "yes"
  then
    # -Wl,--no-undefined is broken(?) on OpenBSD
    LDFLAGS="-Wl,--gc-sections -Wl,--as-needed -Wl,--exclude-libs=ALL -Wl,--discard-all -Wl,-O1 ${LDFLAGS}"
  elif test "$build_mingw" = "yes" && test "$ld_lld" = "yes"
  then
    # mingw lld linker does not support --exclude-libs=ALL or --discard-all (?)
    LDFLAGS="-Wl,--gc-sections -Wl,--as-needed -Wl,--no-undefined -Wl,-O1 ${LDFLAGS}"
  else
    LDFLAGS="-Wl,--gc-sections -Wl,--as-needed -Wl,--exclude-libs=ALL -Wl,--discard-all -Wl,--no-undefined -Wl,-O1 ${LDFLAGS}"
  fi
fi

if test "$build_apple" = "yes"
then
  SHREXT="-shrext .dylib"
  AC_SUBST([SHREXT])
elif test "$build_netbsd" = "yes"
then
  # default libtool is broken(?) on NetBSD for C++ shared modules
  postdeps_CXX=`echo " $postdeps_CXX " | sed 's, -lgcc,,g'`
  postdeps_CXX=`echo " $postdeps_CXX " | sed 's, -lc_s,,g'`
  postdeps_CXX=`echo " $postdeps_CXX " | sed 's, -lm_s,,g'`
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
  [--target-ar=${AR}],[CC=${CC}],[CXX=${CXX}],
  [CFLAGS=${CFLAGS} ${CPPFLAGS} -DDISABLE_BENCODETOOLS -DDISABLE_ZAKALWE -Wno-format -Wno-format-security -Wno-unused-command-line-argument],
  [LDFLAGS=${LDFLAGS} ${EXTRA_LIBS}],
],[])
UADE_CPPFLAGS="-I\${top_srcdir}/uade/src/frontends/include -I\${top_builddir}/uade/src/frontends/include"
AC_SUBST([UADE_CPPFLAGS])
# TODO shutup libtool warning:
# *** Warning: Linking the shared library uade.la against the
# *** static library uade/src/frontends/libuade/libuade.a is not portable!
UADE_LIBS="\${top_builddir}/uade/src/frontends/libuade/libuade.a ${EXTRA_LIBS}"
AC_SUBST([UADE_LIBS])

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES(Makefile
                src/Makefile)

AC_OUTPUT
