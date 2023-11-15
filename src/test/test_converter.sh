#!/bin/sh

set -e

. $(dirname "$0")/common/header.sh

MD5=${top_builddir}/src/test/md5
CONVERTER=${top_builddir}/src/plugin/cli/converter/converter
TESTMOD=${top_srcdir}/testdata/chipsong.med

TEST_NAME=converter
EXPECTED_OUTPUT=b6ea0467707ba8201c26e549db028a47
TEST_OUTPUT=$(${CONVERTER} ${TESTMOD} | ${MD5})

. $(dirname "$0")/common/check.sh
exit 0
