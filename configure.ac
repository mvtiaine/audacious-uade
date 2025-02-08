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

# GNU make required (-Wno-portability)
AM_INIT_AUTOMAKE([tar-ustar dist-bzip2 no-dist-gzip -Wno-portability])
AC_CONFIG_MACRO_DIR([m4])

# AC_CANONICAL_HOST is needed to access the 'host_os' variable
AC_CANONICAL_HOST

# C standard check
AC_LANG_PUSH([C])
for version in 23 2x 17 11 1x 99 9x; do
  CSTD="-std=c${version}"
  AX_CHECK_COMPILE_FLAG([${CSTD}],[break],[CSTD=none])
done
AC_LANG_POP
AS_IF([test "$CSTD" = none], [
  AC_MSG_ERROR([Could not enable at least C9x (C99) - upgrade your compiler])
])

# C++ standard check
AC_LANG_PUSH([C++])
for version in 23 2b 20 2a 17 1z; do
  CXXSTD="-std=c++${version}"
  AX_CHECK_COMPILE_FLAG([${CXXSTD}],[break],[CXXSTD=none])
done
AC_LANG_POP
AS_IF([test "$CXXSTD" = none], [
  AC_MSG_ERROR([Could not enable at least C++1z (C++17) - upgrade your compiler])
])

# defines WORDS_BIGENDIAN
AC_C_BIGENDIAN([
  AM_CONDITIONAL([BIG_ENDIAN],[true])
  AM_CONDITIONAL([LITTLE_ENDIAN],[false])
  AC_DEFINE([WORDS_BIGENDIAN], [1])
],[
  AM_CONDITIONAL([BIG_ENDIAN],[false])
  AM_CONDITIONAL([LITTLE_ENDIAN],[true])
],[
  AC_MSG_ERROR([Could not determine endianness])
],[
  AC_MSG_ERROR([Universal build not supported])
])

# check if static stdlibs are explicitly enabled/disabled
AC_ARG_WITH(static-stdlibs,
  AS_HELP_STRING([--with-static-stdlibs=[yes|no|auto]], [Explicitly enable or disable statically linked stdlibs for the CLI binaries [default=auto]]),
  [static_stdlibs="$withval"],
  [static_stdlibs=""])

# check if static stdlibs are supported
if test "$static_stdlibs" != "no"; then
  STATIC_LDFLAGS="-static-libgcc -static-libstdc++ -Wl,--allow-multiple-definition"
  AC_LANG_PUSH([C++])
  AX_CHECK_LINK_FLAG([$STATIC_LDFLAGS],
    [
      static_stdlibs=yes
      AC_SUBST([STATIC_LDFLAGS])
    ],
    [
      if test "$static_stdlibs" = "yes"; then
        AC_MSG_ERROR([static stdlibs not supported])
      fi
      static_stdlibs=no
    ])
  AC_LANG_POP
fi

# Players
AC_ARG_ENABLE(players,
  AS_HELP_STRING([--enable-players=[PLAYERS]], [List of comma separated players to enable [default=all]]),
  [players="$enableval"],
  [players="all"])

AC_SUBST([players])
m4_foreach_w([player],
  [ft2play hivelytracker it2play libdigibooster3 noisetrekker2 protrekkr1 protrekkr2 st23play st3play uade], [
  AS_VAR_SET([p], [player])
  if test "$players" = "all" || printf ",${players}," | grep -Eq ",${p},"; then
    AC_DEFINE_UNQUOTED([PLAYER_[]player[]], [1], [Enable player ]player)
    AM_CONDITIONAL([PLAYER_]player, [true])
  else
    AC_DEFINE_UNQUOTED([PLAYER_[]player[]], [0], [Disable player ]player)
    AM_CONDITIONAL([PLAYER_]player, [false])
  fi
])

# Audacious plugin
AC_ARG_ENABLE(plugin-audacious,
  AS_HELP_STRING([--enable-plugin-audacious=[yes|no|auto]], [Explicitly enable or disable Audacious plugin [default=auto]]),
  [plugin_audacious="$enableval"],
  [plugin_audacious=""])

if test "$plugin_audacious" != "no"; then
# TODO check also if audacious linking actually works (to avoid export PKG* in crossbuild scripts)
PKG_CHECK_MODULES([AUDACIOUS],[audacious >= 3.8],
  [
    plugin_audacious=yes
    ac_audacious_plugindir=`$PKG_CONFIG --variable=plugin_dir audacious`
    AC_SUBST(AUDACIOUS_CFLAGS)
    AC_SUBST(AUDACIOUS_LIBS)
  ],
  [
    if test "$plugin_audacious" = "yes"; then
      AC_MSG_ERROR([Audacious not found!])
    else
      plugin_audacious=no
      ac_audacious_plugindir=;
      AC_MSG_NOTICE([Audacious not found, only CLI binaries will be built])
    fi
  ])
