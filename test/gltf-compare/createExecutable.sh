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

#!/bin/bash

source test/renderdiff/src/preamble.sh

if [[ "$GITHUB_WORKFLOW" ]]; then
    echo "This is meant to run locally (not part of the CI)"
    exit 1
else
    GOLDEN_BRANCH=$(git log -1 | python3 test/renderdiff/src/commit_msg.py)
fi

./build.sh -f "$@" -p desktop debug gltf_viewer
# -f -W -q arm64-v8a 

