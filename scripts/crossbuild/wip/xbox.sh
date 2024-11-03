 #!/bin/sh

set -e

# Xbox does not actually build yet

export NXDK_DIR=/usr/src/nxdk
export PATH="${NXDK_DIR}/bin:$PATH"

if [ $# -eq 0 ]
then
  docker run --rm -t -v .:/audacious-uade -w /audacious-uade \
    --entrypoint /bin/sh xboxdev/nxdk \
    $0 build
else
  apk add build-base make autoconf automake libtool pkgconf
  autoreconf -i
  CC=nxdk-cc CXX=nxdk-cxx LD=nxdk-link LIB=nxdk-lib AS=nxdk-as \
    CFLAGS="-fno-PIC" \
    LDFLAGS="-L${NXDK_DIR}/lib -L${NXDK_DIR}/lib/xboxkrnl -llibc++ -llibwinapi -llibxboxkrnl -llibxboxrt -llibpdclib -llibnxdk_hal -llibnxdk -lnxdk_usb" \
    ./configure --host=i386-pc-windows --with-sysroot=${NXDK_DIR}
  make clean
  make -j check
fi
