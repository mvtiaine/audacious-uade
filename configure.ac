AC_PREREQ(2.59)
AC_INIT(
  [audacious-uade],
  m4_normalize(m4_include([VERSION])),
  [https://github.com/mvtiaine/audacious-uade/issues],
  [],
  [https://github.com/mvtiaine/audacious-uade])

AC_PROG_CC
AC_PROG_CXX
# for cross compile support
AX_PROG_CC_FOR_BUILD

AM_INIT_AUTOMAKE([tar-ustar dist-bzip2 no-dist-gzip])
AC_CONFIG_MACRO_DIR([m4])

# AC_CANONICAL_HOST is needed to access the 'host_os' variable
AC_CANONICAL_HOST

# gotta catch 'em all
build_linux=no
build_android=no
build_meego=no
build_webos=no

build_windows=no
build_cygwin=no
build_mingw=no

build_darwin=no
build_macos=no
build_ios=no

build_bsd=no
build_freebsd=no
build_openbsd=no
build_netbsd=no
build_dragonflybsd=no

build_haiku=no

build_amiga=no
build_amigaos=no
build_morphos=no
build_amigaos4=no
build_warpos=no
build_aros=no

build_solaris=no
build_irix=no
build_aix=no
build_qnx=no

build_fuchsia=no
build_hurd=no
build_riscos=no
build_freemint=no
build_dos=no

build_cosmocc=no
build_wasix=no
build_emscripten=no

plugin_audacious=yes

static_stdlibs=yes

# Detect the target system
# emscripten and cosmocc not detectable via host* variables ?
case "${CC}" in
  *emcc)
    # see scripts/crossbuild/emscripten.sh
    plugin_audacious=no
    build_emscripten=yes
    # avoid unsupported linker options
    static_stdlibs=no
    # needs custom UADE support
    AC_MSG_WARN(["Emscripten target not yet supported"])
    ;;
  *cosmo*cc)
    # see scripts/crossbuild/cosmocc.sh
    plugin_audacious=no
    build_cosmocc=yes
    ;;
  *)
    case "${host_os}" in
      *android)
        #see scripts/crossbuild/android.sh
        plugin_audacious=no
        build_android=yes
        build_linux=yes
        AC_MSG_WARN(["Android binaries have not been runtime tested"])
        ;;
      linux*)
        build_linux=yes
        case "${host_vendor}" in
          webos)
            #see scripts/crossbuild/webos.sh
            plugin_audacious=no
            build_webos=yes
            AC_MSG_WARN(["webOS binaries have not been runtime tested"])
            ;;
          meego)
            #see scripts/crossbuild/sailfish.sh
            plugin_audacious=no
            build_meego=yes
            ;;
        esac
        ;;
      mingw*|msys*)
        build_mingw=yes
        build_windows=yes
        ;;
      cygwin*)
        build_cygwin=yes
        build_windows=yes
        ;;
      darwin*)
        build_darwin=yes
        case "${CFLAGS}" in
          # see scripts/crossbuild/ios.sh
          *iPhoneOS*|*iPhoneSimulator*|*XROS*|*XRSimulator*)
            plugin_audacious=no
            build_ios=yes
            AC_MSG_WARN(["iOS binaries have not been runtime tested"])
            ;;
          *WatchOS*|*WatchSimulator*|*AppleTVOS*|*AppleTVSimulator*)
            plugin_audacious=no
            build_ios=yes
            # no fork/exec available, needs custom UADE support
            AC_MSG_WARN(["WatchOS and AppleTVOS target not yet supported"])
            ;;
          *)
            build_macos=yes
            ;;
        esac
        ;;
      *bsd*|dragonfly*)
        build_bsd=yes
        case "${host_os}" in
          openbsd*)
            build_openbsd=yes
            static_stdlibs=no
            ;;
          netbsd*)
            build_netbsd=yes
            ;;
          freebsd*)
            build_freebsd=yes
            static_stdlibs=no
            ;;
          dragonfly*)
            build_dragonflybsd=yes
            ;;
        esac
        ;;
      haiku*)
        build_haiku=yes
        static_stdlibs=no
        ;;
      amigaos*)
        plugin_audacious=no
        build_amiga=yes
        case "${host_cpu}" in
          powerpc*)
            if test "$(echo|$CC -dM -E -|grep -q __amigaos4__ && echo yes)" = "yes" ; then
              # see scripts/crossbuild/amigaos4.sh
              # TODO compiles but executables crash at __start
              build_amigaos4=yes
              AC_MSG_WARN(["AmigaOS4 target not yet supported"])
            elif test "$(echo|$CC --specs=warpup -dM -E - 2>/dev/null|grep -q WARPUP && echo yes)" = "yes" ; then
              # see scripts/crossbuild/warpos.sh
              build_warpos=yes
              AC_MSG_WARN(["WarpOS binaries have not been runtime tested"])
            else
              # TODO PowerUP support also possible with mos2wos?
              AC_MSG_WARN(["Unsupported PPC AmigaOS target"])
            fi
            ;;
          m68k)
            # see scripts/crossbuild/amigaos.sh
            build_amigaos=yes
            ;;
          *)
           AC_MSG_WARN(["${host_cpu} target not supported with AmigaOS"])
           ;;
        esac
        ;;
      morphos*)
        # see scripts/crossbuild/morphos.sh
        plugin_audacious=no
        build_amiga=yes
        if test "$(echo|$CC --specs=warpup -dM -E - 2>/dev/null|grep -q WARPUP && echo yes)" = "yes" ; then
          # see scripts/crossbuild/warpos.sh
          build_warpos=yes
          AC_MSG_WARN(["WarpOS binaries have not been runtime tested"])
        else
          build_morphos=yes
        fi
        ;;
      aros*)
        # see scripts/crossbuild/aros.sh
        plugin_audacious=no
        build_aros=yes
        build_amiga=yes
        ;;
      solaris*)
        build_solaris=yes
        ;;
      irix*)
        # see scripts/crossbuild/irix.sh
        # audacious not tested
        plugin_audacious=no
        build_irix=yes
        # compiles, but does not work properly (*printf() seems borked) (compilertron (sgug-rse gcc9))
        AC_MSG_WARN(["IRIX target not yet supported"])
        ;;
      aix*)
        # audacious not tested
        plugin_audacious=no
        build_aix=yes
        ;;
      *qnx*)
        # audacious not tested
        plugin_audacious=no
        build_qnx=yes
        ;;
      *fuchsia*)
        # see scripts/crossbuild/fuchsia.sh
        plugin_audacious=no
        build_fuchsia=yes
        AC_MSG_WARN(["Fuchsia binaries have not been runtime tested"])
        ;;
      wasi*)
        # see scripts/crossbuild/wasix.sh
        plugin_audacious=no
        build_wasix=yes
        # avoid unsupported linker options
        static_stdlibs=no
        # compiles, but binaries either don't work or some even fail to run
        AC_MSG_WARN(["Wasix target not yet supported"])
        ;;
      gnu0.9)
        build_hurd=yes
        ;;
      mint)
        #see scripts/crossbuild/freemint.sh
        plugin_audacious=no
        build_freemint=yes
        AC_MSG_WARN(["FreeMiNT binaries have not been runtime tested"])
        ;;
      *djgpp)
        #see scripts/crossbuild/dos.sh
        plugin_audacious=no
        build_dos=yes
        # needs custom UADE support (at minimum)
        AC_MSG_WARN(["DOS/DJGPP target not yet supported"])
        ;;
      *)
        # inverted os/vendor?
        case "${host_vendor}" in
          riscos)
            # see scripts/crossbuild/riscos.sh
            plugin_audacious=no
            build_riscos=yes
            AC_MSG_WARN(["RISC OS binaries have not been runtime tested"])
            ;;
          *)
            AC_MSG_WARN(["$host_cpu-$host_vendor-$host_os not supported"])
            ;;
        esac
        ;;
    esac
    ;;
