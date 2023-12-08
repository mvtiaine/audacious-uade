#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD=${top_srcdir}/testdata/mod.orchannel
TESTMD5=$($MD5 $TESTMOD)

TEST_NAME=songdb
export SONGDB_DIR=${SONGDB_DIR}
EXPECTED_OUTPUT=$(cd ${SONGDB_DIR}; grep $(echo $TESTMD5 | cut -c 1-12) songlengths.tsv amp.tsv demozoo.tsv modland.tsv unexotica.tsv)
TEST="${SONGDB_BIN} \"${TESTMOD}\""

. $(dirname "$0")/../common/check.sh
exit 0
