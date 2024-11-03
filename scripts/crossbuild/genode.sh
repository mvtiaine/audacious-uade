 #!/bin/sh

set -e

# Genode binaries not tested

#autoreconf -i

# add LIBS += stdcxx to noux-pkg/tar
# make noux-pkg/tar
# copy/paste env.sh
# remove tar specific stuff (-Wl,-rpath-link=/tmp/build.x86_64/noux-pkg/tar)
# remove -l:libgcc.a -l:libc.lib.so -l:libm.lib.so -l:posix.lib.so -l:stdcxx.lib.so -l:libgcc.a from LIBS

export CC='/usr/local/genode/tool/23.05/bin/genode-x86-gcc'
export CXX='/usr/local/genode/tool/23.05/bin/genode-x86-g++'
export LD='/usr/local/genode/tool/23.05/bin/genode-x86-ld'
export AR='/usr/local/genode/tool/23.05/bin/genode-x86-ar'
export NM='/usr/local/genode/tool/23.05/bin/genode-x86-nm'
export RANLIB='/usr/local/genode/tool/23.05/bin/genode-x86-ranlib'
export STRIP='/usr/local/genode/tool/23.05/bin/genode-x86-strip'
export CPPFLAGS='-nostdinc -I. -I/usr/local/src/cross/genode/repos/libports/include/stdcxx -I/usr/local/src/cross/genode/contrib/stdcxx-4eddc2a55a80ed5d3a50fee3f5c25e7ac42afd72/include/stdcxx -I/usr/local/src/cross/genode/contrib/stdcxx-4eddc2a55a80ed5d3a50fee3f5c25e7ac42afd72/include/stdcxx/std -I/usr/local/src/cross/genode/contrib/stdcxx-4eddc2a55a80ed5d3a50fee3f5c25e7ac42afd72/include/stdcxx/c_global -I/usr/local/src/cross/genode/repos/libports/include/stdcxx/../spec/x86_64/stdcxx -I/usr/local/src/cross/genode/contrib/libc-b36b17e21acc3a6b70bc550e19d08abbb15db71a/include/libc -I/usr/local/src/cross/genode/contrib/libc-b36b17e21acc3a6b70bc550e19d08abbb15db71a/include/spec/x86_64/libc -I/usr/local/src/cross/genode/contrib/libc-b36b17e21acc3a6b70bc550e19d08abbb15db71a/include/spec/x86/libc -I/usr/local/src/cross/genode/contrib/libc-b36b17e21acc3a6b70bc550e19d08abbb15db71a/include/libc -I/usr/local/src/cross/genode/contrib/libc-b36b17e21acc3a6b70bc550e19d08abbb15db71a/include/spec/x86_64/libc -I/usr/local/src/cross/genode/contrib/libc-b36b17e21acc3a6b70bc550e19d08abbb15db71a/include/spec/x86/libc -I/usr/local/src/cross/genode/repos/libports/include/libc-genode -I/usr/local/src/cross/genode/repos/libports/include/libc-genode -I/usr/local/src/cross/genode/repos/base/include/spec/x86 -I/usr/local/src/cross/genode/repos/base/include/spec/x86_64 -I/usr/local/src/cross/genode/repos/os/include/spec/x86_64 -I/usr/local/src/cross/genode/repos/libports/include/spec/x86_64 -I/usr/local/src/cross/genode/repos/base/include/spec/64bit -I/usr/local/src/cross/genode/repos/libports/include/spec/64bit -I/usr/local/src/cross/genode/repos/base/include -I/usr/local/src/cross/genode/repos/os/include -I/usr/local/src/cross/genode/repos/demo/include -I/usr/local/src/cross/genode/repos/libports/include -I/usr/local/src/cross/genode/repos/ports/include -I/usr/local/genode/tool/23.05/bin/../lib/gcc/x86_64-pc-elf/12.3.0/include -D_GNU_SOURCE=1 -fPIC'
export CFLAGS='-ffunction-sections -O2 -m64 -mcmodel=large -g'
export CXXFLAGS='-ffunction-sections -O2 -m64 -mcmodel=large -g'
export LDFLAGS='-nostdlib -Wl,-melf_x86_64 -Wl,-gc-sections -Wl,-z -Wl,max-page-size=0x1000 -Wl,-z -Wl,noexecstack -Wl,--hash-style=sysv -Wl,--dynamic-list=/usr/local/src/cross/genode/repos/base/src/ld/genode_dyn.dl -nostdlib -Wl,-nostdlib -Wl,-Ttext=0x01000000 -m64 -mcmodel=large -m64 -mcmodel=large -Wl,-T/usr/local/src/cross/genode/repos/base/src/ld/genode_dyn.ld -Wl,--dynamic-linker=ld.lib.so -Wl,--eh-frame-hdr'
# libs set in configure.ac, otherwise configure fails
#export LIBS='-L/tmp/build.x86_64/noux-pkg/tar -l:libgcc.a -l:libc.lib.so -l:libm.lib.so -l:posix.lib.so -l:stdcxx.lib.so -l:libgcc.a'
export LIBS='-L/tmp/build.x86_64/noux-pkg/tar '
export LIBTOOLFLAGS='--preserve-dup-deps'
export PS1='<gnu_build>'

PATH=/usr/local/genode/tool/23.05/bin:$PATH \
  ./configure --host=x86_64-pc-elf

make clean

PATH=/usr/local/genode/tool/23.05/bin:$PATH make -j check
