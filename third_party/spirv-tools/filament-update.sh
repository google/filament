# Copyright (C) 2023 The Android Open Source Project
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


# This script is used to update SPIRV-Tools from source.
# This script takes in a git commit hash as an argument.

#!/usr/bin/env bash

TOOLS_HASH=$1

function sync_khronos_repo() {
    local REPO=$1
    local HASH=$2
    pushd .
    cd /tmp
    rm -rf ${REPO} && git clone git@github.com:KhronosGroup/${REPO}.git
    cd ${REPO}
    git reset --hard ${HASH}
    popd
}

# First we update SPIRV-Tools to the given git hash
sync_khronos_repo SPIRV-Tools ${TOOLS_HASH}

rsync -r /tmp/SPIRV-Tools/ ./ --delete
rm -rf .git .github
# Recover the filament specific files lost in the above rsync
git checkout filament-specific-changes.patch FILAMENT_README.md filament-update.sh
git apply filament-specific-changes.patch

HEADERS_HASH=`grep  "spirv_headers_revision':" DEPS | awk '{ print $2 }' | sed "s/[\'\,\\n]//g"`

# Next we update SPIRV-Headers to the right hash (dependency as given by SPIRV-Tools)
sync_khronos_repo SPIRV-Headers ${HEADERS_HASH}

pushd .
cd ../spirv-headers
rsync -r /tmp/SPIRV-Headers/ ./ --delete
rm -rf .git .github
# Recover the filament specific files lost in the above rsync
git checkout filament-specific-changes.patch FILAMENT_README.md
git apply filament-specific-changes.patch
popd
