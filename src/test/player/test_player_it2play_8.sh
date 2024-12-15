#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little
export IT2PLAY_DRIVER=sb16

TESTMOD="${top_srcdir}/testdata/nes song thingy.s3m"
TESTMD5_LITTLE=fc16310aff5bad584a1ee24d3c84e291

TEST_NAME="it2play (driver SB16)"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE

. $(dirname "$0")/../common/check.sh

exit 0
