AUTOMAKE_OPTIONS = subdir-objects
AM_CXXFLAGS = -fno-rtti -fvisibility=hidden -fdata-sections -ffunction-sections -Wall
if BUILD_MACOS
AM_LDFLAGS = -Wl,-dead_strip -Wl,-undefined,error
SHREXT = -shrext .dylib
else
AM_LDFLAGS = -Wl,--gc-sections -Wl,--as-needed -Wl,--exclude-libs=ALL -Wl,--discard-all -WL,--no-undefined -Wl,-O1
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
    songend/songend.h \
    songend/songend.cc \
    3rdparty/macos_endian.h \
    3rdparty/md5.h \
    3rdparty/md5.cc \
    3rdparty/proplayer.h \
    3rdparty/SimpleBinStream.h
if !SYSTEM_LIBUADE
uade_la_DEPENDENCIES = $(UADE_LIBS)
endif
uade_la_LIBADD = $(UADE_LIBS) $(AUDACIOUS_LIBS)
uade_la_LDFLAGS = -avoid-version -module -shared  ${AM_LDFLAGS} ${SHREXT}
uade_la_CXXFLAGS = -DAUDACIOUS=1 -DUADEDIR="\"$(uadedir)\"" -DUADEDATADIR="\"$(uadedatadir)\"" ${AM_CXXFLAGS} $(UADE_CFLAGS) $(AUDACIOUS_CFLAGS)

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
	rm -f "$(DESTDIR)$(uadedatadir)/players" || true

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
