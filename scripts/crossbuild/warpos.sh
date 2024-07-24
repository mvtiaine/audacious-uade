 #!/bin/sh

set -e

# WarpOS binaries not tested
# also binaries first need to be converted with Elf2Exe2 (or run with ppclibemu(?))

#autoreconf -i

PATH=/opt/cross/warpos/bin:$PATH \
  CC="ppc-morphos-gcc --specs=warpup" CXX="ppc-morphos-g++ --specs=warpup" \
  ./configure --host=ppc-morphos

make clean

PATH=/opt/cross/warpos/bin:$PATH make -j check
