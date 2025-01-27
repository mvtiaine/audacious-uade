#!/bin/sh

set -e

player=st3play

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/miracle man.s3m"
TESTMD5_LITTLE=17addf5a33be26ca333561da49b46ecd

TEST_NAME="st3play (S3M) + subsongs"
TEST="${PLAYER} \"${TESTMOD}\" 2 | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE
. $(dirname "$0")/../common/check.sh

exit 0
