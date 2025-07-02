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

set -e
if [[ "$GITHUB_WORKFLOW" ]]; then
    set -x
fi

OS_NAME=$(uname -s)
LLVM_VERSION=${GITHUB_LLVM_VERSION-16}
MESA_VERSION=${GITHUB_MESA_VERSION-24.2.1}
ORIG_DIR=$(pwd)
MESA_DIR=${MESA_DIR-${ORIG_DIR}/mesa}

if [[ "$OS_NAME" == "Linux" ]]; then
    sudo apt install python3-venv
fi

# Install python deps
python3 -m venv ${ORIG_DIR}/venv
source ${ORIG_DIR}/venv/bin/activate

NEEDED_PYTHON_DEPS=("mako" "setuptools" "pyyaml")
for cmd in "${NEEDED_PYTHON_DEPS[@]}"; do
    if ! python3 -m pip show -q "${cmd}" >/dev/null 2>&1; then
        python3 -m pip install ${cmd}
    fi
done
deactivate

LOCAL_PKG_CONFIG_PATH=

# Install system deps
if [[ "$OS_NAME" == "Linux" ]]; then
    if [[ "$GITHUB_WORKFLOW" ]]; then
        # We only want to do this if it is a CI machine.
        sudo apt-get -y remove llvm-*

        # We do a manual install of dependencies instead of `apt-get -y build-dep mesa`
        #   because this allows us to compile an older mesa, and also because build-deps
        #   is constantly being updated and sometimes not compatible with the current
        #   linux platform.
        # Note that we assume this platform is compatible with ubuntu-22.04 x86_64
        sudo apt-get -y install \
             autoconf automake autopoint autotools-dev bindgen bison build-essential bzip2 cpp cpp-11 debhelper debugedit dh-autoreconf dh-strip-nondeterminism diffstat directx-headers-dev dpkg-dev dwz flex g++ g++-11 gcc gcc-11 gcc-11-base:amd64 gettext glslang-tools icu-devtools intltool-debian lib32gcc-s1 lib32stdc++6 libarchive-zip-perl libasan6:amd64 libatomic1:amd64 libc-dev-bin libc6-dbg:amd64 libc6-dev:amd64 libc6-i386 libcc1-0:amd64 libclang-${GITHUB_CLANG_VERSION}-dev libclang-common-${GITHUB_CLANG_VERSION}-dev libclang-cpp${GITHUB_CLANG_VERSION} libclang-cpp${GITHUB_CLANG_VERSION}-dev libclang1-14 libclang1-${GITHUB_CLANG_VERSION} libclc-${GITHUB_CLANG_VERSION} libclc-${GITHUB_CLANG_VERSION}-dev libcrypt-dev:amd64 libdebhelper-perl libdpkg-perl libdrm-amdgpu1:amd64 libdrm-dev:amd64 libdrm-intel1:amd64 libdrm-nouveau2:amd64 libdrm-radeon1:amd64 libelf-dev:amd64 libexpat1-dev:amd64 libffi-dev:amd64 libfile-stripnondeterminism-perl libgc1:amd64 libgcc-11-dev:amd64 libgl1:amd64 libgl1-mesa-dri:amd64 libglapi-mesa:amd64 libglvnd-core-dev:amd64 libglvnd0:amd64 libglx-mesa0:amd64 libglx0:amd64 libgomp1:amd64 libicu-dev:amd64 libisl23:amd64 libitm1:amd64 libllvm14:amd64 libllvm${GITHUB_CLANG_VERSION}:amd64 libllvmspirvlib-${GITHUB_CLANG_VERSION}-dev:amd64 libllvmspirvlib${GITHUB_CLANG_VERSION}:amd64 liblsan0:amd64 libmpc3:amd64 libncurses-dev:amd64 libnsl-dev:amd64 libobjc-11-dev:amd64 libobjc4:amd64 libpciaccess-dev:amd64 libpciaccess0f:amd64 libpfm4:amd64 libpthread-stubs0-dev:amd64 libquadmath0:amd64 libsensors-config libsensors-dev:amd64 libsensors5:amd64 libset-scalar-perl libstd-rust-1.75:amd64 libstd-rust-dev:amd64 libstdc++-11-dev:amd64 libsub-override-perl libtinfo-dev:amd64 libtirpc-dev:amd64 libtool libtsan0:amd64 libubsan1:amd64 libva-dev:amd64 libva-drm2:amd64 libva-glx2:amd64 libva-wayland2:amd64 libva-x11-2:amd64 libva2:amd64 libvdpau-dev:amd64 libvdpau1:amd64 libvulkan-dev:amd64 libvulkan1:amd64 libwayland-bin libwayland-client0:amd64 libwayland-cursor0:amd64 libwayland-dev:amd64 libwayland-egl-backend-dev:amd64 libwayland-egl1:amd64 libwayland-server0:amd64 libx11-dev:amd64 libx11-xcb-dev:amd64 libx11-xcb1:amd64 libxau-dev:amd64 libxcb-dri2-0:amd64 libxcb-dri2-0-dev:amd64 libxcb-dri3-0:amd64 libxcb-dri3-dev:amd64 libxcb-glx0:amd64 libxcb-glx0-dev:amd64 libxcb-present-dev:amd64 libxcb-present0:amd64 libxcb-randr0:amd64 libxcb-randr0-dev:amd64 libxcb-render0:amd64 libxcb-render0-dev:amd64 libxcb-shape0:amd64 libxcb-shape0-dev:amd64 libxcb-shm0:amd64 libxcb-shm0-dev:amd64 libxcb-sync-dev:amd64 libxcb-sync1:amd64 libxcb-xfixes0:amd64 libxcb-xfixes0-dev:amd64 libxcb1-dev:amd64 libxdmcp-dev:amd64 libxext-dev:amd64 libxfixes-dev:amd64 libxfixes3:amd64 libxml2-dev:amd64 libxrandr-dev:amd64 libxrandr2:amd64 libxrender-dev:amd64 libxrender1:amd64 libxshmfence-dev:amd64 libxshmfence1:amd64 libxxf86vm-dev:amd64 libxxf86vm1:amd64 libz3-4:amd64 libz3-dev:amd64 libzstd-dev:amd64 linux-libc-dev:amd64 llvm-${LLVM_VERSION} llvm-${LLVM_VERSION}-dev llvm-${LLVM_VERSION}-linker-tools llvm-${LLVM_VERSION}-runtime llvm-${LLVM_VERSION}-tools llvm-spirv-${LLVM_VERSION} lto-disabled-list m4 make meson ninja-build pkg-config po-debconf python3-mako python3-ply python3-pygments quilt rpcsvc-proto rustc spirv-tools valgrind wayland-protocols x11proto-dev xorg-sgml-doctools xtrans-dev zlib1g-dev:amd64 \
             clang-$GITHUB_CLANG_VERSION libc++-$GITHUB_CLANG_VERSION-dev libc++abi-$GITHUB_CLANG_VERSION-dev

        sudo update-alternatives --install /usr/bin/cc cc /usr/bin/clang-${GITHUB_CLANG_VERSION} 100
        sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-${GITHUB_CLANG_VERSION} 100
    else
        set +e
        sudo apt-get -y build-dep mesa
        sudo apt -y remove llvm-18 llvm-18-* llvm-19 llvm-19-*
        set -e
        CURRENT_CLANG_VERSION=$(clang --version | head -n 1 | awk '{ print $4 }' | awk 'BEGIN { FS="\\." } { print $1 }')
        CLANG_VERSION=${CURRENT_CLANG_VERSION:-${LLVM_VERSION}}
        sudo apt-get -y install clang-${CLANG_VERSION} \
             libc++-${CLANG_VERSION}-dev \
             libc++abi-${CLANG_VERSION}-dev \
             llvm-${LLVM_VERSION} \
             llvm-${LLVM_VERSION}-{dev,tools,runtime}
        ! command -v clang > /dev/null 2>&1 && \
            sudo ln -s /usr/bin/clang-${CLANG_VERSION} /usr/bin/clang && \
            sudo ln -s /usr/bin/clang++-${CLANG_VERSION} /usr/bin/clang++
    fi # [[ "$GITHUB_WORKFLOW" ]]
