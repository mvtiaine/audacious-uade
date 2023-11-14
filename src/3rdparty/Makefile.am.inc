# NOT a standalone Makefile.am

noinst_LTLIBRARIES += \
	3rdparty/libmd5.la 3rdparty/libreplay.la

3rdparty_libmd5_la_SOURCES = \
    3rdparty/md5.h \
    3rdparty/md5.cc

replay_hvl_SOURCES = \
    3rdparty/replay/hvl/replay.h \
    3rdparty/replay/hvl/replay.c

replay_dbm_SOURCES = \
    3rdparty/replay/dbm/dsp.h \
    3rdparty/replay/dbm/dsp_echo.c \
    3rdparty/replay/dbm/dsp_fetchinstr.c \
    3rdparty/replay/dbm/dsp_linresampler.c \
    3rdparty/replay/dbm/dsp_panoramizer.c \
    3rdparty/replay/dbm/dsp_wavetable.c \
    3rdparty/replay/dbm/dsp_zeropadder.c \
    3rdparty/replay/dbm/libdigibooster3.h \
    3rdparty/replay/dbm/lists.h \
    3rdparty/replay/dbm/loader.c \
    3rdparty/replay/dbm/musicmodule.h \
    3rdparty/replay/dbm/player.h \
    3rdparty/replay/dbm/player.c

3rdparty_libreplay_la_SOURCES = $(replay_hvl_SOURCES) $(replay_dbm_SOURCES)
3rdparty_libreplay_la_CPPFLAGS = $(AM_CPPFLAGS) -DTARGET_LINUX=1
3rdparty_libreplay_la_CFLAGS = -Wno-parentheses
3rdparty_libreplay_la_CXXFLAGS = -fpermissive