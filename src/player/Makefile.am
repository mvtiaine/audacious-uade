AUTOMAKE_OPTIONS = subdir-objects
noinst_LIBRARIES = libplayer.a

AM_CPPFLAGS = -I$(top_srcdir)/src

libplayer_a_SOURCES = \
    foreach.h \
    player.h \
    player.cc \
    player_dbm.cc \
    player_hvl.cc

libplayer_a_DEPENDENCIES = \
    ../3rdparty/libreplay.a
