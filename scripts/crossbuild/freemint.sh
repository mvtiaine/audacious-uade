 #!/bin/sh

set -e

# FreeMint binaries not tested

#autoreconf -i

SYSROOT=/opt/cross/mint/m68k-atari-mint/sys-root PATH="/opt/cross/mint/bin:$PATH" \
  CFLAGS="--sysroot=$SYSROOT" CXXFLAGS="--sysroot=$SYSROOT" LDFLAGS="--sysroot=$SYSROOT" \
  ./configure --host=m68k-atari-mint --with-sysroot=$SYSROOT

make clean

PATH=/opt/cross/mint/bin:$PATH make -j check
