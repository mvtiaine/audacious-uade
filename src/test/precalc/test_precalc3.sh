#!/bin/sh

set -e

player=uade

. $(dirname "$0")/../common/header.sh

#

TESTMOD="${top_srcdir}/testdata/mdat.turrican loader"
TEST_NAME="Precalc songend - player (uade)"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="9878bd072ac12f04a8cdf353731110cc	0	30844	player	uade	TFMX	0	4728	4bfa0071"
. $(dirname "$0")/../common/check.sh

exit 0
