 #!/bin/sh

set -e

# PSP does not actually build yet

if [ $# -eq 0 ]
then
  docker run --rm -t -v .:/audacious-uade -w /audacious-uade \
    --entrypoint /bin/sh pspdev/pspdev \
    $0 build
else
  apk add build-base make autoconf automake libtool pkgconf
  SYSROOT=/usr/local/pspdev \
    CPPFLAGS="-isystem ${SYSROOT}/psp/sdk/include" \
    ./configure --host=psp --with-sysroot=${SYSROOT}
  make clean
  make -j check
fi
