AUTOMAKE_OPTIONS = subdir-objects

if BUILD_MINGW
uadedir = `cygpath -w -m $(prefix)/share/audacious-uade`
else
uadedir = $(prefix)/share/audacious-uade
endif
uadedatadir = $(uadedir)/share/uade
uadelibdir = $(uadedir)/lib/uade
plugin_audaciousdir = $(AUDACIOUS_PLUGINDIR)/Input

AM_CPPFLAGS = -I$(top_srcdir)/src -DUADEDIR="\"$(uadedir)\"" -DUADEDATADIR="\"$(uadedatadir)\""

EXTRA_DIST = config.h

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
if PLUGIN_AUDACIOUS
include $(srcdir)/plugin/audacious/Makefile.am.inc
endif

install-data-hook:
# TODO better way
# remove .la cruft
if PLUGIN_AUDACIOUS
	rm -f "$(DESTDIR)$(plugin_audaciousdir)/uade.la" || true
if BUILD_WINDOWS
	rm -f "$(DESTDIR)$(plugin_audaciousdir)/uade.dll.a" || true
endif
endif
	$(MKDIR_P) "$(DESTDIR)$(uadedir)"
	rm -f "$(DESTDIR)$(uadedir)/songdb/"* || true
	cp -rf "$(top_srcdir)/conf/songdb" "$(DESTDIR)$(uadedir)/"
	cp -rf "$(top_srcdir)/conf/ext" "$(DESTDIR)$(uadedir)/"
	$(MKDIR_P) "$(DESTDIR)$(uadedir)/doc"
	cp -f "$(top_srcdir)/README" "$(top_srcdir)/NOTICE" "$(top_srcdir)/AUTHORS" "$(top_srcdir)/COPYING"* "$(top_srcdir)/VERSION" "$(top_srcdir)/ChangeLog" "$(DESTDIR)$(uadedir)/doc/"
	$(MKDIR_P) "$(DESTDIR)$(uadedatadir)"
	cp -f "$(top_srcdir)/conf/song.conf" "$(top_srcdir)/conf/uade.conf" "$(DESTDIR)$(uadedatadir)/"
	cp -f "$(top_srcdir)/uade/amigasrc/score/score" "$(top_srcdir)/uade/uaerc" "$(top_srcdir)/uade/eagleplayer.conf" "$(DESTDIR)$(uadedatadir)/"
	cp -rf "$(top_srcdir)/uade/players" "$(DESTDIR)$(uadedatadir)/"
	$(MKDIR_P) -m 755 "$(DESTDIR)$(uadelibdir)"
	$(INSTALL) "$(top_builddir)/uade/src/uadecore" "$(DESTDIR)$(uadelibdir)/"
	chmod -R 755 "$(DESTDIR)$(uadelibdir)"
	$(MKDIR_P) "$(DESTDIR)$(uadedir)/doc/uade"
	cp -f "$(top_srcdir)/uade/AUTHORS" "$(top_srcdir)/uade/COPYING"* "$(top_srcdir)/uade/ChangeLog" "$(top_srcdir)/uade/README" "$(top_srcdir)/uade/version" "$(DESTDIR)$(uadedir)/doc/uade/"

uninstall-hook:
	rm -rf "$(DESTDIR)$(uadedir)" || true
if PLUGIN_AUDACIOUS
	rm -f "$(DESTDIR)$(plugin_audaciousdir)/uade."* || true
endif
