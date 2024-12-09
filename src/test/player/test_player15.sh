#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/superhyllorejv.xm"
TESTMD5_LITTLE=7b3354e064ec10d55f8f736e953bca05

TEST_NAME="ft2play (16-bit samples)"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE
. $(dirname "$0")/../common/check.sh

exit 0
