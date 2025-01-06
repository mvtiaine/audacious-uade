#!/bin/sh

set -e

# -Werror
#make clean && CFLAGS="-Werror" CXXFLAGS="${CFLAGS}" ./configure && make -j && make -j check

# assert disabled
#make clean && CPPFLAGS="-DNDEBUG" CFLAGS="-Werror -Wno-error=unused-but-set-variable" CXXFLAGS="${CFLAGS}" ./configure && make -j && make -j check

# scan-build
#make clean && CFLAGS="-Werror" CXXFLAGS="${CFLAGS}" ./configure && scan-build -disable-checker unix.Errno -disable-checker deadcode.DeadStores -analyze-headers --status-bugs make -j
#make clean && CFLAGS="-Werror" CXXFLAGS="${CFLAGS}" ./configure && scan-build -disable-checker unix.Errno --exclude ./uade/src/ -analyze-headers --status-bugs make -j

# sanitizers
#make clean && \
#    CFLAGS="-fsanitize=address -Werror -Og -g" CXXFLAGS="${CFLAGS}" LDFLAGS=-fsanitize=address ./configure && \
#    make -j check
#make clean && \
#    CFLAGS="-fsanitize=undefined -Werror -Og -g" CXXFLAGS="${CFLAGS}" LDFLAGS=-fsanitize=undefined ./configure && \
#    make -j check
#make clean && \
#    CFLAGS="-fsanitize=thread -Werror -Og -g" CXXFLAGS="${CFLAGS}" LDFLAGS=-fsanitize=thread ./configure && \
#    make -j check

#make clean &&
#    CFLAGS="-fsanitize=memory -Werror" CXXFLAGS="${CFLAGS}" LDFLAGS=-fsanitize=memory ./configure && \
#    make -j check
#make clean && \
#    CFLAGS="-flto -fsanitize=cfi -Werror" CXXFLAGS="${CFLAGS}" LDFLAGS=-fsanitize=cfi ./configure && \
#    make -j check

# valgrind
#make clean && \
#  CFLAGS="-gdwarf-4 -Werror" CXXFLAGS="${CFLAGS}" ./configure && \
#  make -j check && \
# WRAPPER="$(which valgrind || echo valgrind) --quiet --exit-on-first-error=yes --error-exitcode=1 --trace-children=yes --track-origins=yes --leak-check=full " make -j check

# callgrind
#make clean && \
#  CFLAGS="-gdwarf-4 -Werror" CXXFLAGS="${CFLAGS}" ./configure && \
#  make -j check && \
#  WRAPPER="$(which valgrind || echo valgrind) --tool=callgrind" make -j check
