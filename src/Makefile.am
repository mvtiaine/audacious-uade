AUTOMAKE_OPTIONS = subdir-objects

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
    3rdparty/md5.h \
    converter/converter.h \
    player/player.h \
    songend/songend.h

uade_la_DEPENDENCIES = \
    songend/libsongend.a \
    player/libplayer.a \
    converter/libconverter.a \
    3rdparty/lib3rdparty.a \
    $(UADE_LIBS)

uade_la_LIBADD = $(uade_la_DEPENDENCIES) $(AUDACIOUS_LIBS) $(LIBBSD_LIBS)
uade_la_LDFLAGS = -avoid-version -module -shared $(SHREXT)
uade_la_CPPFLAGS = -DAUDACIOUS=1 -DUADEDIR="\"$(uadedir)\"" -DUADEDATADIR="\"$(uadedatadir)\""
uade_la_CFLAGS = $(AUDACIOUS_CFLAGS) $(UADE_CFLAGS)
uade_la_CXXFLAGS = $(AUDACIOUS_CFLAGS) $(UADE_CFLAGS)

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
	cp -f "$(srcdir)/../uade/amigasrc/score/score" "$(srcdir)/../uade/uaerc" "$(srcdir)/../uade/eagleplayer.conf" "$(DESTDIR)$(uadedatadir)/"
	cp -rf "$(srcdir)/../uade/players" "$(DESTDIR)$(uadedatadir)/"
	mkdir -m 755 -p "$(DESTDIR)$(uadelibdir)"
	install "$(top_builddir)/uade/src/uadecore" "$(DESTDIR)$(uadelibdir)/"
	chmod -R 755 "$(DESTDIR)$(uadedir)"

uninstall-hook:
	rm -rf "$(DESTDIR)$(uadedir)" || true
if BUILD_MACOS
	rm -f "$(DESTDIR)$(plugindir)/uade.dylib" || true
else
	rm -f "$(DESTDIR)$(plugindir)/uade.so" || true
endif
