 #!/bin/sh

set -e

# WebOS binaries not tested

#autoreconf -i

TARGET=arm-webos-linux-gnueabi
BIN="/opt/cross/webos/bin" 
SYSROOT=/opt/cross/webos/$TARGET/sysroot PATH=$BIN:$PATH \
   CFLAGS="--sysroot=$SYSROOT" CXXFLAGS="--sysroot=$SYSROOT" LDFLAGS="--sysroot=$SYSROOT" \
   CC=$BIN/$TARGET-gcc CXX=$BIN/$TARGET-g++ AR=$BIN/$TARGET-ar RANLIB=$BIN/$TARGET-ranlib \
  ./configure --host=$TARGET --with-sysroot=$SYSROOT

make clean

PATH=$BIN:$PATH make -j check
