#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD="${top_srcdir}/testdata/oldskoolcrackintrosong.ahx"
TEST_NAME="Precalc songend - volume"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="42439d629c7d645e9668c0eb3878849d	0	72333	volume	uade	AHX v2	0	3149"
. $(dirname "$0")/../common/check.sh

exit 0
