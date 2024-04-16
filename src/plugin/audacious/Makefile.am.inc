# NOT a standalone Makefile.am

plugin_audacious_LTLIBRARIES = plugin/audacious/uade.la
plugin_audacious_uade_la_SOURCES = \
    common/extensions.h \
    common/logger.h \
    common/songend.h \
    common/strings.h \
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

plugin_audacious_uade_la_LDFLAGS = -avoid-version -module -shared -no-undefined $(SHREXT) -Xcompiler $(UADE_LIBS)
if BUILD_CYGWIN
plugin_audacious_uade_la_LDFLAGS += -Xcompiler -lcygwin
endif
if BUILD_MINGW
if LD_GNU
plugin_audacious_uade_la_LDFLAGS += -Xcompiler -lmsvcrt -Xcompiler -lmingwex -Xcompiler -lmsvcrt -Xcompiler -lkernel32 -Xcompiler -lws2_32
endif
endif
plugin_audacious_uade_la_CXXFLAGS = $(AUDACIOUS_CFLAGS)
