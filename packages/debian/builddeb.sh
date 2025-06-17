#!/bin/bash

set -xeuo pipefail

TMP_FOLDER=/tmp/redpandaide
[[ -d $TMP_FOLDER ]] && rm -rf $TMP_FOLDER
mkdir -p "$TMP_FOLDER"

cp -r packages/debian $TMP_FOLDER 
cp -r tools $TMP_FOLDER 
cp -r libs $TMP_FOLDER 
cp -r RedPandaIDE $TMP_FOLDER
cp README.md $TMP_FOLDER
cp LICENSE $TMP_FOLDER
cp NEWS.md $TMP_FOLDER
cp version.json $TMP_FOLDER
cp -r platform $TMP_FOLDER
cp CMakeLists.txt $TMP_FOLDER

cd $TMP_FOLDER
command -v mk-build-deps && mk-build-deps -i -t "apt -y --no-install-recommends" debian/control
dpkg-buildpackage -us -uc
