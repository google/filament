#!/bin/bash

# Copyright 2020 The Marl Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e # Fail on any error.

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd )"
ROOT_DIR="$( cd "${SCRIPT_DIR}/../.." >/dev/null 2>&1 && pwd )"

# Fetch submodules outside of Docker, to avoid a 'detected dubious ownership in repository' git error
git -C "${ROOT_DIR}" submodule update --init

# --privileged is required for some sanitizer builds, as they seem to require PTRACE privileges
docker run --rm -i \
  --privileged \
  --volume "${ROOT_DIR}:${ROOT_DIR}" \
  --volume "${KOKORO_ARTIFACTS_DIR}:/mnt/artifacts" \
  --workdir "${ROOT_DIR}" \
  --env BUILD_SYSTEM=${BUILD_SYSTEM} \
  --env BUILD_TOOLCHAIN=${BUILD_TOOLCHAIN} \
  --env BUILD_TYPE=${BUILD_SHARED:-Debug} \
  --env BUILD_TARGET_ARCH=${BUILD_TARGET_ARCH} \
  --env BUILD_SHARED=${BUILD_SHARED:-0} \
  --env BUILD_SANITIZER=${BUILD_SANITIZER} \
  --env BUILD_ARTIFACTS="/mnt/artifacts" \
  --entrypoint "${SCRIPT_DIR}/docker.sh" \
  "gcr.io/shaderc-build/radial-build:latest"
