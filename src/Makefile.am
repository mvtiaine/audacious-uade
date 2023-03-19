plugindir = $(AUDACIOUS_PLUGINDIR)/Input
plugin_LTLIBRARIES = uade.la
uade_la_SOURCES = plugin.cc modland.cc hacks.c
uade_la_LDFLAGS = -module -avoid-version $(UADE_LIBS) $(AUDACIOUS_LIBS)
uade_la_CFLAGS = $(UADE_CFLAGS) $(AUDACIOUS_CFLAGS)

if BUILD_MACOS
# for MacOS symlink uade.dylib -> uade.so
# TODO better way (build .dylib directly) and avoid installing .la
install-exec-hook:
	ln -sf $(plugindir)/uade.so $(plugindir)/uade.dylib
	rm  $(plugindir)/uade.la
endif
