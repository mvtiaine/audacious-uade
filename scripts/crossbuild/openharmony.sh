 #!/bin/sh

set -e

# OpenHarmony binaries not tested

#autoreconf -i

HOST=aarch64-linux-ohos
#HOST=x86_64-unknown-linux-ohos
TARGET=aarch64-unknown-linux-ohos
#TARGET=x86_64-unknown-linux-ohos
BIN="/opt/cross/openharmony/native/llvm/bin" 
# TODO sed config.sub ?
SYSROOT=/opt/cross/openharmony/native/sysroot PATH=$BIN:$PATH \
   CFLAGS="--sysroot=$SYSROOT" CXXFLAGS="--sysroot=$SYSROOT" LDFLAGS="--sysroot=$SYSROOT -fuse-ld=lld" \
   CC=$TARGET-clang CXX=$TARGET-clang++ LD=ld.lld AR=llvm-ar RANLIB=llvm-ranlib \
  ./configure --host=$HOST --with-sysroot=$SYSROOT

make clean

PATH=$BIN:$PATH make -j check
