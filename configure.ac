AC_PREREQ(2.59)
AC_INIT(
  [audacious-uade],
  m4_normalize(m4_include([VERSION])),
  [https://github.com/mvtiaine/audacious-uade/issues],
  [],
  [https://github.com/mvtiaine/audacious-uade])

CSTD="-std=c17"
CXXSTD="-std=c++20"

AC_PROG_CC
AC_PROG_CXX
# for cross compile support
AX_PROG_CC_FOR_BUILD

AM_INIT_AUTOMAKE([tar-ustar dist-bzip2 no-dist-gzip])
AC_CONFIG_MACRO_DIR([m4])

# AC_CANONICAL_HOST is needed to access the 'host_os' variable
AC_CANONICAL_HOST

build_linux=no
build_windows=no
build_macos=no
build_bsd=no
build_haiku=no
build_amiga=no
build_solaris=no

build_freebsd=no
build_openbsd=no
build_netbsd=no
build_cygwin=no
build_mingw=no
build_amigaos=no
build_morphos=no
build_amigaos4=no
build_warpos=no
build_aros=no

plugin_audacious=yes

static_stdlibs=yes

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
    static_stdlibs=no
    ;;
  netbsd*)
    build_netbsd=yes
    build_bsd=yes
    ;;
  freebsd*)
    build_freebsd=yes
    build_bsd=yes
    static_stdlibs=no
    ;;
  *bsd*|dragonfly*)
    build_bsd=yes
    ;;
  haiku*)
    build_haiku=yes
    static_stdlibs=no
    ;;
  cygwin*)
    build_cygwin=yes
    build_windows=yes
    ;;
  amigaos*)
    plugin_audacious=no
    build_amiga=yes
    case "${host_cpu}" in
      powerpc*)
        if test "$(echo|$CC -dM -E -|grep -q __amigaos4__ && echo yes)" = "yes"
        then
          # TODO compiles but executables crash at __start
          build_amigaos4=yes
          AC_MSG_WARN(["AmigaOS4 target not yet supported"])
        elif test "$(echo|$CC --specs=warpup -dM -E -|grep -q WARPUP && echo yes)" = "yes"
        then
          build_warpos=yes
          # TODO too old GCC (9.3.0) in gcc-mos2wos for proper c++20 support
          # also undefined reference to `__gthr_morphos_active_p' etc.
          AC_MSG_WARN(["WarpOS target not yet supported"])
        else
          # TODO PowerUP support also possible with mos2wos?
          AC_MSG_WARN(["Unsupported PPC AmigaOS target"])
        fi
        ;;
      m68k)
        build_amigaos=yes
        ;;
      *)
       AC_MSG_WARN(["${host_cpu} target not supported with AmigaOS"])
       ;;
    esac
    ;;
  morphos*)
    plugin_audacious=no
    build_morphos=yes
    build_amiga=yes
    ;;
  aros*)
    plugin_audacious=no
    build_aros=yes
    build_amiga=yes
    ;;
  solaris*)
    build_solaris=yes
    ;;
  *)
    AC_MSG_WARN(["$host_os not supported"])
    ;;
esac

AM_CONDITIONAL([BUILD_LINUX], [test "$build_linux" = "yes"])
AM_CONDITIONAL([BUILD_WINDOWS], [test "$build_windows" = "yes"])
AM_CONDITIONAL([BUILD_MACOS], [test "$build_macos" = "yes"])
AM_CONDITIONAL([BUILD_BSD], [test "$build_bsd" = "yes"])
AM_CONDITIONAL([BUILD_FREEBSD], [test "$build_freebsd" = "yes"])
AM_CONDITIONAL([BUILD_OPENBSD], [test "$build_openbsd" = "yes"])
AM_CONDITIONAL([BUILD_NETBSD], [test "$build_netbsd" = "yes"])
AM_CONDITIONAL([BUILD_HAIKU], [test "$build_haiku" = "yes"])
AM_CONDITIONAL([BUILD_CYGWIN], [test "$build_cygwin" = "yes"])
AM_CONDITIONAL([BUILD_MINGW], [test "$build_mingw" = "yes"])
AM_CONDITIONAL([BUILD_AMIGA], [test "$build_amiga" = "yes"])
AM_CONDITIONAL([BUILD_AMIGAOS], [test "$build_amigaos" = "yes"])
AM_CONDITIONAL([BUILD_MORPHOS], [test "$build_morphos" = "yes"])
AM_CONDITIONAL([BUILD_AMIGAOS4], [test "$build_amigaos4" = "yes"])
AM_CONDITIONAL([BUILD_WARPOS], [test "$build_warpos" = "yes"])
AM_CONDITIONAL([BUILD_AROS], [test "$build_aros" = "yes"])
AM_CONDITIONAL([BUILD_SOLARIS], [test "$build_solaris" = "yes"])

