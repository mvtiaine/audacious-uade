#!/bin/sh

set -e

# Fuchsia binaries not tested

#autoreconf -i

SYSROOT=/opt/cross/fuchsia-core/arch/arm64/sysroot \
  CFLAGS="-target aarch64-unknown-fuchsia --sysroot ${SYSROOT}" CC=clang CXX=clang++ LD=ld.lld \
  PATH=/opt/cross/fuchsia-clang/bin:$PATH \
  ./configure --host=aarch64-unknown-fuchsia --with-sysroot=${SYSROOT}

make clean

PATH=/opt/cross/fuchsia-clang/bin:$PATH make -j check
