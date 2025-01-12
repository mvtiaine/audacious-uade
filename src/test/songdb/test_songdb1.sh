#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD=${top_srcdir}/testdata/mod.orchannel

TEST_NAME=songdb1
export SONGDB_DIR=${SONGDB_DIR}
EXPECTED_OUTPUT="songlengths.tsv:025e47e9f0d3	1	0,n 4500,p 4500,p 61560,p
modinfos.tsv:025e47e9f0d3	Protracker	0
combined.tsv:025e47e9f0d3	Matt Furniss	U.S. Gold	Out Run Europa	1991"
TEST="${SONGDB_BIN} \"${TESTMOD}\""

. $(dirname "$0")/../common/check.sh
exit 0
