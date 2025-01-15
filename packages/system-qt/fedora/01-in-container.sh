#!/bin/bash

set -xeuo pipefail

dnf install -y dnf-plugins-core git rpm-build rpmdevtools
rpmdev-setuptree

"$(dirname "$0")/build.sh"

mkdir -p dist
cp ~/rpmbuild/RPMS/$(uname -m)/redpanda-cpp-*.rpm dist/
