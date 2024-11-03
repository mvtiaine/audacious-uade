 #!/bin/sh

set -e

# PS4 binaries not tested

# autoreconf -i

export OO_PS4_TOOLCHAIN=/opt/cross/OpenOrbis/PS4Toolchain
TOOLCHAIN=${OO_PS4_TOOLCHAIN}

CC=clang CXX=clang++ LD=ld.lld \
  CPPFLAGS="-isysroot ${TOOLCHAIN} -isystem ${TOOLCHAIN}/include" \
  CFLAGS="-nostdlib --target=x86_64-pc-freebsd12-elf -fPIC" \
  CXXFLAGS="-isystem ${TOOLCHAIN}/include/c++/v1" \
  LDFLAGS="-fuse-ld=lld -Wl,-m,elf_x86_64 -pie -Wl,--script,${TOOLCHAIN}/link.x -L${TOOLCHAIN}/lib -lc -lkernel -lc++ -Xcompiler ${TOOLCHAIN}/lib/crt1.o" \
  # TODO sed config.sub ?
  ./configure --host=x86_64-scei-ps4 --with-sysroot=${TOOLCHAIN}
  make clean
  make -j check
