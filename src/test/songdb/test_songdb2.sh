#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD="${top_srcdir}/testdata/MOD.forbidden pineapples"

TEST_NAME=songdb2
export SONGDB_DIR=${SONGDB_DIR}
EXPECTED_OUTPUT="combined.tsv:73ef03e9d660	RM	Dekadence	Kesähäxä	2017"
TEST="${SONGDB_BIN} \"${TESTMOD}\" | grep combined"

. $(dirname "$0")/../common/check.sh
exit 0
