#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/spaz on jazz.it"
# arm/x86/ppc 64-bit
TESTMD5_LITTLE=ba0a7674798ad451f4dcdfe5617f980f

TEST_NAME="it2play (IT) + subsongs"
TEST="${PLAYER} \"${TESTMOD}\" 2 | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE

# XXX it2play HQ driver output can depend on CPU, compiler and libc
# TODO figure out root cause (powf?)
OUTPUT=$(eval ${TEST})
TEST="echo ${OUTPUT}"
# more "known good" hashes
if [ "$OUTPUT" = "a0bb95abf45d2bcbf9e7f915ce9283a1" ]; then
    EXPECTED_OUTPUT=a0bb95abf45d2bcbf9e7f915ce9283a1
elif [ "$OUTPUT" = "706e8164f59d8b97cb0c28e84a84040f" ]; then
    EXPECTED_OUTPUT=706e8164f59d8b97cb0c28e84a84040f
elif [ "$OUTPUT" = "7600b05ba66d1d9280f350af38d9ac2a" ]; then
    EXPECTED_OUTPUT=7600b05ba66d1d9280f350af38d9ac2a
fi

. $(dirname "$0")/../common/check.sh

exit 0
