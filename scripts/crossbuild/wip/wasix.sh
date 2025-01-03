#!/bin/sh

set -e

# Wasix support does not actually work yet
# TODO broken: "global is immutable: cannot modify it with `global.set`"

#autoreconf -i

export "PATH=/opt/cross/wasi-sdk-22.0/bin:$PATH"
export PKG_CONFIG_PATH=/dev/null
export PKG_CONFIG_LIBDIR=/dev/null
export PKG_CONFIG_SYSROOT_DIR=/dev/null

SYSROOT=/opt/cross/wasix-sysroot \
  CFLAGS="--sysroot=${SYSROOT}" CXXFLAGS="--sysroot=${SYSROOT}" LDFLAGS="--sysroot=${SYSROOT}" \
  CC=clang CXX=clang++ LD=wasm-ld \
  ./configure --host=wasm32-wasi

make clean

WRAPPER="$(which wasmer || echo wasmer) run --dir $(realpath testdata)" make -j check
#WRAPPER="$(which wasmtime || echo wasmtime) run --dir $(realpath testdata)" make -j check
