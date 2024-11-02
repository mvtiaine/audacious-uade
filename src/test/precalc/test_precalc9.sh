#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD="${top_srcdir}/testdata/hash-tu-raz 2.digi"
TEST_NAME="Precalc songend - player+silence"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="d9a1e268d228018921af3287576c179f	0	172676	player+silence	uade	DIGI Booster V1.6	0	35902"
. $(dirname "$0")/../common/check.sh

exit 0
