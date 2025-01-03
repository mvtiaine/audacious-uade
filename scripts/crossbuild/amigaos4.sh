 #!/bin/sh

set -e

# AmigaOS4 binaries do not actually work yet

#autoreconf -i

export PATH="/opt/cross/amigaos4/bin:$PATH"
export PKG_CONFIG_PATH=/dev/null
export PKG_CONFIG_LIBDIR=/dev/null
export PKG_CONFIG_SYSROOT_DIR=/dev/null

./configure --host=ppc-amigaos

make clean

make -j check
