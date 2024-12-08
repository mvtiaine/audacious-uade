#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/syoa.s3m"
TESTMD5_LITTLE=5cc7ca9fb97bb00eeaac2977b10b99f0

TEST_NAME="it2play (S3M)"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE

# it2play HQ mixer output depends on whether 32-bit or 64-bit CPU
OUTPUT=$(eval ${TEST})
if [ "$OUTPUT" = "4c7d777eabb08b801614758ccc03745e" ]; then
    EXPECTED_OUTPUT=4c7d777eabb08b801614758ccc03745e
fi

. $(dirname "$0")/../common/check.sh

exit 0