fi

if test "$plugin_audacious" = "yes"; then
AC_MSG_CHECKING([for audacious plugindir])
AC_ARG_WITH([audacious-plugindir],
  AS_HELP_STRING([--with-audacious-plugindir=[PLUGINDIR]], [Installs uade plugin under [PLUGINDIR]/Input instead of Audacious default]),
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
fi

AM_CONDITIONAL([PLUGIN_AUDACIOUS], [test "$plugin_audacious" = "yes"])

# gotta catch 'em all
build_linux=no
build_android=no
build_meego=no
build_webos=no
build_openharmony=no
build_tizen=no

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

build_amiga=no
build_amigaos=no
build_morphos=no
build_amigaos4=no # Runtime failure
build_warpos=no
build_aros=no

build_irix=no # Runtime failure
build_solaris=no
build_aix=no
build_unixware=no

build_qnx=no
build_qnx6=no
build_qnx7=no
build_qnx8=no
build_blackberry10=no

build_haiku=no
build_fuchsia=no
build_hurd=no
build_riscos=no
build_freemint=no
build_os2=no
build_redox=no
build_serenity=no
build_genode=no
build_symbian=no  # no new enough libstdc++/libc++ available? (llvm-cbe?)

build_ps4=no

build_cosmocc=no
build_wasix=no # Runtime failure
build_emscripten=no # Runtime failure

# Won't build
build_dos=no
build_vxworks=no
build_ps2=no
build_ps3=no
build_psp=no
build_psvita=no
build_switch=no
build_wiiu=no
build_nds=no
build_3ds=no
build_dreamcast=no
build_xbox=no
build_xbox360=no



# Detect the target system
# emscripten, cosmocc and genode not detectable via host* variables ?
case "${CC}" in
  *emcc)
    # see scripts/crossbuild/emscripten.sh
    build_emscripten=yes
    # needs custom UADE support
    AC_MSG_WARN([Emscripten target not yet supported])
    ;;
  *cosmo*cc)
    # see scripts/crossbuild/cosmocc.sh
    # TODO find out test failure reason:
    # FAIL: test/songend/test_songend.sh
    build_cosmocc=yes
    ;;
  *genode*cc)
    # see scripts/crossbuild/genode.sh
    build_genode=yes
    AC_MSG_WARN([Genode binaries have not been runtime tested])
    ;;
  nxdk-cc)
    # see scripts/crossbuild/wip/xbox.sh
    build_xbox=yes
    # needs custom UADE support (at minimum)
    AC_MSG_WARN([Xbox target not yet supported])
    ;;
  *)
    case "${host_os}" in
      *android)
        # see scripts/crossbuild/android.sh
        build_android=yes
        build_linux=yes
        AC_MSG_WARN([Android binaries have not been runtime tested])
        ;;
      *ohos)
        # see scripts/crossbuild/openharmony.sh
        build_openharmony=yes
        build_linux=yes
        AC_MSG_WARN([OpenHarmony binaries have not been runtime tested])
        ;;
      linux*)
        build_linux=yes
        case "${host_vendor}" in
          webos)
            # see scripts/crossbuild/webos.sh
            build_webos=yes
            AC_MSG_WARN([webOS binaries have not been runtime tested])
            ;;
          meego)
            # see scripts/crossbuild/sailfish.sh
            build_meego=yes
            ;;
          tizen)
            # see scripts/crossbuild/tizen.sh
            build_tizen=yes
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
            build_ios=yes
            AC_MSG_WARN([iOS binaries have not been runtime tested])
            ;;
          *WatchOS*|*WatchSimulator*|*AppleTVOS*|*AppleTVSimulator*)
            build_ios=yes
            # no fork/exec available, needs custom UADE support
            AC_MSG_WARN([WatchOS and AppleTVOS target not yet supported])
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
            ;;
          netbsd*)
            build_netbsd=yes
            ;;
          freebsd*|midnightbsd*)
            build_freebsd=yes
            ;;
          dragonfly*)
            build_dragonflybsd=yes
            ;;
        esac
        ;;
      haiku*)
        build_haiku=yes
        ;;
      amigaos*)
        build_amiga=yes
        case "${host_cpu}" in
          powerpc*)
            if test "$(echo|$CC -dM -E -|grep -q __amigaos4__ && echo yes)" = "yes" ; then
              # see scripts/crossbuild/amigaos4.sh
              # TODO compiles but executables crash at __start
              build_amigaos4=yes
              AC_MSG_WARN([AmigaOS4 target not yet supported])
            elif test "$(echo|$CC --specs=warpup -dM -E - 2>/dev/null|grep -q WARPUP && echo yes)" = "yes" ; then
              # see scripts/crossbuild/warpos.sh
              build_warpos=yes
              AC_MSG_WARN([WarpOS binaries have not been runtime tested])
            else
              # TODO PowerUP support also possible with mos2wos?
              AC_MSG_WARN([Unsupported PPC AmigaOS target])
            fi
            ;;
          m68k)
            # see scripts/crossbuild/amigaos.sh
            build_amigaos=yes
            ;;
          *)
           AC_MSG_WARN([${host_cpu} target not supported with AmigaOS])
           ;;
        esac
        ;;
      morphos*)
        # see scripts/crossbuild/morphos.sh
        build_amiga=yes
        if test "$(echo|$CC --specs=warpup -dM -E - 2>/dev/null|grep -q WARPUP && echo yes)" = "yes" ; then
          # see scripts/crossbuild/warpos.sh
          build_warpos=yes
          AC_MSG_WARN([WarpOS binaries have not been runtime tested])
        else
          build_morphos=yes
        fi
        ;;
      aros*)
        # see scripts/crossbuild/aros.sh
        build_aros=yes
        build_amiga=yes
        ;;
      solaris*)
        build_solaris=yes
        ;;
      irix*)
        # see scripts/crossbuild/irix.sh
        # audacious not tested
        build_irix=yes
        # compiles, but does not work properly (*printf() seems borked) (compilertron (sgug-rse gcc9))
        AC_MSG_WARN([IRIX target not yet supported])
        ;;
      aix*)
        # audacious not tested
        build_aix=yes
        ;;
      *UnixWare*)
        # audacious not tested
        build_unixware=yes
        # compiles, but UADE does not work
        # uade warning: Can not create socketpair: Error 0
        AC_MSG_WARN([UnixWare target not yet supported])
        ;;
      *qnx*)
        build_qnx=yes
        case "${host_vendor}" in
          blackberry)
            # see scripts/crossbuild/blackberry10.sh
            build_blackberry10=yes
            AC_MSG_WARN([BlackBerry 10 binaries have not been runtime tested])
            ;;
          *)
            case "${host_os}" in
              nto-qnx)
                # XXX should check _NTO_VERSION macro instead
                case "${CC}" in
                  *qnx7*)
                    # see scripts/crossbuild/qnx710.sh
                    build_qnx7=yes
                    AC_MSG_WARN([QNX 7 binaries have not been runtime tested])
                    ;;
                  *qnx8*)
                    # see scripts/crossbuild/qnx800.sh
                    build_qnx8=yes
                    AC_MSG_WARN([QNX 8 binaries have not been runtime tested])
                    ;;
                  *)
                    # assume native compile on QNX 6.5.0 with gcc10 from pkgsrc
                    # audacious not tested
                    build_qnx6=yes
                    ;;
                esac
                ;;
              *)
                AC_MSG_WARN([QNX $host_cpu-$host_vendor-$host_os build not tested])
                ;;
            esac
            ;;
        esac
        ;;
      *fuchsia*)
        # see scripts/crossbuild/fuchsia.sh
        build_fuchsia=yes
        AC_MSG_WARN([Fuchsia binaries have not been runtime tested])
        ;;
      wasi*)
        # see scripts/crossbuild/wasix.sh
        build_wasix=yes
        # compiles, but binaries either don't work or some even fail to run
        AC_MSG_WARN([Wasix target not yet supported])
        ;;
      gnu0.9)
        build_hurd=yes
        ;;
      mint)
        # see scripts/crossbuild/freemint.sh
        build_freemint=yes
        AC_MSG_WARN([FreeMiNT binaries have not been runtime tested])
        ;;
      *djgpp)
        # see scripts/crossbuild/wip/dos.sh
        build_dos=yes
        # needs custom UADE support (at minimum)
        AC_MSG_WARN([DOS/DJGPP target not yet supported])
        ;;
      os2-emx)
        # TODO find out test failure reason:
        # FAIL: test/songend/test_songend.sh
        # TODO UADE -Zexe support (native cc)
        build_os2=yes
        ;;
      redox)
        # see scripts/crossbuild/redox.sh
        build_redox=yes
        AC_MSG_WARN([Redox OS binaries have not been runtime tested])
        ;;
      serenity)
        # see scripts/crossbuild/serenity.sh
        build_serenity=yes
        AC_MSG_WARN([SerenityOS binaries have not been runtime tested])
        ;;
      vxworks)
        # see scripts/crossbuild/wip/vxworks.sh
        build_vxworks=yes
        # needs custom UADE support (at minimum)
        AC_MSG_WARN([VxWorks target not yet supported])
        ;;
      ps4)
        # see scripts/crossbuild/ps4.sh
        build_ps4=yes
        AC_MSG_WARN([PS4 binaries have not been runtime tested])
        ;;
      psp)
        # see scripts/crossbuild/wip/psp.sh
        build_psp=yes
        # needs custom UADE support (at minimum)
        AC_MSG_WARN([PSP target not yet supported])
        ;;
      symbian*)
        # see scripts/crossbuild/wip/symbian.sh
        build_symbian=yes
        # needs custom UADE support (at minimum)
        AC_MSG_WARN([Symbian target not yet supported])
        ;;
      *)
        # inverted os/vendor?
        case "${host_vendor}" in
          riscos)
            # see scripts/crossbuild/riscos.sh
            build_riscos=yes
            AC_MSG_WARN([RISC OS binaries have not been runtime tested])
            ;;
          ps2)
            # see scripts/crossbuild/wip/ps2.sh
            build_ps2=yes
            # needs custom UADE support (at minimum)
            AC_MSG_WARN([PS2 target not yet supported])
            ;;
          ps3)
            # see scripts/crossbuild/wip/ps3.sh
            build_ps3=yes
            # needs custom UADE support (at minimum)
            AC_MSG_WARN([PS3 target not yet supported])
            ;;
          vita)
            # see scripts/crossbuild/wip/psvita.sh
            build_psvita=yes
            # needs custom UADE support (at minimum)
            AC_MSG_WARN([PSVita target not yet supported])
            ;;
          *)
            case "${CPPFLAGS}" in
              # XXX no other way?
              *__SWITCH__*)
                # see scripts/crossbuild/wip/switch.sh
                build_switch=yes
                # needs custom UADE support (at minimum)
                AC_MSG_WARN([Switch target not yet supported])
                ;;
              *__WIIU__*)
                # see scripts/crossbuild/wip/wiiu.sh
                build_wiiu=yes
                # needs custom UADE support (at minimum)
                AC_MSG_WARN([Wii U target not yet supported])
                ;;
              *__NDS__*)
                # see scripts/crossbuild/wip/nds.sh
                build_nds=yes
                # needs custom UADE support (at minimum)
                AC_MSG_WARN([NDS target not yet supported])
                ;;
              *__3DS__*)
                # see scripts/crossbuild/wip/nds.sh
                build_3ds=yes
                # needs custom UADE support (at minimum)
                AC_MSG_WARN([3DS target not yet supported])
                ;;
              *__DREAMCAST__*)
                # see scripts/crossbuild/wip/dreamcast.sh
                build_dreamcast=yes
                # needs custom UADE support (at minimum)
                AC_MSG_WARN([Dreamcast target not yet supported])
                ;;
              *XENON*)
                # see scripts/crossbuild/wip/xbox360.sh
                build_xbox360=yes
                # needs custom UADE support (at minimum)
                AC_MSG_WARN([Xbox 360 target not yet supported])
                ;;
              *)
                AC_MSG_WARN([$host_cpu-$host_vendor-$host_os not supported])
                ;;
            esac
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
AM_CONDITIONAL([BUILD_OPENHARMONY], [test "$build_openharmony" = "yes"])
AM_CONDITIONAL([BUILD_TIZEN], [test "$build_tizen" = "yes"])
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
AM_CONDITIONAL([BUILD_AMIGA], [test "$build_amiga" = "yes"])
AM_CONDITIONAL([BUILD_AMIGAOS], [test "$build_amigaos" = "yes"])
AM_CONDITIONAL([BUILD_MORPHOS], [test "$build_morphos" = "yes"])
AM_CONDITIONAL([BUILD_AMIGAOS4], [test "$build_amigaos4" = "yes"])
AM_CONDITIONAL([BUILD_WARPOS], [test "$build_warpos" = "yes"])
AM_CONDITIONAL([BUILD_AROS], [test "$build_aros" = "yes"])
AM_CONDITIONAL([BUILD_IRIX], [test "$build_irix" = "yes"])
AM_CONDITIONAL([BUILD_SOLARIS], [test "$build_solaris" = "yes"])
AM_CONDITIONAL([BUILD_AIX], [test "$build_aix" = "yes"])
AM_CONDITIONAL([BUILD_UNIXWARE], [test "$build_unixware" = "yes"])
AM_CONDITIONAL([BUILD_HAIKU], [test "$build_haiku" = "yes"])
AM_CONDITIONAL([BUILD_QNX], [test "$build_qnx" = "yes"])
AM_CONDITIONAL([BUILD_QNX6], [test "$build_qnx6" = "yes"])
AM_CONDITIONAL([BUILD_QNX7], [test "$build_qnx7" = "yes"])
AM_CONDITIONAL([BUILD_QNX8], [test "$build_qnx8" = "yes"])
AM_CONDITIONAL([BUILD_BLACKBERRY10], [test "$build_blackberry10" = "yes"])
AM_CONDITIONAL([BUILD_FUCHSIA], [test "$build_fuchsia" = "yes"])
AM_CONDITIONAL([BUILD_HURD], [test "$build_hurd" = "yes"])
AM_CONDITIONAL([BUILD_RISCOS], [test "$build_riscos" = "yes"])
AM_CONDITIONAL([BUILD_FREEMINT], [test "$build_freemint" = "yes"])
AM_CONDITIONAL([BUILD_OS2], [test "$build_os2" = "yes"])
AM_CONDITIONAL([BUILD_REDOX], [test "$build_redox" = "yes"])
AM_CONDITIONAL([BUILD_SERENITY], [test "$build_serenity" = "yes"])
AM_CONDITIONAL([BUILD_GENODE], [test "$build_genode" = "yes"])
AM_CONDITIONAL([BUILD_SYMBIAN], [test "$build_symbian" = "yes"])
AM_CONDITIONAL([BUILD_PS4], [test "$build_ps4" = "yes"])
AM_CONDITIONAL([BUILD_COSMOCC], [test "$build_cosmocc" = "yes"])
AM_CONDITIONAL([BUILD_WASIX], [test "$build_wasix" = "yes"])
AM_CONDITIONAL([BUILD_EMSCRIPTEN], [test "$build_emscripten" = "yes"])

