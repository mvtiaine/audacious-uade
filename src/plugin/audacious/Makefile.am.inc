# NOT a standalone Makefile.am

plugin_audacious_LTLIBRARIES = plugin/audacious/uade.la
plugin_audacious_uade_la_SOURCES = \
    plugin/audacious/prefs.h \
    plugin/audacious/uade_prefs.h \
    plugin/audacious/logger.cc \
    plugin/audacious/plugin.cc

plugin/audacious/uade.% : CXXFLAGS += -Wno-format-nonliteral

plugin_audacious_uade_la_LIBADD = \
    songend/libsongend.la \
    songdb/libsongdb.la \
    common/libmd5.la \
    $(UADE_LIBS) \
    $(AUDACIOUS_LIBS)

plugin_audacious_uade_la_LDFLAGS = $(AM_LDFLAGS) -avoid-version -module -shared -no-undefined $(SHREXT)
if BUILD_WINDOWS
plugin_audacious_uade_la_LDFLAGS += -Xcompiler $(UADE_LIBS)
if BUILD_CYGWIN
plugin_audacious_uade_la_LDFLAGS += -Xcompiler -lcygwin
endif
if BUILD_MINGW
if LD_GNU
plugin_audacious_uade_la_LDFLAGS += -Xcompiler -lmsvcrt -Xcompiler -lmingwex -Xcompiler -lmsvcrt -Xcompiler -lkernel32 -Xcompiler -lws2_32
endif
endif
endif
if BUILD_NETBSD
plugin_audacious_uade_la_LDFLAGS += -Xcompiler $(UADE_LIBS)
endif
plugin_audacious_uade_la_CXXFLAGS = $(AM_CXXFLAGS) $(AUDACIOUS_CFLAGS)

plugin/audacious/% : CXXFLAGS += -Wno-missing-field-initializers
