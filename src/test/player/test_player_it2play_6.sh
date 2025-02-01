#!/bin/sh

set -e

player=it2play

. $(dirname "$0")/../common/header.sh

#

# XXX using precalc as it2play HQ driver output can depend on CPU, compiler and libc
# TODO figure out root cause

TESTMOD="${top_srcdir}/testdata/mizzle.it"
TEST_NAME="it2play (IT 16-bit + delta)"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="cc722dd8dfbe85796bebe8d4fd6782a0	1	175528	player+volume	it2play	Impulse Tracker 2.15	17	61211"

. $(dirname "$0")/../common/check.sh

exit 0
