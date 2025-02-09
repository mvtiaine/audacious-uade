#!/bin/sh

set -e

player=protrekkr2

. $(dirname "$0")/../common/header.sh

#

# XXX using precalc as protrekkr output can depend on CPU, compiler and libc
# TODO figure out root cause

TESTMOD="${top_srcdir}/testdata/elevator zax.ptk"
TEST_NAME="ProTrekkr 2"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="1373828675fa04145d5e9eb0244b0e24	1	30700	player	protrekkr2	ProTrekkr 2.x	12	15436"

. $(dirname "$0")/../common/check.sh

exit 0
