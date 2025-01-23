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
elif [ "$OUTPUT" = "58f213c9fdfe7711b5e3a3c64c794ea4" ]; then
    EXPECTED_OUTPUT=58f213c9fdfe7711b5e3a3c64c794ea4
elif [ "$OUTPUT" = "9733d4eea2a4e75bca5c45388ec4d66f" ]; then
    EXPECTED_OUTPUT=9733d4eea2a4e75bca5c45388ec4d66f
elif [ "$OUTPUT" = "d665ca5c94c3f822819420148e550b17" ]; then
    EXPECTED_OUTPUT=d665ca5c94c3f822819420148e550b17
elif [ "$OUTPUT" = "9f2ef0c6aaaf5b1b60878e5ce073e729" ]; then
    EXPECTED_OUTPUT=9f2ef0c6aaaf5b1b60878e5ce073e729
elif [ "$OUTPUT" = "1a5617ed2ff0c848d27bc4f1292005ba" ]; then
    EXPECTED_OUTPUT=1a5617ed2ff0c848d27bc4f1292005ba
elif [ "$OUTPUT" = "61b3ef78d29782ecdba59973a7fbc219" ]; then
    EXPECTED_OUTPUT=61b3ef78d29782ecdba59973a7fbc219
fi

. $(dirname "$0")/../common/check.sh

exit 0
