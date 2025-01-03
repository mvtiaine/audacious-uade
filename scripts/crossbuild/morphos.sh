 #!/bin/sh

set -e

#autoreconf -i

export "PATH=/opt/cross/morphos/bin:$PATH"
export PKG_CONFIG_PATH=/dev/null
export PKG_CONFIG_LIBDIR=/dev/null
export PKG_CONFIG_SYSROOT_DIR=/dev/null

./configure --host=ppc-morphos

make clean

make -j check
