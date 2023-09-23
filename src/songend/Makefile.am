AUTOMAKE_OPTIONS = subdir-objects
EXTRA_PROGRAMS = songend uade_songend
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
    ../uade_common.cc \
    ../hacks.cc \
    ../converter/converter.cc \
    ../converter/common_med.cc \
    ../converter/converter_med4.cc \
    ../3rdparty/md5.h \
    ../3rdparty/md5.cc
uade_songend_CXXFLAGS = -DUADEDIR="\"$(uadedir)\"" -DUADEDATADIR="\"$(uadedatadir)\"" $(UADE_CFLAGS)
uade_songend_DEPENDENCIES = $(UADE_LIBS)
uade_songend_LDADD = $(UADE_LIBS)
