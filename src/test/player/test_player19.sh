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

# it2play HQ driver output depends on whether 32-bit or 64-bit CPU
# XXX different output on x86 64-bit vs arm/ppc 64-bit (with both gcc and clang) and x86 32-bit vs 68k 32-bit
# TODO figure out root cause
OUTPUT=$(eval ${TEST})
if [ "$OUTPUT" = "c7d459a0102f929dd2b9028a98bd4c7e" ]; then
    # x86 64-bit
    EXPECTED_OUTPUT=c7d459a0102f929dd2b9028a98bd4c7e
elif [ "$OUTPUT" = "aa59151ab4a4698af78718ec4da2e8a3" ]; then
    # x86 32-bit
    EXPECTED_OUTPUT=aa59151ab4a4698af78718ec4da2e8a3
elif [ "$OUTPUT" = "75ad6f5a7979b555411e76569c9d55f3" ]; then
    # 68k 32-bit
    EXPECTED_OUTPUT=75ad6f5a7979b555411e76569c9d55f3
fi

. $(dirname "$0")/../common/check.sh

exit 0
