#!/usr/bin/bash

# Copyright (C) 2025 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

DARWIN_X86=1
LINUX_X86=2
DARWIN_ARM=3
LINUX_ARM=4

function add_cargo_path() {
    local BASH_P="${HOME}/.bashrc"
    local PATH_LINE='export PATH=${PATH}:${HOME}/.cargo/bin'
    local TYPE=$1
    if [ ${TYPE} == ${DARWIN_ARM} ] || [ ${TYPE} == ${DARWIN_X86} ]; then
        BASH_P="${HOME}/.bash_profile"
    fi
    if ! (grep "${PATH_LINE}" ${BASH_P}); then
        echo "${PATH_LINE}" >> ${BASH_P}
    fi
    source ${BASH_P}
}

function download_mdbook() {
    if command -v mdbook >/dev/null 2>&1; then
        echo "mdbook already installed"
        exit 0
    fi

    local CHECK_UNAME="
import sys
parts=[a.lower() for a in sys.stdin.read().strip().split(' ')]
def get_type():
  if 'darwin' in parts:
    if 'x86_64' in parts:
      return ${DARWIN_X86}
    elif 'aarch' in parts or 'arm64' in parts:
      return ${DARWIN_ARM}
  elif 'linux' in parts:
    if 'x86_64' in parts:
      return ${LINUX_X86}
    elif 'aarch' in parts:
      return ${LINUX_ARM}
  return 0
print(get_type())
"
    local TYPE=`uname -a | python3 -c "${CHECK_UNAME}"`
    if [ ${TYPE} == ${DARWIN_ARM} ] || [ ${TYPE} == ${LINUX_ARM} ]; then
        # No github prebuilts are available, we build it with rust from source.
        # First, need to install rust and cargo
        if ! (command -v rustc >/dev/null 2>&1) || ! (command -v cargo >/dev/null 2>&1); then
            echo "*** Need to install Rust ***"
            curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
        fi
        source "${HOME}/.cargo/env"
        if ! (command -v cargo >/dev/null 2>&1); then
            echo "*** Still cannot find cargo ***"
            exit 1
        fi
        cargo install --force mdbook
    else
        # Download prebuilts from github
        mkdir -p ${HOME}/.cargo/bin
        echo "*** Downloading mdbook from github release ***"
        DL_URL='https://github.com/rust-lang/mdBook/releases/download/v0.4.43/mdbook-v0.4.43-x86_64-apple-darwin.tar.gz '
        if [ ${TYPE} == ${LINUX_X86} ]; then
            DL_URL='https://github.com/rust-lang/mdBook/releases/download/v0.4.43/mdbook-v0.4.43-x86_64-unknown-linux-gnu.tar.gz'
        fi
        curl -L -o /tmp/mdbook.tar.gz ${DL_URL}
        mkdir -p /tmp/mdbook
        tar -xvzf /tmp/mdbook.tar.gz -C /tmp/mdbook >/dev/null 2>&1
        mv /tmp/mdbook/mdbook ${HOME}/.cargo/bin/
    fi
    add_cargo_path ${TYPE}
}

download_mdbook
