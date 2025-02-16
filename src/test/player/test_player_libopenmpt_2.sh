#!/bin/sh

set -e

player=libopenmpt

. $(dirname "$0")/../common/header.sh

#

# using precalc as libopenmpt is used as system/external library

export PLAYER=${player}

TESTMOD="${top_srcdir}/testdata/burgertime mix.xm"
TEST_NAME="libopenmpt + subsongs"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="8ef21f4e65561e96a6a649ded86460dd	0	88188	player	libopenmpt	FastTracker 2 or compatible	8	131847
8ef21f4e65561e96a6a649ded86460dd	1	5751	player
8ef21f4e65561e96a6a649ded86460dd	2	5112	player
8ef21f4e65561e96a6a649ded86460dd	3	5092	player
8ef21f4e65561e96a6a649ded86460dd	4	34508	player
8ef21f4e65561e96a6a649ded86460dd	5	76226	player
8ef21f4e65561e96a6a649ded86460dd	6	49526	player"

. $(dirname "$0")/../common/check.sh

exit 0
