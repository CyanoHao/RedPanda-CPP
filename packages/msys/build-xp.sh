#!/bin/bash

set -euxo pipefail

REDPANDA_MINGW_VERSION="11.4.0-r0"
FILENAME_MINGW_VERSION="11.4"

_BIT=""
_CLEAN=0
_COMPILER=1
_HOST_MINGW=""
_NSIS="/c/Program Files (x86)/NSIS/makensis.exe"
_QT_INSTALL="/c/Qt"
_REDPANDA_QT=""
_7Z_REPACK=1
while [[ $# -gt 0 ]]; do
  case $1 in
    --arch)
      _BIT="$2"
      shift 2
      ;;
    --clean)
      _CLEAN=1
      shift
      ;;
    --no-compiler)
      _COMPILER=0
      shift
      ;;
    --host-mingw)
      _HOST_MINGW="$2"
      shift 2
      ;;
    --nsis)
      _NSIS="$2"
      shift 2
      ;;
    --qt-install)
      _QT_INSTALL="$2"
      shift 2
      ;;
    --redpanda-qt)
      _REDPANDA_QT="$2"
      shift 2
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

case "$_BIT" in
  32)
    _NSIS_ARCH="x86"
    _DISPLAY_ARCH="x86_winxp"
    _REQUIRED_WINDOWS_BUILD="2600"
    _REQUIRED_WINDOWS_NAME="Windows XP"
    ;;
  64)
    _NSIS_ARCH="x64"
    _DISPLAY_ARCH="x64_ws2003"
    _REQUIRED_WINDOWS_BUILD="3790"
    _REQUIRED_WINDOWS_NAME="Windows Server 2003"
    ;;
  *)
    echo "Please specify --arch 32 or --arch 64"
    exit 1
    ;;
esac

[[ -z $_HOST_MINGW ]] && _HOST_MINGW="$_QT_INSTALL/Tools/mingw810_$_BIT"
[[ -z $_REDPANDA_QT ]] && _REDPANDA_QT="$_QT_INSTALL/5.6.4/mingw81_$_BIT-redpanda"
_QMAKE="$_REDPANDA_QT/bin/qmake.exe"
_HOST_GXX="$_HOST_MINGW/bin/g++.exe"

_MINGW_DIR="mingw$_BIT"
_MINGW_ARCHIVE="mingw$_BIT-$REDPANDA_MINGW_VERSION.7z"
_MINGW_URL="https://github.com/redpanda-cpp/toolchain-win32-mingw-xp/releases/download/$REDPANDA_MINGW_VERSION/$_MINGW_ARCHIVE"

_SRCDIR="$PWD"
_ASSETSDIR="$PWD/assets"
_BUILDDIR="$TEMP/redpanda-xp-$_BIT-build"
_PKGDIR="$TEMP/redpanda-xp-$_BIT-pkg"
_DISTDIR="$PWD/dist"

# _REDPANDA_VERSION=$(sed -nr -e '/APP_VERSION\s*=/ s/APP_VERSION\s*=\s*(([0-9]+\.)*[0-9]+)\s*/\1/p' "$_SRCDIR"/Red_Panda_CPP.pro)
# _REDPANDA_TESTVERSION=$(sed -nr -e '/TEST_VERSION\s*=/ s/TEST_VERSION\s*=\s*([A-Za-z0-9]*)\s*/\1/p' "$_SRCDIR"/Red_Panda_CPP.pro)
# if [[ -n $_REDPANDA_TESTVERSION ]]; then
#   _REDPANDA_VERSION="$_REDPANDA_VERSION.$_REDPANDA_TESTVERSION"
# fi

_REDPANDA_VERSION="2.9900"

if [[ $_COMPILER -eq 1 ]]; then
  _FINALNAME="redpanda-cpp-$_REDPANDA_VERSION-$_DISPLAY_ARCH-mingw$FILENAME_MINGW_VERSION"
else
  _FINALNAME="redpanda-cpp-$_REDPANDA_VERSION-$_DISPLAY_ARCH-none"
fi

function check-toolchain() {
  if [[ ! -x $_QMAKE ]]; then
    echo "Qt not found: $_REDPANDA_QT."
    echo "Please download from https://github.com/redpanda-cpp/qtbase-5.6 and extract to /c/Qt,"
    echo "or specify --redpanda-qt </path/to/qt/prefix>"
    exit 1
  fi
  if [[ ! -x $_HOST_GXX ]]; then
    echo "MinGW not found: $_HOST_MINGW."
    echo "Please install mingw-w64 to default location or specify --host-mingw </path/to/mingw>"
    exit 1
  fi
  if [[ ! -x $_NSIS ]]; then 
    echo "NSIS not found: $_NSIS."
    echo "Please install NSIS to default location or specify --nsis </path/to/makensis.exe>"
    exit 1
  fi

  export PATH="$_HOST_MINGW/bin:$PATH"
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
    [[ -d "$_PKGDIR/$_MINGW_DIR" ]] || 7z x "$_ASSETSDIR/$_MINGW_ARCHIVE" -o"$_PKGDIR"
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
    -DDISPLAY_ARCH="$_DISPLAY_ARCH" \
    -DREQUIRED_WINDOWS_BUILD="$_REQUIRED_WINDOWS_BUILD" \
    -DREQUIRED_WINDOWS_NAME="$_REQUIRED_WINDOWS_NAME" \
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

check-toolchain
prepare-dirs
download-assets
build
package
dist
