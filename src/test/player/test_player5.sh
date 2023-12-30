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

# XXX on x86 (32-bit) HivelyTracker replay produces slightly different output
# TODO figure out root cause
OUTPUT=$(eval ${TEST})
if [ "$OUTPUT" -eq "db12f1eaf5a14ec73cc4e45a56086936" ]; then
    EXPECTED_OUTPUT=db12f1eaf5a14ec73cc4e45a56086936
fi

. $(dirname "$0")/../common/check.sh

exit 0
