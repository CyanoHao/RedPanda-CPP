#!/bin/bash

set -euxo pipefail

function download {
  local file=$1
  local url=$2
  if [[ ! -f $file ]] ; then
    curl -o $file -L $url
  fi
}

apt update
apt upgrade -y
apt install -y --no-install-recommends \
  build-essential \
  libgmp-dev libmpc-dev libmpfr-dev \
  ca-certificates curl

ln -sf bash /bin/sh

mkdir -p assets assets/32 assets/64

# make and support programs
download assets/make-4.4.1.tar.gz \
  https://ftpmirror.gnu.org/make/make-4.4.1.tar.gz
download assets/busybox-1.36.1.tar.bz2 \
  https://busybox.net/downloads/busybox-1.36.1.tar.bz2

# XP toolchain
download assets/binutils-2.25.1.tar.bz2 \
  https://ftpmirror.gnu.org/binutils/binutils-2.25.1.tar.bz2
download assets/cygwin-cygwin-2_5_2-release.tar.gz \
  https://github.com/cygwin/cygwin/archive/refs/tags/cygwin-2_5_2-release.tar.gz
download assets/gcc-5.5.0.tar.xz \
  https://ftpmirror.gnu.org/gcc/gcc-5.5.0/gcc-5.5.0.tar.xz
download assets/mingw-w64-v4.0.5.tar.bz2 \
  https://sourceforge.net/projects/mingw-w64/files/mingw-w64/mingw-w64-release/mingw-w64-v4.0.5.tar.bz2

# Cygwin bootstrap
download assets/64/cygwin-devel-2.5.2-1.tar.xz \
  http://ctm.crouchingtigerhiddenfruitbat.org/pub/cygwin/circa/64bit/2016/08/30/104235/x86_64/release/cygwin/cygwin-devel/cygwin-devel-2.5.2-1.tar.xz
download assets/32/cygwin-devel-2.5.2-1.tar.xz \
  http://ctm.crouchingtigerhiddenfruitbat.org/pub/cygwin/circa/2016/08/30/104223/x86/release/cygwin/cygwin-devel/cygwin-devel-2.5.2-1.tar.xz

sha256sum -c sha256sum.txt
