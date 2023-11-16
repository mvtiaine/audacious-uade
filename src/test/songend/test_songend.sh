#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

PLAYER="${top_builddir}/src/plugin/cli/player/player 8062"
TESTMOD="${top_srcdir}/testdata/spellbound dizzy.bp"

export SONGEND_MODE=1
TEST_NAME="Songend loop detection"
TEST_OUTPUT=$(${PLAYER} "${TESTMOD}" | ${SONGEND})
EXPECTED_OUTPUT=220368
. $(dirname "$0")/../common/check.sh

exit 0
