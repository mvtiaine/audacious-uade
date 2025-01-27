#!/bin/sh

set -e

player=libdigibooster3

. $(dirname "$0")/../common/header.sh

#

TESTMOD="${top_srcdir}/testdata/tomaatti.dbm"
TEST_NAME="Precalc songend - player (dbm)"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="9a8ff8bc0d82b479255c87a013babe05	0	169093	player	libdigibooster3	DigiBooster Pro 2.21	6	12994"
. $(dirname "$0")/../common/check.sh

exit 0
