#!/bin/sh

set -e

player=protrekkr

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/elevator zax.ptk"
TESTMD5_LITTLE=da8d02449e5a3ea1af0167590be72c0f

TEST_NAME="ProTrekkr 2"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE

# XXX protrekkr output can depend on CPU, compiler and libc
# TODO figure out root cause
OUTPUT=$(eval ${TEST})
TEST="echo ${OUTPUT}"
# TODO more "known good" hashes

. $(dirname "$0")/../common/check.sh

exit 0
