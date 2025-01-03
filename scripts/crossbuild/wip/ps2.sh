 #!/bin/sh

set -e

# PS2 does not actually build yet

if [ $# -eq 0 ]
then
  docker run --rm -t -v .:/audacious-uade -w /audacious-uade \
    --entrypoint /bin/sh ps2dev/ps2dev \
    $0 build
else
  apk add build-base make autoconf automake libtool pkgconf
  autoreconf -i
  SYSROOT=/usr/local/ps2dev/ps2sdk/ee \
    CPPFLAGS="-isystem /usr/local/ps2dev/ps2sdk/common/include -D_EE" \
    CFLAGS="--sysroot=${SYSROOT}" CXXFLAGS="${CFLAGS}" \
    ./configure --host=mips64r5900el-ps2-elf --with-sysroot=${SYSROOT}
  make clean
  make -j check
fi
