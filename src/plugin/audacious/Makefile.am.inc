# NOT a standalone Makefile.am

plugin_audacious_LTLIBRARIES = plugin/audacious/uade.la
plugin_audacious_uade_la_SOURCES = \
    common/common.h \
    common/extensions.h \
    common/logger.h \
    plugin/audacious/prefs.h \
    plugin/audacious/uade_prefs.h \
	plugin/audacious/logger.cc \
    plugin/audacious/plugin.cc

plugin_audacious_uade_la_LIBADD = \
    songend/libsongend.la \
    songdb/libsongdb.la \
    common/libmd5.la \
	$(UADE_LIBS) \
	$(AUDACIOUS_LIBS)

plugin_audacious_uade_la_LDFLAGS = -avoid-version -module -shared $(SHREXT)
plugin_audacious_uade_la_CXXFLAGS = $(AUDACIOUS_CFLAGS)
