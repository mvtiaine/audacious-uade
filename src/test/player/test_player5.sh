#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD=${top_srcdir}/testdata/monkmusings.hvl
TESTMD5_LITTLE=18ca60d4c39ecda3a1346af80ebaa97d

TEST_NAME="HivelyTracker"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE
. $(dirname "$0")/../common/check.sh

exit 0
