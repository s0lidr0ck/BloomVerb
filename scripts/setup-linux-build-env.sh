#!/usr/bin/env bash
set -euo pipefail

if ! command -v apt-get >/dev/null 2>&1; then
  echo "This setup script currently supports apt-based Linux distributions."
  exit 1
fi

if [[ "${EUID:-$(id -u)}" -eq 0 ]]; then
  APT_PREFIX=""
elif command -v sudo >/dev/null 2>&1; then
  APT_PREFIX="sudo"
else
  echo "Need root privileges or sudo to install build dependencies."
  exit 1
fi

PACKAGES=(
  build-essential
  gcc
  g++
  cmake
  pkg-config
  libasound2-dev
  libfreetype-dev
  libfontconfig1-dev
  libgl1-mesa-dev
  libx11-dev
  libxext-dev
  libxinerama-dev
  libxcursor-dev
  libxrandr-dev
  libxrender-dev
  libxcomposite-dev
  libxdamage-dev
  libxfixes-dev
  libx11-xcb-dev
  libxkbcommon-dev
)

echo "Installing BloomVerb Linux build dependencies..."
$APT_PREFIX apt-get update
$APT_PREFIX apt-get install -y "${PACKAGES[@]}"

if command -v g++ >/dev/null 2>&1; then
  GXX_MAJOR="$(g++ -dumpfullversion -dumpversion | cut -d. -f1)"
  LIBSTDCPP_DEV_PACKAGE="libstdc++-${GXX_MAJOR}-dev"
  if apt-cache show "${LIBSTDCPP_DEV_PACKAGE}" >/dev/null 2>&1; then
    echo "Installing ${LIBSTDCPP_DEV_PACKAGE} for reliable C++ linking..."
    $APT_PREFIX apt-get install -y "${LIBSTDCPP_DEV_PACKAGE}"
  fi
fi

echo
echo "Environment setup complete."
echo "Build command:"
echo "cmake --preset linux-release && cmake --build --preset linux-release-build"
