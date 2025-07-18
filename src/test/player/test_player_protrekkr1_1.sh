#!/bin/sh

set -e

player=protrekkr1

. $(dirname "$0")/../common/header.sh

#

# XXX using precalc as protrekkr output can depend on CPU, compiler and libc
# TODO figure out root cause

TESTMOD="${top_srcdir}/testdata/devenirunpoulet.ptk"
TEST_NAME="ProTrekkr 1"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="f7a1130e128676f7977933bf9c1e33bd	1	191980	player	protrekkr1	ProTrekkr 1.x	12	49283	7af961e7"

. $(dirname "$0")/../common/check.sh

exit 0
