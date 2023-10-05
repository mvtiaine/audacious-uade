#!/bin/sh

set -e

sudo apt install build-essential audacious-dev libbsd-dev autotools-dev autoconf autoconf-archive automake libtool pkg-config
VERSION=$(cat VERSION)
sed s/VERSION/${VERSION}/g debian/changelog.in > debian/changelog
autoreconf -i
./configure
make clean
make -j distcheck
mv audacious-uade-${VERSION}.tar.bz2 audacious-uade-${VERSION}.orig.tar.bz2
dpkg-buildpackage
