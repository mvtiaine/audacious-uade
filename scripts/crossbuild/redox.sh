 #!/bin/sh

set -e

# Redox OS binaries not tested

#autoreconf -i

export PATH="/opt/cross/redox/bin:$PATH"
export PKG_CONFIG_PATH=/dev/null
export PKG_CONFIG_LIBDIR=/dev/null
export PKG_CONFIG_SYSROOT_DIR=/dev/null

./configure --host=x86_64-unknown-redox

make clean

make -j check
