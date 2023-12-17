#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD=${top_srcdir}/testdata/monkmusings.hvl
TESTMD5_LITTLE=f52dea21bd60cf27f0e529666fad05aa

TEST_NAME="HivelyTracker"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE
. $(dirname "$0")/../common/check.sh

exit 0
