#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/mizzle.it"
# arm/ppc 64-bit
TESTMD5_LITTLE=017ac554cd7ac142c8d3f8282c18bf7d

TEST_NAME="it2play (IT 16-bit + delta)"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE

# XXX it2play HQ driver output can depend on CPU, compiler and libc
# TODO figure out root cause (powf?)
OUTPUT=$(eval ${TEST})
# more "known good" hashes
if [ "$OUTPUT" = "c7d459a0102f929dd2b9028a98bd4c7e" ]; then
    EXPECTED_OUTPUT=c7d459a0102f929dd2b9028a98bd4c7e
elif [ "$OUTPUT" = "aa59151ab4a4698af78718ec4da2e8a3" ]; then
    EXPECTED_OUTPUT=aa59151ab4a4698af78718ec4da2e8a3
elif [ "$OUTPUT" = "3854ff169e82e5efe5b67a28b3066aee" ]; then
    EXPECTED_OUTPUT=3854ff169e82e5efe5b67a28b3066aee
elif [ "$OUTPUT" = "49a24ad5c32682a02c20cca6b9c8a1c3" ]; then
    EXPECTED_OUTPUT=49a24ad5c32682a02c20cca6b9c8a1c3
fi

. $(dirname "$0")/../common/check.sh

exit 0
