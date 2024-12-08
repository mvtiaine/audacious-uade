#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/ezisopth.it"
TESTMD5_LITTLE=9e0157f0af09ee27a8620e3a1de3adf2

TEST_NAME="it2play (IT) UseFPUCode=true (Impulse Tracker 2.15)"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE

# XXX different output on x86 vs arm (with both gcc and clang)
# TODO figure out root cause
OUTPUT=$(eval ${TEST})
if [ "$OUTPUT" = "33aa281667a8b7f8070ea678c8e60967" ]; then
    EXPECTED_OUTPUT=33aa281667a8b7f8070ea678c8e60967
fi

. $(dirname "$0")/../common/check.sh

exit 0
