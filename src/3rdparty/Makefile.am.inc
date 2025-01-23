# NOT a standalone Makefile.am

noinst_LTLIBRARIES += \
    3rdparty/replay/libreplay_hivelytracker.la \
    3rdparty/replay/libreplay_libdigibooster3.la \
    3rdparty/replay/libreplay_ft2play.la \
    3rdparty/replay/libreplay_st3play.la \
    3rdparty/replay/libreplay_it2play.la \
    3rdparty/replay/libreplay_st23play.la \
    3rdparty/replay/libreplay_protrekkr.la \
    3rdparty/libreplay.la 


3rdparty_replay_libreplay_hivelytracker_la_SOURCES = \
    3rdparty/replay/hivelytracker/replay.h \
    3rdparty/replay/hivelytracker/replay.c

# try keep byte-by-byte consistent audio output across platforms, by disabling some floating point optimizations
3rdparty/replay/hivelytracker/% : CFLAGS += -ffp-contract=off -fno-fast-math -Wno-sign-compare -Wno-overflow -Wno-implicit-fallthrough


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
3rdparty/replay/libdigibooster3/% : CFLAGS += -Wno-parentheses -Wno-unused -Wno-overflow


3rdparty_replay_libreplay_ft2play_la_SOURCES = \
    3rdparty/replay/ft2play/ft2play.h \
    3rdparty/replay/ft2play/ft2play_play.cc \
    3rdparty/replay/ft2play/ft2play_probe.cc

3rdparty/replay/ft2play/% : CXXFLAGS += -Wno-volatile -Wno-unknown-warning-option


3rdparty_replay_libreplay_st3play_la_SOURCES = \
    3rdparty/replay/st3play/st3play.h \
    3rdparty/replay/st3play/st3play_play.cc \
    3rdparty/replay/st3play/st3play_probe.cc

3rdparty/replay/st3play/% : CXXFLAGS += -Wno-keyword-macro


3rdparty_replay_libreplay_it2play_la_SOURCES = \
    3rdparty/replay/it2play/it2play.h \
    3rdparty/replay/it2play/it2play_play.cc \
    3rdparty/replay/it2play/it2play_probe.cc

# try keep byte-by-byte consistent audio output across platforms, by disabling some floating point optimizations
3rdparty/replay/it2play/% : CXXFLAGS += -ffp-contract=off -fno-fast-math -Wno-misleading-indentation


3rdparty_replay_libreplay_st23play_la_SOURCES = \
    3rdparty/replay/st23play/st23play.h \
    3rdparty/replay/st23play/st23play_play.cc \
    3rdparty/replay/st23play/st23play_probe.cc


3rdparty_replay_libreplay_protrekkr_la_SOURCES = \
    3rdparty/replay/protrekkr/protrekkr.h \
    3rdparty/replay/protrekkr/protrekkr_play.cc \
    3rdparty/replay/protrekkr/protrekkr_probe.cc \
    3rdparty/miniz/miniz.c

# try keep byte-by-byte consistent audio output across platforms, by disabling some floating point optimizations
3rdparty/replay/protrekkr/% : CXXFLAGS += -ffp-contract=off -fno-fast-math -Wno-char-subscripts -Wno-unused -Wno-sign-compare -Wno-multichar


3rdparty_libreplay_la_SOURCES =
3rdparty_libreplay_la_LIBADD = \
    3rdparty/replay/libreplay_hivelytracker.la \
    3rdparty/replay/libreplay_libdigibooster3.la \
    3rdparty/replay/libreplay_ft2play.la \
    3rdparty/replay/libreplay_st3play.la \
    3rdparty/replay/libreplay_it2play.la \
    3rdparty/replay/libreplay_st23play.la \
    3rdparty/replay/libreplay_protrekkr.la


