 #!/bin/sh

set -e

# PS4 binaries not tested

# autoreconf -i

export OO_PS4_TOOLCHAIN=/opt/cross/OpenOrbis/PS4Toolchain
TOOLCHAIN=${OO_PS4_TOOLCHAIN}

export PKG_CONFIG_PATH=/dev/null
export PKG_CONFIG_LIBDIR=/dev/null
export PKG_CONFIG_SYSROOT_DIR=/dev/null

# XXX OS 'ps4' not recognized
if ! grep -q ps4 config.sub ; then
  gsed -i 's/ ultrix\* \| irix\* / ps4\* \| ultrix\* \| irix\* /g' config.sub
  gsed -i 's/none--\*)/\*-ps4\*- \| none--\*)/g' config.sub
fi

CC=clang CXX=clang++ LD=ld.lld \
  CPPFLAGS="-isysroot ${TOOLCHAIN} -isystem ${TOOLCHAIN}/include" \
  CFLAGS="-nostdlib --target=x86_64-pc-freebsd12-elf -fPIC" \
  CXXFLAGS="${CFLAGS} -isystem ${TOOLCHAIN}/include/c++/v1" \
  LDFLAGS="-fuse-ld=lld -Wl,-m,elf_x86_64 -pie -Wl,--script,${TOOLCHAIN}/link.x -L${TOOLCHAIN}/lib -lc -lkernel -lc++ -Xcompiler ${TOOLCHAIN}/lib/crt1.o" \
  ./configure --host=x86_64-scei-ps4 --with-sysroot=${TOOLCHAIN} \
    --enable-plugin-deadbeef=no # XXX header paths messed up?

make clean
make -j check
