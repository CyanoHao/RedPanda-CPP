#!/bin/ash

set -xeuo pipefail


function set_variables() {
  PORT_TEMPLATE="$(dirname "$0")"
  PORT_DIR=/tmp/unsupported/redpanda-cpp

  # 1. try git describe: <tag>-<nth-commit>-g<hash>, e.g. v1.2-3-g456789ab
  # 2. format tag
  #    - remove leading 'v'
  #    - replace remaining part with '_p<nth-commit>'
  #    e.g. 1.2_p3
  # 3. if fail (e.g. forked repo without tag), treat as v0.0, e.g. 0.0_p123
  VERSION="$(git describe --long --tags | sed 's/^v//;s/-\([^-]*\)-g[^-]*/_p\1/')" || \
    VERSION="0.0_p$(git rev-list HEAD --count)"
}

function prepare_port_dir() {
  [[ -d "$PORT_DIR" ]] && rm -rf "$PORT_DIR" || true
  mkdir -p "$PORT_DIR"
}

function prepare_port_files() {
  sed "s/__VERSION__/$VERSION/g" "$PORT_TEMPLATE/APKBUILD.in" >"$PORT_DIR/APKBUILD"
}

function prepare_source_files() {
  git archive --prefix="RedPanda-CPP/" -o "$PORT_DIR/RedPanda-CPP.tar.gz" HEAD
}

function build_port() {
  cd "$PORT_DIR"
  abuild -F checksum
  abuild -Fr
}

set_variables
prepare_port_dir
prepare_port_files
prepare_source_files
build_port
