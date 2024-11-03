 #!/bin/sh

set -e

# TODO testaa kääntää puhelimella suoraan (ja että samat build_meego/host_* variablet)

#autoreconf -i

#TARGET=aarch64-meego-linux-gnu # missing aarch64 target in docker image?
#SYSROOT=/srv/mer/targets/SailfishOS-4.5.0.18-aarch64
#TARGET=armv7hl-meego-linux-gnueabi
#SYSROOT=/srv/mer/targets/SailfishOS-4.5.0.18-armv7hl
TARGET=i486-meego-linux-gnu
SYSROOT=/srv/mer/targets/SailfishOS-4.5.0.18-i486
BIN=/srv/mer/toolings/SailfishOS-4.5.0.18/opt/cross/bin:/srv/mer/toolings/SailfishOS-4.5.0.18/usr/bin

if [ $# -eq 0 ]
then
  docker run --rm -t -v .:/audacious-uade -w /audacious-uade \
    --entrypoint /bin/bash r1tschy/sailfishos-platform-sdk \
    $0 build
else
  # XXX ugly hack
  sudo ln -s $BIN/$TARGET-as /usr/local/bin/as
  sudo ln -s $BIN/$TARGET-ld /usr/local/bin/ld
  PATH=$BIN:$PATH \
    CPP="$TARGET-cpp --sysroot=$SYSROOT" CXXCPP="$CPP" \
    CFLAGS="--sysroot=$SYSROOT" CXXFLAGS="--sysroot=$SYSROOT" LDFLAGS="--sysroot=$SYSROOT" \
    ./configure --host=$TARGET --with-sysroot=$SYSROOT

  PATH=$BIN:$PATH make clean

  PATH=$BIN:$PATH make -j check
fi
