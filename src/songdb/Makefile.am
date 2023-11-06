AUTOMAKE_OPTIONS = subdir-objects
noinst_LIBRARIES = libsongdb.a
noinst_PROGRAMS = uade_songend
EXTRA_DIST = songdb.sh

uadedir = $(prefix)/share/audacious-uade
uadedatadir = $(uadedir)/share/uade

AM_CPPFLAGS = -I$(top_srcdir)/src

libsongdb_a_SOURCES = \
    modland.h \
    modland.cc \
    songdb.h \
    songdb.cc \
    ../common.h

libsongdb_a_DEPENDENCIES = \
    ../uade/libuadesupport.a

libsongdb_a_CPPFLAGS = $(AM_CPPFLAGS) -DUADEDIR="\"$(uadedir)\"" -DUADEDATADIR="\"$(uadedatadir)\""

uade_songend_SOURCES = \
    uade_songend_main.cc

uade_songend_DEPENDENCIES = \
    ../uade/libuadesupport.a \
    ../player/libplayer.a \
    ../converter/libconverter.a \
    ../3rdparty/libreplay.a \
    ../3rdparty/libmd5.a \
    ../songend/libsongend.a \
    $(UADE_LIBS)

uade_songend_LDADD = $(uade_songend_DEPENDENCIES) $(LIBBSD_LIBS)
uade_songend_CPPFLAGS = $(AM_CPPFLAGS) -DUADEDIR="\"$(uadedir)\"" -DUADEDATADIR="\"$(uadedatadir)\""
uade_songend_CFLAGS = $(UADE_CFLAGS)
uade_songend_CXXFLAGS = $(UADE_CFLAGS)
