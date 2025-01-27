#!/bin/sh

set -e

player=it2play

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/nes song thingy.s3m"
# arm/x86/ppc 64-bit
TESTMD5_LITTLE=2a0f8f99edc887082fd92d2649072602

TEST_NAME="it2play (S3M 16-bit)"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE

# XXX it2play HQ driver output can depend on CPU, compiler and libc
# TODO figure out root cause (powf?)
OUTPUT=$(eval ${TEST})
TEST="echo ${OUTPUT}"
# more "known good" hashes
if [ "$OUTPUT" = "48abba4cec9fcbdfba2ea73fea4dac77" ]; then
    EXPECTED_OUTPUT=48abba4cec9fcbdfba2ea73fea4dac77
elif [ "$OUTPUT" = "3b18e3eb40b27b2964b9b3928ac256ac" ]; then
    EXPECTED_OUTPUT=3b18e3eb40b27b2964b9b3928ac256ac
elif [ "$OUTPUT" = "a1baf62efb918e0b07ff58bdd7d1c59a" ]; then
    EXPECTED_OUTPUT=a1baf62efb918e0b07ff58bdd7d1c59a
fi

. $(dirname "$0")/../common/check.sh

exit 0
