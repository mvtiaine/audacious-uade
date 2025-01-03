 #!/bin/sh

set -e

# WebOS binaries not tested

#autoreconf -i

TARGET=arm-webos-linux-gnueabi
BIN="/opt/cross/webos/bin" 
export PATH="$BIN:$PATH"
export PKG_CONFIG_PATH=/dev/null
export PKG_CONFIG_LIBDIR=/dev/null
export PKG_CONFIG_SYSROOT_DIR=/dev/null

SYSROOT=/opt/cross/webos/$TARGET/sysroot \
   CFLAGS="--sysroot=$SYSROOT" CXXFLAGS="--sysroot=$SYSROOT" LDFLAGS="--sysroot=$SYSROOT" \
   CC=$BIN/$TARGET-gcc CXX=$BIN/$TARGET-g++ AR=$BIN/$TARGET-ar RANLIB=$BIN/$TARGET-ranlib \
  ./configure --host=$TARGET --with-sysroot=$SYSROOT

make clean

make -j check
