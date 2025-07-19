#!/bin/sh

set -e

player=libopenmpt

. $(dirname "$0")/../common/header.sh

#

# using precalc as libopenmpt is used as system/external library

TESTMOD="${top_srcdir}/testdata/starport bbs introtune.s3m"
TEST_NAME="libopenmpt"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="825419d139d7c8bba169d779d1bdd519	0	38342	player	libopenmpt	Scream Tracker 3.00 \(SB\)	9	4130	d2185f7c"

. $(dirname "$0")/../common/check.sh

exit 0