AM_CONDITIONAL([PLUGIN_AUDACIOUS], [test "$plugin_audacious" = "yes"])

# defines WORDS_BIGENDIAN
AC_C_BIGENDIAN([
  AM_CONDITIONAL([BIG_ENDIAN],[true])
  AM_CONDITIONAL([LITTLE_ENDIAN],[false])
  AC_DEFINE(WORDS_BIGENDIAN, 1)
],[
  AM_CONDITIONAL([BIG_ENDIAN],[false])
  AM_CONDITIONAL([LITTLE_ENDIAN],[true])
])

AC_COMPILE_IFELSE(
  [AC_LANG_PROGRAM([
    #if defined __FAST_MATH__
    #error Building with -ffast-math and/or -Ofast not supported!
    #endif
  ])],
  [],
  [AC_MSG_ERROR([Building with -ffast-math and/or -Ofast not supported!])])

if test "$plugin_audacious" = "yes"
then
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
CXXFLAGS="$CXXSTD $AUDACIOUS_CFLAGS"
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
fi # PLUGIN_AUDACIOUS

EXTRA_LIBS=""
CPPFLAGS="-D_DEFAULT_SOURCE -D_GNU_SOURCE ${CPPFLAGS}"
CFLAGS="-fvisibility=hidden ${CFLAGS} -fno-exceptions"
# XXX shutup spammy AROS warning: "__PIC__" redefined.
# also AROS and MorphOS doesn't support --gc-sections
if test "$build_aros" != "yes" && test "$build_morphos" != "yes" && test "$build_warpos" != "yes"
then
  CFLAGS="-fPIC -fdata-sections -ffunction-sections ${CFLAGS}"
fi
if test "$build_solaris" = "yes" || test "$build_aros" = "yes"
then
  # on OpenIndiana -std=<any> breaks PATH_MAX for some reason
  # on AROS it breaks asm macros
  CSTD=""
fi

# see AX_PROG_CC_FOR_BUILD
CFLAGS_FOR_BUILD="${CSTD} ${CFLAGS}"

if test "$build_haiku" = "yes"
then
  EXTRA_LIBS="-lbsd -lnetwork"
  # fix constexpr support (needed at least with beta4)
  CXXSTD="-std=c++2b"
elif test "$build_cygwin" = "yes"
then
  EXTRA_LIBS="-lcygwin"
elif test "$build_mingw" = "yes"
then
  CPPFLAGS="-I/usr/include ${CPPFLAGS}"
  CFLAGS="-I/usr/include ${CFLAGS}"
  LIBS="-lws2_32 ${LIBS}"
elif test "$build_solaris" = "yes"
then
  EXTRA_LIBS="-lsocket -lnsl"
elif test "$build_amigaos" = "yes"
then
  # -noixemul uses libnix
  CFLAGS="-noixemul -m68020-60 ${CFLAGS} -fno-PIC"
  # TODO with -O2, -foptimize-sibling-calls causes some exes to crash before main() (songdb/player/precalc)
  CFLAGS="-fno-optimize-sibling-calls ${CFLAGS}"
  # XXX some weirdness with <limits.h>/PATH_MAX not getting defined with libnix
  CPPFLAGS="-DPATH_MAX=1024 ${CPPFLAGS}"
  # XXX fix some c vs c++ header issues with clib2
  CPPFLAGS="-include stdint.h -include math.h ${CPPFLAGS}"
