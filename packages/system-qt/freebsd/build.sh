#!/bin/sh

set -xeuo pipefail

set_variables() {
  PORT_TEMPLATE="$(dirname "$0")"
  PORT_DIR=/tmp/redpanda-cpp

  # 1. try git describe: <tag>-<nth-commit>-g<sha>, e.g. v1.2-3-g456789ab
  # 2. format tag
  #    - remove leading 'v'
  #    - add 'r' before <nth-commit>
  #    - replace '-' with '.'
  #    e.g. 1.2.r3.456789ab
  # 3. if fail (e.g. forked repo without tag), treat as v0.0, e.g. 0.0.r123.g456789ab
  VERSION="$(git describe --long --tags | sed 's/^v//;s/\([^-]*-g\)/r\1/;s/-/./g')" || \
    VERSION="0.0.r$(git rev-list HEAD --count).g$(git rev-parse --short HEAD)"
}

prepare_port_dir() {
  [ -d "$PORT_DIR" ] && rm -rf "$PORT_DIR" || true
  mkdir -p "$PORT_DIR"
}

prepare_port_files() {
  sed "s/__VERSION__/$VERSION/g" "$PORT_TEMPLATE/Makefile.in" >"$PORT_DIR/Makefile"
  cp "$PORT_TEMPLATE/pkg-descr" "$PORT_DIR/pkg-descr"
  cp "$PORT_TEMPLATE/pkg-plist.in" "$PORT_DIR/pkg-plist"
  find platform/linux/templates/ -type f | sed 's|^platform/linux|share/RedPandaCPP|' >>"$PORT_DIR/pkg-plist"
}

prepare_source_files() {
  git archive --prefix="RedPanda-CPP/" -o "$PORT_DIR/RedPanda-CPP.tar.gz" HEAD
}

build_port() {
  cd "$PORT_DIR"
  make DISTDIR="$PORT_DIR" makesum
  make DISTDIR="$PORT_DIR" package
}

set_variables
prepare_port_dir
prepare_port_files
prepare_source_files
build_port
