#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD=${top_srcdir}/testdata/tomaatti.dbm
TESTMD5_LITTLE=e149f7b5210d61cb13083f5cd6641c9f

TEST_NAME="libdigibooster3"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE
. $(dirname "$0")/../common/check.sh

exit 0
