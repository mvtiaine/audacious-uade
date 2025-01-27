#!/bin/sh

set -e

player=uade

. $(dirname "$0")/../common/header.sh

#

TESTMOD="${top_srcdir}/testdata/chipsong.med"
TESTMD5_BIG=58e92a4b6be4ca25ccc7bebbc1de61b8
TESTMD5_LITTLE=109e14e01fffcad9cf8084bd7b29526f

TEST_NAME="UADE native endian + converted"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
if [ $ENDIAN -eq $BIG ]; then
  echo 'Detected big endian host'
  EXPECTED_OUTPUT=$TESTMD5_BIG
else
  echo 'Detected little endian host'
  EXPECTED_OUTPUT=$TESTMD5_LITTLE
fi
. $(dirname "$0")/../common/check.sh

exit 0

