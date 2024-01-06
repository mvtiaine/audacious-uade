# NOT a standalone Makefile.am

noinst_LTLIBRARIES += \
    3rdparty/replay/libreplay_hivelytracker.la \
    3rdparty/replay/libreplay_libdigibooster3.la \
    3rdparty/libreplay.la


3rdparty_replay_libreplay_hivelytracker_la_SOURCES = \
    3rdparty/replay/hivelytracker/replay.h \
    3rdparty/replay/hivelytracker/replay.c

# try keep byte-by-byte consistent audio output across platforms, by disabling some floating point optimizations
3rdparty_replay_libreplay_hivelytracker_la_CFLAGS = $(AM_CFLAGS) -ffp-contract=off -fno-fast-math

3rdparty_replay_libreplay_libdigibooster3_la_SOURCES = \
    3rdparty/replay/libdigibooster3/dsp.h \
    3rdparty/replay/libdigibooster3/dsp_echo.c \
    3rdparty/replay/libdigibooster3/dsp_fetchinstr.c \
    3rdparty/replay/libdigibooster3/dsp_linresampler.c \
    3rdparty/replay/libdigibooster3/dsp_panoramizer.c \
    3rdparty/replay/libdigibooster3/dsp_wavetable.c \
    3rdparty/replay/libdigibooster3/dsp_zeropadder.c \
    3rdparty/replay/libdigibooster3/libdigibooster3.h \
    3rdparty/replay/libdigibooster3/lists.h \
    3rdparty/replay/libdigibooster3/loader.c \
    3rdparty/replay/libdigibooster3/musicmodule.h \
    3rdparty/replay/libdigibooster3/player.h \
    3rdparty/replay/libdigibooster3/player.c

3rdparty_replay_libreplay_libdigibooster3_la_CPPFLAGS = $(AM_CPPFLAGS) -DTARGET_LINUX=1
3rdparty_replay_libreplay_libdigibooster3_la_CFLAGS = $(AM_CFLAGS) -Wno-parentheses


3rdparty_libreplay_la_SOURCES =
3rdparty_libreplay_la_LIBADD = \
    3rdparty/replay/libreplay_hivelytracker.la \
    3rdparty/replay/libreplay_libdigibooster3.la

