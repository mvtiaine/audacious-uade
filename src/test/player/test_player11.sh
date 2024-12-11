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

# it2play HQ driver output depends on whether 32-bit or 64-bit CPU
# XXX different output on x86 32-bit vs 68k 32-bit
# TODO figure out root cause
OUTPUT=$(eval ${TEST})
if [ "$OUTPUT" = "a0bb95abf45d2bcbf9e7f915ce9283a1" ]; then
    # x86 32-bit
    EXPECTED_OUTPUT=a0bb95abf45d2bcbf9e7f915ce9283a1
elif [ "$OUTPUT" = "706e8164f59d8b97cb0c28e84a84040f" ]; then
    # 68k 32-bit
    EXPECTED_OUTPUT=706e8164f59d8b97cb0c28e84a84040f
fi

. $(dirname "$0")/../common/check.sh

exit 0
