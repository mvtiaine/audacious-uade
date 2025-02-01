#!/bin/sh

set -e

player=it2play

. $(dirname "$0")/../common/header.sh

#

export IT2PLAY_DRIVER=wavwriter

# XXX using precalc as it2play WAVWriter driver output can depend on CPU, compiler and libc
# TODO figure out root cause

TESTMOD="${top_srcdir}/testdata/nes song thingy.s3m"
TEST_NAME="it2play (driver WAVWriter)"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="64db875e6483430c92b22908b3004285	1	176367	player	it2play	Impulse Tracker 2.14+	5	15740"

. $(dirname "$0")/../common/check.sh

exit 0
