#!/bin/sh

set -e

player=st3play

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/happiness.s3m"
TESTMD5_LITTLE=ae2fc33531f2749e027f3643cf2a1510

TEST_NAME="st3play (16-bit)"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE

. $(dirname "$0")/../common/check.sh

exit 0
