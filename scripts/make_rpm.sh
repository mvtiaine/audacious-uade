#!/bin/sh

set -e

VERSION=$(cat VERSION)

sudo yum install rpmdevtools audacious-devel gcc-c++ make autoconf automake libtool pkg-config
autoreconf -i && ./configure && make clean && make dist
mkdir -p build-rpm
cd build-rpm
rm -rf *
mkdir -p rpmbuild/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
ln -sf ../../../audacious-uade-${VERSION}.tar.bz2 rpmbuild/SOURCES/
HOME="${PWD}" rpmbuild -ba ../scripts/audacious-uade.spec
