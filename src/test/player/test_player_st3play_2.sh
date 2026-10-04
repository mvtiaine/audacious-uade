#!/bin/sh

set -e

player=st3play

. $(dirname "$0")/../common/header.sh

#

# XXX using precalc as st3play output can depend on CPU, compiler and libc
# TODO figure out root cause

TESTMOD="${top_srcdir}/testdata/starport bbs introtune.s3m"
TEST_NAME="st3play OPL"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="825419d139d7c8bba169d779d1bdd519	1	38339	player	st3play	Scream Tracker 3.00 \(SB\)	9	4130	d2185f7c	a7316e24"

. $(dirname "$0")/../common/check.sh

exit 0
