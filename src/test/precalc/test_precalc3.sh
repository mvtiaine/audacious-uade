#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD="${top_srcdir}/testdata/mdat.turrican loader"
TEST_NAME="Precalc songend - player (uade)"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="9878bd072ac12f04a8cdf353731110cc	0	30845	player	4728"
. $(dirname "$0")/../common/check.sh

exit 0
