#!/bin/sh

set -e

player=it2play

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/long white clouds.it"
# arm/ppc/x86 64-bit
TESTMD5_LITTLE=27ebf1a6650419c7aa0bbddd187836d9

TEST_NAME="it2play (IT 16-bit unsigned)"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE

# XXX it2play HQ driver output can depend on CPU, compiler and libc
# TODO figure out root cause (powf?)
OUTPUT=$(eval ${TEST})
TEST="echo ${OUTPUT}"
# more "known good" hashes
if [ "$OUTPUT" = "44c5e2a30befcea5f92c349b2575a081" ]; then
    EXPECTED_OUTPUT=44c5e2a30befcea5f92c349b2575a081
elif [ "$OUTPUT" = "f249f7a1122d7b502225753613e1f16c" ]; then
    EXPECTED_OUTPUT=f249f7a1122d7b502225753613e1f16c
elif [ "$OUTPUT" = "8d72db3ba44264df09f5bb9de38eb8e7" ]; then
    EXPECTED_OUTPUT=8d72db3ba44264df09f5bb9de38eb8e7
fi

. $(dirname "$0")/../common/check.sh

exit 0
