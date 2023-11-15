#!/bin/sh

set -e

. $(dirname "$0")/common/header.sh

MD5=${top_builddir}/src/test/md5
SONGDB_BIN=${top_builddir}/src/plugin/cli/songdb/songdb
SONGDB_DIR=${top_srcdir}/conf/songdb
MODDIR=${top_srcdir}/testdata
TESTMOD=${MODDIR}/mod.orchannel
TESTMD5=$($MD5 $TESTMOD)

TEST_NAME=songdb
export SONGDB_DIR=${SONGDB_DIR}
EXPECTED_OUTPUT=$(cd ${SONGDB_DIR}; grep $TESTMD5 amp.tsv modland.tsv unexotica.tsv)
TEST_OUTPUT=$(${SONGDB_BIN} ${TESTMOD})

. $(dirname "$0")/common/check.sh
exit 0
