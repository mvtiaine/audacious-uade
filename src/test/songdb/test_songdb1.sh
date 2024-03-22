#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD=${top_srcdir}/testdata/mod.orchannel
TESTMD5=$($MD5 $TESTMOD)

TEST_NAME=songdb
export SONGDB_DIR=${SONGDB_DIR}
EXPECTED_OUTPUT="songlengths.tsv:025e47e9f0d3	1	0,n 4500,p 4500,p 61560,p
modinfos.tsv:025e47e9f0d3	Protracker	0
amp.tsv:025e47e9f0d3	Matt Furniss
demozoo.tsv:025e47e9f0d3	Matt Furniss			1991
modland.tsv:025e47e9f0d3	Matt Furniss
unexotica.tsv:025e47e9f0d3	Furniss_Matt/Out_Run_Europa	U.S. Gold	1991"
TEST="${SONGDB_BIN} \"${TESTMOD}\""

. $(dirname "$0")/../common/check.sh
exit 0