elif test "$build_morphos" = "yes"
then
  CFLAGS="-noixemul ${CFLAGS} -fno-PIC"
  # <cstdint> does not work without this
  CPPFLAGS="-D__STDC_LIMIT_MACROS ${CPPFLAGS}"
elif test "$build_amigaos4" = "yes"
then
  # static libstdc++ needs also postdeps_CXX hack below
  CFLAGS="-athread=native ${CFLAGS}"
  LIBS="-lauto ${LIBS}"
elif test "$build_warpos" = "yes"
then
  CPPFLAGS="--specs=warpup -D__STDC_LIMIT_MACROS ${CPPFLAGS}"
  CFLAGS="--specs=warpup -noixemul -mclib=newlib -pthread ${CFLAGS} -fno-PIC"
  LDFLAGS="--specs=warpup ${LDFLAGS}"
elif test "$build_aros" = "yes"
then
  LIBS="-lpthread ${LIBS}"
  # <cstdint> does not work without this
  CPPFLAGS="-D__STDC_LIMIT_MACROS ${CPPFLAGS}"
fi

# TODO proper freopen() test
if test "$build_haiku" = "yes" || test "$build_openbsd" = "yes" || test "$build_netbsd" = "yes" || test "$build_mingw" = "yes" || test "$build_amiga" = "yes" || test "$build_solaris" = "yes"
then
  CPPFLAGS="-DNO_FREOPEN ${CPPFLAGS}"
fi

CXXFLAGS="-fvisibility-inlines-hidden -Wall ${CFLAGS} ${CXXSTD} ${CXXFLAGS} -fno-rtti -fno-exceptions"
CFLAGS="${CSTD} ${CFLAGS}"

# must be called after C(XX)FLAGS tweaks, as for example on MorphOS -noixemul must be set before LT_INIT
LT_INIT

ld_apple=no
ld_lld=no
ld_gnu=no
ld_solaris=no

linker_version=$( ($LD -v >/dev/null 2>&1 && $LD -v 2>&1) || ($LD -V 2>&1) )
case "$linker_version" in
    GNU*|mold*)
        ld_gnu=yes
        ;;
    LLD*)
        ld_lld=yes
        ;;
    *dyld*|*PROGRAM:ld*PROJECT:ld*)
        ld_apple=yes
        # does not work
        static_stdlibs=no
        ;;
    *Solaris*)
        ld_solaris=yes
        ;;
    *)
        AC_MSG_WARN(["Linker $LD ($linker_version) is not supported"])
        ;;
esac

AM_CONDITIONAL([LD_APPLE], [test "$ld_apple" = "yes"])
AM_CONDITIONAL([LD_LLD], [test "$ld_lld" = "yes"])
AM_CONDITIONAL([LD_GNU], [test "$ld_gnu" = "yes"])
AM_CONDITIONAL([LD_SOLARIS], [test "$ld_solaris" = "yes"])

# static libgcc/libstdc++ used for executables (except on macOS)
AM_CONDITIONAL([STATIC_STDLIBS], [test "$static_stdlibs" = "yes"])

if test "$ld_apple" = "yes"
then
  LDFLAGS="-Wl,-dead_strip -Wl,-dead_strip_dylibs -Wl,-undefined,error -Wl,-no_weak_exports ${LDFLAGS}"
elif test "$ld_solaris" = "yes"
then
  LDFLAGS="-Wl,-B,reduce -Wl,-z,redlocsym -Wl,--no-undefined ${LDFLAGS}"
