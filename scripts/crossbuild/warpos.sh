 #!/bin/sh

set -e

# WarpOS binaries not tested
# also binaries first need to be converted with Elf2Exe2 (or run with ppclibemu(?))

#autoreconf -i

export PATH="/opt/cross/warpos/bin:$PATH"
export PKG_CONFIG_PATH=/dev/null
export PKG_CONFIG_LIBDIR=/dev/null
export PKG_CONFIG_SYSROOT_DIR=/dev/null

CC="ppc-morphos-gcc --specs=warpup" CXX="ppc-morphos-g++ --specs=warpup" \
  ./configure --host=ppc-morphos

make clean

make -j check
