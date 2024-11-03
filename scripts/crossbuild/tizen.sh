 #!/bin/sh

set -e

#autoreconf -i

PLATFORM=8.0

#TARGET=aarch64-linux-gnu
#HOST=aarch64-tizen-linux-gnu
#DEVICE=tizen-8.0-device64.core
TARGET=x86_64-linux-gnu
HOST=x86_64-tizen-linux-gnu
DEVICE=tizen-8.0-emulator64.core

BIN="/opt/cross/tizen-studio/tools/$TARGET-gcc-9.2/bin"

SYSROOT=/opt/cross/tizen-studio/platforms/tizen-$PLATFORM/tizen/rootstraps/$DEVICE PATH=$BIN:$PATH \
   CFLAGS="--sysroot=$SYSROOT" CXXFLAGS="--sysroot=$SYSROOT" LDFLAGS="--sysroot=$SYSROOT" \
   CC=$TARGET-gcc CXX=$TARGET-g++ \
  ./configure --host=$HOST --with-sysroot=$SYSROOT

make clean

PATH=$BIN:$PATH make -j check
