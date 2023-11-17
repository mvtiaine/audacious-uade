#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD=${top_srcdir}/testdata/mod.orchannel
TESTMD5_LITTLE=5e1ff996d4996d7adc7cdcb61054c4b6
SUBSONG=4

export PLAYER_ENDIAN=little

TEST_NAME="UADE little endian + subsong"
TEST="${PLAYER} \"${TESTMOD}\" ${SUBSONG} | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE
. $(dirname "$0")/../common/check.sh

exit 0