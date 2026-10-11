#!/bin/sh

set -e

player=v2redux

. $(dirname "$0")/../common/header.sh

#

# v2redux mixes at 44100 Hz only
PLAYER="${top_builddir}/src/plugin/cli/player/player 44100"
if [ "${WRAPPER}" != "" ]; then
    PLAYER="${WRAPPER} ${PLAYER}"
fi
export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/flybye.v2m"
TESTMD5_LITTLE=d6bfd8169e5e2161048ec0d087a94081

TEST_NAME="v2redux (v2m) bit-exact output"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE
. $(dirname "$0")/../common/check.sh

exit 0
