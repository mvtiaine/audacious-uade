AUTOMAKE_OPTIONS = subdir-objects
noinst_LIBRARIES = libmd5.a libreplay.a

AM_CPPFLAGS = -I$(top_srcdir)/src

libmd5_a_SOURCES = \
    md5.h \
    md5.cc

hvl_SOURCES = \
    replay/hvl/replay.h \
    replay/hvl/replay.c

dbm_SOURCES = \
    replay/dbm/dsp.h \
    replay/dbm/dsp_echo.c \
    replay/dbm/dsp_fetchinstr.c \
    replay/dbm/dsp_linresampler.c \
    replay/dbm/dsp_panoramizer.c \
    replay/dbm/dsp_wavetable.c \
    replay/dbm/dsp_zeropadder.c \
    replay/dbm/libdigibooster3.h \
    replay/dbm/lists.h \
    replay/dbm/loader.c \
    replay/dbm/musicmodule.h \
    replay/dbm/player.h \
    replay/dbm/player.c

libreplay_a_SOURCES = $(hvl_SOURCES) $(dbm_SOURCES)
libreplay_a_CPPFLAGS = $(AM_CPPFLAGS) -DTARGET_LINUX=1
libreplay_a_CFLAGS = -Wno-parentheses
libreplay_a_CXXFLAGS = -fpermissive
