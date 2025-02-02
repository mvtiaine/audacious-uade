# NOT a standalone Makefile.am

noinst_LTLIBRARIES += \
    3rdparty/libreplay.la \
    3rdparty/libminiz.la

3rdparty_libreplay_la_SOURCES =

3rdparty_libminiz_la_SOURCES = \
    3rdparty/miniz/miniz.h \
    3rdparty/miniz/miniz.c

3rdparty_libreplay_la_LIBADD = 3rdparty/libminiz.la

if PLAYER_hivelytracker
noinst_LTLIBRARIES += 3rdparty/replay/libreplay_hivelytracker.la
3rdparty_libreplay_la_LIBADD += 3rdparty/replay/libreplay_hivelytracker.la

3rdparty_replay_libreplay_hivelytracker_la_SOURCES = \
    3rdparty/replay/hivelytracker/replay.h \
    3rdparty/replay/hivelytracker/replay.c

3rdparty/replay/hivelytracker/% : CFLAGS += -Wno-sign-compare -Wno-overflow -Wno-implicit-fallthrough -Wno-stringop-truncation -Wno-unknown-warning-option
endif

if PLAYER_libdigibooster3
noinst_LTLIBRARIES += 3rdparty/replay/libreplay_libdigibooster3.la
3rdparty_libreplay_la_LIBADD += 3rdparty/replay/libreplay_libdigibooster3.la

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
endif

if PLAYER_ft2play
noinst_LTLIBRARIES += 3rdparty/replay/libreplay_ft2play.la
3rdparty_libreplay_la_LIBADD += 3rdparty/replay/libreplay_ft2play.la

3rdparty_replay_libreplay_ft2play_la_SOURCES = \
    3rdparty/replay/ft2play/ft2play.h \
    3rdparty/replay/ft2play/ft2play_play.cc \
    3rdparty/replay/ft2play/ft2play_probe.cc

3rdparty/replay/ft2play/% : CXXFLAGS += -Wno-volatile -Wno-unknown-warning-option
endif

if PLAYER_st3play
noinst_LTLIBRARIES += 3rdparty/replay/libreplay_st3play.la
3rdparty_libreplay_la_LIBADD += 3rdparty/replay/libreplay_st3play.la

3rdparty_replay_libreplay_st3play_la_SOURCES = \
    3rdparty/replay/st3play/st3play.h \
    3rdparty/replay/st3play/st3play_play.cc \
    3rdparty/replay/st3play/st3play_probe.cc

3rdparty/replay/st3play/% : CXXFLAGS += -Wno-keyword-macro
endif

if PLAYER_it2play
noinst_LTLIBRARIES += 3rdparty/replay/libreplay_it2play.la
3rdparty_libreplay_la_LIBADD += 3rdparty/replay/libreplay_it2play.la

3rdparty_replay_libreplay_it2play_la_SOURCES = \
    3rdparty/replay/it2play/it2play.h \
    3rdparty/replay/it2play/it2play_play.cc \
    3rdparty/replay/it2play/it2play_probe.cc

3rdparty/replay/it2play/% : CXXFLAGS += -Wno-misleading-indentation
endif

if PLAYER_st23play
noinst_LTLIBRARIES += 3rdparty/replay/libreplay_st23play.la
3rdparty_libreplay_la_LIBADD += 3rdparty/replay/libreplay_st23play.la

3rdparty_replay_libreplay_st23play_la_SOURCES = \
    3rdparty/replay/st23play/st23play.h \
    3rdparty/replay/st23play/st23play_play.cc \
    3rdparty/replay/st23play/st23play_probe.cc
endif

if PLAYER_noisetrekker2
noinst_LTLIBRARIES += 3rdparty/replay/libreplay_noisetrekker2.la
3rdparty_libreplay_la_LIBADD += 3rdparty/replay/libreplay_noisetrekker2.la
3rdparty_replay_libreplay_noisetrekker2_la_SOURCES = \
    3rdparty/replay/noisetrekker2/noisetrekker2.h \
    3rdparty/replay/noisetrekker2/noisetrekker2_play.cc \
    3rdparty/replay/noisetrekker2/noisetrekker2_probe.cc

3rdparty/replay/noisetrekker2/% : CXXFLAGS += -Wno-char-subscripts -Wno-unused -Wno-sign-compare -Wno-deprecated-declarations -Wno-type-limits
endif

if PLAYER_protrekkr1
noinst_LTLIBRARIES += 3rdparty/replay/libreplay_protrekkr1.la
3rdparty_libreplay_la_LIBADD += 3rdparty/replay/libreplay_protrekkr1.la

3rdparty_replay_libreplay_protrekkr1_la_SOURCES = \
    3rdparty/replay/protrekkr1/protrekkr1.h \
    3rdparty/replay/protrekkr1/protrekkr1_play.cc \
    3rdparty/replay/protrekkr1/protrekkr1_probe.cc

3rdparty/replay/protrekkr1/% : CXXFLAGS += -Wno-char-subscripts -Wno-unused -Wno-sign-compare -Wno-multichar -Wno-implicit-fallthrough -Wno-type-limits
endif

if PLAYER_protrekkr2
noinst_LTLIBRARIES += 3rdparty/replay/libreplay_protrekkr2.la
3rdparty_libreplay_la_LIBADD += 3rdparty/replay/libreplay_protrekkr2.la

3rdparty_replay_libreplay_protrekkr2_la_SOURCES = \
    3rdparty/replay/protrekkr2/protrekkr2.h \
    3rdparty/replay/protrekkr2/protrekkr2_play.cc \
    3rdparty/replay/protrekkr2/protrekkr2_probe.cc

