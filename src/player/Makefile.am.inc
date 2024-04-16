# NOT a standalone Makefile.am

noinst_LTLIBRARIES += \
    player/libplayer.la

player_libplayer_la_SOURCES = \
    player/player.h \
    player/player.cc \
    player/players/player_libdigibooster3.cc \
    player/players/player_hivelytracker.cc \
    player/players/player_uade.cc \
    common/compat.h \
    common/foreach.h \
    common/logger.h \
    common/songend.h \
    common/strings.h

player_libplayer_la_LIBADD = \
    converter/libconverter.la \
    3rdparty/libreplay.la \
    $(UADE_LIBS)

player_libplayer_la_CPPFLAGS = $(AM_CPPFLAGS) $(UADE_CPPFLAGS)
