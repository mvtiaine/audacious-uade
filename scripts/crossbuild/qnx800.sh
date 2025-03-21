 #!/bin/sh

set -e

# QNX 8.0.0 binaries not tested

#autoreconf -i

QNX_ABI=x86_64-pc-nto-qnx8.0.0
QNX_BASE=/opt/cross/qnx800
export QNX_HOST=$QNX_BASE/host/linux/x86_64
export QNX_TARGET=$QNX_BASE/target/qnx

export PATH=$QNX_HOST/usr/bin:$PATH
export PKG_CONFIG_PATH=/dev/null
export PKG_CONFIG_LIBDIR=/dev/null
export PKG_CONFIG_SYSROOT_DIR=/dev/null

./configure --host=${QNX_ABI}

make clean

make -j check
