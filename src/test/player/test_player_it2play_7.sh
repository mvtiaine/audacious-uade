#!/bin/sh

set -e

player=it2play

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little
export IT2PLAY_DRIVER=sb16mmx

TESTMOD="${top_srcdir}/testdata/nes song thingy.s3m"
TESTMD5_LITTLE=3d455d885105c9d2dc25ba3ee00dcbc9

TEST_NAME="it2play (driver SB16MMX)"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE

. $(dirname "$0")/../common/check.sh

exit 0
