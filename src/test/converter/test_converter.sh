#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD=${top_srcdir}/testdata/chipsong.med

TEST_NAME=converter
EXPECTED_OUTPUT=b6ea0467707ba8201c26e549db028a47
TEST="${CONVERTER} \"${TESTMOD}\" | ${MD5}"

. $(dirname "$0")/../common/check.sh
exit 0
