# NOT a standalone Makefile.am

noinst_LTLIBRARIES += \
    3rdparty/replay/libreplay_hivelytracker.la \
    3rdparty/replay/libreplay_libdigibooster3.la \
    3rdparty/replay/libreplay_ft2play.la \
    3rdparty/replay/libreplay_st3play.la \
    3rdparty/replay/libreplay_it2play.la \
    3rdparty/libreplay.la


3rdparty_replay_libreplay_hivelytracker_la_SOURCES = \
    3rdparty/replay/hivelytracker/replay.h \
    3rdparty/replay/hivelytracker/replay.c

# try keep byte-by-byte consistent audio output across platforms, by disabling some floating point optimizations
3rdparty_replay_libreplay_hivelytracker_la_CFLAGS = $(AM_CFLAGS) -ffp-contract=off -fno-fast-math
if !BUILD_AMIGAOS4
# XXX -Os breaks amigaos4 build
3rdparty_replay_libreplay_hivelytracker_la_CFLAGS += -Os
endif


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


3rdparty_replay_libreplay_ft2play_la_SOURCES = \
    3rdparty/replay/ft2play/ft2play.h \
    3rdparty/replay/ft2play/ft2play_play.cc \
    3rdparty/replay/ft2play/ft2play_probe.cc

3rdparty_replay_libreplay_ft2play_la_CXXFLAGS = $(AM_CXXFLAGS) -Wno-unknown-warning-option -Wno-volatile


3rdparty_replay_libreplay_st3play_la_SOURCES = \
    3rdparty/replay/st3play/st3play.h \
    3rdparty/replay/st3play/st3play_play.cc \
    3rdparty/replay/st3play/st3play_probe.cc


3rdparty_replay_libreplay_it2play_la_SOURCES = \
    3rdparty/replay/it2play/it2play.h \
    3rdparty/replay/it2play/it2play_play.cc \
    3rdparty/replay/it2play/it2play_probe.cc


3rdparty_libreplay_la_SOURCES =
3rdparty_libreplay_la_LIBADD = \
    3rdparty/replay/libreplay_hivelytracker.la \
    3rdparty/replay/libreplay_libdigibooster3.la \
    3rdparty/replay/libreplay_ft2play.la \
    3rdparty/replay/libreplay_st3play.la \
    3rdparty/replay/libreplay_it2play.la


EXTRA_DIST += \
    3rdparty/replay/hivelytracker/LICENSE \
    3rdparty/replay/libdigibooster3/CHANGELOG \
    3rdparty/replay/libdigibooster3/COPYING \
    3rdparty/replay/ft2play/LICENSE \
    3rdparty/replay/ft2play/README.md \
    3rdparty/replay/ft2play/pmplay.h \
    3rdparty/replay/ft2play/pmplay.c \
    3rdparty/replay/ft2play/pmp_main.h \
    3rdparty/replay/ft2play/pmp_main.c \
    3rdparty/replay/ft2play/pmp_mix.h \
    3rdparty/replay/ft2play/pmp_mix.c \
    3rdparty/replay/ft2play/snd_masm.h \
    3rdparty/replay/ft2play/snd_masm.c \
    3rdparty/replay/ft2play/tables.h \
    3rdparty/replay/ft2play/tables.c \
    3rdparty/replay/st3play/LICENSE \
    3rdparty/replay/st3play/st3play.c \
    3rdparty/replay/it2play/LICENSE \
    3rdparty/replay/it2play/README.md \
    3rdparty/replay/it2play/cpu.h \
    3rdparty/replay/it2play/it_d_rm.h \
    3rdparty/replay/it2play/it_d_rm.c \
    3rdparty/replay/it2play/it_m_eff.h \
    3rdparty/replay/it2play/it_m_eff.c \
    3rdparty/replay/it2play/it_music.h \
    3rdparty/replay/it2play/it_music.c \
    3rdparty/replay/it2play/it_structs.h \
    3rdparty/replay/it2play/it_structs.c \
    3rdparty/replay/it2play/it_tables.h \
    3rdparty/replay/it2play/it_tables.c \
    3rdparty/replay/it2play/loaders/it.h \
    3rdparty/replay/it2play/loaders/it.c \
    3rdparty/replay/it2play/loaders/s3m.h \
    3rdparty/replay/it2play/loaders/s3m.c \
    3rdparty/replay/it2play/loaders/mmcmp/mmcmp.h \
    3rdparty/replay/it2play/loaders/mmcmp/mmcmp.c
