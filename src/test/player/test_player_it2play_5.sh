#!/bin/sh

set -e

player=it2play

. $(dirname "$0")/../common/header.sh

#

# XXX using precalc as it2play HQ driver output can depend on CPU, compiler and libc
# TODO figure out root cause

TESTMOD="${top_srcdir}/testdata/long white clouds.it"
TEST_NAME="it2play (IT 16-bit unsigned)"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="158eb5a6916b9bf97fa54139dc1e7fcd	1	189589	player	it2play	Impulse Tracker 1.06	12	178697"

. $(dirname "$0")/../common/check.sh

exit 0
