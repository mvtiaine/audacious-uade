#!/bin/sh

set -e

player=uade

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/MOD.forbidden pineapples"
TESTMD5_LITTLE=a99adc7dad18e3a6093382db62225a8c

TEST_NAME="MOD prefix"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE
. $(dirname "$0")/../common/check.sh

exit 0
