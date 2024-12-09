#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

export PLAYER_ENDIAN=little

TESTMOD="${top_srcdir}/testdata/long white clouds.it"
# arm/ppc 64-bit
TESTMD5_LITTLE=27ebf1a6650419c7aa0bbddd187836d9

TEST_NAME="it2play (IT 16-bit unsigned)"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
EXPECTED_OUTPUT=$TESTMD5_LITTLE

. $(dirname "$0")/../common/check.sh

exit 0
