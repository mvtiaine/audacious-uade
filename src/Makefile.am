AUTOMAKE_OPTIONS = subdir-objects
AM_CXXFLAGS = -fno-rtti -flto -fvisibility=hidden -fdata-sections -ffunction-sections -Wall
if BUILD_MACOS
AM_LDFLAGS = -Wl,-dead_strip -Wl,-undefined,error
SHREXT = -shrext .dylib
else
AM_LDFLAGS = -Wl,--gc-sections -Wl,--as-needed -Wl,--exclude-libs=ALL -Wl,--discard-all -WL,--no-undefined -Wl,-O1
endif
uadedir = $(prefix)/share/audacious/uade
uadedatadir = $(uadedir)/share/uade
uadelibdir = $(uadedir)/lib/uade
plugindir = $(AUDACIOUS_PLUGINDIR)/Input
plugin_LTLIBRARIES = uade.la
uade_la_SOURCES = \
    plugin.cc \
    modland.cc \
    hacks.cc \
    uade_config.cc \
    converter/converter.cc \
    converter/common_med.cc \
    converter/converter_med4.cc \
    3rdparty/md5.cc
if !SYSTEM_LIBUADE
uade_la_DEPENDENCIES = $(UADE_LIBS)
endif
uade_la_LIBADD = $(UADE_LIBS) $(AUDACIOUS_LIBS)
uade_la_LDFLAGS = -avoid-version -module -shared  ${AM_LDFLAGS} ${SHREXT}
uade_la_CXXFLAGS = -DUADEDIR="\"$(uadedir)\"" -DUADEDATADIR="\"$(uadedatadir)\"" ${AM_CXXFLAGS} $(UADE_CFLAGS) $(AUDACIOUS_CFLAGS)

install-data-hook:
# TODO better way
# remove .la cruft
	rm -f "$(plugindir)/uade.la" || true
	mkdir -p "$(uadedir)"
	cp -f ../conf/allmods_md5_amiga.txt "$(uadedir)/"
	mkdir -p "$(uadedatadir)"
	cp -f ../conf/contentdb ../conf/song.conf ../conf/uade.conf "$(uadedatadir)/"
# make sure to remove any leftover symlinks if --with-system-libuade was used previously
	rm -f "$(uadedatadir)/eagleplayer.conf" || true
	rm -f "$(uadedatadir)/players" || true

if !SYSTEM_LIBUADE
	cp -f ../uade/amigasrc/score/score ../uade/uaerc ../uade/eagleplayer.conf "$(uadedatadir)/"
	cp -rf ../uade/players "$(uadedatadir)/"
	mkdir -m 755 -p "$(uadelibdir)"
	install "../uade/src/uadecore" "$(uadelibdir)/"
else
# XXX hack but there seems no other way. system score, uaerc and uadecore locations are set via API
	ln -sf "$(UADE_SYSTEM_DATADIR)/eagleplayer.conf" "$(uadedatadir)/eagleplayer.conf"
	ln -sf "$(UADE_SYSTEM_DATADIR)/players" "$(uadedatadir)/players"
endif

uninstall-hook:
	rm -rf "$(uadedir)" || true
if BUILD_MACOS
	rm -f "$(plugindir)/uade.dylib" || true
else
	rm -f "$(plugindir)/uade.so" || true
endif
