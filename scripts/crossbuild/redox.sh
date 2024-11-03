 #!/bin/sh

set -e

# Redox OS binaries not tested

#autoreconf -i

PATH=/opt/cross/redox/bin:$PATH \
  ./configure --host=x86_64-unknown-redox

make clean

PATH=/opt/cross/redox/bin:$PATH make -j check
