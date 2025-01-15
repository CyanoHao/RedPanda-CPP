#!/bin/bash

set -euxo pipefail

if [[ ! -v ARCH || ! -v APPIMAGE_RUNTIME ]]; then
  echo 'This script should be run in a given container.'
  exit 1
fi

function install_packages() {
  apt update
  DEBIAN_FRONTDEND=noninteractive \
    apt install -y --no-install-recommends \
      rpm
}

function set_variables() {
  PROJECT_ROOT="$PWD"
  BUILD_DIR=/build
  APPIMAGE_DIR=/redpanda-cpp-static.AppDir

  # 1. try git describe: <tag>-<nth-commit>-g<sha>, e.g. v1.2-3-g456789ab
  # 2. format tag
  #    - remove leading 'v'
  #    - add 'r' before <nth-commit>
  #    - replace '-' with '.'
  #    e.g. 1.2.r3.456789ab
  # 3. if fail (e.g. forked repo without tag), treat as v0.0, e.g. 0.0.r123.g456789ab
  VERSION="$(git describe --long --tags | sed 's/^v//;s/\([^-]*-g\)/r\1/;s/-/./g')" || \
    VERSION="0.0.r$(git rev-list HEAD --count).g$(git rev-parse --short HEAD)"
  export VERSION

  case "$ARCH" in
    x86_64)
      DEBIAN_ARCH=amd64
      RPM_ARCH=x86_64
      ;;
    aarch64)
      DEBIAN_ARCH=arm64
      RPM_ARCH=aarch64
      ;;
    riscv64)
      DEBIAN_ARCH=riscv64
      RPM_ARCH=riscv64
      ;;
    loong64)
      DEBIAN_ARCH=loong64
      RPM_ARCH=loongarch64
      ;;
    i686)
      DEBIAN_ARCH=i386
      RPM_ARCH=i686
      ;;
    *)
      echo "Unsupported architecture: $ARCH"
      exit 1
      ;;
  esac
}

function prepare_dirs() {
  for dir in "$APPIMAGE_DIR" "$BUILD_DIR" /deb /rpm
  do
    [[ -d "$dir" ]] && rm -rf "$dir" || true
  done
  mkdir -p "$APPIMAGE_DIR" "$BUILD_DIR" "$PROJECT_ROOT/dist" /deb/DEBIAN /rpm
}

function prepare_appimage_resource() {
  ln -sf usr/share/applications/RedPandaIDE.desktop "$APPIMAGE_DIR/RedPandaIDE.desktop"
  ln -sf usr/share/icons/hicolor/scalable/apps/redpandaide.svg "$APPIMAGE_DIR/redpandaide.svg"
  # following files may come from Windows filesystem, use `install` to preseve file permission
  install -m755 "$(dirname "$0")/AppRun.sh" "$APPIMAGE_DIR/AppRun"
  install -m644 "$PROJECT_ROOT/platform/linux/redpandaide.png" "$APPIMAGE_DIR/.DirIcon"
}

function prepare_deb_files() {
  sed "s/__VERSION__/$VERSION/; s/__ARCH__/$DEBIAN_ARCH/" "$(dirname "$0")/control.in" >/deb/DEBIAN/control
}

function prepare_rpm_files() {
  sed "s/__VERSION__/$VERSION/" "$(dirname "$0")/redpanda-cpp-static.spec.in" >/rpm/redpanda-cpp-static.spec
}

function build() {
  cd "$BUILD_DIR"
  qmake PREFIX=/usr QMAKE_LFLAGS=-static-pie "$PROJECT_ROOT/Red_Panda_CPP.pro"
  make -j$(nproc)
}

function package_appimage() {
  local appimage_file="$PROJECT_ROOT/dist/redpanda-cpp-static-$VERSION-$ARCH.AppImage"
  local runtime_size="$(wc -c <"$APPIMAGE_RUNTIME")"
  cd "$BUILD_DIR"
  make INSTALL_ROOT="$APPIMAGE_DIR" install

  # create AppImage
  mksquashfs "$APPIMAGE_DIR" "$appimage_file" -offset "$runtime_size" -comp zstd -root-owned -noappend -b 1M -mkfs-time 0
  dd if="$APPIMAGE_RUNTIME" of="$appimage_file" conv=notrunc
  chmod +x "$appimage_file"
}

function package_tarball() {
  local pkg_name="redpanda-cpp-static-$VERSION-$ARCH"
  cd "$BUILD_DIR"
  make INSTALL_ROOT="/$pkg_name" install
  tar -C / -c "$pkg_name" | gzip -9 >"$PROJECT_ROOT/dist/$pkg_name.tgz"
}

function package_deb() {
  cd "$BUILD_DIR"
  make INSTALL_ROOT=/deb install
  dpkg-deb -Z gzip -z 9 \
    --build /deb "$PROJECT_ROOT/dist/redpanda-cpp-static_${VERSION}_${DEBIAN_ARCH}.deb"
}

function package_rpm() {
  cd "$BUILD_DIR"
  make INSTALL_ROOT=/rpm/redpanda-cpp-static install
  cd /rpm
  rpmbuild -bb \
    --build-in-place \
    --target "$RPM_ARCH-unknown-linux" \
    --define "_binary_payload w9.gzdio" \
    redpanda-cpp-static.spec
  cp "$HOME/rpmbuild/RPMS/$RPM_ARCH/redpanda-cpp-static-$VERSION-1.$RPM_ARCH.rpm" "$PROJECT_ROOT/dist/"
}

install_packages
set_variables
prepare_dirs
prepare_appimage_resource
prepare_deb_files
prepare_rpm_files
build
package_appimage
package_tarball
package_deb
package_rpm