AM_CONDITIONAL([BUILD_DOS], [test "$build_dos" = "yes"])
AM_CONDITIONAL([BUILD_VXWORKS], [test "$build_vxworks" = "yes"])
AM_CONDITIONAL([BUILD_PS2], [test "$build_ps2" = "yes"])
AM_CONDITIONAL([BUILD_PS3], [test "$build_ps3" = "yes"])
AM_CONDITIONAL([BUILD_PSP], [test "$build_psp" = "yes"])
AM_CONDITIONAL([BUILD_PSVITA], [test "$build_psvita" = "yes"])
AM_CONDITIONAL([BUILD_SWITCH], [test "$build_switch" = "yes"])
AM_CONDITIONAL([BUILD_WIIU], [test "$build_wiiu" = "yes"])
AM_CONDITIONAL([BUILD_NDS], [test "$build_nds" = "yes"])
AM_CONDITIONAL([BUILD_3DS], [test "$build_3ds" = "yes"])
AM_CONDITIONAL([BUILD_DREAMCAST], [test "$build_dreamcast" = "yes"])
AM_CONDITIONAL([BUILD_XBOX], [test "$build_xbox" = "yes"])
AM_CONDITIONAL([BUILD_XBOX360], [test "$build_xbox360" = "yes"])

if test "$build_solaris" = "yes" || test "$build_aros" = "yes" ; then
  # on OpenIndiana -std=<any> breaks PATH_MAX for some reason
  # on AROS it breaks asm macros
  CSTD=""
