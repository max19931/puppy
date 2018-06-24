#!/bin/sh
#
# Copyright 2018 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# courtesy of https://wiki.osdev.org/Automated_Build_Using_CircleCI
binutils_version="2.29"
gcc_version="7.2.0"

target=i686-elf
prefix=$HOME/cross/$target

# TODO: see if we can preserve VM state
mkdir -p /tmp/toolchain
cd /tmp/toolchain
rm -rf /tmp/toolchain/*

# Download gcc sources if they are not yet downloaded.
if [ ! -f gcc-$gcc_version.tar.gz ]
then
    wget -c -O gcc-$gcc_version.tar.gz http://ftp.gnu.org/gnu/gcc/gcc-$gcc_version/gcc-$gcc_version.tar.gz
    tar -xf gcc-$gcc_version.tar.gz

    # download GCC prereqs
    cd /tmp/toolchain/gcc-$gcc_version

    curl http://gcc.gnu.org/pub/gcc/infrastructure/gmp-6.1.0.tar.bz2 > gmp.tar.bz2
    tar -xf gmp.tar.bz2
    mv gmp-6.1.0 gmp

    curl http://gcc.gnu.org/pub/gcc/infrastructure/mpfr-3.1.4.tar.bz2 > mpfr.tar.bz2
    tar -xf mpfr.tar.bz2
    mv mpfr-3.1.4 mpfr

    curl http://gcc.gnu.org/pub/gcc/infrastructure/mpc-1.0.3.tar.gz > mpc.tar.bz2
    tar -xf mpc.tar.bz2
    mv mpc-1.0.3 mpc

    curl http://gcc.gnu.org/pub/gcc/infrastructure/isl-0.16.1.tar.bz2 > isl.tar.bz2
    tar -xf isl.tar.bz2
    mv isl-0.16.1 isl
fi

cd /tmp/toolchain

# Download binutils sources if they are not yet downloaded.
if [ ! -f binutils-$binutils_version.tar.bz2 ]
then
    wget -c -O binutils-$binutils_version.tar.bz2 http://ftp.gnu.org/gnu/binutils/binutils-$binutils_version.tar.bz2
    tar -xf binutils-$binutils_version.tar.bz2
fi

# Create build paths.
mkdir -p /tmp/toolchain/build-binutils
mkdir -p /tmp/toolchain/build-gcc

# Build binutils.
cd /tmp/toolchain/build-binutils
sudo rm -rf *
/tmp/toolchain/binutils-$binutils_version/configure --target=$target --prefix=$prefix --with-sysroot --disable-nls --disable-werror 2>&1
make all 2>&1
make install 2>&1
sudo rm -rf *

export PATH=$PATH:$prefix/bin

# Build gcc and libgcc.
cd /tmp/toolchain/build-gcc
/tmp/toolchain/gcc-$gcc_version/configure --target=$target --prefix=$prefix --disable-nls --enable-languages=c,c++ --without-headers 2>&1
make all-gcc 2>&1
make install-gcc 2>&1
make all-target-libgcc 2>&1
make install-target-libgcc 2>&1

# Make sure that our cross compiler will be found by creating links.
# Alternative: Add the $prefix/bin directory to your $PATH.
sudo ln -s -f $prefix/bin/* /usr/local/bin/

sudo apt-get install python3 genisoimage xorriso nasm
sudo DEBIAN_FRONTEND=noninteractive apt-get -y install grub-pc grub2