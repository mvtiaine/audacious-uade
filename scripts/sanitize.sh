#!/bin/sh

set -e

# scan-build needs this
export CXXFLAGS=-std=gnu++20
autoreconf -i && ./configure
#make clean && scan-build -disable-checker deadcode.DeadStores -analyze-headers --status-bugs make -j
#make clean && scan-build --exclude ./uade/src/ -analyze-headers --status-bugs make -j

make clean && \
    CC=clang CXX=clang++ CFLAGS=-fsanitize=address LDFLAGS=-fsanitize=address ./configure && \
    make -j check
make clean && \
    CC=clang CXX=clang++ CFLAGS=-fsanitize=undefined LDFLAGS=-fsanitize=undefined ./configure && \
    make -j check
make clean && \
    CC=clang CXX=clang++ CFLAGS=-fsanitize=thread LDFLAGS=-fsanitize=thread ./configure && \
    make -j check
#make clean &&
#    CC=clang CXX=clang++ CFLAGS=-fsanitize=memory LDFLAGS=-fsanitize=memory ./configure && \
#    make -j check
#make clean && \
#    CC=clang CXX=clang++ CFLAGS="-flto -fsanitize=cfi" LDFLAGS=-fsanitize=cfi ./configure && \
#    make -j check
