# NOT a standalone Makefile.am

noinst_LTLIBRARIES += \
    common/libmd5.la

common_libmd5_la_SOURCES = \
    common/md5.h \
    common/md5.cc \
    common/getdelim.h \
    common/getdelim.c
