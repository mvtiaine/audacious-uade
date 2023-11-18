#!/bin/sh

set -e

# scan-build needs this
autoreconf -i

# -Werror
make clean && CFLAGS="-Werror" ./configure && make -j && make -j check

# assert disabled
make clean && CPPFLAGS=-DNDEBUG CFLAGS="-Werror" ./configure && make -j && make -j check

# scan-build
#export CXXFLAGS=-std=gnu++20
#make clean && CFLAGS="-Werror" ./configure && scan-build -disable-checker deadcode.DeadStores -analyze-headers --status-bugs make -j
#make clean && CFLAGS="-Werror" ./configure && scan-build --exclude ./uade/src/ -analyze-headers --status-bugs make -j

# sanitizers
make clean && \
    CFLAGS="-fsanitize=address -Werror" LDFLAGS=-fsanitize=address ./configure && \
    make -j check
make clean && \
    CFLAGS="-fsanitize=undefined -Werror" LDFLAGS=-fsanitize=undefined ./configure && \
    make -j check
make clean && \
    CFLAGS="-fsanitize=thread -Werror" LDFLAGS=-fsanitize=thread ./configure && \
    make -j check

#make clean &&
#    CFLAGS="-fsanitize=memory -Werror" LDFLAGS=-fsanitize=memory ./configure && \
#    make -j check
#make clean && \
#    CFLAGS="-flto -fsanitize=cfi -Werror" LDFLAGS=-fsanitize=cfi ./configure && \
#    make -j check

# valgrind
#make clean && \
#  CFLAGS="-Werror" ./configure && \
#  VALGRIND="$(which valgrind || echo valgrind) --quiet --error-exitcode=1 --track-origins=yes --leak-check=full " make -j check
