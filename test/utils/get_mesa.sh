# Copyright (C) 2024 The Android Open Source Project
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

#!/usr/bin/bash

set -xe

CLANG_VERSION=`clang --version | head -n 1 | awk '{ print $4 }' | python3 -c "import sys; print(sys.stdin.read().split('.')[0])"`

sudo apt-get -y build-dep mesa

# Uninstall llvm-18 (and above) as they conflict with the mesa version we are compiling.
set +e
sudo apt -y remove llvm-18 llvm-18-*
set -e

sudo apt -y install clang-${CLANG_VERSION} \
     libc++-${CLANG_VERSION}-dev \
     libc++abi-${CLANG_VERSION}-dev \
     llvm-${CLANG_VERSION} \
     llvm-${CLANG_VERSION}-{dev,tools,runtime}

export CXX=`which clang++` && export CC=`which clang`

git clone https://gitlab.freedesktop.org/mesa/mesa.git

pushd .

cd mesa

git checkout mesa-23.2.1

mkdir -p out

# -Dosmesa=true    => builds OSMesa, which is an offscreen GL context
# -Dgallium-drivers=swrast  => builds GL software rasterizer
# -Dvulkan-drivers=swrast   => builds VK software rasterizer
meson setup builddir/ -Dprefix="$(pwd)/out" -Dosmesa=true -Dglx=xlib -Dgallium-drivers=swrast -Dvulkan-drivers=swrast
meson install -C builddir/

popd
