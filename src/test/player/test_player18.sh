#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/long white clouds.it"
# arm/ppc/x86 64-bit
TESTMD5_LITTLE=27ebf1a6650419c7aa0bbddd187836d9

TEST_NAME="it2play (IT 16-bit unsigned)"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE

# it2play HQ mixer output depends on whether 32-bit or 64-bit CPU
# XXX different output on x86 32-bit vs 68k 32-bit
# TODO figure out root cause
OUTPUT=$(eval ${TEST})
if [ "$OUTPUT" = "44c5e2a30befcea5f92c349b2575a081" ]; then
    # x86 32-bit
    EXPECTED_OUTPUT=44c5e2a30befcea5f92c349b2575a081
elif [ "$OUTPUT" = "f249f7a1122d7b502225753613e1f16c" ]; then
    # 68k 32-bit
    EXPECTED_OUTPUT=f249f7a1122d7b502225753613e1f16c
fi

. $(dirname "$0")/../common/check.sh

exit 0
