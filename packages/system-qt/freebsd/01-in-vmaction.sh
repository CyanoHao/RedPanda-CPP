#!/bin/sh

set -xeuo pipefail

pkg install -y curl git qt6-base qt6-svg qt6-tools
curl https://download.freebsd.org/ports/ports/ports.tar.zst | zstdcat | tar -C /usr -x
git config --global --add safe.directory .

"$(dirname "$0")/build.sh"

mkdir -p dist
cp /tmp/redpanda-cpp/work/pkg/* dist/
