#! /bin/sh

set -e

# iOS binaries not tested

export PKG_CONFIG_PATH=/dev/null
export PKG_CONFIG_LIBDIR=/dev/null
export PKG_CONFIG_SYSROOT_DIR=/dev/null

# TODO parametrina SDK ?
Build() {
    export CFLAGS="${ARCH_FLAGS} -isysroot $(xcrun --sdk ${SDK} --show-sdk-path)"
    export CXXFLAGS="${CFLAGS}"
    ./configure --host="${CHOST}"
    make clean
    # TODO wrapper for tests
    make -j check
}
CHOST="${CHOST:=arm-apple-darwin}"
#CHOST="x86_64-apple-darwin"
ARCH_FLAGS="${ARCH_FLAGS}"
#ARCH_FLAGS="-arch x86_64"
SDK="${SDK:=iphonesimulator}"
#SDK="iphonesimulator"
#SDK="iphoneos"
#SDK="xros"
#SDK="xrsimulator"

# needs custom UADE support, no fork/exec
#SDK="watchos"
#SDK="watchsimulator"
#SDK="appletvos"
#SDK="appletvsimulator"

Build
