#!/bin/sh

set -e

player=it2play

. $(dirname "$0")/../common/header.sh

#

# XXX using precalc as it2play HQ driver output can depend on CPU, compiler and libc
# TODO figure out root cause

TESTMOD="${top_srcdir}/testdata/ezisopth.it"
TEST_NAME="it2play (IT) UseFPUCode=true (Impulse Tracker 2.15)"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="1e504e4b63e411922004dd063498d61f	1	176280	player+volume	it2play	Impulse Tracker 2.15	6	4061"

. $(dirname "$0")/../common/check.sh

exit 0
