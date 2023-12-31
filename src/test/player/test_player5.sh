#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD=${top_srcdir}/testdata/monkmusings.hvl
TESTMD5_LITTLE=f52dea21bd60cf27f0e529666fad05aa

TEST_NAME="HivelyTracker"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE

# XXX on x86/68k/... (32-bit) hosts HivelyTracker replay produces slightly different output vs 64-bit hosts
# TODO figure out root cause
OUTPUT=$(eval ${TEST})
if [ "$OUTPUT" = "db12f1eaf5a14ec73cc4e45a56086936" ]; then
    EXPECTED_OUTPUT=db12f1eaf5a14ec73cc4e45a56086936
fi

. $(dirname "$0")/../common/check.sh

exit 0