3rdparty/replay/protrekkr2/% : CXXFLAGS += -Wno-char-subscripts -Wno-unused -Wno-sign-compare -Wno-multichar -Wno-implicit-fallthrough -Wno-volatile -Wno-unknown-warning-option
endif

EXTRA_DIST += \
    3rdparty/README \
    3rdparty/miniz/ChangeLog.md \
    3rdparty/miniz/LICENSE \
    3rdparty/miniz/readme.md \
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
    3rdparty/replay/noisetrekker2/whatisnew.txt \
    3rdparty/replay/noisetrekker2/file_id.diz \
    3rdparty/replay/noisetrekker2/readme.txt \
    3rdparty/replay/noisetrekker2/NtkSourceCode/tb303.cpp \
    3rdparty/replay/noisetrekker2/NtkSourceCode/Alphatrack.cpp \
    3rdparty/replay/noisetrekker2/NtkSourceCode/CSynth_gui_pro.cpp \
    3rdparty/replay/noisetrekker2/NtkSourceCode/IMPORTANT_READ.TXT \
    3rdparty/replay/noisetrekker2/NtkSourceCode/cubicspline.cpp \
    3rdparty/replay/noisetrekker2/NtkSourceCode/main.cpp \
    3rdparty/replay/protrekkr1/release/distrib/history.txt \
    3rdparty/replay/protrekkr1/release/distrib/replay/lib/include/endianness.h \
    3rdparty/replay/protrekkr1/release/distrib/replay/lib/include/tb_303.h \
    3rdparty/replay/protrekkr1/release/distrib/replay/lib/include/spline.h \
    3rdparty/replay/protrekkr1/release/distrib/replay/lib/include/synth.h \
    3rdparty/replay/protrekkr1/release/distrib/replay/lib/include/ptk_def_properties.h \
    3rdparty/replay/protrekkr1/release/distrib/replay/lib/include/cubic_spline.h \
    3rdparty/replay/protrekkr1/release/distrib/replay/lib/include/samples_unpack.h \
    3rdparty/replay/protrekkr1/release/distrib/replay/lib/include/replay.h \
    3rdparty/replay/protrekkr1/release/distrib/replay/lib/include/ptkreplay.h \
    3rdparty/replay/protrekkr1/release/distrib/replay/lib/samples_unpack.cpp \
    3rdparty/replay/protrekkr1/release/distrib/replay/lib/tb_303.cpp \
    3rdparty/replay/protrekkr1/release/distrib/replay/lib/synth.cpp \
    3rdparty/replay/protrekkr1/release/distrib/replay/lib/replay.cpp \
    3rdparty/replay/protrekkr1/release/distrib/replay/lib/spline.cpp \
    3rdparty/replay/protrekkr1/release/distrib/replay/lib/endianness.cpp \
    3rdparty/replay/protrekkr1/src/editors/include/patterns_blocks.h \
    3rdparty/replay/protrekkr1/src/editors/patterns_blocks.cpp \
    3rdparty/replay/protrekkr1/src/files/files.cpp \
    3rdparty/replay/protrekkr2/LICENSE \
    3rdparty/replay/protrekkr2/README.md \
    3rdparty/replay/protrekkr2/release/distrib/changes.txt \
    3rdparty/replay/protrekkr2/release/distrib/license.txt \
    3rdparty/replay/protrekkr2/release/distrib/replay/lib/include/cubic_spline.h \
    3rdparty/replay/protrekkr2/release/distrib/replay/lib/include/endianness.h \
    3rdparty/replay/protrekkr2/release/distrib/replay/lib/include/ptk_def_properties.h \
    3rdparty/replay/protrekkr2/release/distrib/replay/lib/include/ptkreplay.h \
    3rdparty/replay/protrekkr2/release/distrib/replay/lib/include/replay.h \
    3rdparty/replay/protrekkr2/release/distrib/replay/lib/include/samples_unpack.h \
    3rdparty/replay/protrekkr2/release/distrib/replay/lib/include/spline.h \
    3rdparty/replay/protrekkr2/release/distrib/replay/lib/include/synth.h \
    3rdparty/replay/protrekkr2/release/distrib/replay/lib/include/tb_303.h \
    3rdparty/replay/protrekkr2/release/distrib/replay/lib/include/wavpack.h \
    3rdparty/replay/protrekkr2/release/distrib/replay/lib/endianness.cpp \
    3rdparty/replay/protrekkr2/release/distrib/replay/lib/replay.cpp \
    3rdparty/replay/protrekkr2/release/distrib/replay/lib/samples_unpack.cpp \
    3rdparty/replay/protrekkr2/release/distrib/replay/lib/spline.cpp \
    3rdparty/replay/protrekkr2/release/distrib/replay/lib/synth.cpp \
    3rdparty/replay/protrekkr2/release/distrib/replay/lib/tb_303.cpp \
    3rdparty/replay/protrekkr2/release/distrib/replay/lib/wavpack.cpp \
    3rdparty/replay/protrekkr2/src/editors/include/patterns_blocks.h \
    3rdparty/replay/protrekkr2/src/editors/patterns_blocks.cpp \
    3rdparty/replay/protrekkr2/src/files/303s.cpp \
    3rdparty/replay/protrekkr2/src/files/files.cpp \
    3rdparty/replay/protrekkr2/src/files/reverbs.cpp \
    3rdparty/replay/protrekkr2/src/files/synths.cpp
