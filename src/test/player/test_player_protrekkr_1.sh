#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/elevator zax.ptk"
TESTMD5_LITTLE=56af9822e8333690c29f2e8341ef9f47

TEST_NAME="ProTrekkr 2"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE

# XXX protrekkr output can depend on CPU, compiler and libc
# TODO figure out root cause
OUTPUT=$(eval ${TEST})
TEST="echo ${OUTPUT}"
# more "known good" hashes
if [ "$OUTPUT" = "530d561780e253f1f520914fc74428c5" ]; then
    EXPECTED_OUTPUT=530d561780e253f1f520914fc74428c5
fi

. $(dirname "$0")/../common/check.sh

exit 0
