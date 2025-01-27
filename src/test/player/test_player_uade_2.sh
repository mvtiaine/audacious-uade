#!/bin/sh

set -e

player=uade

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=big

TESTMOD="${top_srcdir}/testdata/mod.orchannel"
TESTMD5_BIG=76e9b7f61b8d5bff63929e4d73e705c0
SUBSONG=4

TEST_NAME="UADE big endian + subsong"
TEST="${PLAYER} \"${TESTMOD}\" ${SUBSONG} | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_BIG
. $(dirname "$0")/../common/check.sh

exit 0
