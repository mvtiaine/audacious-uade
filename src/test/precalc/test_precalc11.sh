#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD="${top_srcdir}/testdata/dimension five.bp"
TEST_NAME="Precalc songend - loop+silence"
TEST_OUTPUT=$(${PRECALC} "${TESTMOD}")
EXPECTED_OUTPUT="7d48b1f103d5a31444dbbb2d7c71bbfb	0	179725	loop+silence	18198"
. $(dirname "$0")/../common/check.sh

exit 0
