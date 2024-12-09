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

# it2play HQ mixer output depends on whether 32-bit or 64-bit CPU
# XXX different output on x86 64-bit vs arm/ppc 64-bit (with both gcc and clang) and x86 32-bit vs 68k 32-bit
# TODO figure out root cause
OUTPUT=$(eval ${TEST})
if [ "$OUTPUT" = "c7d459a0102f929dd2b9028a98bd4c7e" ]; then
    # x86 64-bit
    EXPECTED_OUTPUT=c7d459a0102f929dd2b9028a98bd4c7e
elif [ "$OUTPUT" = "3683455362f699985bc5b0f5af104498" ]; then
    # x86 32-bit
    EXPECTED_OUTPUT=3683455362f699985bc5b0f5af104498
elif [ "$OUTPUT" = "094e35084dfb0ee5445efd3ee121bd75" ]; then
    # 68k 32-bit
    EXPECTED_OUTPUT=094e35084dfb0ee5445efd3ee121bd75
fi

. $(dirname "$0")/../common/check.sh

exit 0
