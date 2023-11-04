AUTOMAKE_OPTIONS = subdir-objects
AM_CFLAGS = -fvisibility=hidden -fdata-sections -ffunction-sections
AM_CXXFLAGS = ${AM_CFLAGS} -fno-rtti -Wall
if BUILD_MACOS
AM_LDFLAGS = -Wl,-dead_strip -Wl,-undefined,error
SHREXT = -shrext .dylib
else
AM_LDFLAGS = -Wl,--gc-sections -Wl,--as-needed -Wl,--exclude-libs=ALL -Wl,--discard-all -Wl,--no-undefined -Wl,-O1
endif
uadedir = $(prefix)/share/audacious-uade
uadedatadir = $(uadedir)/share/uade
uadelibdir = $(uadedir)/lib/uade
plugindir = $(AUDACIOUS_PLUGINDIR)/Input
plugin_LTLIBRARIES = uade.la
uade_la_SOURCES = \
    common.h \
    extensions.h \
    prefs.h \
    plugin.cc \
    hacks.h \
    hacks.cc \
    modland.h \
    modland.cc \
    songdb.h \
    songdb.cc \
    uade_common.h \
    uade_common.cc \
    uade_config.h \
    uade_config.cc \
    converter/big_endian.h \
    converter/converter.h \
    converter/converter.cc \
    converter/common_med.h \
    converter/common_med.cc \
    converter/converter_med4.cc \
    player/player.h \
    player/player.cc \
    player/player_hvl.cc \
    player/player_dbm.cc \
    songend/songend.h \
    songend/songend.cc

lib3rdparty_a_SOURCES = \
    3rdparty/macos_endian.h \
    3rdparty/md5.h \
    3rdparty/md5.cc \
    3rdparty/proplayer.h \
    3rdparty/SimpleBinStream.h \
    3rdparty/hvl/replay.h \
    3rdparty/hvl/replay.c \
    3rdparty/dbm/dsp.h \
    3rdparty/dbm/dsp_echo.c \
    3rdparty/dbm/dsp_fetchinstr.c \
    3rdparty/dbm/dsp_linresampler.c \
    3rdparty/dbm/dsp_panoramizer.c \
    3rdparty/dbm/dsp_wavetable.c \
    3rdparty/dbm/dsp_zeropadder.c \
    3rdparty/dbm/libdigibooster3.h \
    3rdparty/dbm/lists.h \
    3rdparty/dbm/loader.c \
    3rdparty/dbm/musicmodule.h \
    3rdparty/dbm/player.h \
    3rdparty/dbm/player.c

lib3rdparty_a_CPPFLAGS = -DTARGET_LINUX=1
lib3rdparty_a_CFLAGS = ${AM_CFLAGS} -Wno-parentheses -fPIC
lib3rdparty_a_CXXFLAGS = ${AM_CFLAGS} -fpermissive -fPIC
noinst_LIBRARIES = lib3rdparty.a

if !SYSTEM_LIBUADE
uade_la_DEPENDENCIES = lib3rdparty.a $(UADE_LIBS)
endif
uade_la_LIBADD = lib3rdparty.a $(UADE_LIBS) $(AUDACIOUS_LIBS) $(LIBBSD_LIBS)
uade_la_LDFLAGS = -avoid-version -module -shared ${AM_LDFLAGS} ${SHREXT}
uade_la_CPPFLAGS = -DAUDACIOUS=1 -DUADEDIR="\"$(uadedir)\"" -DUADEDATADIR="\"$(uadedatadir)\""
uade_la_CFLAGS = ${AM_CFLAGS} $(UADE_CFLAGS) $(AUDACIOUS_CFLAGS)
uade_la_CXXFLAGS = ${AM_CXXFLAGS} $(UADE_CFLAGS) $(AUDACIOUS_CFLAGS)

install-data-hook:
# TODO better way
# remove .la cruft
	rm -f "$(DESTDIR)$(plugindir)/uade.la" || true
	mkdir -p "$(DESTDIR)$(uadedir)"
	cp -rf "$(srcdir)/../conf/songdb" "$(DESTDIR)$(uadedir)/"
	cp -rf "$(srcdir)/../conf/ext" "$(DESTDIR)$(uadedir)/"
	mkdir -p "$(DESTDIR)$(uadedir)/doc"
	cp -f "$(srcdir)/../README" "$(srcdir)/../NOTICE" "$(srcdir)/../AUTHORS" "$(srcdir)/../COPYING" "$(srcdir)/../VERSION" "$(srcdir)/../ChangeLog" "$(DESTDIR)$(uadedir)/doc/"
	mkdir -p "$(DESTDIR)$(uadedatadir)"
	cp -f "$(srcdir)/../conf/song.conf" "$(srcdir)/../conf/uade.conf" "$(DESTDIR)$(uadedatadir)/"
# make sure to remove any leftover symlinks if --with-system-libuade was used previously
	rm -f "$(DESTDIR)$(uadedatadir)/eagleplayer.conf" || true
	rm -rf "$(DESTDIR)$(uadedatadir)/players" || true

if !SYSTEM_LIBUADE
	cp -f "$(srcdir)/../uade/amigasrc/score/score" "$(srcdir)/../uade/uaerc" "$(srcdir)/../uade/eagleplayer.conf" "$(DESTDIR)$(uadedatadir)/"
	cp -rf "$(srcdir)/../uade/players" "$(DESTDIR)$(uadedatadir)/"
	mkdir -m 755 -p "$(DESTDIR)$(uadelibdir)"
	install "$(top_builddir)/uade/src/uadecore" "$(DESTDIR)$(uadelibdir)/"
else
# XXX hack but there seems no other way. system score, uaerc and uadecore locations are set via API
	ln -sf "$(UADE_SYSTEM_DATADIR)/eagleplayer.conf" "$(DESTDIR)$(uadedatadir)/eagleplayer.conf"
	ln -sf "$(UADE_SYSTEM_DATADIR)/players" "$(DESTDIR)$(uadedatadir)/players"
endif
	chmod -R 755 "$(DESTDIR)$(uadedir)"

uninstall-hook:
	rm -rf "$(DESTDIR)$(uadedir)" || true
if BUILD_MACOS
	rm -f "$(DESTDIR)$(plugindir)/uade.dylib" || true
else
	rm -f "$(DESTDIR)$(plugindir)/uade.so" || true
endif
