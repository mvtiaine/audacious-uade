#!/bin/sh

set -e

player=uade

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=big

TESTMOD="${top_srcdir}/testdata/mod.orchannel"
TESTMD5_BIG=b8ed2fe7706a0f7e25bac357cb138ed2
SUBSONG=4

TEST_NAME="UADE big endian + subsong"
TEST="${PLAYER} \"${TESTMOD}\" ${SUBSONG} | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_BIG
. $(dirname "$0")/../common/check.sh

exit 0
