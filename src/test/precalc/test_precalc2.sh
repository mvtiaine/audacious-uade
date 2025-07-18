#!/bin/sh

set -e

player=uade

. $(dirname "$0")/../common/header.sh

#

TESTMOD="${top_srcdir}/testdata/i wanna show you all my warez.ahx"
TEST_NAME="Precalc songend - repeat"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="82f244a2ce0dd8fdbf5bcac3e9476337	0	9000	repeat	uade	AHX v2	0	26993	2a2d02f0"
. $(dirname "$0")/../common/check.sh

exit 0
