#!/bin/bash

set -euxo pipefail

REDPANDA_MINGW_VERSION="11.4.0-r0"
FILENAME_MINGW_VERSION="11.4"

if [[ ! -v MSYSTEM ]]; then
  echo "This script must be run from MSYS2 shell"
  exit 1
fi

case $MSYSTEM in
  MINGW32|CLANG32)
    _NSIS_ARCH=x86
    _BIT=32
    ;;
  MINGW64|UCRT64|CLANG64)
    _NSIS_ARCH=x64
    _BIT=64
    ;;
  *)
    echo "This script must be run from one of following MSYS2 shells:"
    echo "  - MINGW32/CLANG32"
    echo "  - MINGW64/UCRT64/CLANG64"
    exit 1
    ;;
esac

_CLEAN=0
_COMPILER=1
_SKIP_DEPS_CHECK=0
_7Z_REPACK=1
while [[ $# -gt 0 ]]; do
  case $1 in
    --clean)
      _CLEAN=1
      shift
      ;;
    --no-compiler)
      _COMPILER=0
      shift
      ;;
    --skip-deps-check)
      _SKIP_DEPS_CHECK=1
      shift
      ;;
    --no-7z)
      _7Z_REPACK=0
      shift
      ;;
    *)
      echo "Unknown argument: $1"
      exit 1
      ;;
  esac
done

_QMAKE="$MINGW_PREFIX/qt5-static/bin/qmake"
_NSIS="/mingw32/bin/makensis"

_MINGW_DIR="mingw$_BIT"
_MINGW_ARCHIVE="mingw$_BIT-$REDPANDA_MINGW_VERSION.7z"
_MINGW_URL="https://github.com/redpanda-cpp/toolchain-win32-mingw-xp/releases/download/$REDPANDA_MINGW_VERSION/$_MINGW_ARCHIVE"

_SRCDIR="$PWD"
_ASSETSDIR="$PWD/assets"
_BUILDDIR="$TEMP/redpanda-mingw-$MSYSTEM-build"
_PKGDIR="$TEMP/redpanda-mingw-$MSYSTEM-pkg"
_DISTDIR="$PWD/dist"

# _REDPANDA_VERSION=$(sed -nr -e '/APP_VERSION\s*=/ s/APP_VERSION\s*=\s*(([0-9]+\.)*[0-9]+)\s*/\1/p' "$_SRCDIR"/Red_Panda_CPP.pro)
# _REDPANDA_TESTVERSION=$(sed -nr -e '/TEST_VERSION\s*=/ s/TEST_VERSION\s*=\s*([A-Za-z0-9]*)\s*/\1/p' "$_SRCDIR"/Red_Panda_CPP.pro)
# if [[ -n $_REDPANDA_TESTVERSION ]]; then
#   _REDPANDA_VERSION="$_REDPANDA_VERSION.$_REDPANDA_TESTVERSION"
# fi

_REDPANDA_VERSION="2.9900"

if [[ $_COMPILER -eq 1 ]]; then
  _FINALNAME="redpanda-cpp-$_REDPANDA_VERSION-$_NSIS_ARCH-mingw$FILENAME_MINGW_VERSION"
else
  _FINALNAME="redpanda-cpp-$_REDPANDA_VERSION-$_NSIS_ARCH-none"
fi

function check-deps() {
  # MSYS2’s `pacman -Q` is 100x slower than Arch Linux. Allow skipping the check.
  [[ $_SKIP_DEPS_CHECK -eq 1 ]] && return
  case $MSYSTEM in
    MINGW32|MINGW64|UCRT64)
      local compiler=gcc
      ;;
    CLANG32|CLANG64|CLANGARM64)
      local compiler=clang
      ;;
  esac
  local deps=(
    $MINGW_PACKAGE_PREFIX-{$compiler,make,qt5-static}
    mingw-w64-i686-nsis
  )
  [[ _7Z_REPACK -eq 1 ]] && deps+=("$MINGW_PACKAGE_PREFIX-7zip")
  for dep in "${deps[@]}"; do
    pacman -Q "$dep" >/dev/null 2>&1 || (
      echo "Missing dependency: $dep"
      exit 1
    )
  done
}

function prepare-dirs() {
  if [[ $_CLEAN -eq 1 ]]; then
    [[ -d "$_BUILDDIR" ]] && rm -rf "$_BUILDDIR"
    [[ -d "$_PKGDIR" ]] && rm -rf "$_PKGDIR"
  fi
  mkdir -p "$_ASSETSDIR" "$_BUILDDIR" "$_PKGDIR" "$_DISTDIR"
}

function download-assets() {
  [[ -f "$_ASSETSDIR/$_MINGW_ARCHIVE" ]] || curl -L -o "$_ASSETSDIR/$_MINGW_ARCHIVE" "$_MINGW_URL"
}

function build() {
  pushd "$_BUILDDIR"
  "$_QMAKE" PREFIX="$_PKGDIR" "$_SRCDIR"
  time mingw32-make -j$(nproc)
  mingw32-make install

  cp "$_SRCDIR"/packages/msys/{main.nsi,lang.nsh} "$_PKGDIR"
  if [[ $_COMPILER -eq 1 ]]; then
    [[ -d "$_PKGDIR/$_MINGW_DIR" ]] || bsdtar -C "$_PKGDIR" -xf "$_ASSETSDIR/$_MINGW_ARCHIVE"
  fi
  popd
}

function package() {
  pushd "$_PKGDIR"
  local components=()
  [[ $_COMPILER -eq 1 ]] && components+=("-DHAVE_MINGW$_BIT")
  "$_NSIS" \
    -DVERSION="$_REDPANDA_VERSION" \
    -DARCH="$_NSIS_ARCH" \
    -DDISPLAY_ARCH="$_NSIS_ARCH" \
    -DREQUIRED_WINDOWS_BUILD="7600" \
    -DREQUIRED_WINDOWS_NAME="Windows 7" \
    -DFINALNAME="$_FINALNAME.exe" \
    "${components[@]}" \
    main.nsi
  if [[ $_7Z_REPACK -eq 1 ]]; then
    7z x "$_FINALNAME.exe" -o"RedPanda-CPP" -xr'!$PLUGINSDIR' -x"!uninstall.exe"
    7z a -t7z -mx=9 -ms=on -mqs=on -mf=BCJ2 -m0="LZMA2:d=128m:fb=273:c=2g" "$_FINALNAME.7z" "RedPanda-CPP"
    rm -rf "RedPanda-CPP"
  fi
  popd
}

function dist() {
  cp "$_PKGDIR/$_FINALNAME.exe" "$_DISTDIR"
  [[ $_7Z_REPACK -eq 1 ]] && cp "$_PKGDIR/$_FINALNAME.7z" "$_DISTDIR" || true
}

check-deps
prepare-dirs
download-assets
build
package
dist
