#!/bin/bash

set -xeuo pipefail

echo 'FEATURES="$FEATURES getbinpkg"' >>/etc/portage/make.conf
sed -i 's/priority = 1/priority = 9999/' /etc/portage/binrepos.conf/gentoobinhost.conf
if [[ -v MIRROR && -n "$MIRROR" ]]
then
  echo "GENTOO_MIRRORS=\"https://$MIRROR/gentoo/\"" >>/etc/portage/make.conf
  sed -i "s|distfiles.gentoo.org|$MIRROR/gentoo|" /etc/portage/binrepos.conf/gentoobinhost.conf
fi

emerge-webrsync
getuto
eselect profile set default/linux/amd64/23.0/desktop
emerge --update --deep --newuse dev-vcs/git dev-qt/qtbase dev-qt/qtsvg dev-qt/qttools

"$(dirname "$0")/build.sh"

mkdir -p dist
cp /var/cache/binpkgs/dev-util/redpanda-cpp/redpanda-cpp-*.gpkg.tar dist/
