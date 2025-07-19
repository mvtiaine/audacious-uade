# NOT a standalone Makefile.am

# XXX use aaa_ prefix to make it highest prio plugin
plugin_deadbeef_LTLIBRARIES = plugin/deadbeef/aaa_uade.la
plugin_deadbeef_aaa_uade_la_SOURCES = \
    plugin/common/copyright.h \
    plugin/deadbeef/logger.cc \
    plugin/deadbeef/plugin.cc

plugin/deadbeef/% : CXXFLAGS += -Wno-format-nonliteral -Wno-missing-field-initializers

plugin_deadbeef_aaa_uade_la_LIBADD = \
    songend/libsongend.la \
    songdb/libsongdb.la \
    3rdparty/libxxhash.la \
    $(UADE_LIBS) \
    $(LIBOPENMPT_LIBS) \
    $(LIBXMP_LIBS)

plugin_deadbeef_aaa_uade_la_LDFLAGS = $(AM_LDFLAGS) -avoid-version -module -shared -no-undefined $(SHREXT)
plugin_deadbeef_aaa_uade_la_LDFLAGS += -Xcompiler -Wno-odr -Xcompiler -Wno-lto-type-mismatch
if BUILD_WINDOWS
plugin_deadbeef_aaa_uade_la_LDFLAGS += -Xcompiler $(UADE_LIBS)
if BUILD_CYGWIN
plugin_deadbeef_aaa_uade_la_LDFLAGS += -Xcompiler -lcygwin
endif
if BUILD_MINGW
if LD_GNU
plugin_deadbeef_aaa_uade_la_LDFLAGS += -Xcompiler -lmsvcrt -Xcompiler -lmingwex -Xcompiler -lmsvcrt -Xcompiler -lkernel32 -Xcompiler -lws2_32
endif
endif
endif
if BUILD_NETBSD
plugin_deadbeef_aaa_uade_la_LDFLAGS += -Xcompiler $(UADE_LIBS)
endif
plugin_deadbeef_aaa_uade_la_CXXFLAGS = $(AM_CXXFLAGS) $(LIBOPENMPT_CFLAGS) $(LIBXMP_CFLAGS)
