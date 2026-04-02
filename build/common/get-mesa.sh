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
        DPKG_ARCH=$(dpkg --print-architecture)
        EXTRA_PACKAGES=""
        if [[ "$DPKG_ARCH" == "amd64" ]]; then
            EXTRA_PACKAGES="lib32gcc-s1 lib32stdc++6 libc6-i386"
        fi
        sudo apt-get -y install \
             autoconf automake autopoint autotools-dev bindgen bison build-essential bzip2 cpp cpp-11 debhelper debugedit dh-autoreconf dh-strip-nondeterminism diffstat directx-headers-dev dpkg-dev dwz flex g++ g++-11 gcc gcc-11 gcc-11-base:${DPKG_ARCH} gettext glslang-tools icu-devtools intltool-debian ${EXTRA_PACKAGES} libarchive-zip-perl libasan6:${DPKG_ARCH} libatomic1:${DPKG_ARCH} libc-dev-bin libc6-dbg:${DPKG_ARCH} libc6-dev:${DPKG_ARCH} libcc1-0:${DPKG_ARCH} libclang-${GITHUB_CLANG_VERSION}-dev libclang-common-${GITHUB_CLANG_VERSION}-dev libclang-cpp${GITHUB_CLANG_VERSION} libclang-cpp${GITHUB_CLANG_VERSION}-dev libclang1-14 libclang1-${GITHUB_CLANG_VERSION} libclc-${GITHUB_CLANG_VERSION} libclc-${GITHUB_CLANG_VERSION}-dev libcrypt-dev:${DPKG_ARCH} libdebhelper-perl libdpkg-perl libdrm-amdgpu1:${DPKG_ARCH} libdrm-dev:${DPKG_ARCH} libdrm-intel1:${DPKG_ARCH} libdrm-nouveau2:${DPKG_ARCH} libdrm-radeon1:${DPKG_ARCH} libelf-dev:${DPKG_ARCH} libexpat1-dev:${DPKG_ARCH} libffi-dev:${DPKG_ARCH} libfile-stripnondeterminism-perl libgc1:${DPKG_ARCH} libgcc-11-dev:${DPKG_ARCH} libgl1:${DPKG_ARCH} libgl1-mesa-dri:${DPKG_ARCH} libglapi-mesa:${DPKG_ARCH} libglvnd-core-dev:${DPKG_ARCH} libglvnd0:${DPKG_ARCH} libglx-mesa0:${DPKG_ARCH} libglx0:${DPKG_ARCH} libgomp1:${DPKG_ARCH} libicu-dev:${DPKG_ARCH} libisl23:${DPKG_ARCH} libitm1:${DPKG_ARCH} libllvm14:${DPKG_ARCH} libllvm${GITHUB_CLANG_VERSION}:${DPKG_ARCH} libllvmspirvlib-${GITHUB_CLANG_VERSION}-dev:${DPKG_ARCH} libllvmspirvlib${GITHUB_CLANG_VERSION}:${DPKG_ARCH} liblsan0:${DPKG_ARCH} libmpc3:${DPKG_ARCH} libncurses-dev:${DPKG_ARCH} libnsl-dev:${DPKG_ARCH} libobjc-11-dev:${DPKG_ARCH} libobjc4:${DPKG_ARCH} libpciaccess-dev:${DPKG_ARCH} libpciaccess0f:${DPKG_ARCH} libpfm4:${DPKG_ARCH} libpthread-stubs0-dev:${DPKG_ARCH} libquadmath0:${DPKG_ARCH} libsensors-config libsensors-dev:${DPKG_ARCH} libsensors5:${DPKG_ARCH} libset-scalar-perl libstd-rust-1.75:${DPKG_ARCH} libstd-rust-dev:${DPKG_ARCH} libstdc++-11-dev:${DPKG_ARCH} libsub-override-perl libtinfo-dev:${DPKG_ARCH} libtirpc-dev:${DPKG_ARCH} libtool libtsan0:${DPKG_ARCH} libubsan1:${DPKG_ARCH} libva-dev:${DPKG_ARCH} libva-drm2:${DPKG_ARCH} libva-glx2:${DPKG_ARCH} libva-wayland2:${DPKG_ARCH} libva-x11-2:${DPKG_ARCH} libva2:${DPKG_ARCH} libvdpau-dev:${DPKG_ARCH} libvdpau1:${DPKG_ARCH} libvulkan-dev:${DPKG_ARCH} libvulkan1:${DPKG_ARCH} libwayland-bin libwayland-client0:${DPKG_ARCH} libwayland-cursor0:${DPKG_ARCH} libwayland-dev:${DPKG_ARCH} libwayland-egl-backend-dev:${DPKG_ARCH} libwayland-egl1:${DPKG_ARCH} libwayland-server0:${DPKG_ARCH} libx11-dev:${DPKG_ARCH} libx11-xcb-dev:${DPKG_ARCH} libx11-xcb1:${DPKG_ARCH} libxau-dev:${DPKG_ARCH} libxcb-dri2-0:${DPKG_ARCH} libxcb-dri2-0-dev:${DPKG_ARCH} libxcb-dri3-0:${DPKG_ARCH} libxcb-dri3-dev:${DPKG_ARCH} libxcb-glx0:${DPKG_ARCH} libxcb-glx0-dev:${DPKG_ARCH} libxcb-present-dev:${DPKG_ARCH} libxcb-present0:${DPKG_ARCH} libxcb-randr0:${DPKG_ARCH} libxcb-randr0-dev:${DPKG_ARCH} libxcb-render0:${DPKG_ARCH} libxcb-render0-dev:${DPKG_ARCH} libxcb-shape0:${DPKG_ARCH} libxcb-shape0-dev:${DPKG_ARCH} libxcb-shm0:${DPKG_ARCH} libxcb-shm0-dev:${DPKG_ARCH} libxcb-sync-dev:${DPKG_ARCH} libxcb-sync1:${DPKG_ARCH} libxcb-xfixes0:${DPKG_ARCH} libxcb-xfixes0-dev:${DPKG_ARCH} libxcb1-dev:${DPKG_ARCH} libxdmcp-dev:${DPKG_ARCH} libxext-dev:${DPKG_ARCH} libxfixes-dev:${DPKG_ARCH} libxfixes3:${DPKG_ARCH} libxml2-dev:${DPKG_ARCH} libxrandr-dev:${DPKG_ARCH} libxrandr2:${DPKG_ARCH} libxrender-dev:${DPKG_ARCH} libxrender1:${DPKG_ARCH} libxshmfence-dev:${DPKG_ARCH} libxshmfence1:${DPKG_ARCH} libxxf86vm-dev:${DPKG_ARCH} libxxf86vm1:${DPKG_ARCH} libz3-4:${DPKG_ARCH} libz3-dev:${DPKG_ARCH} libzstd-dev:${DPKG_ARCH} linux-libc-dev:${DPKG_ARCH} llvm-${LLVM_VERSION} llvm-${LLVM_VERSION}-dev llvm-${LLVM_VERSION}-linker-tools llvm-${LLVM_VERSION}-runtime llvm-${LLVM_VERSION}-tools llvm-spirv-${LLVM_VERSION} lto-disabled-list m4 make meson ninja-build pkg-config po-debconf python3-mako python3-ply python3-pygments quilt rpcsvc-proto rustc spirv-tools valgrind wayland-protocols x11proto-dev xorg-sgml-doctools xtrans-dev zlib1g-dev:${DPKG_ARCH} \
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
    if command -v brew > /dev/null 2>&1; then
        HOMEBREW_NO_INSTALLED_DEPENDENTS_CHECK=true brew install autoconf automake libx11 libxext libxrandr \
                                                        llvm@${LLVM_VERSION} ninja meson pkg-config libxshmfence
        brew link --overwrite llvm@${LLVM_VERSION}
        # For reasons unknown, this is necessary for pkg-config to find homebrew's packages
        LOCAL_PKG_CONFIG_PATH="/opt/homebrew/lib/pkgconfig:$PKG_CONFIG_PATH"
    elif command -v port > /dev/null 2>&1; then
        sudo port install autoconf automake libx11 libXext libXrandr llvm-${LLVM_VERSION} \
             ninja meson pkgconfig libxshmfence
        # For reasons unknown, this is necessary for pkg-config to find macport's packages
        LOCAL_PKG_CONFIG_PATH="/opt/local/lib/pkgconfig:$PKG_CONFIG_PATH"
    else
        echo "Error: need to install homebrew or macports to continue"
        exit 1
    fi
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
    # This is to properly link lib-xcb-present on the mac build (though we won't be drawing to any
    # real hardware surface).
    sed -I '' "s/dep_xcb_present = null_dep/dep_xcb_present = dependency('xcb-present')/g" meson.build
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
