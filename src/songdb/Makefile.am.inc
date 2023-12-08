# NOT a standalone Makefile.am

noinst_LTLIBRARIES += \
	songdb/libsongdb.la

songdb_libsongdb_la_SOURCES = \
    songdb/songdb.h \
    songdb/songdb.cc \
    songdb/blacklist.cc \
    songdb/sources/amp.cc \
    songdb/sources/demozoo.cc \
    songdb/sources/modland.cc \
    songdb/sources/unexotica.cc \
    common/common.h \
    common/extensions.h \
    common/logger.h
