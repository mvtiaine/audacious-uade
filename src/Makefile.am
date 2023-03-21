AUTOMAKE_OPTIONS = subdir-objects
plugindir = $(AUDACIOUS_PLUGINDIR)/Input
plugin_LTLIBRARIES = uade.la
uade_la_SOURCES = \
    plugin.cc \
    modland.cc \
    hacks.cc \
    converter/converter.cc \
    converter/common_med.cc \
    converter/converter_med4.cc \
    3rdparty/md5.cc
uade_la_LDFLAGS = -avoid-version -module -shared $(UADE_LIBS) $(AUDACIOUS_LIBS)
uade_la_CFLAGS = $(UADE_CFLAGS) $(AUDACIOUS_CFLAGS)

install-data-hook:
# TODO better way (build .dylib directly)
# for MacOS symlink uade.dylib -> uade.so
if BUILD_MACOS
	ln -sf $(plugindir)/uade.so $(plugindir)/uade.dylib
endif
# TODO better way
# remove .la cruft
	rm -f $(plugindir)/uade.la
