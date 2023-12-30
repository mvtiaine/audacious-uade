#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD="${top_srcdir}/testdata/introduction3.sa"
TEST_NAME="Precalc songend - player+volume"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="eaafce62280cc4df58c8222089598ec2	0	28367	player+volume	uade	Sonic Arranger	0	2660"
. $(dirname "$0")/../common/check.sh

exit 0
