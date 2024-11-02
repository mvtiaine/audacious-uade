#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD="${top_srcdir}/testdata/gyroscope.mon"
TEST_NAME="Precalc songend - loop"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="06b890430af28b89de13435e00f820b2	1	48010	loop	uade	MON	0	7788"
. $(dirname "$0")/../common/check.sh

exit 0
