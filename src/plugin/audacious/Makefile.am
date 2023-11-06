AUTOMAKE_OPTIONS = subdir-objects

uadedir = $(prefix)/share/audacious-uade
uadedatadir = $(uadedir)/share/uade
uadelibdir = $(uadedir)/lib/uade
plugindir = $(AUDACIOUS_PLUGINDIR)/Input

AM_CPPFLAGS = -I$(top_srcdir)/src

plugin_LTLIBRARIES = uade.la
uade_la_SOURCES = \
    ../../common.h \
    ../../extensions.h \
    prefs.h \
    plugin.cc \
    uade_config.h \
    uade_config.cc

uade_la_DEPENDENCIES = \
    ../../songdb/libsongdb.a \
    ../../player/libplayer.a \
    ../../uade/libuadesupport.a \
    ../../songend/libsongend.a \
    ../../converter/libconverter.a \
    ../../3rdparty/libreplay.a \
    ../../3rdparty/libmd5.a \
    $(UADE_LIBS)

uade_la_LIBADD = $(uade_la_DEPENDENCIES) $(AUDACIOUS_LIBS) $(LIBBSD_LIBS)
uade_la_LDFLAGS = -avoid-version -module -shared $(SHREXT)
uade_la_CPPFLAGS = $(AM_CPPFLAGS) -DAUDACIOUS=1 -DUADEDIR="\"$(uadedir)\"" -DUADEDATADIR="\"$(uadedatadir)\""
uade_la_CFLAGS = $(AUDACIOUS_CFLAGS) $(UADE_CFLAGS)
uade_la_CXXFLAGS = $(AUDACIOUS_CFLAGS) $(UADE_CFLAGS)

install-data-hook:
# TODO better way
# remove .la cruft
	rm -f "$(DESTDIR)$(plugindir)/uade.la" || true
	mkdir -p "$(DESTDIR)$(uadedir)"
	cp -rf "$(top_srcdir)/conf/songdb" "$(DESTDIR)$(uadedir)/"
	cp -rf "$(top_srcdir)/conf/ext" "$(DESTDIR)$(uadedir)/"
	mkdir -p "$(DESTDIR)$(uadedir)/doc"
	cp -f "$(top_srcdir)/README" "$(top_srcdir)/NOTICE" "$(top_srcdir)/AUTHORS" "$(top_srcdir)/COPYING" "$(top_srcdir)/VERSION" "$(top_srcdir)/ChangeLog" "$(DESTDIR)$(uadedir)/doc/"
	mkdir -p "$(DESTDIR)$(uadedatadir)"
	cp -f "$(top_srcdir)/conf/song.conf" "$(top_srcdir)/conf/uade.conf" "$(DESTDIR)$(uadedatadir)/"
	cp -f "$(top_srcdir)/uade/amigasrc/score/score" "$(top_srcdir)/uade/uaerc" "$(top_srcdir)/uade/eagleplayer.conf" "$(DESTDIR)$(uadedatadir)/"
	cp -rf "$(top_srcdir)/uade/players" "$(DESTDIR)$(uadedatadir)/"
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
