#!/bin/bash

set -euxo pipefail

REDPANDA_MINGW_VERSION="11.4.0-r0"
REDPANDA_LLVM_VERSION="17-r0"
FILENAME_LLVM_VERSION="17"
WINDOWS_TERMINAL_VERSION="1.18.3181.0"

if [[ ! -v MSYSTEM ]]; then
  echo "This script must be run from MSYS2 shell"
  exit 1
fi

case $MSYSTEM in
  MINGW32|CLANG32)
    _NATIVE_ARCH=i686
    _NSIS_ARCH=x86
    ;;
  MINGW64|UCRT64|CLANG64)
    _NATIVE_ARCH=x86_64
    _NSIS_ARCH=x64
    ;;
  CLANGARM64)
    _NATIVE_ARCH=aarch64
    _NSIS_ARCH=arm64
    ;;
  *)
    echo "This script must be run from one of following MSYS2 shells:"
    echo "  - MINGW32/CLANG32"
    echo "  - MINGW64/UCRT64/CLANG64"
    echo "  - CLANGARM64"
    exit 1
    ;;
esac

_CLEAN=0
_SKIP_DEPS_CHECK=0
_UNIFIED=0
_7Z_REPACK=0
while [[ $# -gt 0 ]]; do
  case $1 in
    --clean)
      _CLEAN=1
      shift
      ;;
    --skip-deps-check)
      _SKIP_DEPS_CHECK=1
      shift
      ;;
    --unified)
      _UNIFIED=1
      shift
      ;;
    --7z)
      _7Z_REPACK=1
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

function mingw_dir() {
  local bit="$1"
  echo "mingw$bit"
}
function mingw_archive() {
  local bit="$1"
  echo "mingw$bit-$REDPANDA_MINGW_VERSION.7z"
}
function mingw_url() {
  local bit="$1"
  echo "https://github.com/redpanda-cpp/toolchain-win32-mingw-xp/releases/download/$REDPANDA_MINGW_VERSION/$(mingw_archive $bit)"
}

_LLVM_DIR="llvm-mingw"
_LLVM_ARCHES=("x86_64" "i686" "aarch64")
_LLVM_ARCHIVE="$_LLVM_DIR-$REDPANDA_LLVM_VERSION-$_NATIVE_ARCH.7z"
_LLVM_URL="https://github.com/redpanda-cpp/toolchain-win32-llvm/releases/download/$REDPANDA_LLVM_VERSION/$_LLVM_ARCHIVE"

_WINDOWS_TERMINAL_DIR="terminal-${WINDOWS_TERMINAL_VERSION}"
_WINDOWS_TERMINAL_ARCHIVE="Microsoft.WindowsTerminal_${WINDOWS_TERMINAL_VERSION}_$_NSIS_ARCH.zip"
_WINDOWS_TERMINAL_URL="https://github.com/microsoft/terminal/releases/download/v${WINDOWS_TERMINAL_VERSION}/${_WINDOWS_TERMINAL_ARCHIVE}"

_SRCDIR="$PWD"
_ASSETSDIR="$PWD/assets"
_BUILDDIR="$TEMP/redpanda-clang-$MSYSTEM-build"
_PKGDIR="$TEMP/redpanda-clang-$MSYSTEM-pkg"
_DISTDIR="$PWD/dist"

# _REDPANDA_VERSION=$(sed -nr -e '/APP_VERSION\s*=/ s/APP_VERSION\s*=\s*(([0-9]+\.)*[0-9]+)\s*/\1/p' "$_SRCDIR"/Red_Panda_CPP.pro)
# _REDPANDA_TESTVERSION=$(sed -nr -e '/TEST_VERSION\s*=/ s/TEST_VERSION\s*=\s*([A-Za-z0-9]*)\s*/\1/p' "$_SRCDIR"/Red_Panda_CPP.pro)
# if [[ -n $_REDPANDA_TESTVERSION ]]; then
#   _REDPANDA_VERSION="$_REDPANDA_VERSION.$_REDPANDA_TESTVERSION"
# fi

_REDPANDA_VERSION="2.9900"

