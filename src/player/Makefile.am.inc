# NOT a standalone Makefile.am

noinst_LTLIBRARIES += \
    player/libplayer.la

player_libplayer_la_SOURCES = \
    player/player.h \
    player/player.cc \
    player/players/internal.h

if PLAYER_ft2play
player_libplayer_la_SOURCES += player/players/player_ft2play.cc
endif

if PLAYER_hivelytracker
player_libplayer_la_SOURCES += player/players/player_hivelytracker.cc
endif

if PLAYER_it2play
player_libplayer_la_SOURCES += player/players/player_it2play.cc
endif

if PLAYER_libdigibooster3
player_libplayer_la_SOURCES += player/players/player_libdigibooster3.cc
endif

if PLAYER_protrekkr
player_libplayer_la_SOURCES += player/players/player_protrekkr.cc
endif

if PLAYER_st23play
player_libplayer_la_SOURCES += player/players/player_st23play.cc
endif

if PLAYER_st3play
player_libplayer_la_SOURCES += player/players/player_st3play.cc
endif

if PLAYER_uade
player_libplayer_la_SOURCES += player/players/player_uade.cc
# TODO fix libuade dependency tracking
player_libplayer_la_CPPFLAGS = $(AM_CPPFLAGS) $(UADE_CPPFLAGS)
endif

player_libplayer_la_LIBADD = \
    converter/libconverter.la \
    3rdparty/libreplay.la

player/players/% : CXXFLAGS += -Wno-keyword-macro
