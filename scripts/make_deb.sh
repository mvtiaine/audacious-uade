#!/bin/sh

set -e

VERSION=$(cat VERSION)

sudo apt install build-essential audacious-dev libopenmpt-dev libxmp-dev autotools-dev autoconf automake libtool pkg-config debhelper
autoreconf -i && ./configure --enable-players=all --enable-plugin-audacious=yes --with-static-stdlibs=no && make clean && make dist
mkdir -p build-deb
cd build-deb
rm -rf *
ln -sf ../audacious-uade-${VERSION}.tar.bz2 audacious-uade_${VERSION}.orig.tar.bz2
tar xvjf ../audacious-uade-${VERSION}.tar.bz2
cp -rp ../scripts/debian audacious-uade-${VERSION}/
cd audacious-uade-${VERSION}
sed s/VERSION/${VERSION}/g debian/changelog.in > debian/changelog
dpkg-buildpackage