esac

AM_CONDITIONAL([BUILD_LINUX], [test "$build_linux" = "yes"])
AM_CONDITIONAL([BUILD_ANDROID], [test "$build_android" = "yes"])
AM_CONDITIONAL([BUILD_MEEGO], [test "$build_meego" = "yes"])
AM_CONDITIONAL([BUILD_WEBOS], [test "$build_webos" = "yes"])
AM_CONDITIONAL([BUILD_WINDOWS], [test "$build_windows" = "yes"])
AM_CONDITIONAL([BUILD_CYGWIN], [test "$build_cygwin" = "yes"])
AM_CONDITIONAL([BUILD_MINGW], [test "$build_mingw" = "yes"])
AM_CONDITIONAL([BUILD_DARWIN], [test "$build_darwin" = "yes"])
AM_CONDITIONAL([BUILD_MACOS], [test "$build_macos" = "yes"])
AM_CONDITIONAL([BUILD_IOS], [test "$build_ios" = "yes"])
AM_CONDITIONAL([BUILD_BSD], [test "$build_bsd" = "yes"])
AM_CONDITIONAL([BUILD_FREEBSD], [test "$build_freebsd" = "yes"])
AM_CONDITIONAL([BUILD_OPENBSD], [test "$build_openbsd" = "yes"])
AM_CONDITIONAL([BUILD_NETBSD], [test "$build_netbsd" = "yes"])
AM_CONDITIONAL([BUILD_DRAGONFLYBSD], [test "$build_dragonflybsd" = "yes"])
AM_CONDITIONAL([BUILD_HAIKU], [test "$build_haiku" = "yes"])
AM_CONDITIONAL([BUILD_AMIGA], [test "$build_amiga" = "yes"])
AM_CONDITIONAL([BUILD_AMIGAOS], [test "$build_amigaos" = "yes"])
AM_CONDITIONAL([BUILD_MORPHOS], [test "$build_morphos" = "yes"])
AM_CONDITIONAL([BUILD_AMIGAOS4], [test "$build_amigaos4" = "yes"])
AM_CONDITIONAL([BUILD_WARPOS], [test "$build_warpos" = "yes"])
AM_CONDITIONAL([BUILD_AROS], [test "$build_aros" = "yes"])
AM_CONDITIONAL([BUILD_SOLARIS], [test "$build_solaris" = "yes"])
AM_CONDITIONAL([BUILD_IRIX], [test "$build_irix" = "yes"])
AM_CONDITIONAL([BUILD_AIX], [test "$build_aix" = "yes"])
AM_CONDITIONAL([BUILD_QNX], [test "$build_qnx" = "yes"])
AM_CONDITIONAL([BUILD_FUCHSIA], [test "$build_fuchsia" = "yes"])
AM_CONDITIONAL([BUILD_HURD], [test "$build_hurd" = "yes"])
AM_CONDITIONAL([BUILD_RISCOS], [test "$build_riscos" = "yes"])
AM_CONDITIONAL([BUILD_FREEMINT], [test "$build_freemint" = "yes"])
AM_CONDITIONAL([BUILD_DOS], [test "$build_dos" = "yes"])
AM_CONDITIONAL([BUILD_COSMOCC], [test "$build_cosmocc" = "yes"])
AM_CONDITIONAL([BUILD_WASIX], [test "$build_wasix" = "yes"])
AM_CONDITIONAL([BUILD_EMSCRIPTEN], [test "$build_emscripten" = "yes"])

