#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD="${top_srcdir}/testdata/MOD.forbidden pineapples"
TESTMD5=$($MD5 "$TESTMOD")

TEST_NAME=songdb
export SONGDB_DIR=${SONGDB_DIR}
EXPECTED_OUTPUT="demozoo.tsv:1070b78e8d71	Ricky Martin	Dekadence	Kesähäxä	2017"
TEST="${SONGDB_BIN} \"${TESTMOD}\" | grep demozoo"

. $(dirname "$0")/../common/check.sh
exit 0
