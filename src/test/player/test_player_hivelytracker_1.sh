#!/bin/sh

set -e

player=hivelytracker

. $(dirname "$0")/../common/header.sh

#

# XXX using precalc as HivelyTracker replay may produce slightly different output depending on host
# TODO figure out root cause

TESTMOD="${top_srcdir}/testdata/monkmusings.hvl"
TEST_NAME="HivelyTracker"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="ce594632803f682e9c3c644087b4e58b	0	45991	player	hivelytracker	HivelyTracker	7	2378"

. $(dirname "$0")/../common/check.sh

exit 0
