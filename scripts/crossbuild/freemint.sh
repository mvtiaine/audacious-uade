 #!/bin/sh

set -e

# FreeMint binaries not tested

#autoreconf -i

CROSSDIR=/opt/cross/mint
SYSROOT="${CROSSDIR}/m68k-atari-mint/sys-root"

export PATH="${CROSSDIR}/bin:$PATH"
export PKG_CONFIG_PATH=
export PKG_CONFIG_LIBDIR="${CROSSDIR}/crosstools/lib/pkgconfig"
export PKG_CONFIG_SYSROOT_DIR=${SYSROOT}

SYSROOT="${SYSROOT}" CFLAGS="--sysroot=$SYSROOT" CXXFLAGS="--sysroot=$SYSROOT" LDFLAGS="--sysroot=$SYSROOT" \
  ./configure --host=m68k-atari-mint --with-sysroot=$SYSROOT

make clean

make -j check
