AUTOMAKE_OPTIONS = subdir-objects
noinst_LIBRARIES = libuadesupport.a

uadedir = $(prefix)/share/audacious-uade
uadedatadir = $(uadedir)/share/uade

AM_CPPFLAGS = -I$(top_srcdir)/src

libuadesupport_a_SOURCES = \
    hacks.h \
    hacks.cc \
    uade_common.h \
    uade_common.cc \
    ../common.h \
    ../extensions.h

libuadesupport_a_DEPENDENCIES = \
    ../songend/libsongend.a \
    ../player/libplayer.a \
    $(UADE_LIBS)

libuadesupport_a_CPPFLAGS = $(AM_CPPFLAGS) -DUADEDIR="\"$(uadedir)\"" -DUADEDATADIR="\"$(uadedatadir)\""
