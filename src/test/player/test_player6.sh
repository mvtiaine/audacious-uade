#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD=${top_srcdir}/testdata/tomaatti.dbm
TESTMD5_LITTLE=7ee7c819bad8d828aa535809ddad5c7c

TEST_NAME="libdigibooster3"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE
. $(dirname "$0")/../common/check.sh

exit 0
