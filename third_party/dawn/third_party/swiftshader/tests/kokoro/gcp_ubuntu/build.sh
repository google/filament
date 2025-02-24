#!/bin/bash

# Copyright 2022 The SwiftShader Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e # Fail on any error.

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd )"
ROOT_DIR="$( cd "${SCRIPT_DIR}/../../.." >/dev/null 2>&1 && pwd )"

# Inside the docker VM, we clone the project to a new directory.
# We do this so that the docker script can be tested in a local development
# checkout, without having the build litter the local checkout with artifacts.
# This directory is mapped to the host temporary directory.
# Kokoro uses a '/tmpfs' root, where as most linux enviroments just have '/tmp'
if [ -d "/tmpfs" ]; then
    TMP_DIR=/tmpfs
else
    TMP_DIR=/tmp
fi

# --privileged is required for some sanitizer builds, as they seem to require PTRACE privileges
docker run --rm -i \
  --privileged \
  --volume "${ROOT_DIR}:${ROOT_DIR}" \
  --volume "${TMP_DIR}:/src" \
  --volume "${KOKORO_ARTIFACTS_DIR}:/mnt/artifacts" \
  --workdir "${ROOT_DIR}" \
  --env SRC_DIR="/src/swiftshader" \
  --env BUILD_TYPE=$BUILD_TYPE \
  --env REACTOR_BACKEND=$REACTOR_BACKEND \
  --env LLVM_VERSION=$LLVM_VERSION \
  --entrypoint "${SCRIPT_DIR}/docker.sh" \
  "gcr.io/shaderc-build/radial-build:latest"
