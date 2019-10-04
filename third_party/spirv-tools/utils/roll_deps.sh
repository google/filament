#!/usr/bin/env bash
# Copyright (c) 2019 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Attempts to roll all entries in DEPS to origin/master and creates a
# commit.
#
# Depends on roll-dep from depot_path being in PATH.

# This script assumes it's parent directory is the repo root.
repo_path=$(dirname "$0")/..

effcee_dir="external/effcee/"
googletest_dir="external/googletest/"
re2_dir="external/re2/"
spirv_headers_dir="external/spirv-headers/"

cd "$repo_path"

roll-dep "$@" "${effcee_dir}" "${googletest_dir}" "${re2_dir}" "${spirv_headers_dir}"
