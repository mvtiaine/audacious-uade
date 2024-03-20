# NOT a standalone Makefile.am

noinst_LTLIBRARIES += \
	songdb/libsongdb.la

songdb_libsongdb_la_SOURCES = \
    songdb/songdb.h \
    songdb/songdb.cc \
    songdb/blacklist.cc \
    songdb/sources/demozoo.cc \
    songdb/sources/modland.cc \
    songdb/sources/unexotica.cc \
    common/common.h \
    common/extensions.h \
    common/logger.h

if BIG_ENDIAN
songdb_libsongdb_la_CPPFLAGS = $(AM_CPPFLAGS) -D__BYTE_ORDER=4321
else
songdb_libsongdb_la_CPPFLAGS = $(AM_CPPFLAGS) -D__BYTE_ORDER=1234
endif
