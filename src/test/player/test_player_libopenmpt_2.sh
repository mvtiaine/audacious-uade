#!/bin/sh

set -e

player=libopenmpt

. $(dirname "$0")/../common/header.sh

#

# using precalc as libopenmpt is used as system/external library

export PLAYER=${player}

TESTMOD="${top_srcdir}/testdata/happiness.s3m"
TEST_NAME="libopenmpt S3M 16-bit samples"
TEST="${PRECALC} \"${TESTMOD}\""
# XXX can depend on libopenmpt version
EXPECTED_OUTPUT="1fa15058e64664d91ccfee10de5d16dc	0	228[45][0-9][0-9]	player\+silence	libopenmpt	Scream Tracker 3.01 \(GUS\)	14	173644	04007dc6	f9b5a8a9"

. $(dirname "$0")/../common/check.sh

exit 0
