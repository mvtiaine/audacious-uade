#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD=${top_srcdir}/testdata/mod.orchannel
TESTMD5_LITTLE=592c189d0efc976ba06a705725ca1240
SUBSONG=4

export PLAYER_ENDIAN=little

TEST_NAME="UADE little endian + subsong"
TEST="${PLAYER} \"${TESTMOD}\" ${SUBSONG} | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE
. $(dirname "$0")/../common/check.sh

exit 0
