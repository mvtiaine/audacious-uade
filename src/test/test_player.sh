#!/bin/sh

set -e

. $(dirname "$0")/common/header.sh

BIG=0
LITTLE=1
ENDIAN=$(echo -n I | od -to2 | head -n1 | tr -d "[:blank:]" | tail -c2)

MD5=${top_builddir}/src/test/md5
PLAYER="${top_builddir}/src/plugin/cli/player/player 8000"

#

TESTMOD=${top_srcdir}/testdata/chipsong.med
TESTMD5_BIG=ed0b58caf1944307cf346f202398ec76
TESTMD5_LITTLE=b1d139e407b2dc22d7cacc0995ed3420

TEST_NAME="UADE native endian + converted"
TEST_OUTPUT=$(${PLAYER} ${TESTMOD} | ${MD5})
if [ $ENDIAN == $BIG ]; then
  echo 'Detected big endian host'
  EXPECTED_OUTPUT=$TESTMD5_BIG
else
  echo 'Detected little endian host'
  EXPECTED_OUTPUT=$TESTMD5_LITTLE
fi
. $(dirname "$0")/common/check.sh

#

export PLAYER_ENDIAN=big

TESTMOD=${top_srcdir}/testdata/mod.orchannel
TESTMD5_BIG=3076251a8903097f16557334e4f0cb55
TESTMD5_LITTLE=5e1ff996d4996d7adc7cdcb61054c4b6
SUBSONG=4

TEST_NAME="UADE big endian + subsong"
TEST_OUTPUT=$(${PLAYER} ${TESTMOD} ${SUBSONG} | ${MD5})
EXPECTED_OUTPUT=$TESTMD5_BIG
. $(dirname "$0")/common/check.sh

export PLAYER_ENDIAN=little

TEST_NAME="UADE little endian + subsong"
TEST_OUTPUT=$(${PLAYER} ${TESTMOD} ${SUBSONG} | ${MD5})
EXPECTED_OUTPUT=$TESTMD5_LITTLE
. $(dirname "$0")/common/check.sh

#

TESTMOD="${top_srcdir}/testdata/mdat.turrican loader"
TESTMD5_LITTLE=f3e4c544376e3adf0ede0c31c7a815d6

TEST_NAME="UADE extload"
TEST_OUTPUT=$(${PLAYER} "${TESTMOD}" | ${MD5})
EXPECTED_OUTPUT=$TESTMD5_LITTLE
. $(dirname "$0")/common/check.sh

#

TESTMOD=${top_srcdir}/testdata/monkmusings.hvl
TESTMD5_LITTLE=18ca60d4c39ecda3a1346af80ebaa97d

TEST_NAME="HivelyTracker"
TEST_OUTPUT=$(${PLAYER} "${TESTMOD}" | ${MD5})
EXPECTED_OUTPUT=$TESTMD5_LITTLE
. $(dirname "$0")/common/check.sh

#

TESTMOD=${top_srcdir}/testdata/tomaatti.dbm
TESTMD5_LITTLE=46df5c69a62fae5c85ae7972a1ebed64

TEST_NAME="libdigibooster3"
TEST_OUTPUT=$(${PLAYER} "${TESTMOD}" | ${MD5})
EXPECTED_OUTPUT=$TESTMD5_LITTLE
. $(dirname "$0")/common/check.sh


exit 0
