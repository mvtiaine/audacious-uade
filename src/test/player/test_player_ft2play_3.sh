#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/superhyllorejv.xm"
TESTMD5_LITTLE=7d4da8c1712da2a14ee73b6204f1ad3f

TEST_NAME="ft2play (16-bit samples)"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE
. $(dirname "$0")/../common/check.sh

exit 0
