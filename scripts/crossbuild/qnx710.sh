 #!/bin/sh

set -e

# QNX 7.1.0 binaries not tested

#autoreconf -i

QNX_ABI=x86_64-pc-nto-qnx7.1.0
QNX_BASE=/opt/cross/qnx710
export QNX_HOST=$QNX_BASE/host/linux/x86_64
export QNX_TARGET=$QNX_BASE/target/qnx7

PATH=$QNX_HOST/usr/bin:$PATH ./configure \
  --host=${QNX_ABI}

make clean

PATH=$QNX_HOST/usr/bin:$PATH make -j check
