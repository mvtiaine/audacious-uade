#!/bin/sh

set -e

player=libxmp

. $(dirname "$0")/../common/header.sh

#

# using precalc as libxmp is used as system/external library

export PLAYER=${player}

TESTMOD="${top_srcdir}/testdata/burgertime mix.xm"
TEST_NAME="libxmp + subsongs"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="8ef21f4e65561e96a6a649ded86460dd	0	88193	player	libxmp	FastTracker v2.00 XM 1.04	8	131847	115bcbd7
8ef21f4e65561e96a6a649ded86460dd	1	5767	player
8ef21f4e65561e96a6a649ded86460dd	2	5124	player
8ef21f4e65561e96a6a649ded86460dd	3	5103	player
8ef21f4e65561e96a6a649ded86460dd	4	34521	player
8ef21f4e65561e96a6a649ded86460dd	5	76237	player
8ef21f4e65561e96a6a649ded86460dd	6	49532	player"

. $(dirname "$0")/../common/check.sh

exit 0
