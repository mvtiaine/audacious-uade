 #!/bin/sh

set -e

# DOS/DJGPP does not actually build yet

#autoreconf -i

export PATH=/"opt/cross/djgpp/bin:$PATH"
export PKG_CONFIG_PATH=/dev/null
export PKG_CONFIG_LIBDIR=/dev/null
export PKG_CONFIG_SYSROOT_DIR=/dev/null

./configure --host=i586-pc-msdosdjgpp

make clean

make -j check
