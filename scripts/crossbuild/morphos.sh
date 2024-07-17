 #!/bin/sh

set -e

#autoreconf -i

PATH=/opt/cross/morphos/bin:$PATH \
  ./configure --host=ppc-morphos

make clean

PATH=/opt/cross/morphos/bin:$PATH make -j check
