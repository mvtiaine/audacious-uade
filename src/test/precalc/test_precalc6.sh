#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD="${top_srcdir}/testdata/rubber spine.ahx"
TEST_NAME="Precalc songend - error"
TEST_OUTPUT=$(${PRECALC} "${TESTMOD}")
EXPECTED_OUTPUT="9a3fbe962d46a2335a2eb99e938c3ccb	0	0	error	6035"
. $(dirname "$0")/../common/check.sh

exit 0
