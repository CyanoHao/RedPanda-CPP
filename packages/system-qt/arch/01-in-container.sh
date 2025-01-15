#!/bin/bash

set -xeuo pipefail

if [[ -v MIRROR && -n "$MIRROR" ]]
then
  echo Server = "http://$MIRROR/archlinux/\$repo/os/\$arch" >/etc/pacman.d/mirrorlist
fi

pacman -Syu --noconfirm --needed base-devel git
sed -i '/exit \$E_ROOT/d' /usr/bin/makepkg  # allow run as root
sed -i -E '/^OPTIONS=/s/!?debug/!debug/' /etc/makepkg.conf  # disable debug info
echo 'MAKEFLAGS="-j$(nproc)"' >/etc/makepkg.conf.d/jobs.conf  # enable parallel build

"$(dirname "$0")/build.sh"

mkdir -p dist
cp /tmp/redpanda-cpp/redpanda-cpp-*.pkg.tar.zst dist/
