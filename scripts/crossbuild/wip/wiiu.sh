 #!/bin/sh

set -e

# Wii U does not actually build yet

# TODO test if libwut works on wii

# XXX devkitpro/devkitppc outdated ? (missing wiiuvars.sh which is included in devkitA64)

# devkitppcvars.sh
export DEVKITPRO=/opt/devkitpro
export DEVKITPPC=${DEVKITPRO}/devkitPPC
export PORTLIBS_ROOT=${DEVKITPRO}/portlibs
export PATH=${DEVKITPRO}/tools/bin:${DEVKITPRO}/devkitPPC/bin:$PATH
export TOOL_PREFIX=powerpc-eabi-
export CC=${TOOL_PREFIX}gcc
export CXX=${TOOL_PREFIX}g++
export AR=${TOOL_PREFIX}gcc-ar
export RANLIB=${TOOL_PREFIX}gcc-ranlib

# wiiuvars.sh
export PORTLIBS_PREFIX=${PORTLIBS_ROOT}/wiiu
export PORTLIBS_PPC=${PORTLIBS_ROOT}/ppc
export PORTLIBS_WIIU=${PORTLIBS_PREFIX}

export CFLAGS="-mcpu=750 -meabi -mhard-float -O2 -ffunction-sections -fdata-sections"
export CXXFLAGS="${CFLAGS}"
export CPPFLAGS="-DESPRESSO -D__WIIU__ -D__WUT__ -I${PORTLIBS_WIIU}/include -I${PORTLIBS_PPC}/include -I${DEVKITPRO}/wut/include"
export LDFLAGS="-L${PORTLIBS_WIIU}/lib -L${PORTLIBS_PPC}/lib -L${DEVKITPRO}/wut/lib -specs=${DEVKITPRO}/wut/share/wut.specs"
# must be in configure.ac
#export LIBS="-lwut -lm"

export PATH=${PORTLIBS_WIIU}/bin:${PORTLIBS_PPC}/bin:$PATH

if [ $# -eq 0 ]
then
  docker run --rm -t -v .:/audacious-uade -w /audacious-uade \
    --entrypoint /bin/sh devkitpro/devkitppc \
    $0 build
else
  apt update && apt install -y build-essential autoconf automake libtool pkg-config
  LDFLAGS="${LDFLAGS} -lwut -lm" \
    ./configure --host=powerpc-eabi
  make clean
  make -j check
fi
