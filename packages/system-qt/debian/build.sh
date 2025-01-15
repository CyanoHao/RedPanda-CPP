#!/bin/bash

set -xeuo pipefail

function set_variables() {
  DEB_TEMPLATE="$(dirname "$0")"
  DEB_DIR=/tmp/redpanda-cpp

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

function prepare_deb_dir() {
  [[ -d "$DEB_DIR" ]] && rm -rf "$DEB_DIR" || true
  mkdir -p "$DEB_DIR/debian"
}

function prepare_deb_files() {
  cp "$DEB_TEMPLATE"/{compat,control,copyright,rules} "$DEB_DIR/debian"
  echo "redpanda-cpp ($VERSION) unstable; urgency=medium" >"$DEB_DIR/debian/changelog"
  echo "  This is a dummy changelog. See realese page [1] for details." >>"$DEB_DIR/debian/changelog"
  echo "  [1] https://github.com/royqh1979/RedPanda-CPP/releases" >>"$DEB_DIR/debian/changelog"
  echo " -- Red Panda C++ Build Bot <redpanda-cpp@github.com>  $(date -R)" >>"$DEB_DIR/debian/changelog"
}

function prepare_source_files() {
  git archive HEAD | tar -C "$DEB_DIR" -x
  cp "$DEB_TEMPLATE/compiler_hint.lua" "$DEB_DIR/"
}

function build_deb() {
  cd "$DEB_DIR"
  command -v mk-build-deps && mk-build-deps -i -t "apt -y --no-install-recommends" debian/control || true
  dpkg-buildpackage -us -uc
}

set_variables
prepare_deb_dir
prepare_deb_files
prepare_source_files
build_deb
