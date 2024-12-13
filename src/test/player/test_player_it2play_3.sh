#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/ezisopth.it"
# arm/ppc 64-bit
TESTMD5_LITTLE=9e0157f0af09ee27a8620e3a1de3adf2

TEST_NAME="it2play (IT) UseFPUCode=true (Impulse Tracker 2.15)"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE

# XXX it2play HQ driver output can depend on CPU, compiler and libc
# TODO figure out root cause (powf?)
OUTPUT=$(eval ${TEST})
# more "known good" hashes
if [ "$OUTPUT" = "33aa281667a8b7f8070ea678c8e60967" ]; then
    EXPECTED_OUTPUT=33aa281667a8b7f8070ea678c8e60967
elif [ "$OUTPUT" = "3683455362f699985bc5b0f5af104498" ]; then
    EXPECTED_OUTPUT=3683455362f699985bc5b0f5af104498
elif [ "$OUTPUT" = "094e35084dfb0ee5445efd3ee121bd75" ]; then
    EXPECTED_OUTPUT=094e35084dfb0ee5445efd3ee121bd75
elif [ "$OUTPUT" = "ccca0070ca5f698a6f7fd1c9772f04d8" ]; then
    EXPECTED_OUTPUT=ccca0070ca5f698a6f7fd1c9772f04d8
elif [ "$OUTPUT" = "1cb585457639ea91137e74579ade31e6" ]; then
    EXPECTED_OUTPUT=1cb585457639ea91137e74579ade31e6
elif [ "$OUTPUT" = "bcea995e948b7908fe8359ad05503021" ]; then
    EXPECTED_OUTPUT=bcea995e948b7908fe8359ad05503021
fi

. $(dirname "$0")/../common/check.sh

exit 0
