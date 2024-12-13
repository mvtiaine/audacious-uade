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

# XXX it2play HQ driver output can depend on CPU, compiler and libc
# TODO figure out root cause (powf?)
OUTPUT=$(eval ${TEST})
# more "known good" hashes
if [ "$OUTPUT" = "4c7d777eabb08b801614758ccc03745e" ]; then
    EXPECTED_OUTPUT=4c7d777eabb08b801614758ccc03745e
elif [ "$OUTPUT" = "c6109c8aeb1777ccc0170a7f3defe3ef" ]; then
    EXPECTED_OUTPUT=c6109c8aeb1777ccc0170a7f3defe3ef
elif [ "$OUTPUT" = "1066717f7db834e43ea5ab10612cfa7b" ]; then
    EXPECTED_OUTPUT=1066717f7db834e43ea5ab10612cfa7b
fi

. $(dirname "$0")/../common/check.sh

exit 0
