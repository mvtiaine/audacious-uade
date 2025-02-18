#!/bin/sh

TOOL=$(which dnf 2>/dev/null)
if [ -z "$TOOL" ]; then
  TOOL=$(which zypper 2>/dev/null)
  if [ -z "$TOOL" ]; then
    TOOL=$(which yum 2>/dev/null)
    if [ -z "$TOOL" ]; then
        echo Could not find dnf, zypper or yum
        exit 1
    fi
  fi
fi

set -e

VERSION=$(cat VERSION)

sudo $TOOL install rpmdevtools rpm-build audacious-devel libopenmpt-devel libxmp-devel gcc-c++ make autoconf automake libtool pkg-config
autoreconf -i && ./configure --enable-players=all --enable-plugin-audacious=yes --with-static-stdlibs=no && make clean && make dist
mkdir -p build-rpm
cd build-rpm
rm -rf *
mkdir -p rpmbuild/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
ln -sf ../../../audacious-uade-${VERSION}.tar.bz2 rpmbuild/SOURCES/
HOME="${PWD}" rpmbuild -ba ../scripts/audacious-uade.spec
