AUTOMAKE_OPTIONS = subdir-objects
noinst_LIBRARIES = lib3rdparty.a
noinst_PROGRAMS = songend uade_songend
EXTRA_DIST = songdb.sh

uadedir = $(prefix)/share/audacious-uade
uadedatadir = $(uadedir)/share/uade

songenddir = ${prefix}/bin
songend_SOURCES = \
    songend_main.cc \
    songend.cc

uade_songenddir = $(prefix)/bin
uade_songend_SOURCES = \
    uade_songend_main.cc \
    songend.h \
    songend.cc \
    ../uade_common.h \
    ../uade_common.cc \
    ../hacks.h \
    ../hacks.cc \
    ../converter/big_endian.h \
    ../converter/converter.h \
    ../converter/converter.cc \
    ../converter/common_med.h \
    ../converter/common_med.cc \
    ../converter/converter_med4.cc \
    ../player/player.h \
    ../player/player.cc \
    ../player/player_hvl.cc \
    ../player/player_dbm.cc

lib3rdparty_a_SOURCES = \
    ../3rdparty/md5.h \
    ../3rdparty/md5.cc \
    ../3rdparty/hvl/replay.h \
    ../3rdparty/hvl/replay.c \
    ../3rdparty/dbm/dsp.h \
    ../3rdparty/dbm/dsp_echo.c \
    ../3rdparty/dbm/dsp_fetchinstr.c \
    ../3rdparty/dbm/dsp_linresampler.c \
    ../3rdparty/dbm/dsp_panoramizer.c \
    ../3rdparty/dbm/dsp_wavetable.c \
    ../3rdparty/dbm/dsp_zeropadder.c \
    ../3rdparty/dbm/libdigibooster3.h \
    ../3rdparty/dbm/lists.h \
    ../3rdparty/dbm/loader.c \
    ../3rdparty/dbm/musicmodule.h \
    ../3rdparty/dbm/player.h \
    ../3rdparty/dbm/player.c

lib3rdparty_a_CPPFLAGS = -DTARGET_LINUX=1
lib3rdparty_a_CFLAGS = -Wno-parentheses -fPIC
lib3rdparty_a_CXXFLAGS = -fpermissive -fPIC

uade_songend_CPPFLAGS = -DUADEDIR="\"$(uadedir)\"" -DUADEDATADIR="\"$(uadedatadir)\""
uade_songend_CFLAGS = $(UADE_CFLAGS)
uade_songend_CXXFLAGS = $(UADE_CFLAGS) -fno-rtti -Wall
uade_songend_DEPENDENCIES = lib3rdparty.a $(UADE_LIBS)
uade_songend_LDADD = lib3rdparty.a $(UADE_LIBS) $(LIBBSD_LIBS)
if BUILD_MACOS
uade_songend_LDFLAGS = -Wl,-dead_strip -Wl,-undefined,error
SHREXT = -shrext .dylib
else
uade_songend_LDFLAGS = -Wl,--gc-sections -Wl,--as-needed -Wl,--exclude-libs=ALL -Wl,--discard-all -Wl,--no-undefined -Wl,-O1
endif
