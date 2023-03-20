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

install-exec-hook:
# for MacOS symlink uade.dylib -> uade.so
# TODO better way (build .dylib directly)
if BUILD_MACOS
	ln -sf $(plugindir)/uade.so $(plugindir)/uade.dylib
endif
# TODO avoid installing .la
if !BUILD_WIN32
	rm -f $(plugindir)/uade.la
endif
