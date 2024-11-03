 #!/bin/sh

set -e

# SerenityOS binaries not tested

#autoreconf -i

PATH=/opt/cross/serenityos/bin:$PATH \
  ./configure --host=x86_64-pc-serenity

make clean

PATH=/opt/cross/serenityos/bin:$PATH make -j check