fi

# see AX_PROG_CC_FOR_BUILD
CFLAGS_FOR_BUILD="${CSTD} ${CFLAGS}"

CPPFLAGS="-D_DEFAULT_SOURCE ${CPPFLAGS}"
# XXX error: conflicting types for 'basename' (libgen.h vs string.h)
if test "$build_openharmony" != "yes" && test "$build_ps4" != "yes" && test "$build_dreamcast" != "yes" ; then
  CPPFLAGS="-D_GNU_SOURCE ${CPPFLAGS}"
fi
COMMON_CFLAGS="-fvisibility=hidden -fmerge-all-constants -fno-exceptions"
CXXFLAGS="${CXXSTD} ${CXXFLAGS} ${COMMON_CFLAGS} -fvisibility-inlines-hidden -fno-rtti"
CFLAGS="${CSTD} ${CFLAGS} ${COMMON_CFLAGS}"
ARCHFLAGS=""
EXTRA_LIBS=""
if test "$build_haiku" = "yes" ; then
  EXTRA_LIBS="-lbsd -lnetwork"
elif test "$build_cygwin" = "yes" ; then
  EXTRA_LIBS="-lcygwin"
elif test "$build_mingw" = "yes" ; then
  CPPFLAGS="-I/usr/include ${CPPFLAGS}"
  ARCHFLAGS="-I/usr/include ${ARCHFLAGS}"
  LIBS="-lws2_32 ${LIBS}"
