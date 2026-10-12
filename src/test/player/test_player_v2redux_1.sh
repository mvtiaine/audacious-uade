#!/bin/sh

set -e

player=v2redux

. $(dirname "$0")/../common/header.sh

#

# XXX using precalc as v2redux output can depend on CPU, compiler and libc
# TODO figure out root cause

TESTMOD="${top_srcdir}/testdata/flybye.v2m"
TEST_NAME="v2redux (v2m)"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="b45e3a6affe598074d6998787e7af761	1	138500	player	v2redux	V2M v1	13	26080	438ded1c	83868b8d"

. $(dirname "$0")/../common/check.sh

exit 0