AM_CONDITIONAL([PLUGIN_AUDACIOUS], [test "$plugin_audacious" = "yes"])

AC_LANG_PUSH([C])
for version in 23 2x 17 11 1x 99 9x; do
  CSTD="-std=c${version}"
  AX_CHECK_COMPILE_FLAG([${CSTD}],[break],[CSTD=none])
done
AC_LANG_POP
AS_IF([test "$CSTD" == none], [
  AC_MSG_ERROR([Could not enable at least C9x (C99) - upgrade your compiler])
])

AC_LANG_PUSH([C++])
for version in 23 2b 20 2a 17 1z; do
  CXXSTD="-std=c++${version}"
  AX_CHECK_COMPILE_FLAG([${CXXSTD}],[break],[CXXSTD=none])
done
AC_LANG_POP
AS_IF([test "$CXXSTD" == none], [
  AC_MSG_ERROR([Could not enable at least C++1z (C++17) - upgrade your compiler])
])

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

if test "$plugin_audacious" = "yes" ; then
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
CFLAGS="-fvisibility=hidden ${CFLAGS} -fno-exceptions -fno-asynchronous-unwind-tables"
# XXX shutup spammy AROS warning: "__PIC__" redefined.
# also AROS, MorphOS, IRIX/compilertron and AIX linker don't support --gc-sections
if test "$build_aros" != "yes" && test "$build_morphos" != "yes" && test "$build_warpos" != "yes" && test "$build_irix" != "yes" && test "$build_aix" != "yes" && test "$build_freemint" != "yes" ; then
  CFLAGS="-fPIC -fdata-sections -ffunction-sections ${CFLAGS}"
