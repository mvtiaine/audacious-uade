#!/bin/sh

set -e

player=st3play

. $(dirname "$0")/../common/header.sh

#

# XXX using precalc as st3play output can depend on CPU, compiler and libc
# TODO figure out root cause

TESTMOD="${top_srcdir}/testdata/miracle man.s3m"
TEST_NAME="st3play GUS + subsongs"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="45900ecc57e518f51b8ba7ef2cf66206	1	163871	player+silence	st3play	Scream Tracker 3.01 \(GUS\)	8	79744	bb1f2cbb	d9c451aa
45900ecc57e518f51b8ba7ef2cf66206	2	15352	player"

. $(dirname "$0")/../common/check.sh

exit 0
