 #!/bin/sh

set -e

#autoreconf -i

PATH=/opt/cross/aros-i386:$PATH \
  ./configure --host=i386-aros

make clean

PATH=/opt/cross/aros-i386:$PATH make -j check
