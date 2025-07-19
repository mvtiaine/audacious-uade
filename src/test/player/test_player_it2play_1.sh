#!/bin/sh

set -e

player=it2play

. $(dirname "$0")/../common/header.sh

#

# XXX using precalc as it2play HQ driver output can depend on CPU, compiler and libc
# TODO figure out root cause

TESTMOD="${top_srcdir}/testdata/spaz on jazz.it"
TEST_NAME="it2play (IT) + subsongs"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="534dc2e8437ddabcdbf45f92d6dc77b1	1	170135	player\+silence	it2play	Impulse Tracker 2.06	14	140446	3363ee2e
534dc2e8437ddabcdbf45f92d6dc77b1	2	45232	player"

. $(dirname "$0")/../common/check.sh

exit 0
