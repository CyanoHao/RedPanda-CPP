#!/bin/bash

set -xeuo pipefail

function set_variables() {
  SPEC_TEMPLATE="$(dirname "$0")"
  SPEC_DIR=$(rpm --eval %{_specdir})
  SOURCE_DIR=$(rpm --eval %{_sourcedir})

  # 1. try git describe: <tag>-<nth-commit>-g<sha>, e.g. v1.2-3-g456789ab
  # 2. format tag
  #    - remove leading 'v'
  #    - add 'r' before <nth-commit>
  #    - replace '-' with '.'
  #    e.g. 1.2.r3.456789ab
  # 3. if fail (e.g. forked repo without tag), treat as v0.0, e.g. 0.0.r123.g456789ab
  VERSION="$(git describe --long --tags | sed 's/^v//;s/\([^-]*-g\)/r\1/;s/-/./g')" || \
    VERSION="0.0.r$(git rev-list HEAD --count).g$(git rev-parse --short HEAD)"

  (( EUID == 0 )) && SUDO="" || SUDO="sudo"
}

function prepare_rpm_dir() {
  local build_dir="$(rpm --eval %{_builddir})/redpanda-cpp"
  [[ -d "$build_dir" ]] && rm -rf "$build_dir" || true
}

function prepare_rpm_files() {
  sed "s/__VERSION__/$VERSION/g" "$SPEC_TEMPLATE/redpanda-cpp.spec.in" >"$SPEC_DIR/redpanda-cpp.spec"
}

function prepare_source_files() {
  git archive --prefix="RedPanda-CPP/" -o "$SOURCE_DIR/RedPanda-CPP.tar.gz" HEAD
}

function build_rpm() {
  $SUDO zypper in -y $(rpmspec -q --buildrequires $SPEC_DIR/redpanda-cpp.spec)

  # RPATHs are forbidden by openSUSE, but qmake is not patched to prevent RPATHs.
  # It's a minor issue, simply disable the check.
  # `export NO_BRP_CHECK_RPATH=true` does not work.
  # (https://en.opensuse.org/openSUSE:Packaging_checks#Beware_of_Rpath)
  QA_RPATHS=$((0x0001)) \
    rpmbuild --nodebuginfo -bb "$SPEC_DIR/redpanda-cpp.spec"
}

set_variables
prepare_rpm_dir
prepare_rpm_files
prepare_source_files
build_rpm
