#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD="${top_srcdir}/testdata/space blipper.sa"
TEST_NAME="Precalc songend - player+volume"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="03aa443cc7c5e023cef4e1f9ca07c053	0	412757	player+volume	33346"
. $(dirname "$0")/../common/check.sh

exit 0
