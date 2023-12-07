#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD="${top_srcdir}/testdata/karatekid2.cus"
TEST_NAME="Precalc songend - silence"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="048e361de72990f8ba9e24a4d4a9bef8	0	264368	silence	4732"
. $(dirname "$0")/../common/check.sh

exit 0
