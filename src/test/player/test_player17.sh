#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/nes song thingy.s3m"
# arm/x86/ppc 64-bit
TESTMD5_LITTLE=2a0f8f99edc887082fd92d2649072602

TEST_NAME="it2play (S3M 16-bit)"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE

# it2play HQ mixer output depends on whether 32-bit or 64-bit CPU
# XXX different output on x86 32-bit vs 68k 32-bit
# TODO figure out root cause
OUTPUT=$(eval ${TEST})
if [ "$OUTPUT" = "48abba4cec9fcbdfba2ea73fea4dac77" ]; then
    # x86 32-bit
    EXPECTED_OUTPUT=48abba4cec9fcbdfba2ea73fea4dac77
elif [ "$OUTPUT" = "3b18e3eb40b27b2964b9b3928ac256ac" ]; then
    # 68k 32-bit
    EXPECTED_OUTPUT=3b18e3eb40b27b2964b9b3928ac256ac
fi

. $(dirname "$0")/../common/check.sh

exit 0
