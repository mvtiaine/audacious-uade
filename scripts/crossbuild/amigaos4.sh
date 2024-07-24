 #!/bin/sh

set -e

# AmigaOS4 binaries do not actually work yet

#autoreconf -i

PATH=/opt/cross/amigaos4/bin:$PATH \
  ./configure --host=ppc-amigaos

make clean

PATH=/opt/cross/amigaos4/bin:$PATH make -j check
