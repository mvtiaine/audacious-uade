 #!/bin/sh

set -e

#autoreconf -i

if [ $# -gt 0 ]
then
  top_dir=$(realpath $(dirname "$0")/../..)
  CPPFLAGS="\
    -DUADE_CORE_FILE=\\\"${top_dir}/uade/src/uadecore\\\" \
    -DUADE_BASE_DIR=\\\"${top_dir}/build-tmp/uade\\\" \
    -DSONGDB_DIR=\\\"top_dir:conf/songdb\\\""
fi

PATH=/opt/cross/amigaos/bin:$PATH \
  CPPFLAGS="${CPPFLAGS}" \
  ./configure --host=m68k-amigaos

make clean

PATH=/opt/cross/amigaos/bin:$PATH \
  make -j

if [ $# -gt 0 ]
then
  # TODO vamos doesn't support env variables (PLAYER_ENDIAN)
  # TODO UADE doesn't work with vamos (PIPE: not supported)
  # XXX vamos fails with multiple concurrent instances
  PATH=/opt/cross/amigaos/bin:$PATH \
    WRAPPER="$(which vamos || echo vamos) -C 020 -s 16 -m 262144 -H disable -V top_dir:${top_dir}" \
    make check
else
  PATH=/opt/cross/amigaos/bin:$PATH \
    make check
fi
