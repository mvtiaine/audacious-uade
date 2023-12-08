AUTOMAKE_OPTIONS = subdir-objects

uadedir = $(prefix)/share/audacious-uade
uadedatadir = $(uadedir)/share/uade
uadelibdir = $(uadedir)/lib/uade
plugin_audaciousdir = $(AUDACIOUS_PLUGINDIR)/Input

AM_CPPFLAGS = -I$(top_srcdir)/src -DUADEDIR="\"$(uadedir)\"" -DUADEDATADIR="\"$(uadedatadir)\""

EXTRA_DIST = 

## Libs

noinst_LTLIBRARIES = 

include $(srcdir)/3rdparty/Makefile.am.inc
include $(srcdir)/common/Makefile.am.inc
include $(srcdir)/converter/Makefile.am.inc
include $(srcdir)/player/Makefile.am.inc
include $(srcdir)/songdb/Makefile.am.inc
include $(srcdir)/songend/Makefile.am.inc

## Programs

noinst_PROGRAMS = 

include $(srcdir)/plugin/cli/Makefile.am.inc

## Tests

check_PROGRAMS =

include $(srcdir)/test/Makefile.am.inc

## Audacious plugin

include $(srcdir)/plugin/audacious/Makefile.am.inc

install-data-hook:
# TODO better way
# remove .la cruft
	rm -f "$(DESTDIR)$(plugin_audaciousdir)/uade.la" || true
	mkdir -p "$(DESTDIR)$(uadedir)"
	rm -f "$(DESTDIR)$(uadedir)/songdb/"* || true
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
	rm -f "$(DESTDIR)$(plugin_audaciousdir)/uade.dylib" || true
else
	rm -f "$(DESTDIR)$(plugin_audaciousdir)/uade.so" || true
endif
