#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=big

TESTMOD=${top_srcdir}/testdata/mod.orchannel
TESTMD5_BIG=7da0aac6a7299ad9732ccc293f620eb6
SUBSONG=4

TEST_NAME="UADE big endian + subsong"
TEST="${PLAYER} \"${TESTMOD}\" ${SUBSONG} | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_BIG
. $(dirname "$0")/../common/check.sh

exit 0
