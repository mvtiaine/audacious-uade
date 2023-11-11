# NOT a standalone Makefile.am

noinst_LTLIBRARIES += \
	songdb/libsongdb.la

songdb_libsongdb_la_SOURCES = \
    songdb/songdb.h \
    songdb/songdb.cc \
    songdb/amp.cc \
    songdb/modland.cc \
    songdb/unexotica.cc \
    songdb/blacklist.cc \
    common/common.h \
    common/extensions.h \
    common/logger.h
