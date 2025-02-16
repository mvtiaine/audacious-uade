#!/bin/sh

set -e

player=libxmp

. $(dirname "$0")/../common/header.sh

#

# using precalc as libxmp is used as system/external library

TESTMOD="${top_srcdir}/testdata/silly venture.mgt"
TEST_NAME="libxmp"
TEST="${PRECALC} \"${TESTMOD}\""
EXPECTED_OUTPUT="e33a4643d758738626a11aa8fa49ea6d	0	244466	player	libxmp	Megatracker MGT v1.1	12	157178"

. $(dirname "$0")/../common/check.sh

exit 0
