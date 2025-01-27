#!/bin/sh

set -e

# -Werror
make clean && CFLAGS="-Wall -Wpedantic -Wextra -Werror" CXXFLAGS="${CFLAGS}" ./configure && make -j check

# assert disabled
#make clean && CPPFLAGS="-DNDEBUG" CFLAGS="-Wall -Wpedantic -Wextra -Werror -Wno-unused" CXXFLAGS="${CFLAGS}" ./configure && make -j check

# scan-build
#make clean && ./configure && scan-build -disable-checker unix.Errno -disable-checker deadcode.DeadStores -analyze-headers --status-bugs make -j
#make clean && ./configure && scan-build -disable-checker unix.Errno --exclude ./uade/src/ -analyze-headers --status-bugs make -j

# -fanalyzer (no C++ support)
#make clean && \
#    CFLAGS="-fanalyzer" CXXFLAGS="${CFLAGS}" ./configure && \
#    make -j check

# sanitizers
#make clean && \
#    CFLAGS="-fsanitize=address -Og -g" CXXFLAGS="${CFLAGS}" LDFLAGS=-fsanitize=address ./configure && \
#    make -j check
#make clean && \
#    CFLAGS="-fsanitize=undefined -Og -g" CXXFLAGS="${CFLAGS}" LDFLAGS=-fsanitize=undefined ./configure && \
#    make -j check
#make clean && \
#    CFLAGS="-fsanitize=thread -Og -g" CXXFLAGS="${CFLAGS}" LDFLAGS=-fsanitize=thread ./configure && \
#    make -j check

#make clean &&
#    CFLAGS="-fsanitize=memory" CXXFLAGS="${CFLAGS}" LDFLAGS=-fsanitize=memory ./configure && \
#    make -j check
#make clean && \
#    CFLAGS="-flto -fsanitize=cfi" CXXFLAGS="${CFLAGS}" LDFLAGS=-fsanitize=cfi ./configure && \
#    make -j check

# valgrind
#make clean && \
#  CFLAGS="-gdwarf-4" CXXFLAGS="${CFLAGS}" ./configure && \
#  make -j check && \
# WRAPPER="$(which valgrind || echo valgrind) --quiet --exit-on-first-error=yes --error-exitcode=1 --trace-children=yes --track-origins=yes --leak-check=full " make -j check

# callgrind
#make clean && \
#  CFLAGS="-gdwarf-4" CXXFLAGS="${CFLAGS}" ./configure && \
#  make -j check && \
#  WRAPPER="$(which valgrind || echo valgrind) --tool=callgrind" make -j check
