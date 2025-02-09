#!/bin/sh

set -e

# -Werror
make clean && CFLAGS="-Wall -Wpedantic -Wextra -Werror" CXXFLAGS="${CFLAGS}" ./configure && make -j check

# assert disabled
#make clean && CPPFLAGS="-DNDEBUG" CFLAGS="-Wall -Wpedantic -Wextra -Werror -Wno-unused" CXXFLAGS="${CFLAGS}" ./configure && make -j check

# -Wformat
#make clean && CFLAGS="-Wformat=2 -Werror" CXXFLAGS="${CFLAGS}" ./configure && make -j check

# -D_FORTIFY_SOURCE=3 -fstrict-flex-arrays=3
#make clean && CFLAGS="-O -fstrict-flex-arrays=3 -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 -Werror" CXXFLAGS="${CFLAGS}" ./configure && make -j check

# -fstack-clash-protection -fstack-protector-strong -fcf-protection=full
#make clean && CFLAGS="-fstack-clash-protection -fstack-protector-strong -fcf-protection=full -Werror" CXXFLAGS="${CFLAGS}" ./configure && make -j check

# scan-build
#make clean && ./configure && scan-build -disable-checker unix.Errno -disable-checker deadcode.DeadStores -analyze-headers --status-bugs make -j
#make clean && ./configure && scan-build -disable-checker unix.Errno --exclude ./uade/src/ -analyze-headers --status-bugs make -j

# -fanalyzer (no C++ support)
#make clean && \
#    CFLAGS="-fanalyzer" CXXFLAGS="${CFLAGS}" ./configure && \
#    make -j check

# sanitizers
#make clean && \
#    CFLAGS="-fsanitize=address -fno-sanitize-recover=all -Og -g" \
#    CXXFLAGS="${CFLAGS}" LDFLAGS="-fsanitize=address" \
#    ./configure && make -j check
#make clean && \
#    CFLAGS="-fsanitize=undefined -fno-sanitize-recover=all -fsanitize-ignorelist=$(realpath scripts/ubignorelist.txt) -Og -g" \
#    CXXFLAGS="${CFLAGS}" LDFLAGS="-fsanitize=undefined" \
#    ./configure && make -j check
#make clean && \
#    CFLAGS="-fsanitize=thread -fno-sanitize-recover=all -Og -g" \
#    CXXFLAGS="${CFLAGS}" LDFLAGS="-fsanitize=thread" \
#    ./configure && make -j check

#make clean &&
#    CFLAGS="-fsanitize=memory -fno-sanitize-recover=all " \
#    CXXFLAGS="${CFLAGS}" LDFLAGS="-fsanitize=memory" \
#    ./configure && make -j check
#make clean && \
#    CFLAGS="-flto -fsanitize=cfi -fno-sanitize-recover=all " \
#    CXXFLAGS="${CFLAGS}" LDFLAGS="-fsanitize=cfi" \
#     ./configure && make -j check

# valgrind
#make clean && \
#  CFLAGS="-gdwarf-4" CXXFLAGS="${CFLAGS}" ./configure && \
#  make -j check && \
# WRAPPER="$(which valgrind || echo valgrind) --quiet --exit-on-first-error=yes --error-exitcode=1 --trace-children=yes --track-origins=yes --leak-check=full " make -j2 check

# callgrind
#make clean && \
#  CFLAGS="-gdwarf-4" CXXFLAGS="${CFLAGS}" ./configure && \
#  make -j check && \
#  WRAPPER="$(which valgrind || echo valgrind) --tool=callgrind" make -j check
