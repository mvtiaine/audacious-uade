#!/bin/sh

set -e

player=st3play

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/starport bbs introtune.s3m"
TESTMD5_LITTLE=370ceeb9d906db6cb38e85d0458eff7c

TEST_NAME="st3play OPL"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE

. $(dirname "$0")/../common/check.sh

exit 0
