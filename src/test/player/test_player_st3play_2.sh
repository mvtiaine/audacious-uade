#!/bin/sh

set -e

player=st3play

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/happiness.s3m"
TESTMD5_LITTLE=df151c0e648c66e3485dcb538be3b509

TEST_NAME="st3play (16-bit)"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE
. $(dirname "$0")/../common/check.sh

exit 0