elif test "$build_solaris" = "yes" || test "$build_unixware" = "yes"; then
  EXTRA_LIBS="-lsocket -lnsl"
elif test "$build_irix" = "yes" ; then
  # irix(6.5.0)+gcc9(compilertron) quirks
  CPPFLAGS="-D_XOPEN_SOURCE=600 -D_POSIX_SOURCE -D_SGI_SOURCE -D__EXTENSIONS__ -DPATH_MAX=1024 ${CPPFLAGS}"
  EXTRA_LIBS="-lgen"
elif test "$build_aix" = "yes" ; then
  # -pthread needed for std::mutex support
  ARCHFLAGS="-pthread ${ARCHFLAGS}"
  LDFLAGS="-pthread ${LDFLAGS}"
elif test "$build_qnx6" = "yes" ; then
  # qnx(6.5.0)+gcc10(pkgsrc) quirks
  CPPFLAGS="-D_XOPEN_SOURCE=600 -D_QNX_SOURCE -D_HAS_C9X -D__UINTPTR_TYPE__=uintptr_t -include stdint.h -include math.h ${CPPFLAGS}"
  # dwarf version >= 4 not supported by linker (qnx 6.5.0)
  ARCHFLAGS="-gdwarf-3 ${ARCHFLAGS}"
  LIBS="-lsocket ${LIBS}"
