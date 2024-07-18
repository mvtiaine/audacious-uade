 #!/bin/sh

set -e

#autoreconf -i

PATH=/opt/cross/amigaos/bin:$PATH \
  ./configure --host=m68k-amigaos

make clean

PATH=/opt/cross/amigaos/bin:$PATH make -j check
