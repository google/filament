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

# Attempts to roll all entries in DEPS to tip-of-tree and create a commit.
#
# Depends on roll-dep from depot_path being in PATH.

effcee_dir="external/effcee/"
effcee_trunk="origin/main"
googletest_dir="external/googletest/"
googletest_trunk="origin/master"
re2_dir="external/re2/"
re2_trunk="origin/master"
spirv_headers_dir="external/spirv-headers/"
spirv_headers_trunk="origin/master"

# This script assumes it's parent directory is the repo root.
repo_path=$(dirname "$0")/..

cd "$repo_path"

if [[ $(git diff --stat) != '' ]]; then
    echo "Working tree is dirty, commit changes before attempting to roll DEPS"
    exit 1
fi

old_head=$(git rev-parse HEAD)

roll-dep --ignore-dirty-tree --roll-to="${effcee_trunk}" "${effcee_dir}"
roll-dep --ignore-dirty-tree --roll-to="${googletest_trunk}" "${googletest_dir}"
roll-dep --ignore-dirty-tree --roll-to="${re2_trunk}" "${re2_dir}"
roll-dep --ignore-dirty-tree --roll-to="${spirv_headers_trunk}" "${spirv_headers_dir}"

git rebase --interactive "${old_head}"