elif test "$build_qnx" = "yes" ; then
  CPPFLAGS="-D_XOPEN_SOURCE=700 -D_QNX_SOURCE"
  LIBS="-lsocket ${LIBS}"
  EXTRA_LIBS="-lm"
  if test "$build_blackberry10" = "yes" ; then
    # XXX hack for compat.h/stoi() as _NTO_VERSION can't be used
    CPPFLAGS="-D__BLACKBERRY__ ${CPPFLAGS}"
  fi
elif test "$build_hurd" = "yes" ; then
  # PATH_MAX not defined?
  CPPFLAGS="-DPATH_MAX=1024 ${CPPFLAGS}"
elif test "$build_amigaos" = "yes" ; then
  # TODO UADE crashes with bebbo gcc6 (works on gcc13.2)
  # TODO -mhard-float fails to compile with bebbo gcc13.2, crashes songdb on gcc6
  # -noixemul uses libnix
  ARCHFLAGS="-noixemul -m68020-60 ${ARCHFLAGS}"
  # XXX with -O2, -foptimize-sibling-calls causes some exes to crash before main() (songdb/player/precalc)
  ARCHFLAGS="-fno-optimize-sibling-calls ${ARCHFLAGS}"
elif test "$build_morphos" = "yes" ; then
  ARCHFLAGS="-noixemul ${ARCHFLAGS}"
  # <cstdint> does not work without this
  CPPFLAGS="-D__STDC_LIMIT_MACROS ${CPPFLAGS}"
elif test "$build_amigaos4" = "yes" ; then
  # static libstdc++ needs also postdeps_CXX hack below
  ARCHFLAGS="-athread=native ${ARCHFLAGS}"
  LIBS="-lauto ${LIBS}"
elif test "$build_warpos" = "yes" ; then
  CPPFLAGS="--specs=warpup -D__STDC_LIMIT_MACROS ${CPPFLAGS}"
  ARCHFLAGS="--specs=warpup -noixemul -mclib=newlib -pthread ${ARCHFLAGS}"
  LDFLAGS="--specs=warpup ${LDFLAGS}"
elif test "$build_aros" = "yes" ; then
  LIBS="-lpthread ${LIBS}"
  # <cstdint> does not work without this
  CPPFLAGS="-D__STDC_LIMIT_MACROS ${CPPFLAGS}"
elif test "$build_freemint" = "yes" ; then
  # XXX crossmint include path handling seems to be broken and uade compiler.h conflicts with system compiler.h
  CPPFLAGS="-include ${SYSROOT}/usr/include/compiler.h ${CPPFLAGS}"
  # shutup warning about fvisibility not supported
  ARCHFLAGS="-Wno-attributes ${ARCHFLAGS}"
elif test "$build_os2" = "yes" ; then
  # shutup warning about fvisibility not supported
  ARCHFLAGS="-Zexe -Wno-attributes ${ARCHFLAGS}"
  # XXX fixme
  sed -i s/'NATIVECFLAGS = \$'/'NATIVECFLAGS = -Zexe \$'/g uade/src/Makefile.in
elif test "$build_redox" = "yes" ; then
  # --static required?
  LDFLAGS="--static ${LDFLAGS}"
elif test "$build_genode" = "yes" ; then
  LIBS="-l:libgcc.a -l:libc.lib.so -l:libm.lib.so -l:posix.lib.so -l:stdcxx.lib.so -l:libgcc.a ${LIBS}"
  # XXX fixme
  sed -i s/'LIBRARIES = -lm'/'LIBRARIES ='/g uade/src/Makefile.in
elif test "$build_ps4" = "yes" ; then
  # XXX fix /opt/cross/OpenOrbis/PS4Toolchain/include/c++/v1/cmath:341:9: error: no member named 'abs' in the global namespace; did you mean 'fabs'?
  CPPFLAGS="-include stdlib.h ${CPPFLAGS}"
