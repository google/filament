#!/usr/bin/bash

# Copyright (C) 2025 The Android Open Source Project
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

FILAMENT_BOT_TOKEN=$2

function update_to_main() {
    python3 docs_src/build/run.py
    mkdir -p tmp
    pushd .
    cd tmp
    git clone https://x-access-token:${FILAMENT_BOT_TOKEN}@github.com/google/filament.git
    cd filament
    rm -rf docs/*
    cp -r ../../docs_src/src_mdbook/book/* docs
    git add -A docs/*
    git commit -a \
        -m "[automated] Updating /docs due to commit ${COMMIT_HASH:0:7}" \
        -m "" \
        -m "Full commit hash is ${COMMIT_HASH}" \
        -m "" \
        -m "DOCS_ALLOW_DIRECT_EDITS"

    git push origin main
    popd
}

COMMIT_HASH=$1
HAS_EDITS=$((python3 docs_src/build/checks.py --do-or="source_edits,commit_docs_force" $COMMIT_HASH > /dev/null && echo "true") || echo "false")
DO_BYPASS=$((python3 docs_src/build/checks.py --do-and="commit_docs_bypass" $COMMIT_HASH && echo "true") || echo "false")

if [[ "${HAS_EDITS}" == "true" && "${DO_BYPASS}" == "false" ]]; then
    # Run again to output the edited files:
    python3 docs_src/build/checks.py --do-or="source_edits,commit_docs_force" $COMMIT_HASH
    update_to_main
else
    echo "has edits (to /docs_src): ${HAS_EDITS}"
    echo "bypass: ${DO_BYPASS}"
fi
