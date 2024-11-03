 #!/bin/sh

set -e

# Dreamcast does not actually build yet

if [ $# -eq 0 ]
then
  docker run --rm -t -v .:/audacious-uade -w /audacious-uade \
    --entrypoint /bin/bash kazade/dreamcast-sdk \
    $0 build
else
  dnf install -y gcc make autoconf automake libtool pkg-config which diffutils
  . /opt/toolchains/dc/kos/environ.sh
  # XXX redefine -D__DREAMCAST__ also in CPPFLAGS for configure.ac detection
  CC=${KOS_CC} CXX=${KOS_CCPLUS} LD=${KOS_LD} AR=${KOS_AR} RANLIB=${KOS_RANLIB} \
    CPPFLAGS="-D__DREAMCAST__" CFLAGS="${KOS_CFLAGS}" CXXFLAGS="${KOS_CPPFLAGS}" \
    LDFLAGS="${KOS_LDFLAGS} ${KOS_LIBS}" \
    ./configure --host=sh-elf
  make clean
  make -j check
fi