elif test "$build_wasix" = "yes" ; then
  CPPFLAGS="-D_WASI_EMULATED_PROCESS_CLOCKS ${CPPFLAGS}"
  LIBS="-lwasi-emulated-process-clocks ${LIBS}"
elif test "$build_emscripten" = "yes" ; then
  #LDFLAGS="-sASSERTIONS -sALLOW_MEMORY_GROWTH -sNODERAWFS=1 -lwebsocket.js -sPROXY_POSIX_SOCKETS -pthread -sPROXY_TO_PTHREAD ${LDFLAGS}"
  LDFLAGS="-sASSERTIONS -sALLOW_MEMORY_GROWTH -sNODERAWFS=1 ${LDFLAGS}"
elif test "$build_symbian" = "yes" ; then
  if test "${EPOCROOT}" = "" ; then
    AC_MSG_ERROR([EPOCROOT not set])
  fi
  # PATH_MAX not defined?
  CPPFLAGS="-DPATH_MAX=1024 ${CPPFLAGS}"
  CPPFLAGS="-isystem ${EPOCROOT}/epoc32/include/stdapis -isystem ${EPOCROOT}/epoc32/include/libc -isystem ${EPOCROOT}/epoc32/include ${CPPFLAGS}"
  CPPFLAGS="-include gcce/gcce.h ${CPPFLAGS}"
fi

# must be called after C(XX)FLAGS tweaks, as for example on MorphOS -noixemul must be set before LT_INIT
LT_INIT

ld_apple=no
ld_lld=no
ld_gnu=no
ld_solaris=no
ld_aix=no
ld_os2=no
ld_wasm=no

# wasix linker not detectable from version
if test "$build_wasix" = "yes" ; then
  ld_wasm=yes
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
        ;;
    *Solaris*)
        ld_solaris=yes
        ;;
    emcc*)
        ld_wasm=yes
        ;;
    *)
        if test "$build_aix" = "yes" ; then
          # ld: LD 7.2.4(2/13/18)
          ld_aix=yes
        elif test "$build_os2" = "yes" ; then
          # does not support -v or -V to get version
          ld_os2=yes
        else
          AC_MSG_WARN([Linker $LD ($linker_version) is not supported])
        fi
        ;;
  esac
fi

CFLAGS="${ARCHFLAGS} ${CFLAGS} -Wno-unused-parameter"
CXXFLAGS="${ARCHFLAGS} ${CXXFLAGS} -Wno-unused-parameter"

AM_CONDITIONAL([LD_APPLE], [test "$ld_apple" = "yes"])
AM_CONDITIONAL([LD_LLD], [test "$ld_lld" = "yes"])
AM_CONDITIONAL([LD_GNU], [test "$ld_gnu" = "yes"])
AM_CONDITIONAL([LD_SOLARIS], [test "$ld_solaris" = "yes"])
AM_CONDITIONAL([LD_AIX], [test "$ld_aix" = "yes"])
AM_CONDITIONAL([LD_OS2], [test "$ld_os2" = "yes"])
AM_CONDITIONAL([LD_WASM], [test "$ld_wasm" = "yes"])

# static libgcc/libstdc++ used for CLI binaries
AM_CONDITIONAL([STATIC_STDLIBS], [test "$static_stdlibs" = "yes"])

if test "$ld_wasm" = "yes" ; then
  # wasm-ld doesn't support other options
  LDFLAGS="-Wl,--gc-sections -Wl,-O1 ${LDFLAGS}"
elif test "$ld_apple" = "yes" ; then
  LDFLAGS="-Wl,-dead_strip -Wl,-dead_strip_dylibs -Wl,-undefined,error -Wl,-no_weak_exports ${LDFLAGS}"
elif test "$ld_solaris" = "yes" ; then
  LDFLAGS="-Wl,-B,reduce -Wl,-z,redlocsym -Wl,--no-undefined ${LDFLAGS}"
elif test "$ld_aix" = "yes" || test "$ld_os2" = "yes" ; then
  LDFLAGS="${LDFLAGS}"
