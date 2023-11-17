#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD="${top_srcdir}/testdata/hash-tu-raz 2.digi"
TEST_NAME="Precalc songend - player+silence"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="d9a1e268d228018921af3287576c179f	0	172733	player+silence	35902"
. $(dirname "$0")/../common/check.sh

exit 0
