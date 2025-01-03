 #!/bin/sh

set -e

# VxWorks does not actually build yet

#autoreconf -i

export PKG_CONFIG_PATH=/dev/null
export PKG_CONFIG_LIBDIR=/dev/null
export PKG_CONFIG_SYSROOT_DIR=/dev/null

. /opt/cross/wrsdk-vxworks7-qemu/sdkenv.sh

./configure --host=x86_64-pc-vxworks

make clean

make -j check
