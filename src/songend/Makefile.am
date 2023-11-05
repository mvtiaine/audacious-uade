AUTOMAKE_OPTIONS = subdir-objects
noinst_LIBRARIES = libsongend.a
noinst_PROGRAMS = songend uade_songend
EXTRA_DIST = songdb.sh

uadedir = $(prefix)/share/audacious-uade
uadedatadir = $(uadedir)/share/uade

libsongend_a_SOURCES = \
    songend.h \
    songend.cc

songend_SOURCES = \
    songend_main.cc
songend_DEPENDENCIES = libsongend.a
songend_LDADD = $(songend_DEPENDENCIES)

uade_songend_SOURCES = \
    uade_songend_main.cc \
    ../uade_common.h \
    ../uade_common.cc \
    ../hacks.h \
    ../hacks.cc \
    ../converter/converter.h \
    ../player/player.h

uade_songend_DEPENDENCIES = \
    libsongend.a \
    ../player/libplayer.a \
    ../converter/libconverter.a \
    ../3rdparty/lib3rdparty.a \
    $(UADE_LIBS)

uade_songend_LDADD = $(uade_songend_DEPENDENCIES) $(LIBBSD_LIBS)
uade_songend_CPPFLAGS = -DUADEDIR="\"$(uadedir)\"" -DUADEDATADIR="\"$(uadedatadir)\""
uade_songend_CFLAGS = $(UADE_CFLAGS)
uade_songend_CXXFLAGS = $(UADE_CFLAGS)
