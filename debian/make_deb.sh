#!/bin/sh

set -e

sudo apt install build-essential audacious-dev libbsd-dev autotools-dev autoconf autoconf-archive automake libtool pkg-config debhelper
VERSION=$(cat VERSION)
autoreconf -i && ./configure && make clean && make -j distcheck && make clean
mkdir -p build-deb
cd build-deb
rm -rf *
ln -sf ../audacious-uade-${VERSION}.tar.bz2 audacious-uade_${VERSION}.orig.tar.bz2
tar xvjf ../audacious-uade-${VERSION}.tar.bz2
cp -rp ../debian audacious-uade-${VERSION}/
cd audacious-uade-${VERSION}
sed s/VERSION/${VERSION}/g debian/changelog.in > debian/changelog
dpkg-buildpackage
