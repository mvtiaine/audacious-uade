 #!/bin/sh

set -e

# Switch does not actually build yet

if [ $# -eq 0 ]
then
  docker run --rm -t -v .:/audacious-uade -w /audacious-uade \
    --entrypoint /bin/sh devkitpro/devkita64 \
    $0 build
else
  apt update && apt install -y build-essential autoconf automake libtool pkg-config
  . /opt/devkitpro/switchvars.sh
  # must be set in configure.ac
  export LIBS=""
  LDFLAGS="-specs=/opt/devkitpro/libnx/switch.specs ${LDFLAGS} -lnx" \
    ./configure --host=aarch64-none-elf
  make clean
  make -j check
fi
