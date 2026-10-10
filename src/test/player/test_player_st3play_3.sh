#!/bin/sh

set -e

player=st3play

. $(dirname "$0")/../common/header.sh

#

# XXX using precalc as st3play output can depend on CPU, compiler and libc
# TODO figure out root cause

TESTMOD="${top_srcdir}/testdata/hologram rose.s3m"
TEST_NAME="st3play SB"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="0b0386fe8d63f509f6bf432052e23f12	1	264159	player	st3play	Scream Tracker 3.01 \(SB\)	4	108328	da5b3f7c	2cf5ae7c"

. $(dirname "$0")/../common/check.sh

exit 0
