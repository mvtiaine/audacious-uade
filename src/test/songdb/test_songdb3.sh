#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD="${top_srcdir}/testdata/a new beginning.ahx"

TEST_NAME="songdb3 (duplicate subsongs)"
export SONGDB_DIR=${SONGDB_DIR}
EXPECTED_OUTPUT="songlengths.tsv:743dca7720e9	0	174340,p 174340,p,!"
TEST="${SONGDB_BIN} \"${TESTMOD}\" | grep songlengths"

. $(dirname "$0")/../common/check.sh
exit 0
