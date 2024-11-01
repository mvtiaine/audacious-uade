# NOT a standalone Makefile.am

check_PROGRAMS += \
    test/md5

test_md5_SOURCES = \
    test/md5_main.cc

test_md5_LDADD = \
    common/libmd5.la

test_md5_SOURCES += plugin/cli/common/compat.cc

if STATIC_STDLIBS
test_md5_LDFLAGS = $(AM_LDFLAGS) $(STATIC_LDFLAGS)
test_md5_SOURCES += plugin/cli/common/tinynew.cc
endif

if BUILD_MINGW
uade_core_file=`cygpath -w -m $(top_builddir)/uade/src/uadecore`
uade_base_dir=`cygpath -w -m $(top_builddir)/build-tmp/uade`
else
uade_core_file=$(top_builddir)/uade/src/uadecore
uade_base_dir=$(top_builddir)/build-tmp/uade
endif

TESTS_ENVIRONMENT = \
    export top_srcdir=$(top_srcdir); \
    export top_builddir=$(top_builddir); \
    export UADE_CORE_FILE=$(uade_core_file); \
    export UADE_BASE_DIR=$(uade_base_dir); \
    $(MKDIR_P) $(top_builddir)/build-tmp/uade 2>/dev/null; \
    cp -f $(top_srcdir)/conf/*.conf $(top_builddir)/build-tmp/uade/ 2>/dev/null; \
    cp -f $(top_srcdir)/uade/eagleplayer.conf $(top_builddir)/build-tmp/uade/ 2>/dev/null; \
    cp -f $(top_srcdir)/uade/uaerc $(top_builddir)/build-tmp/uade/ 2>/dev/null; \
    cp -f $(top_srcdir)/uade/amigasrc/score/score $(top_builddir)/build-tmp/uade/ 2>/dev/null; \
    cp -rf $(top_srcdir)/uade/players $(top_builddir)/build-tmp/uade/ 2>/dev/null; \
    sleep 1; \
    chmod -R 755 $(top_builddir)/build-tmp/uade;

# should be sorted by longest running ones first for better parallelisation
TESTS = \
    test/songend/test_songend.sh \
    test/precalc/test_precalc13.sh \
    test/precalc/test_precalc12.sh \
    test/precalc/test_precalc11.sh \
    test/precalc/test_precalc2.sh \
    test/precalc/test_precalc1.sh \
    test/precalc/test_precalc10.sh \
    test/precalc/test_precalc9.sh \
    test/precalc/test_precalc8.sh \
    test/precalc/test_precalc7.sh \
    test/precalc/test_precalc6.sh \
    test/precalc/test_precalc5.sh \
    test/precalc/test_precalc4.sh \
    test/precalc/test_precalc3.sh \
    test/converter/test_converter.sh \
    test/songdb/test_songdb1.sh \
    test/songdb/test_songdb2.sh \
    test/player/test_player1.sh \
    test/player/test_player2.sh \
    test/player/test_player3.sh \
    test/player/test_player4.sh \
    test/player/test_player5.sh \
    test/player/test_player6.sh \
    test/player/test_player7.sh \
    test/player/test_player8.sh \
    test/player/test_player9.sh \
    test/player/test_player10.sh

EXTRA_DIST += \
    test/common/header.sh \
    test/common/check.sh \
    $(TESTS)
