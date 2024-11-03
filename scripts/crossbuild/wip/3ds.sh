 #!/bin/sh

set -e

# 3DS does not actually build yet

# XXX devkitpro/devkitarm outdated ? (missing 3dsvars.sh which is included in devkitA64)

# devkitarmvars.sh
export DEVKITPRO=/opt/devkitpro
export DEVKITARM=${DEVKITPRO}/devkitARM
export PORTLIBS_ROOT=${DEVKITPRO}/portlibs
export PATH=${DEVKITPRO}/tools/bin:$DEVKITARM/bin:$PATH
export TOOL_PREFIX=arm-none-eabi-
export CC=${TOOL_PREFIX}gcc
export CXX=${TOOL_PREFIX}g++
export AR=${TOOL_PREFIX}gcc-ar
export RANLIB=${TOOL_PREFIX}gcc-ranlib

# 3dsvars.sh
PORTLIBS_PREFIX=${PORTLIBS_ROOT}/3ds
PATH=$PORTLIBS_PREFIX/bin:$PATH

export CFLAGS="-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft -O2 -mword-relocations -ffunction-sections -fdata-sections"
export CXXFLAGS="${CFLAGS}"
export CPPFLAGS="-D_3DS -D__3DS__ -I${PORTLIBS_PREFIX}/include -I${DEVKITPRO}/libctru/include"
export LDFLAGS="-L${PORTLIBS_PREFIX}/lib -L${DEVKITPRO}/libctru/lib"
# must be set in configure.ac
#export LIBS="-lctru"

if [ $# -eq 0 ]
then
  docker run --rm -t -v .:/audacious-uade -w /audacious-uade \
    --entrypoint /bin/sh devkitpro/devkitarm \
    $0 build
else
  apt update && apt install -y build-essential autoconf automake libtool pkg-config
  LDFLAGS="${LDFLAGS} -lctru" \
    ./configure --host=arm-none-eabi
  make clean
  make -j check
fi
