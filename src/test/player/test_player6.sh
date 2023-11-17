#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD=${top_srcdir}/testdata/tomaatti.dbm
TESTMD5_LITTLE=46df5c69a62fae5c85ae7972a1ebed64

TEST_NAME="libdigibooster3"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE
. $(dirname "$0")/../common/check.sh

exit 0