fi
if test "$build_solaris" = "yes" || test "$build_aros" = "yes" ; then
  # on OpenIndiana -std=<any> breaks PATH_MAX for some reason
  # on AROS it breaks asm macros
  CSTD=""
fi

# see AX_PROG_CC_FOR_BUILD
CFLAGS_FOR_BUILD="${CSTD} ${CFLAGS}"

if test "$build_haiku" = "yes" ; then
  EXTRA_LIBS="-lbsd -lnetwork"
elif test "$build_cygwin" = "yes" ; then
  EXTRA_LIBS="-lcygwin"
elif test "$build_mingw" = "yes" ; then
  CPPFLAGS="-I/usr/include ${CPPFLAGS}"
  CFLAGS="-I/usr/include ${CFLAGS}"
  LIBS="-lws2_32 ${LIBS}"
elif test "$build_solaris" = "yes" ; then
  EXTRA_LIBS="-lsocket -lnsl"
elif test "$build_irix" = "yes" ; then
  # irix(6.5.0)+gcc9(compilertron) quirks
  CPPFLAGS="-D_XOPEN_SOURCE=600 -D_POSIX_SOURCE -D_SGI_SOURCE -D__EXTENSIONS__ -DPATH_MAX=1024 ${CPPFLAGS}"
  EXTRA_LIBS="-lgen"
elif test "$build_aix" = "yes" ; then
  # -pthread needed for std::mutex support
  CFLAGS="-pthread ${CFLAGS}"
  LDFLAGS="-pthread ${LDFLAGS}"
elif test "$build_qnx" = "yes" ; then
  # qnx(6.5.0)+gcc10(pkgsrc) quirks
  CPPFLAGS="-D_XOPEN_SOURCE=600 -D_QNX_SOURCE -D_HAS_C9X -D__UINTPTR_TYPE__=uintptr_t -include stdint.h -include math.h ${CPPFLAGS}"
  # dwarf version >= 4 not supported by linker (qnx 6.5.0)
  CFLAGS="-gdwarf-3 ${CFLAGS}"
  LIBS="-lsocket ${LIBS}"
elif test "$build_hurd" = "yes" ; then
  # PATH_MAX not defined
  CPPFLAGS="-DPATH_MAX=1024 ${CPPFLAGS}"
elif test "$build_amigaos" = "yes" ; then
  # -noixemul uses libnix
  CFLAGS="-noixemul -m68020-60 ${CFLAGS} -fno-PIC"
  # TODO with -O2, -foptimize-sibling-calls causes some exes to crash before main() (songdb/player/precalc)
  CFLAGS="-fno-optimize-sibling-calls ${CFLAGS}"
  # XXX some weirdness with <limits.h>/PATH_MAX not getting defined with libnix
  CPPFLAGS="-DPATH_MAX=1024 ${CPPFLAGS}"
  # XXX fix some c vs c++ header issues with clib2
  CPPFLAGS="-include stdint.h -include math.h ${CPPFLAGS}"
elif test "$build_morphos" = "yes" ; then
  CFLAGS="-noixemul ${CFLAGS} -fno-PIC"
  # <cstdint> does not work without this
  CPPFLAGS="-D__STDC_LIMIT_MACROS ${CPPFLAGS}"
elif test "$build_amigaos4" = "yes" ; then
  # static libstdc++ needs also postdeps_CXX hack below
  CFLAGS="-athread=native ${CFLAGS}"
  LIBS="-lauto ${LIBS}"
elif test "$build_warpos" = "yes" ; then
  CPPFLAGS="--specs=warpup -D__STDC_LIMIT_MACROS ${CPPFLAGS}"
  CFLAGS="--specs=warpup -noixemul -mclib=newlib -pthread ${CFLAGS} -fno-PIC"
  LDFLAGS="--specs=warpup ${LDFLAGS}"
elif test "$build_aros" = "yes" ; then
  LIBS="-lpthread ${LIBS}"
  # <cstdint> does not work without this
  CPPFLAGS="-D__STDC_LIMIT_MACROS ${CPPFLAGS}"
