#!/bin/sh

set -e

player=uade

. $(dirname "$0")/../common/header.sh

#

TESTMOD="${top_srcdir}/testdata/exquisiet.ahx"
TEST_NAME="Precalc songend - repeat2"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="d2a716b5365491c0d7f082f9a438a56b	0	89705	repeat	uade	AHX v2	0	10518"
. $(dirname "$0")/../common/check.sh

exit 0
