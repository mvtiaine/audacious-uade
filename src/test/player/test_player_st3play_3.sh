#!/bin/sh

set -e

player=st3play

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/hologram rose.s3m"
TESTMD5_LITTLE=90585ef58118ad2a8bcefe667b95bfa5

TEST_NAME="st3play SB"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE

. $(dirname "$0")/../common/check.sh

exit 0
