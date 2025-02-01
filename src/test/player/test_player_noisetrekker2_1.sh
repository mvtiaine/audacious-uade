#!/bin/sh

set -e

player=noisetrekker2

. $(dirname "$0")/../common/header.sh

#

# XXX using precalc as noisterkker2 output can depend on CPU, compiler and libc
# TODO figure out root cause

TESTMOD="${top_srcdir}/testdata/unionjack.ntk"
TEST_NAME="NoiseTrekker 2"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="38abe9fd6b9ceb6d5049c38462bebe31	1	7060	player	noisetrekker2	NoiseTrekker 2.x	5	191700"

. $(dirname "$0")/../common/check.sh

exit 0
