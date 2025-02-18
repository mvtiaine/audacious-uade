# NOT a standalone Makefile.am

 noinst_HEADERS = \
    common/compat.h \
    common/constexpr.h \
    common/endian.h \
    common/foreach.h \
    common/logger.h \
    common/songend.h \
    common/strings.h \
    common/std/functional.h \
    common/std/optional.h \
    common/std/string_view.h

noinst_LTLIBRARIES += \
    common/libmd5.la

common_libmd5_la_SOURCES = \
    common/md5.h \
    common/md5.cc
