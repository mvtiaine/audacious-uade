# NOT a standalone Makefile.am

noinst_LTLIBRARIES += \
    player/libplayer.la

player_libplayer_la_SOURCES = \
    player/player.h \
    player/player.cc \
    player/players/player_ft2play.cc \
    player/players/player_hivelytracker.cc \
    player/players/player_it2play.cc \
    player/players/player_libdigibooster3.cc \
    player/players/player_st3play.cc \
    player/players/player_uade.cc

player_libplayer_la_LIBADD = \
    converter/libconverter.la \
    3rdparty/libreplay.la

# XXX wasm-ld fails with "section too large" if libuade.a is included here
if !BUILD_WASIX
player_libplayer_la_LIBADD += $(UADE_LIBS)
endif

player_libplayer_la_CPPFLAGS = $(AM_CPPFLAGS) $(UADE_CPPFLAGS)
