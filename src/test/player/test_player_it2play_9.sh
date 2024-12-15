#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little
export IT2PLAY_DRIVER=wavwriter

TESTMOD="${top_srcdir}/testdata/nes song thingy.s3m"
# arm/ppc/x86 64-bit, 68k 32-bit
TESTMD5_LITTLE=538ec8a6288861d5b11115651f555b3d

TEST_NAME="it2play (driver WAVWriter)"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE

# XXX it2play WAVWriter driver output can depend on CPU, compiler and libc
# TODO figure out root cause (powf?)
OUTPUT=$(eval ${TEST})
TEST="echo ${OUTPUT}"
# more "known good" hashes
if [ "$OUTPUT" = "6cc033c5d072d607886eeaca5e8643f8" ]; then
    EXPECTED_OUTPUT=6cc033c5d072d607886eeaca5e8643f8
elif [ "$OUTPUT" = "649caa6bbc9a3d92dedeabe9c338e627" ]; then
    EXPECTED_OUTPUT=649caa6bbc9a3d92dedeabe9c338e627
fi

. $(dirname "$0")/../common/check.sh

exit 0
