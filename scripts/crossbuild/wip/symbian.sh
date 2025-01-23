 #!/bin/sh

set -e

# Symbian does not actually build yet

#autoreconf -i

export PATH="/opt/cross/gcc4symbian/bin:$PATH"
export PKG_CONFIG_PATH=/dev/null
export PKG_CONFIG_LIBDIR=/dev/null
export PKG_CONFIG_SYSROOT_DIR=/dev/null

SDK=Nokia_N97_SDK_v1.0
#SDK=Nokia_Symbian_Belle_SDK_v1.0
#SDK=S60_3rd_FP2_SDK_v1.1
#SDK=S60_5th_Edition_SDK_v1.0
#SDK=UIQ3.3SDK_BETA

export EPOCROOT="/opt/cross/gcc4symbian/sdk/$SDK"

./configure --host=arm-none-symbianelf

make clean

make -j check
