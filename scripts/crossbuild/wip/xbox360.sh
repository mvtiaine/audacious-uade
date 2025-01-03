 #!/bin/sh

set -e

# Xbox 360 does not actually build yet

export DEVKITXENON="/usr/local/xenon"
export PATH="$PATH:$DEVKITXENON/bin:$DEVKITXENON/usr/bin"

if [ $# -eq 0 ]
then
  docker run --rm -t -v .:/audacious-uade -w /audacious-uade \
    --entrypoint /bin/sh free60/libxenon \
    $0 build
else
  CC=xenon-gcc CXX=xenon-g++ LD=xenon-ld AR=xenon-ar RANLIB=xenon-ranlib AS=xenon-as NM=xenon-nm \
    CPPFLAGS="-DXENON -isystem ${DEVKITXENON}/usr/include" \
    CFLAGS="-m32 -maltivec -fno-pic -mpowerpc64 -mhard-float" CXXFLAGS="${CFLAGS}" \
    LDFLAGS="-L${DEVKITXENON}/xenon/lib/32 -L${DEVKITXENON}/usr/lib -Wl,--script,${DEVKITXENON}/app.lds -lxenon" \
    ./configure --host=ppc-elf
  make clean
  make -j check
fi
