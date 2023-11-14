# NOT a standalone Makefile.am

noinst_LTLIBRARIES += \
	player/libplayer.la

player_libplayer_la_SOURCES = \
    player/foreach.h \
    player/player.h \
    player/player.cc \
    player/players/player_dbm.cc \
    player/players/player_hvl.cc \
    player/players/player_uade.cc \
    common/common.h \
    common/logger.h

player_libplayer_la_LIBADD = \
    converter/libconverter.la \
    3rdparty/libreplay.la \
    $(UADE_LIBS)

player_libplayer_la_CPPFLAGS = $(AM_CPPFLAGS) $(UADE_CPPFLAGS)
