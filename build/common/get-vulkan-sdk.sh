#!/bin/bash

# This script is from https://github.com/humbletim/install-vulkan-sdk
# with modifications.  The license from humbletim/install-vulkan-sdk have
# been included below.

# MIT License
#
# Copyright (c) 2022 humbletim
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# helper functions for downloading/installing platform-specific Vulkan SDKs
# originally meant for use from GitHub Actions
#   see: https://github.com/humbletim/install-vulkan-sdk
# -- humbletim 2022.02

VULKAN_SDK_VERSION=${GITHUB_VULKANSDK_VERSION-1.4.321.0}
VULKAN_SDK_DIR=$(pwd)/vulkansdk

function _get_os() {
  runner_os=${RUNNER_OS:-`uname -s`}
  case $runner_os in
    macOS|Darwin) echo "mac" ;;
    Linux) echo "linux" ;;
    *) echo "unknown runner_os: $runner_os" ; ;;
  esac
}

function _os_filename() {
  local os=$(_get_os)
  case $os in
    mac) echo vulkansdk-macos-${VULKAN_SDK_VERSION}.zip ;;
    linux) echo vulkan_sdk.tar.gz ;;
    *) echo "unknown $os" >&2 ; exit 9 ;;
  esac
}

function _preferred_os_filename() {
  local os=$(_get_os)
  case $os in
    mac) echo vulkan_sdk.zip ;;
    linux) echo vulkan_sdk.tar.gz ;;
    *) echo "unknown $os" >&2 ; exit 9 ;;
  esac
}

function download_vulkan_installer() {
  local os=$(_get_os)
  local dl_filename=$(_os_filename ${VULKAN_SDK_VERSION})
  local filename=$(_preferred_os_filename)
  local url=https://sdk.lunarg.com/sdk/download/$VULKAN_SDK_VERSION/$os/$dl_filename?Human=true
  echo "_download_os_installer $os $filename $url" >&2
  if [[ -f $filename ]] ; then
    echo "using cached: $filename" >&2
  else
    curl --fail-with-body -s -L -o ${filename}.tmp $url || { echo "curl failed with error code: $?" >&2 ; curl -s -L --head $url >&2 ; }
    test -f ${filename}.tmp
    mv -v ${filename}.tmp ${filename}
  fi
  ls -lh $filename >&2
}

function unpack_vulkan_installer() {
  local os=$(_get_os)
  local filename=$(_preferred_os_filename $os)
  test -f $filename
  install_${os}
}

function install_linux() {
  test -d $VULKAN_SDK_DIR && test -f vulkan_sdk.tar.gz
  echo "extract just the SDK's prebuilt binaries ($VULKAN_SDK_VERSION/x86_64) from vulkan_sdk.tar.gz into $VULKAN_SDK" >&2
  tar -C "$VULKAN_SDK_DIR" --strip-components 2 -xf vulkan_sdk.tar.gz $VULKAN_SDK_VERSION/x86_64
}

function install_mac() {
  mkdir -p ${VULKAN_SDK_DIR}
  test -d $VULKAN_SDK_DIR && test -f vulkan_sdk.zip
  unzip vulkan_sdk.zip
  local InstallVulkan
  if [[ -d InstallVulkan-${VULKAN_SDK_VERSION}.app/Contents ]] ; then
    InstallVulkan=InstallVulkan-${VULKAN_SDK_VERSION}
  elif [[ -d vulkansdk-macOS-${VULKAN_SDK_VERSION}.app/Contents ]] ; then
    InstallVulkan=vulkansdk-macOS-${VULKAN_SDK_VERSION}
  elif [[ -d InstallVulkan.app/Contents ]] ; then
    InstallVulkan=InstallVulkan
  else
    echo "expecting ..vulkan.app/Contents folder (perhaps lunarg changed the archive layout again?): vulkan_sdk.zip" >&2
    echo "file vulkan_sdk.zip" >&2
    file vulkan_sdk.zip
    echo "unzip -t vulkan_sdk.zip" >&2
    unzip -t vulkan_sdk.zip
  fi
  echo "recognized zip layout 'vulkan_sdk.zip' ${InstallVulkan}.app/Contents" >&2
  local sdk_temp=${VULKAN_SDK_DIR}.tmp
  sudo ${InstallVulkan}.app/Contents/MacOS/${InstallVulkan} --root "$sdk_temp" --accept-licenses --default-answer --confirm-command install
  du -hs $sdk_temp
  test -d $sdk_temp/macOS || { echo "unrecognized dmg folder layout: $sdk_temp" ; ls -l $sdk_temp ; }
  cp -r $sdk_temp/macOS/* $VULKAN_SDK_DIR/
  if [[ -d ${InstallVulkan}.app/Contents ]] ; then
    sudo rm -rf "$sdk_temp"
    rm -rf ${InstallVulkan}.app
  fi
}
