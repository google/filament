#!/bin/bash
#
# Copyright 2025 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

set -euo pipefail

# Version format: vYYYYMMDD.HHMMSS or vYYYYMMDD.HHMMSS-FORKOWNER.dawn.BRANCHNAME
VERSION_DATETIME=$(git show -s --date=format:'%Y%m%d.%H%M%S' --format=%cd)
VERSION_SUFFIX=${GITHUB_REPOSITORY/\//.}.${GITHUB_REF_NAME}
if [[ "$VERSION_SUFFIX" != "google.dawn.main" ]] ; then
    PKG_VERSION=v${VERSION_DATETIME}-${VERSION_SUFFIX}
else
    PKG_VERSION=v${VERSION_DATETIME}
fi
PKG_FILE=emdawnwebgpu_pkg-${PKG_VERSION}.zip
REMOTE_PORT_FILE=emdawnwebgpu-${PKG_VERSION}.remoteport.py

# Initialize emsdk so we can use emcmake. Other dependencies wll be downloaded
# later via DAWN_FETCH_DEPENDENCIES (set in dawn-ci.cmake).
git submodule update --init --depth=1 third_party/emsdk
python3 tools/activate-emsdk

# First build the link test in debug mode as a basic test.
third_party/emsdk/upstream/emscripten/emcmake cmake -S=. -B=out/wasm \
    -C=.github/workflows/dawn-ci.cmake \
    -DCMAKE_BUILD_TYPE=Debug
make -j4 -C out/wasm emdawnwebgpu_link_test

# Switch the build type (in-place to save time), rebuild the link test (this
# time with Closure, which verifies the linked JS to some extent), and build the
# final package (which is not actually affected by build type).
# TODO: If we have Ninja (from depot_tools), we could use -G'Ninja Multi-Config'
# to do multiple build types more cleanly.
# https://cmake.org/cmake/help/latest/generator/Ninja%20Multi-Config.html
cmake -S=. -B=out/wasm -DCMAKE_BUILD_TYPE=Release
make -j4 -C out/wasm emdawnwebgpu_pkg emdawnwebgpu_link_test

# Get variables for documentation.
SHA=$(git rev-parse HEAD)
EMSDK_VERSION=$(python3 tools/activate-emsdk --get-emsdk-version)

# Create zip
cat << EOF > out/wasm/emdawnwebgpu_pkg/VERSION.txt
Dawn release ${PKG_VERSION} at revision <https://dawn.googlesource.com/dawn/+log/${SHA}>.
Built/tested with emsdk release ${EMSDK_VERSION}.
EOF
(cd out/wasm && zip -9roX - emdawnwebgpu_pkg > "../../${PKG_FILE}")
PKG_FILE_SHA512=$(python3 -c 'import hashlib, sys; print(hashlib.sha512(sys.stdin.buffer.read()).hexdigest())' < "${PKG_FILE}")

cat << EOF > "$REMOTE_PORT_FILE"
# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

# https://dawn.googlesource.com/dawn/+/${SHA}/src/emdawnwebgpu/pkg/README.md
r"""
The full README of Emdawnwebgpu follows.

$(cat out/wasm/emdawnwebgpu_pkg/README.md)
"""

TAG = '${PKG_VERSION}'

EXTERNAL_PORT = f'https://github.com/${GITHUB_REPOSITORY}/releases/download/{TAG}/emdawnwebgpu_pkg-{TAG}.zip'
SHA512 = '${PKG_FILE_SHA512}'
PORT_FILE = 'emdawnwebgpu_pkg/emdawnwebgpu.port.py'

# Port information (required)

# - Visible in emcc --show-ports and emcc --use-port=emdawnwebgpu:help
LICENSE = "Some files: BSD 3-Clause License. Other files: Emscripten's license (available under both MIT License and University of Illinois/NCSA Open Source License)"

# - Visible in emcc --use-port=emdawnwebgpu:help
DESCRIPTION = "Emdawnwebgpu implements webgpu.h on WebGPU, replacing -sUSE_WEBGPU. **For info on usage and filing feedback, see link below.**"
URL = 'https://dawn.googlesource.com/dawn/+/${SHA}/src/emdawnwebgpu/pkg/README.md'
EOF

# Create RELEASE_INFO.md
cat << EOF > RELEASE_INFO.md
$(cat out/wasm/emdawnwebgpu_pkg/VERSION.txt)

Use either the \`emdawnwebgpu-*.remoteport.py\` file (Emscripten 4.0.10+) or the \`emdawnwebgpu_pkg-*.zip\`.
For full instructions, see the [README](https://dawn.googlesource.com/dawn/+/${SHA}/src/emdawnwebgpu/pkg/README.md) which is included in both files.
EOF

# Save version numbers for later steps
if [[ -n "${GITHUB_OUTPUT:-}" ]] ; then
    echo "PKG_VERSION=$PKG_VERSION" >> $GITHUB_OUTPUT
    echo "PKG_FILE=$PKG_FILE" >> $GITHUB_OUTPUT
    echo "REMOTE_PORT_FILE=$REMOTE_PORT_FILE" >> $GITHUB_OUTPUT
fi
