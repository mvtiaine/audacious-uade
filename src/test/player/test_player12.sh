#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/syoa.s3m"
# arm/x86/ppc 64-bit
TESTMD5_LITTLE=5cc7ca9fb97bb00eeaac2977b10b99f0

TEST_NAME="it2play (S3M)"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE

# it2play HQ driver output depends on whether 32-bit or 64-bit CPU
# XXX different output on x86 32-bit vs 68k 32-bit
# TODO figure out root cause
OUTPUT=$(eval ${TEST})
if [ "$OUTPUT" = "4c7d777eabb08b801614758ccc03745e" ]; then
    # x86 32-bit
    EXPECTED_OUTPUT=4c7d777eabb08b801614758ccc03745e
elif [ "$OUTPUT" = "c6109c8aeb1777ccc0170a7f3defe3ef" ]; then
    # 68k 32-bit
    EXPECTED_OUTPUT=c6109c8aeb1777ccc0170a7f3defe3ef
fi

. $(dirname "$0")/../common/check.sh

exit 0
