#!/bin/sh

set -e

player=libdigibooster3

. $(dirname "$0")/../common/header.sh

#

TESTMOD="${top_srcdir}/testdata/DBM.Tower Of The Six Winds"
TEST_NAME="Precalc songend - loop+silence"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="28bb7e78268d7925943070c2b3ace712	0	26140	loop\+silence	libdigibooster3	DigiBooster Pro 2.21	14	181911"
. $(dirname "$0")/../common/check.sh

exit 0
