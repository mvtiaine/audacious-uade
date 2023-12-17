#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD=${top_srcdir}/testdata/tomaatti.dbm
TESTMD5_LITTLE=3bc10b4aaa678ac37bf724e1a5818911

TEST_NAME="libdigibooster3"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE
. $(dirname "$0")/../common/check.sh

exit 0
