#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/ezisopth.it"
# arm 64-bit
TESTMD5_LITTLE=9e0157f0af09ee27a8620e3a1de3adf2

TEST_NAME="it2play (IT) UseFPUCode=true (Impulse Tracker 2.15)"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE

# XXX different output on x86 vs arm (with both gcc and clang)
# TODO figure out root cause
OUTPUT=$(eval ${TEST})
if [ "$OUTPUT" = "33aa281667a8b7f8070ea678c8e60967" ]; then
    # x86 64-bit
    EXPECTED_OUTPUT=33aa281667a8b7f8070ea678c8e60967
elif [ "$OUTPUT" = "3683455362f699985bc5b0f5af104498" ]; then
    # x86 32-bit
    EXPECTED_OUTPUT=3683455362f699985bc5b0f5af104498
fi

. $(dirname "$0")/../common/check.sh

exit 0
