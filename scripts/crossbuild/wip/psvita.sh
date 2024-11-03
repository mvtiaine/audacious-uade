 #!/bin/sh

set -e

# PSVita does not actually build yet

if [ $# -eq 0 ]
then
  docker run --rm -t -v .:/audacious-uade -w /audacious-uade \
    --entrypoint /bin/sh gnuton/vitasdk-docker \
    $0 build
else
  apt install -y build-essential autoconf automake libtool pkg-config
  autoreconf -i
  SYSROOT=/usr/local/vitasdk \
    CPPFLAGS="-DPATH_MAX=1024" \
    ./configure --host=arm-vita-eabi --with-sysroot=${SYSROOT}
  make clean
  make -j check
fi
