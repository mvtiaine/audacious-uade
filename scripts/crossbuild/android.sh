#!/bin/sh

set -e

# Android binaries not tested

export NDK=/opt/cross/android/NDK

# Only choose one of these, depending on your build machine...
export TOOLCHAIN="${TOOLCHAIN:=$NDK/toolchains/llvm/prebuilt/darwin-x86_64}"
#export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/darwin-x86_64
#export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64
# Only choose one of these, depending on your device...
export TARGET="${TARGET:=aarch64-linux-android}"
#export TARGET=aarch64-linux-android
#export TARGET=armv7a-linux-androideabi
#export TARGET=i686-linux-android
#export TARGET=x86_64-linux-android
# Set this to your minSdkVersion.
export API="${API:=28}"
#export API=28
# Configure and build.
export AR=$TOOLCHAIN/bin/llvm-ar
export CC=$TOOLCHAIN/bin/$TARGET$API-clang
export AS=$CC
export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
export LD=$TOOLCHAIN/bin/ld
export RANLIB=$TOOLCHAIN/bin/llvm-ranlib
export STRIP=$TOOLCHAIN/bin/llvm-strip

export PKG_CONFIG_PATH=/dev/null
export PKG_CONFIG_LIBDIR=/dev/null
export PKG_CONFIG_SYSROOT_DIR=/dev/null

./configure --host $TARGET
make clean
# TODO wrapper for tests
make -j check
