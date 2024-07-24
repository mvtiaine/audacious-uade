 #!/bin/sh

set -e

# RISC OS binaries not tested

#autoreconf -i

PATH=/opt/cross/riscos/cross/bin:$PATH \
  ./configure --host=arm-riscos-gnueabi

make clean

PATH=/opt/cross/riscos/cross/bin:$PATH make -j check
