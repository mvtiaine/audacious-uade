#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD="${top_srcdir}/testdata/mdat.turrican loader"
TESTMD5_LITTLE=f3e4c544376e3adf0ede0c31c7a815d6

TEST_NAME="UADE extload"
TEST_OUTPUT=$(${PLAYER} "${TESTMOD}" | ${MD5})
EXPECTED_OUTPUT=$TESTMD5_LITTLE
. $(dirname "$0")/../common/check.sh

exit 0
