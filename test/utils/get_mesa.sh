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

# GITHUB_CLANG_VERSION is set in build/linux/ci-common.sh

if [[ "$GITHUB_WORKFLOW" ]]; then
    # We only want to do this if it is a CI machine.
    sudo apt-get -y remove llvm-*

    # We do a manual install of dependencies instead of `apt-get -y build-dep mesa`
    #   because this allows us to compile an older mesa, and also because build-deps
    #   is constantly being updated and sometimes not compatible with the current
    #   linux platform.
    # Note that we assume this platform is compatible with ubuntu-22.04 x86_64
    sudo apt-get -y install \
         autoconf automake autopoint autotools-dev bindgen bison build-essential bzip2 cpp cpp-11 debhelper debugedit dh-autoreconf dh-strip-nondeterminism diffstat directx-headers-dev dpkg-dev dwz flex g++ g++-11 gcc gcc-11 gcc-11-base:amd64 gettext glslang-tools icu-devtools intltool-debian lib32gcc-s1 lib32stdc++6 libarchive-zip-perl libasan6:amd64 libatomic1:amd64 libc-dev-bin libc6-dbg:amd64 libc6-dev:amd64 libc6-i386 libcc1-0:amd64 libclang-${GITHUB_CLANG_VERSION}-dev libclang-common-${GITHUB_CLANG_VERSION}-dev libclang-cpp${GITHUB_CLANG_VERSION} libclang-cpp${GITHUB_CLANG_VERSION}-dev libclang1-14 libclang1-${GITHUB_CLANG_VERSION} libclc-${GITHUB_CLANG_VERSION} libclc-${GITHUB_CLANG_VERSION}-dev libcrypt-dev:amd64 libdebhelper-perl libdpkg-perl libdrm-amdgpu1:amd64 libdrm-dev:amd64 libdrm-intel1:amd64 libdrm-nouveau2:amd64 libdrm-radeon1:amd64 libelf-dev:amd64 libexpat1-dev:amd64 libffi-dev:amd64 libfile-stripnondeterminism-perl libgc1:amd64 libgcc-11-dev:amd64 libgl1:amd64 libgl1-mesa-dri:amd64 libglapi-mesa:amd64 libglvnd-core-dev:amd64 libglvnd0:amd64 libglx-mesa0:amd64 libglx0:amd64 libgomp1:amd64 libicu-dev:amd64 libisl23:amd64 libitm1:amd64 libllvm14:amd64 libllvm${GITHUB_CLANG_VERSION}:amd64 libllvmspirvlib-${GITHUB_CLANG_VERSION}-dev:amd64 libllvmspirvlib${GITHUB_CLANG_VERSION}:amd64 liblsan0:amd64 libmpc3:amd64 libncurses-dev:amd64 libnsl-dev:amd64 libobjc-11-dev:amd64 libobjc4:amd64 libpciaccess-dev:amd64 libpciaccess0:amd64 libpfm4:amd64 libpthread-stubs0-dev:amd64 libquadmath0:amd64 libsensors-config libsensors-dev:amd64 libsensors5:amd64 libset-scalar-perl libstd-rust-1.75:amd64 libstd-rust-dev:amd64 libstdc++-11-dev:amd64 libsub-override-perl libtinfo-dev:amd64 libtirpc-dev:amd64 libtool libtsan0:amd64 libubsan1:amd64 libva-dev:amd64 libva-drm2:amd64 libva-glx2:amd64 libva-wayland2:amd64 libva-x11-2:amd64 libva2:amd64 libvdpau-dev:amd64 libvdpau1:amd64 libvulkan-dev:amd64 libvulkan1:amd64 libwayland-bin libwayland-client0:amd64 libwayland-cursor0:amd64 libwayland-dev:amd64 libwayland-egl-backend-dev:amd64 libwayland-egl1:amd64 libwayland-server0:amd64 libx11-dev:amd64 libx11-xcb-dev:amd64 libx11-xcb1:amd64 libxau-dev:amd64 libxcb-dri2-0:amd64 libxcb-dri2-0-dev:amd64 libxcb-dri3-0:amd64 libxcb-dri3-dev:amd64 libxcb-glx0:amd64 libxcb-glx0-dev:amd64 libxcb-present-dev:amd64 libxcb-present0:amd64 libxcb-randr0:amd64 libxcb-randr0-dev:amd64 libxcb-render0:amd64 libxcb-render0-dev:amd64 libxcb-shape0:amd64 libxcb-shape0-dev:amd64 libxcb-shm0:amd64 libxcb-shm0-dev:amd64 libxcb-sync-dev:amd64 libxcb-sync1:amd64 libxcb-xfixes0:amd64 libxcb-xfixes0-dev:amd64 libxcb1-dev:amd64 libxdmcp-dev:amd64 libxext-dev:amd64 libxfixes-dev:amd64 libxfixes3:amd64 libxml2-dev:amd64 libxrandr-dev:amd64 libxrandr2:amd64 libxrender-dev:amd64 libxrender1:amd64 libxshmfence-dev:amd64 libxshmfence1:amd64 libxxf86vm-dev:amd64 libxxf86vm1:amd64 libz3-4:amd64 libz3-dev:amd64 libzstd-dev:amd64 linux-libc-dev:amd64 llvm-${GITHUB_CLANG_VERSION} llvm-${GITHUB_CLANG_VERSION}-dev llvm-${GITHUB_CLANG_VERSION}-linker-tools llvm-${GITHUB_CLANG_VERSION}-runtime llvm-${GITHUB_CLANG_VERSION}-tools llvm-spirv-${GITHUB_CLANG_VERSION} lto-disabled-list m4 make meson ninja-build pkg-config po-debconf python3-mako python3-ply python3-pygments quilt rpcsvc-proto rustc spirv-tools valgrind wayland-protocols x11proto-dev xorg-sgml-doctools xtrans-dev zlib1g-dev:amd64 \
         clang-$GITHUB_CLANG_VERSION libc++-$GITHUB_CLANG_VERSION-dev libc++abi-$GITHUB_CLANG_VERSION-dev

    sudo update-alternatives --install /usr/bin/cc cc /usr/bin/clang-${GITHUB_CLANG_VERSION} 100
    sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-${GITHUB_CLANG_VERSION} 100
else
    set +e
    sudo apt-get -y build-dep mesa
    sudo apt -y remove llvm-18 llvm-18-* llvm-19 llvm-19-*
    set -e

    CURRENT_CLANG_VERSION=$(clang --version | head -n 1 | awk '{ print $4 }' | awk 'BEGIN { FS="\\." } { print $1 }')
    GITHUB_CLANG_VERSION=${GITHUB_CLANG_VERSION:-${CURRENT_CLANG_VERSION}}

    sudo apt-get -y install clang-${GITHUB_CLANG_VERSION} \
         libc++-${GITHUB_CLANG_VERSION}-dev \
         libc++abi-${GITHUB_CLANG_VERSION}-dev \
         llvm-${GITHUB_CLANG_VERSION} \
         llvm-${GITHUB_CLANG_VERSION}-{dev,tools,runtime}
fi


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
