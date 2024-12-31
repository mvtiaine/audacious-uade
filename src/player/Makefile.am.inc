# NOT a standalone Makefile.am

noinst_LTLIBRARIES += \
    player/libplayer.la

player_libplayer_la_SOURCES = \
    player/player.h \
    player/player.cc \
    player/players/internal.h \
    player/players/player_ft2play.cc \
    player/players/player_hivelytracker.cc \
    player/players/player_it2play.cc \
    player/players/player_libdigibooster3.cc \
    player/players/player_st23play.cc \
    player/players/player_st3play.cc \
    player/players/player_uade.cc

player_libplayer_la_LIBADD = \
    converter/libconverter.la \
    3rdparty/libreplay.la

# TODO fix libuade dependency tracking

player_libplayer_la_CPPFLAGS = $(AM_CPPFLAGS) $(UADE_CPPFLAGS)
