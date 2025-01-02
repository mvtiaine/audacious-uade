#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/monkmusings.hvl"
TESTMD5_LITTLE=f52dea21bd60cf27f0e529666fad05aa

TEST_NAME="HivelyTracker"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE

# XXX HivelyTracker replay may produce slightly different output depending on host
# TODO figure out root cause
OUTPUT=$(eval ${TEST})
TEST="echo ${OUTPUT}"
if [ "$OUTPUT" = "db12f1eaf5a14ec73cc4e45a56086936" ]; then
    EXPECTED_OUTPUT=db12f1eaf5a14ec73cc4e45a56086936
elif [ "$OUTPUT" = "f0c3b8b358b3d70d00b1caa19f1bc92c" ]; then
    EXPECTED_OUTPUT=f0c3b8b358b3d70d00b1caa19f1bc92c
fi

. $(dirname "$0")/../common/check.sh

exit 0
