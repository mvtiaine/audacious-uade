#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/spaz on jazz.it"
TESTMD5_LITTLE=ba0a7674798ad451f4dcdfe5617f980f

TEST_NAME="it2play (IT) + subsongs"
TEST="${PLAYER} \"${TESTMOD}\" 2 | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE

# it2play HQ mixer output depends on whether 32-bit or 64-bit CPU
OUTPUT=$(eval ${TEST})
if [ "$OUTPUT" = "a0bb95abf45d2bcbf9e7f915ce9283a1" ]; then
    EXPECTED_OUTPUT=a0bb95abf45d2bcbf9e7f915ce9283a1
fi

. $(dirname "$0")/../common/check.sh

exit 0
