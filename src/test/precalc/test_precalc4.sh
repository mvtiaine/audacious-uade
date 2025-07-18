#!/bin/sh

set -e

player=hivelytracker

. $(dirname "$0")/../common/header.sh

#

TESTMOD="${top_srcdir}/testdata/monkmusings.hvl"
TEST_NAME="Precalc songend - player (hvl)"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="ce594632803f682e9c3c644087b4e58b	0	45991	player	hivelytracker	HivelyTracker	7	2378	08145fe3"
. $(dirname "$0")/../common/check.sh

exit 0
