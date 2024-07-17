 #!/bin/sh

set -e

# IRIX binaries do not work properly yet

#autoreconf -i

if [ $# -eq 0 ]
then
  docker run --rm -t -v /opt/cross/irix:/opt/irix -v .:/audacious-uade -w /audacious-uade \
    --entrypoint /bin/bash unxmaal/compilertron \
    scripts/crossbuild/irix.sh build
else
  PATH=/opt/irix/sgug/bin:$PATH \
    CPPFLAGS="-I/opt/irix/root/usr/sgug/include/c++/9 -I/opt/irix/root/usr/sgug/include/c++/9/mips-sgi-irix6.5" \
    LDFLAGS="-L/opt/irix/root/usr/sgug/lib/gcc/mips-sgi-irix6.5/9" \
    ./configure --host=mips-sgi-irix6.5

  make clean

  PATH=/opt/irix/sgug/bin:$PATH make -j check
fi
