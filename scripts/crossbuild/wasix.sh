#!/bin/sh

set -e

# Wasix support does not actually work yet

#autoreconf -i

SYSROOT=/opt/cross/wasix-sysroot \
  CFLAGS="--sysroot=${SYSROOT}" CXXFLAGS="--sysroot=${SYSROOT}" LDFLAGS="--sysroot=${SYSROOT}" \
  PATH=/opt/cross/wasi-sdk-22.0/bin:$PATH CC=clang CXX=clang++ LD=wasm-ld \
  ./configure --host=wasm32-wasi

make clean

PATH=/opt/cross/wasi-sdk-22.0/bin:$PATH \
  WRAPPER="$(which wasmer || echo wasmer) run --dir $(realpath testdata)" make -j check