elif [[ "$OS_NAME" == "Darwin" ]]; then
    if [[ ! "$GITHUB_WORKFLOW" ]]; then
        if ! command -v brew > /dev/null 2>&1; then
            echo "Error: need to install homebrew to continue"
            exit 1
        fi
    fi
    HOMEBREW_NO_INSTALLED_DEPENDENTS_CHECK=true brew install autoconf automake libx11 libxext libxrandr llvm@${LLVM_VERSION} ninja meson pkg-config libxshmfence

    # For reasons unknown, this is necessary for pkg-config to find homebrew's packages
    LOCAL_PKG_CONFIG_PATH="/opt/homebrew/lib/pkgconfig:$PKG_CONFIG_PATH"
fi # [[ "$OS_NAME" == x ]]

LOCAL_LDFLAGS=${LDFLAGS}
LOCAL_CPPFLAGS=${CPPFLAGS}
LOCAL_PATH=${PATH}
LOCAL_CXX=$(which clang++)
LOCAL_CC=$(which clang)

CHECKOUT_MESA=false
if [[ -d "${MESA_DIR}" ]]; then
    cd ${MESA_DIR}
    if ! git fsck --connectivity-only > /dev/null 2>&1; then
        echo "git fsck failed for mesa; try redownloading"
        CHECKOUT_MESA=true;
    fi
    cd ..;
