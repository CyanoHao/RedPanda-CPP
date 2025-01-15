#!/bin/bash

set -xeuo pipefail

function set_variables() {
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

function prepare_port_dir() {
  [[ -d "$PORT_DIR" ]] && rm -rf "$PORT_DIR" || true
  mkdir -p "$PORT_DIR"
}

function prepare_port_files() {
  sed "s/__VERSION__/$VERSION/g" "$PORT_TEMPLATE/PKGBUILD.in" >"$PORT_DIR/PKGBUILD"
}

function prepare_source_files() {
  git archive --prefix="RedPanda-CPP/" -o "$PORT_DIR/RedPanda-CPP.tar.gz" HEAD
  cp "$PORT_TEMPLATE/compiler_hint.lua" "$PORT_DIR/"
}

function build_port() {
  cd "$PORT_DIR"
  makepkg -s --noconfirm
}

set_variables
prepare_port_dir
prepare_port_files
prepare_source_files
build_port
