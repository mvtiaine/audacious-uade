#!/bin/sh

set -e

. $(dirname "$0")/../common/header.sh

#

TESTMOD=${top_srcdir}/testdata/chipsong.med
TESTMD5_BIG=ed0b58caf1944307cf346f202398ec76
TESTMD5_LITTLE=b1d139e407b2dc22d7cacc0995ed3420

TEST_NAME="UADE native endian + converted"
TEST="${PLAYER} \"${TESTMOD}\" | ${MD5}"
if [ $ENDIAN == $BIG ]; then
  echo 'Detected big endian host'
  EXPECTED_OUTPUT=$TESTMD5_BIG
else
  echo 'Detected little endian host'
  EXPECTED_OUTPUT=$TESTMD5_LITTLE
fi
. $(dirname "$0")/../common/check.sh

exit 0