EXTRA_DIST += \
    3rdparty/miniz/ChangeLog.md \
    3rdparty/miniz/LICENSE \
    3rdparty/miniz/readme.md \
    3rdparty/miniz/miniz.h \
    3rdparty/miniz/miniz.c \
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
    3rdparty/replay/it2play/it2drivers/hq_m.h \
    3rdparty/replay/it2play/it2drivers/hq_m.c \
    3rdparty/replay/it2play/it2drivers/hq.h \
    3rdparty/replay/it2play/it2drivers/hq.c \
    3rdparty/replay/it2play/it2drivers/sb16_m.h \
    3rdparty/replay/it2play/it2drivers/sb16_m.c \
    3rdparty/replay/it2play/it2drivers/sb16.h \
    3rdparty/replay/it2play/it2drivers/sb16.c \
    3rdparty/replay/it2play/it2drivers/sb16mmx_m.h \
    3rdparty/replay/it2play/it2drivers/sb16mmx_m.c \
    3rdparty/replay/it2play/it2drivers/sb16mmx.h \
    3rdparty/replay/it2play/it2drivers/sb16mmx.c \
    3rdparty/replay/it2play/it2drivers/wavwriter_m.h \
    3rdparty/replay/it2play/it2drivers/wavwriter_m.c \
    3rdparty/replay/it2play/it2drivers/wavwriter.h \
    3rdparty/replay/it2play/it2drivers/wavwriter.c \
    3rdparty/replay/it2play/it2drivers/zerovol.h \
    3rdparty/replay/it2play/it2drivers/zerovol.c \
    3rdparty/replay/it2play/loaders/it.h \
    3rdparty/replay/it2play/loaders/it.c \
    3rdparty/replay/it2play/loaders/s3m.h \
    3rdparty/replay/it2play/loaders/s3m.c \
    3rdparty/replay/st23play/LICENSE \
    3rdparty/replay/st23play/st23play.c \
    3rdparty/replay/protrekkr/LICENSE \
    3rdparty/replay/protrekkr/README.md \
    3rdparty/replay/protrekkr/release/distrib/changes.txt \
    3rdparty/replay/protrekkr/release/distrib/license.txt \
    3rdparty/replay/protrekkr/release/distrib/replay/lib/include/cubic_spline.h \
    3rdparty/replay/protrekkr/release/distrib/replay/lib/include/endianness.h \
    3rdparty/replay/protrekkr/release/distrib/replay/lib/include/ptk_def_properties.h \
    3rdparty/replay/protrekkr/release/distrib/replay/lib/include/ptkreplay.h \
    3rdparty/replay/protrekkr/release/distrib/replay/lib/include/replay.h \
    3rdparty/replay/protrekkr/release/distrib/replay/lib/include/samples_unpack.h \
    3rdparty/replay/protrekkr/release/distrib/replay/lib/include/spline.h \
    3rdparty/replay/protrekkr/release/distrib/replay/lib/include/synth.h \
    3rdparty/replay/protrekkr/release/distrib/replay/lib/include/tb_303.h \
    3rdparty/replay/protrekkr/release/distrib/replay/lib/include/wavpack.h \
    3rdparty/replay/protrekkr/release/distrib/replay/lib/endianness.cpp \
    3rdparty/replay/protrekkr/release/distrib/replay/lib/replay.cpp \
    3rdparty/replay/protrekkr/release/distrib/replay/lib/samples_unpack.cpp \
    3rdparty/replay/protrekkr/release/distrib/replay/lib/spline.cpp \
    3rdparty/replay/protrekkr/release/distrib/replay/lib/synth.cpp \
    3rdparty/replay/protrekkr/release/distrib/replay/lib/tb_303.cpp \
    3rdparty/replay/protrekkr/release/distrib/replay/lib/wavpack.cpp \
    3rdparty/replay/protrekkr/src/editors/include/patterns_blocks.h \
    3rdparty/replay/protrekkr/src/editors/patterns_blocks.cpp \
    3rdparty/replay/protrekkr/src/files/303s.cpp \
    3rdparty/replay/protrekkr/src/files/files.cpp \
    3rdparty/replay/protrekkr/src/files/reverbs.cpp \
    3rdparty/replay/protrekkr/src/files/synths.cpp
