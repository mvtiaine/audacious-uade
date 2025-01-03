 #!/bin/sh

set -e

# OpenHarmony binaries not tested

#autoreconf -i

HOST=aarch64-linux-ohos
#HOST=x86_64-unknown-linux-ohos
TARGET=aarch64-unknown-linux-ohos
#TARGET=x86_64-unknown-linux-ohos
BIN="/opt/cross/openharmony/native/llvm/bin" 
# XXX OS 'ohos' not recognized
if ! grep -q ohos config.sub ; then
  gsed -i 's/ ultrix\* \| irix\* / ohos\* \| ultrix\* \| irix\* /g' config.sub
  gsed -i 's/none--\*)/\*-ohos\*- \| none--\*)/g' config.sub
fi
export PATH="$BIN:$PATH"
export PKG_CONFIG_PATH=/dev/null
export PKG_CONFIG_LIBDIR=/dev/null
export PKG_CONFIG_SYSROOT_DIR=/dev/null

SYSROOT=/opt/cross/openharmony/native/sysroot \
   CFLAGS="--sysroot=$SYSROOT" CXXFLAGS="--sysroot=$SYSROOT" LDFLAGS="--sysroot=$SYSROOT -fuse-ld=lld" \
   CC=$TARGET-clang CXX=$TARGET-clang++ LD=ld.lld AR=llvm-ar RANLIB=llvm-ranlib \
  ./configure --host=$HOST --with-sysroot=$SYSROOT

make clean

make -j check
