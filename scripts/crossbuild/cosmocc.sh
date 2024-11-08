#!/bin/sh

set -e

# NOTE: building/linking fat binaries directly does not seem to work (libtool issue?)
# TODO separate build dirs + apelink to produce fat binaries ?

ARCH="${ARCH:=aarch64}"
WRAPPER="${WRAPPER:=ape}"
#ARCH=x86_64
#WRAPPER=ape-x86_64.macho
#WRAPPER=ape-aarch64.elf
#WRAPPER=ape-x86_64.elf

#autoreconf -i

#CC=cosmocc CXX=cosmoc++ AR=cosmoar
PATH=/opt/cross/cosmocc/bin:$PATH \
  CC=${ARCH}-unknown-cosmo-cc CXX=${ARCH}-unknown-cosmo-c++ AR=${ARCH}-unknown-cosmo-ar \
  LD=${ARCH}-linux-cosmo-ld RANLIB=${ARCH}-linux-cosmo-ranlib \
  ./configure --host=${ARCH}-unknown-none

make clean

PATH=/opt/cross/cosmocc/bin:$PATH WRAPPER="${WRAPPER}" make -j check
