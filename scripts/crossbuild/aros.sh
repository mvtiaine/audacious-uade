 #!/bin/sh

set -e

#autoreconf -i

export PATH="/opt/cross/aros-i386:$PATH"
export PKG_CONFIG_PATH=/dev/null
export PKG_CONFIG_LIBDIR=/dev/null
export PKG_CONFIG_SYSROOT_DIR=/dev/null

./configure --host=i386-aros

make clean

make -j check
