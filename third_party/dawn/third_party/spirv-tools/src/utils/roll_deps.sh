#!/usr/bin/env bash
# Copyright (c) 2021 Google LLC
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
# Depends on roll-dep from depot_tools
# (https://chromium.googlesource.com/chromium/tools/depot_tools) being in PATH.

set -eo pipefail

function ExitIfIsInterestingError() {
  local return_code=$1
  if [[ ${return_code} -ne 0 && ${return_code} -ne 2 ]]; then
    exit ${return_code}
  fi
  return 0
}


declare -A dependency_to_branch_map
dependency_to_branch_map["external/abseil_cpp"]="origin/master"
dependency_to_branch_map["external/effcee/"]="origin/main"
dependency_to_branch_map["external/googletest/"]="origin/main"
dependency_to_branch_map["external/re2/"]="origin/main"
dependency_to_branch_map["external/spirv-headers/"]="origin/main"

# This script assumes it's parent directory is the repo root.
repo_path=$(dirname "$0")/..

cd "$repo_path"

if [[ $(git diff --stat) != '' ]]; then
    echo "Working tree is dirty, commit changes before attempting to roll DEPS"
    exit 1
fi

echo "*** Ignore messages about running 'git cl upload' ***"

old_head=$(git rev-parse HEAD)

set +e

for dep in ${!dependency_to_branch_map[@]}; do
  branch=${dependency_to_branch_map[$dep]}
  echo "Rolling $dep"
  roll-dep --ignore-dirty-tree --roll-to="${branch}" "${dep}"
  ExitIfIsInterestingError $?
done
