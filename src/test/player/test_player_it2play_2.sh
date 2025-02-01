#!/bin/sh

set -e

player=it2play

. $(dirname "$0")/../common/header.sh

#

# XXX using precalc as it2play HQ driver output can depend on CPU, compiler and libc
# TODO figure out root cause

TESTMOD="${top_srcdir}/testdata/syoa.s3m"
TEST_NAME="it2play (S3M)"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="071c79fb4583effc70d473c9b976f1a0	1	203169	player+silence	it2play	Impulse Tracker 2.14	14	51174"

. $(dirname "$0")/../common/check.sh

exit 0
