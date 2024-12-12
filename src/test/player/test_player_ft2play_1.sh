#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/burgertime mix.xm"
TESTMD5_LITTLE=2cab8d018e379ced69810bdd1ffbaef7

TEST_NAME="ft2play (XM) + subsongs"
TEST="${PLAYER} \"${TESTMOD}\" 5 | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE
. $(dirname "$0")/../common/check.sh

exit 0
