#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD="${top_srcdir}/testdata/mod.orchannel"
TEST_NAME="Precalc songend - subsongs + nosound"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="025e47e9f0d32124588b712263aea971	1	0	nosound	35174
025e47e9f0d32124588b712263aea971	2	4493	player
025e47e9f0d32124588b712263aea971	3	4493	player
025e47e9f0d32124588b712263aea971	4	61570	player"
. $(dirname "$0")/../common/check.sh

exit 0
