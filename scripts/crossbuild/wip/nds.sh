 #!/bin/sh

set -e

# NDS does not actually build yet

# XXX devkitpro/devkitarm outdated ? (missing ndsvars.sh which is included in devkitA64)

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

# ndsvars.sh
PORTLIBS_PREFIX=${PORTLIBS_ROOT}/nds
PATH=$PORTLIBS_PREFIX/bin:$PATH

export CFLAGS="-march=armv5te -mtune=arm946e-s -O2 -ffunction-sections -fdata-sections"
export CXXFLAGS="${CFLAGS}"
export CPPFLAGS="-D__NDS__ -DARM9 -I${PORTLIBS_PREFIX}/include -I${DEVKITPRO}/libnds/include"
export LDFLAGS="-L${PORTLIBS_PREFIX}/lib -L${DEVKITPRO}/libnds/lib"
# must be set in configure.ac
#export LIBS="-lnds9"

if [ $# -eq 0 ]
then
  docker run --rm -t -v .:/audacious-uade -w /audacious-uade \
    --entrypoint /bin/sh devkitpro/devkitarm \
    $0 build
else
  apt update && apt install -y build-essential autoconf automake libtool pkg-config
  LDFLAGS="${LDFLAGS} -lnds9" \
    ./configure --host=arm-none-eabi
  make clean
  make -j check
fi
