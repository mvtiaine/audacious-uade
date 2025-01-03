 #!/bin/sh

set -e

# RISC OS binaries not tested

#autoreconf -i

export PATH="/opt/cross/riscos/cross/bin:$PATH"
export PKG_CONFIG_PATH=/dev/null
export PKG_CONFIG_LIBDIR=/dev/null
export PKG_CONFIG_SYSROOT_DIR=/dev/null

./configure --host=arm-riscos-gnueabi

make clean

make -j check