else
  # assume GNU compatible linker
  LDFLAGS="-Wl,--as-needed -Wl,-O1 ${LDFLAGS}"
  if test "$build_openbsd" = "yes"
  then
    # -Wl,--no-undefined is broken(?) on OpenBSD
    LDFLAGS="-Wl,--gc-sections -Wl,--exclude-libs=ALL -Wl,--discard-all -Wl,--sort-common ${LDFLAGS}"
  elif test "$build_mingw" = "yes" && test "$ld_lld" = "yes"
  then
    # mingw lld linker does not support --exclude-libs=ALL,--discard-all,--sort-common (?)
    LDFLAGS="-Wl,--gc-sections -Wl,--no-undefined ${LDFLAGS}"
  elif test "$build_amigaos" = "yes"
  then
    # no support for --exclude-libs=ALL in bebbos toolchain
    LDFLAGS="-Wl,--gc-sections -Wl,--discard-all -Wl,--no-undefined -Wl,--sort-common ${LDFLAGS}"
  elif test "$build_morphos" = "yes" || test "$build_warpos" = "yes"
  then
    # no support for --exclude-libs=ALL, --gc-sections broken
    LDFLAGS="-Wl,--discard-all -Wl,--no-undefined -Wl,--sort-common ${LDFLAGS}"
  elif test "$build_aros" = "yes"
  then
    # no support for --gc-sections
    LDFLAGS="-Wl,--exclude-libs=ALL -Wl,--discard-all -Wl,--no-undefined  -Wl,--sort-common ${LDFLAGS}"
  else
    LDFLAGS="-Wl,--gc-sections -Wl,--exclude-libs=ALL -Wl,--discard-all -Wl,--no-undefined -Wl,--sort-common ${LDFLAGS}"
  fi
fi

if test "$build_macos" = "yes"
then
  SHREXT="-shrext .dylib"
  AC_SUBST([SHREXT])
elif test "$build_netbsd" = "yes"
then
  # default libtool is broken(?) on NetBSD for C++ shared modules
  postdeps_CXX=`echo " $postdeps_CXX " | sed 's, -lgcc,,g'`
  postdeps_CXX=`echo " $postdeps_CXX " | sed 's, -lc_s,,g'`
  postdeps_CXX=`echo " $postdeps_CXX " | sed 's, -lm_s,,g'`
elif test "$build_amigaos" = "yes" || test "$build_warpos" = "yes"
then
  _UADE_TARGET_OS="--target-os=AmigaOS"
elif test "$build_morphos" = "yes"
then
  _UADE_TARGET_OS="--target-os=MorphOS"
elif test "$build_amigaos4" = "yes"
then
  _UADE_TARGET_OS="--target-os=AmigaOS4"
  # hack to support statically linked libgcc and libstdc++
  postdeps_CXX=`echo " $postdeps_CXX " | sed 's,-lgcc,,g'`
  postdeps_CXX=`echo " $postdeps_CXX " | sed 's,-lstdc++,,g'`
elif test "$build_aros" = "yes"
then
  _UADE_TARGET_OS="--target-os=AROS"
fi

uade_prefix=${prefix}
if test "$uade_prefix" = "NONE"
then
  uade_prefix=/usr/local
fi
_UADE_CPPFLAGS="${CPPFLAGS} -DDISABLE_BENCODETOOLS -DDISABLE_ZAKALWE"
_UADE_CFLAGS="${CFLAGS} -Wno-format -Wno-format-security -Wno-unused-command-line-argument"
_UADE_LDFLAGS="${LDFLAGS} ${EXTRA_LIBS}"
if test "$static_stdlibs" = "yes"
then
  _UADE_LDFLAGS="-static-libgcc ${_UADE_LDFLAGS}"
fi
AX_SUBDIRS_CONFIGURE([uade], [
  [--without-uade123], [--without-uadesimple], [--without-write-audio],[--without-uadefs],[--without-libuade],
  [--prefix=\${uade_prefix}/share/audacious-uade],
  [--bindir=/dev/null], [--mandir=/dev/null], [--sharedir=/dev/null],
  [--bencode-tools-prefix=/dev/null], [--libzakalwe-prefix=/dev/null],
  [--target-ar=${AR}],[--target-cc=${CC}],[CC=${CC_FOR_BUILD}],[CXX=${CXX}],
  [CFLAGS=${_UADE_CPPFLAGS} ${_UADE_CFLAGS}],
  [LDFLAGS=${_UADE_LDFLAGS} ],
  [${_UADE_TARGET_OS}],
],[])
UADE_CPPFLAGS="-I\${top_srcdir}/uade/src/frontends/include -I\${top_builddir}/uade/src/frontends/include"
AC_SUBST([UADE_CPPFLAGS])
UADE_LIBS="\${top_builddir}/uade/src/frontends/common/libuade.a ${EXTRA_LIBS}"
AC_SUBST([UADE_LIBS])

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES(Makefile
                src/Makefile)

AC_OUTPUT