else
  # assume GNU compatible linker
  LDFLAGS="-Wl,-O1 -Wl,--as-needed ${LDFLAGS}"
  AX_CHECK_LINK_FLAG([-Wl,--exclude-libs=ALL], [LDFLAGS="-Wl,--exclude-libs=ALL ${LDFLAGS}"], [])
  AX_CHECK_LINK_FLAG([-Wl,--discard-all], [LDFLAGS="-Wl,--discard-all ${LDFLAGS}"], [])
  AX_CHECK_LINK_FLAG([-Wl,--sort-common], [LDFLAGS="-Wl,--sort-common ${LDFLAGS}"], [])
  AX_CHECK_LINK_FLAG([-Wl,--icf=all], [LDFLAGS="-Wl,--icf=all ${LDFLAGS}"], [])
  # broken(?) on MorphOS/WarpOS, IRIX/compilertron
  if test "$build_morphos" != "yes" && test "$build_warpos" != "yes" && test "$build_irix" != "yes"; then
    AX_CHECK_LINK_FLAG([-Wl,--gc-sections],
    [
      LDFLAGS="-Wl,--gc-sections ${LDFLAGS}"
      # use only if --gc-sections is supported
      CFLAGS="-ffunction-sections -fdata-sections ${CFLAGS}"
      CXXFLAGS="-ffunction-sections -fdata-sections ${CXXFLAGS}"
    ],[])
  fi
  # broken(?) on OpenBSD
  if test "$build_openbsd" != "yes" ; then
    AX_CHECK_LINK_FLAG([-Wl,--no-undefined], [LDFLAGS="-Wl,--no-undefined ${LDFLAGS}"], [])
  fi
fi

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
  if test "$static_stdlibs" = "yes" ; then
    # hack to support statically linked libgcc and libstdc++
    postdeps_CXX=`echo " $postdeps_CXX " | sed 's,-lgcc,,g'`
    postdeps_CXX=`echo " $postdeps_CXX " | sed 's,-lstdc++,,g'`
  fi
elif test "$build_aros" = "yes" ; then
  _UADE_TARGET_OS="--target-os=AROS"
elif test "$build_tizen" = "yes" || test "$build_blackberry10" = "yes" ; then
  if test "$static_stdlibs" = "yes" ; then
    # hack to support statically linked libstdc++
    postdeps_CXX=`echo " $postdeps_CXX " | sed 's,-lstdc++,,g'`
  fi
fi

if test "$players" = "all" || printf ",${players}," | grep -Eq ",${p},"; then
uade_prefix=${prefix}
if test "$uade_prefix" = "NONE" ; then
  uade_prefix=/usr/local
fi
_UADE_CPPFLAGS="${CPPFLAGS} -DDISABLE_BENCODETOOLS -DDISABLE_ZAKALWE"
_UADE_CFLAGS="${CFLAGS} -Wno-unused -Wno-deprecated-declarations -Wno-tautological-compare -Wno-pedantic -Wno-implicit-function-declaration"
_UADE_CFLAGS="${_UADE_CFLAGS} -Wno-zero-length-array -Wno-variadic-macros -Wno-gnu-zero-variadic-macro-arguments"
_UADE_CFLAGS="${_UADE_CFLAGS} -Wno-format -Wno-format-nonliteral -Wno-format-security"
_UADE_CFLAGS="${_UADE_CFLAGS} -Wno-sign-compare -Wno-shift-negative-value"
_UADE_LDFLAGS="${LDFLAGS} ${EXTRA_LIBS}"
if test "$static_stdlibs" = "yes" ; then
  _UADE_LDFLAGS="-static-libgcc ${_UADE_LDFLAGS}"
fi
if test "$plugin_audacious" = "yes" ; then
  _UADE_CFLAGS="${pic_flag} ${_UADE_CFLAGS}"
fi
AX_SUBDIRS_CONFIGURE([uade], [
  [--without-uade123], [--without-uadesimple], [--without-write-audio],[--without-uadefs],[--without-libuade],[--without-avx2],
  [--prefix=\${uade_prefix}/share/audacious-uade],
  [--bindir=/dev/null], [--mandir=/dev/null], [--sharedir=/dev/null],
  [--bencode-tools-prefix=/dev/null], [--libzakalwe-prefix=/dev/null],
  [--target-ar=${AR}],[--target-cc=${CC}],[CC=${CC_FOR_BUILD}],[CXX=${CXX}],
  [CFLAGS=${_UADE_CPPFLAGS} ${_UADE_CFLAGS}],
  [LDFLAGS=${_UADE_LDFLAGS} ],
  [${_UADE_TARGET_OS}],
])
UADE_CPPFLAGS="-I\${top_srcdir}/uade/src/frontends/include -I\${top_builddir}/uade/src/frontends/include"
AC_SUBST([UADE_CPPFLAGS])
# abs_top_builddir is needed here at least by OpenBSD ports
UADE_LIBS="\${abs_top_builddir}/uade/src/frontends/common/libuade.a ${EXTRA_LIBS}"
AC_SUBST([UADE_LIBS])
fi # uade

# XXX autoheader does not support dynamic variables with AC_DEFINE_UNQUOTED
#AC_CONFIG_HEADERS([src/config.h])
cp confdefs.h "${srcdir}/src/config.h"
AC_CONFIG_FILES(Makefile
                src/Makefile)

AC_OUTPUT
