#!/bin/sh

set -e

player=uade

. $(dirname "$0")/../common/header.sh

#

TESTMOD="${top_srcdir}/testdata/spellbound dizzy.bp"

export SONGEND_MODE=1
TEST_NAME="Songend loop detection"
TEST="${PLAYER} \"${TESTMOD}\" | ${SONGEND}"
EXPECTED_OUTPUT=220299
. $(dirname "$0")/../common/check.sh

exit 0
