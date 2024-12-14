#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/burgertime mix.xm"
TESTMD5_LITTLE=9d49fa1c465c0e5d5eff9a80e30f9b4c

TEST_NAME="ft2play (XM) + subsongs"
TEST="${PLAYER} \"${TESTMOD}\" 5 | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE
. $(dirname "$0")/../common/check.sh

exit 0