if [[ $_UNIFIED -eq 1 ]]; then
  _FINALNAME="redpanda-cpp-$_REDPANDA_VERSION-$_NSIS_ARCH-unified"
else
  _FINALNAME="redpanda-cpp-$_REDPANDA_VERSION-$_NSIS_ARCH-llvm$FILENAME_LLVM_VERSION"
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
  if [[ $_UNIFIED -eq 1 ]]; then
    [[ -f "$_ASSETSDIR/$(mingw_archive 32)" ]] || curl -L -o "$_ASSETSDIR/$(mingw_archive 32)" "$(mingw_url 32)"
    [[ -f "$_ASSETSDIR/$(mingw_archive 64)" ]] || curl -L -o "$_ASSETSDIR/$(mingw_archive 64)" "$(mingw_url 64)"
  fi
  [[ -f "$_ASSETSDIR/$_LLVM_ARCHIVE" ]] || curl -L -o "$_ASSETSDIR/$_LLVM_ARCHIVE" "$_LLVM_URL"
  [[ -f "$_ASSETSDIR/$_WINDOWS_TERMINAL_ARCHIVE" ]] || curl -L -o "$_ASSETSDIR/$_WINDOWS_TERMINAL_ARCHIVE" "$_WINDOWS_TERMINAL_URL"
}

function prepare-openconsole() {
  local windows_terminal_dir="$_BUILDDIR/$_WINDOWS_TERMINAL_DIR"
  if [[ ! -d "$windows_terminal_dir" ]]; then
    bsdtar -C "$_BUILDDIR" -xf "$_ASSETSDIR/$_WINDOWS_TERMINAL_ARCHIVE"
  fi
}

function prepare-src() {
  cp "$_SRCDIR"/RedPandaIDE/RedPandaIDE.pro{,.bak}
  sed -i '/CONFIG += ENABLE_LUA_ADDON/ { s/^#\s*// }' "$_SRCDIR"/RedPandaIDE/RedPandaIDE.pro
}

function restore-src() {
  mv "$_SRCDIR"/RedPandaIDE/RedPandaIDE.pro{.bak,}
}

function build() {
  pushd "$_BUILDDIR"
  "$_QMAKE" PREFIX="$_PKGDIR" "$_SRCDIR"
  time mingw32-make WINDOWS_PREFER_OPENCONSOLE=ON -j$(nproc)
  mingw32-make install

  cp "$_SRCDIR"/packages/msys/{main.nsi,lang.nsh,compiler_hint.lua} "$_PKGDIR"
  cp "$_WINDOWS_TERMINAL_DIR/OpenConsole.exe" "$_PKGDIR"
  if [[ $_UNIFIED -eq 1 ]]; then
    [[ -d "$_PKGDIR/mingw32" ]] || bsdtar -C "$_PKGDIR" -xf "$_ASSETSDIR/$(mingw_archive 32)"
    if [[ $_NATIVE_ARCH != "i686" ]]; then
      [[ -d "$_PKGDIR/mingw64" ]] || bsdtar -C "$_PKGDIR" -xf "$_ASSETSDIR/$(mingw_archive 64)"
    fi
  fi
  [[ -d "$_PKGDIR/llvm-mingw" ]] || bsdtar -C "$_PKGDIR" -xf "$_ASSETSDIR/$_LLVM_ARCHIVE"
  popd
}

function package() {
  pushd "$_PKGDIR"
  local components=("-DHAVE_COMPILER_HINT" "-DHAVE_OPENCONSOLE" "-DHAVE_LLVM")
  if [[ $_UNIFIED -eq 1 ]]; then
    components+=("-DHAVE_MINGW32")
    [[ $_NATIVE_ARCH != "i686" ]] && components+=("-DHAVE_MINGW64")
  fi
  "$_NSIS" \
    -DVERSION="$_REDPANDA_VERSION" \
    -DARCH="$_NSIS_ARCH" \
    -DDISPLAY_ARCH="$_NSIS_ARCH" \
    -DREQUIRED_WINDOWS_BUILD="17763" \
    -DREQUIRED_WINDOWS_NAME="Windows 10 v1809" \
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
prepare-openconsole
prepare-src
trap restore-src EXIT INT TERM
build
package
dist
