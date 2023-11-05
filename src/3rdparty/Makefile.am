AUTOMAKE_OPTIONS = subdir-objects
noinst_LIBRARIES = lib3rdparty.a

md5_SOURCES = \
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

lib3rdparty_a_SOURCES = $(md5_SOURCES) $(hvl_SOURCES) $(dbm_SOURCES)
lib3rdparty_a_CPPFLAGS = -DTARGET_LINUX=1
lib3rdparty_a_CFLAGS = -Wno-parentheses -fPIC
lib3rdparty_a_CXXFLAGS = -fpermissive -fPIC
