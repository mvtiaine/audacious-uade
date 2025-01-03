 #!/bin/sh

set -e

# PS3 does not actually build yet

if [ $# -eq 0 ]
then
  docker run --rm -t -v .:/audacious-uade -w /audacious-uade \
    --entrypoint /bin/sh hldtux/ps3dev \
    $0 build
else
  SYSROOT=/usr/local/ps3dev/ppu/ppu \
     CPPFLAGS="-isystem /usr/local/ps3dev/ppu/include" \
     CFLAGS="--sysroot=${SYSROOT}" \
     CXXFLAGS="${CFLAGS}" \
    ./configure --host=powerpc64-ps3-elf --with-sysroot=${SYSROOT}
  make clean
  make -j check
fi
