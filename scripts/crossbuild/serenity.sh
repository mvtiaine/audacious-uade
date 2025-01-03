 #!/bin/sh

set -e

# SerenityOS binaries not tested

#autoreconf -i

export PATH="/opt/cross/serenityos/bin:$PATH"
export PKG_CONFIG_PATH=/dev/null
export PKG_CONFIG_LIBDIR=/dev/null
export PKG_CONFIG_SYSROOT_DIR=/dev/null

./configure --host=x86_64-pc-serenity

make clean

make -j check
