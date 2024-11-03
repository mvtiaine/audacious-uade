 #!/bin/sh

set -e

# VxWorks does not actually build yet

#autoreconf -i

. /opt/cross/wrsdk-vxworks7-qemu/sdkenv.sh

./configure --host=x86_64-pc-vxworks

make clean

make -j check
