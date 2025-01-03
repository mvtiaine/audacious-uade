#!/bin/sh

set -e

# Fuchsia binaries not tested

#autoreconf -i

export PATH="/opt/cross/fuchsia-clang/bin:$PATH"
export PKG_CONFIG_PATH=/dev/null
export PKG_CONFIG_LIBDIR=/dev/null
export PKG_CONFIG_SYSROOT_DIR=/dev/null

SYSROOT=/opt/cross/fuchsia-core/arch/arm64/sysroot \
  CC=clang CXX=clang++ LD=ld.lld \
  CFLAGS="-target aarch64-unknown-fuchsia --sysroot ${SYSROOT}" CXXFLAGS="${CFLAGS}" \
  ./configure --host=aarch64-unknown-fuchsia --with-sysroot=${SYSROOT}

make clean

make -j check
