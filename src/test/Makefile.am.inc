# NOT a standalone Makefile.am

check_PROGRAMS += \
    test/md5

test_md5_SOURCES = \
    test/md5_main.cc

test_md5_LDADD = \
	3rdparty/libmd5.la

TESTS_ENVIRONMENT = \
    export top_builddir=$(top_builddir); \
    export top_srcdir=$(top_srcdir);

TESTS = \
    test/test_converter.sh \
    test/test_songdb.sh

EXTRA_DIST += \
    test/common/header.sh \
    test/common/check.sh \
    $(TESTS)
