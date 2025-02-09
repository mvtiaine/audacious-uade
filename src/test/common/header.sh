#!/bin/sh

set -e

if [ "$player" != "" ] && [ "$players" != "all" ] && ! printf ",${players}," | grep -Eq ",${player},"; then
  exit 77
fi

DIRNAME=$(dirname "$0")
: ${top_srcdir:="$DIRNAME/../../.."}
: ${top_builddir:="$DIRNAME/../../.."}

echo top_builddir=$top_builddir
echo top_srcdir=$top_srcdir

BIG=0
LITTLE=1
ENDIAN=$(echo -n I | od -to2 | head -n1 | tr -d "[:blank:]" | tail -c2)

MD5=${top_builddir}/src/test/md5
CONVERTER=${top_builddir}/src/plugin/cli/converter/converter
PRECALC="${top_builddir}/src/plugin/cli/precalc/precalc"
SONGEND="${top_builddir}/src/plugin/cli/songend/songend"
PLAYER="${top_builddir}/src/plugin/cli/player/player 8062"
SONGDB_BIN=${top_builddir}/src/plugin/cli/songdb/songdb
SONGDB_DIR=${top_srcdir}/conf/songdb

if [ "${WRAPPER}" != "" ]; then
    MD5="${WRAPPER} ${MD5}"
    CONVERTER="${WRAPPER} ${CONVERTER}"
    PRECALC="${WRAPPER} ${PRECALC}"
    SONGEND="${WRAPPER} ${SONGEND}"
    PLAYER="${WRAPPER} ${PLAYER}"
    SONGDB_BIN="${WRAPPER} ${SONGDB_BIN}"
fi
