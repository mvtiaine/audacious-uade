#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD=${top_srcdir}/testdata/mod.orchannel

TEST_NAME=songdb1
export SONGDB_DIR=${SONGDB_DIR}
EXPECTED_OUTPUT="songlengths.tsv:7b05bfe48966	1	0,n 4500,p 4500,p 61560,p
modinfos.tsv:7b05bfe48966	Protracker	0
metadata.tsv:7b05bfe48966	Matt Furniss	Probe & U.S. Gold	Out Run Europa	1991"
TEST="${SONGDB_BIN} \"${TESTMOD}\""

. $(dirname "$0")/../common/check.sh
exit 0