elif test "$build_freemint" = "yes" ; then
  # XXX crossmint include path handling seems to be broken and uade compiler.h conflicts with system compiler.h
  CPPFLAGS="-include ${SYSROOT}/usr/include/compiler.h ${CPPFLAGS}"
  # shutup warning about fvisibility not supported
  CFLAGS="-Wno-attributes ${CFLAGS}"
elif test "$build_wasix" = "yes" ; then
  CPPFLAGS="-D_WASI_EMULATED_PROCESS_CLOCKS ${CPPFLAGS}"
  LIBS="-lwasi-emulated-process-clocks ${LIBS}"
elif test "$build_emscripten" = "yes" ; then
  #CFLAGS="-s NODERAWFS=1 -lwebsocket.js -sPROXY_POSIX_SOCKETS -pthread -sPROXY_TO_PTHREAD ${CFLAGS}"
  CFLAGS="-s NODERAWFS=1 ${CFLAGS}"
fi

# TODO proper freopen() test
# TODO invert (-DNO_FREOPEN -> -DNEED_FREOPEN)?
if test "$build_haiku" = "yes" || test "$build_openbsd" = "yes" || test "$build_netbsd" = "yes" || test "$build_mingw" = "yes" || test "$build_amiga" = "yes" || test "$build_solaris" = "yes" || test "$build_irix" = "yes" || test "$build_aix" = "yes" || test "$build_qnx" = "yes" || test "$build_wasix" = "yes" ; then
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
ld_aix=no
ld_cosmocc=no
ld_wasix=no
ld_emscripten=no

# cosmocc and wasix linker not detectable from version
if test "$build_cosmocc" = "yes" ; then
  ld_cosmocc=yes
elif test "$build_wasix" = "yes" ; then
  ld_wasix=yes
else
  linker_version=$( ($LD -v >/dev/null 2>&1 && $LD -v 2>&1) || ($LD -V 2>&1) )
  case "$linker_version" in
    GNU*|mold*)
        ld_gnu=yes
        ;;
    *LLD*)
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
    emcc*)
        ld_emscripten=yes
        ;;
    *)
        if test "$build_aix" = "yes" ; then
          # ld: LD 7.2.4(2/13/18)
          ld_aix=yes
        else
          AC_MSG_WARN(["Linker $LD ($linker_version) is not supported"])
        fi
        ;;
  esac
fi

AM_CONDITIONAL([LD_APPLE], [test "$ld_apple" = "yes"])
AM_CONDITIONAL([LD_LLD], [test "$ld_lld" = "yes"])
AM_CONDITIONAL([LD_GNU], [test "$ld_gnu" = "yes"])
AM_CONDITIONAL([LD_SOLARIS], [test "$ld_solaris" = "yes"])
AM_CONDITIONAL([LD_AIX], [test "$ld_aix" = "yes"])
AM_CONDITIONAL([LD_COSMOCC], [test "$ld_cosmocc" = "yes"])
AM_CONDITIONAL([LD_WASIX], [test "$ld_wasix" = "yes"])
AM_CONDITIONAL([LD_EMSCRIPTEN], [test "$ld_emscripten" = "yes"])

# static libgcc/libstdc++ used for executables (except on macOS)
AM_CONDITIONAL([STATIC_STDLIBS], [test "$static_stdlibs" = "yes"])

if test "$ld_cosmocc" = "yes" ; then
  # GNU (BFD) linker
  LDFLAGS="-Wl,--gc-sections -Wl,--exclude-libs=ALL -Wl,--discard-all -Wl,--no-undefined -Wl,--sort-common ${LDFLAGS}"
elif test "$ld_wasix" = "yes" || test "$ld_emscripten" = "yes" ; then
  # wasm-ld doesn't support other options
  LDFLAGS="-Wl,--gc-sections -Wl,-O1 ${LDFLAGS}"
elif test "$ld_apple" = "yes" ; then
  LDFLAGS="-Wl,-dead_strip -Wl,-dead_strip_dylibs -Wl,-undefined,error -Wl,-no_weak_exports ${LDFLAGS}"
elif test "$ld_solaris" = "yes" ; then
  LDFLAGS="-Wl,-B,reduce -Wl,-z,redlocsym -Wl,--no-undefined ${LDFLAGS}"
elif test "$ld_aix" = "yes" ; then
  LDFLAGS="${LDFLAGS}"
