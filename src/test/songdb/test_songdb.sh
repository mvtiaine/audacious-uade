#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD=${top_srcdir}/testdata/mod.orchannel
TESTMD5=$($MD5 $TESTMOD)

TEST_NAME=songdb
export SONGDB_DIR=${SONGDB_DIR}
EXPECTED_OUTPUT=$(cd ${SONGDB_DIR}; grep $TESTMD5 amp.tsv modland.tsv unexotica.tsv)
TEST_OUTPUT=$(${SONGDB_BIN} ${TESTMOD})

. $(dirname "$0")/../common/check.sh
exit 0
