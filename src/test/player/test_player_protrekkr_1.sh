#!/bin/sh

set -e

player=protrekkr

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/elevator zax.ptk"
TESTMD5_LITTLE=bc34686a2df0ec88239a6baad2f0f20c

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