else
    CHECKOUT_MESA=true
fi

if [[ "$CHECKOUT_MESA" = "true" ]]; then
    rm -rf ${MESA_DIR}

    git clone https://gitlab.freedesktop.org/mesa/mesa.git
    if [[ "${MESA_DIR}" != "${ORIG_DIR}/mesa" ]]; then
        mv mesa ${MESA_DIR}
    fi
fi

pushd .
cd ${MESA_DIR}

# Need >= 24 to have llvmpipe for swrast.  llvmpipe is needed for GL >= 4.1.
git checkout mesa-${MESA_VERSION}

mkdir -p out

source ${ORIG_DIR}/venv/bin/activate

if  [[ "$OS_NAME" == "Darwin" ]]; then
    LOCAL_LDFLAGS="-L/opt/homebrew/opt/llvm@${LLVM_VERSION}/lib"
    LOCAL_CPPFLAGS="-I/opt/homebrew/opt/llvm@${LLVM_VERSION}/include -I/opt/homebrew/include"
    LOCAL_PATH=${PATH}:/opt/homebrew/opt/llvm@${LLVM_VERSION}/bin

    # This is necessary to be able to build vk (lavapipe) on macOS.  Doesn't seem like a real dependency.
    sed -I '' "s/error('Vulkan drivers require dri3 for X11 support')//g" meson.build
fi

# -Dosmesa=true    => builds OSMesa, which is an offscreen GL context
# -Dgallium-drivers=swrast  => builds GL software rasterizer
# -Dvulkan-drivers=swrast   => builds VK software rasterizer
# -Dgallium-drivers=llvmpipe is needed for GL >= 4.1 pipe-screen (see src/gallium/auxiliary/target-helpers/inline_sw_helper.h)
PKG_CONFIG_PATH=${LOCAL_PKG_CONFIG_PATH} PATH=${LOCAL_PATH} \
CXX=${LOCAL_CXX} CC=${LOCAL_CC} LDFLAGS=${LOCAL_LDFLAGS} CPPFLAGS=${LOCAL_CPPFLAGS} \
   meson setup --wipe builddir/ -Dprefix="${MESA_DIR}/out" -Dglx=xlib -Dosmesa=true -Dgallium-drivers=llvmpipe,swrast -Dvulkan-drivers=swrast
PKG_CONFIG_PATH=${LOCAL_PKG_CONFIG_PATH} PATH=${LOCAL_PATH} \
CXX=${LOCAL_CXX} CC=${LOCAL_CC} LDFLAGS=${LOCAL_LDFLAGS} CPPFLAGS=${LOCAL_CPPFLAGS} \
   meson install -C builddir/

# Disable python venv
deactivate

popd

if [[ "$GITHUB_WORKFLOW" ]]; then
    set +x
fi
set +e
