#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/miracle man.s3m"
TESTMD5_LITTLE=32f3c2d2831b5e5c01ee1c23f0044a18

TEST_NAME="st3play (S3M) + subsongs"
TEST="${PLAYER} \"${TESTMOD}\" 2 | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE
. $(dirname "$0")/../common/check.sh

exit 0
