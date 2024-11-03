 #!/bin/sh

set -e

# DOS/DJGPP does not actually build yet

#autoreconf -i

PATH=/opt/cross/djgpp/bin:$PATH \
  ./configure --host=i586-pc-msdosdjgpp

make clean

PATH=/opt/cross/djgpp/bin:$PATH make -j check