else
  # assume GNU compatible linker
  LDFLAGS="-Wl,--as-needed -Wl,-O1 ${LDFLAGS}"
  if test "$build_openbsd" = "yes" ;   then
    # -Wl,--no-undefined is broken(?) on OpenBSD
    LDFLAGS="-Wl,--gc-sections -Wl,--exclude-libs=ALL -Wl,--discard-all -Wl,--sort-common ${LDFLAGS}"
  elif test "$build_mingw" = "yes" && test "$ld_lld" = "yes" ; then
    # mingw lld linker does not support --exclude-libs=ALL,--discard-all,--sort-common (?)
    LDFLAGS="-Wl,--gc-sections -Wl,--no-undefined ${LDFLAGS}"
  elif test "$build_amigaos" = "yes" || test "$build_freemint" = "yes" ; then
    # no support for --exclude-libs=ALL in bebbos toolchain or crossmint
    LDFLAGS="-Wl,--gc-sections -Wl,--discard-all -Wl,--no-undefined -Wl,--sort-common ${LDFLAGS}"
  elif test "$build_morphos" = "yes" || test "$build_warpos" = "yes" ; then
    # no support for --exclude-libs=ALL, --gc-sections broken
    LDFLAGS="-Wl,--discard-all -Wl,--no-undefined -Wl,--sort-common ${LDFLAGS}"
  elif test "$build_aros" = "yes" || test "$build_irix" = "yes" ; then
    # no support for --gc-sections on AROS, broken(?) on IRIX/compilertron
    LDFLAGS="-Wl,--exclude-libs=ALL -Wl,--discard-all -Wl,--no-undefined  -Wl,--sort-common ${LDFLAGS}"
  else
    LDFLAGS="-Wl,--gc-sections -Wl,--exclude-libs=ALL -Wl,--discard-all -Wl,--no-undefined -Wl,--sort-common ${LDFLAGS}"
  fi
fi

STATIC_LDFLAGS="-static-libgcc -static-libstdc++"
if test "$ld_aix" != "yes" ; then
  STATIC_LDFLAGS="${STATIC_LDFLAGS} -Wl,--allow-multiple-definition"
fi
AC_SUBST([STATIC_LDFLAGS])

if test "$build_darwin" = "yes" ; then
  SHREXT="-shrext .dylib"
  AC_SUBST([SHREXT])
elif test "$build_netbsd" = "yes" ; then
  # default libtool is broken(?) on NetBSD for C++ shared modules
  postdeps_CXX=`echo " $postdeps_CXX " | sed 's, -lgcc,,g'`
  postdeps_CXX=`echo " $postdeps_CXX " | sed 's, -lc_s,,g'`
  postdeps_CXX=`echo " $postdeps_CXX " | sed 's, -lm_s,,g'`
elif test "$build_amigaos" = "yes" || test "$build_warpos" = "yes" ; then
  _UADE_TARGET_OS="--target-os=AmigaOS"
elif test "$build_morphos" = "yes" ; then
  _UADE_TARGET_OS="--target-os=MorphOS"
elif test "$build_amigaos4" = "yes" ; then
  _UADE_TARGET_OS="--target-os=AmigaOS4"
  # hack to support statically linked libgcc and libstdc++
  postdeps_CXX=`echo " $postdeps_CXX " | sed 's,-lgcc,,g'`
  postdeps_CXX=`echo " $postdeps_CXX " | sed 's,-lstdc++,,g'`
elif test "$build_aros" = "yes" ; then
  _UADE_TARGET_OS="--target-os=AROS"
fi

uade_prefix=${prefix}
if test "$uade_prefix" = "NONE" ; then
  uade_prefix=/usr/local
fi
_UADE_CPPFLAGS="${CPPFLAGS} -DDISABLE_BENCODETOOLS -DDISABLE_ZAKALWE"
_UADE_CFLAGS="${CFLAGS} -Wno-format -Wno-format-security -Wno-unused-command-line-argument"
_UADE_LDFLAGS="${LDFLAGS} ${EXTRA_LIBS}"
if test "$static_stdlibs" = "yes" ; then
  CPPFLAGS="-D_LIBCPP_NO_EXCEPTIONS -D_LIBCPP_HAS_NO_EXCEPTIONS ${CPPFLAGS}"
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
