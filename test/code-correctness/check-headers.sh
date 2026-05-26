# Copyright (C) 2026 The Android Open Source Project
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

#!/usr/bin/env bash

#
# check-headers.sh
#
# Checks if header files are properly organized according to reorganize-headers/run.py
#
# Usage:
#   tools/check-headers.sh <commit-hash>
#   tools/check-headers.sh <file1> [<file2> ...]

set -e

if [ $# -eq 0 ]; then
    echo "Usage: $0 <commit-hash> | <file>..."
    exit 1
fi

FILES=()
# Check if the first argument is a valid commit hash and not a file
if [ $# -eq 1 ] && git rev-parse --verify --quiet "$1^{commit}" >/dev/null && [ ! -f "$1" ]; then
    COMMIT="$1"
    # Extract added, copied, modified, renamed, and type-changed files (exclude deleted)
    while IFS= read -r -d '' file; do
        FILES+=("$file")
    done < <(git diff-tree --no-commit-id -z --name-only --diff-filter=d -r "$COMMIT")
else
    FILES=("$@")
fi

FAILED=0
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REORG_SCRIPT="$SCRIPT_DIR/../../tools/reorganize-headers/run.py"

if [ ! -f "$REORG_SCRIPT" ]; then
    echo "Error: Cannot find $REORG_SCRIPT"
    exit 1
fi

for file in "${FILES[@]}"; do
    if [ ! -f "$file" ]; then
        continue
    fi

    # Filter for C/C++ files
    if [[ ! "$file" =~ \.(c|cpp|h|hpp)$ ]]; then
        continue
    fi

    TMP_ORIG=$(mktemp)
    # Copy preserving attributes, so we can restore timestamps later if needed
    cp -p "$file" "$TMP_ORIG"

    # Run the reorganization script
    python3 "$REORG_SCRIPT" "$file" > /dev/null

    # diff -u returns 1 if files differ, 0 if they are identical
    if ! diff -u "$TMP_ORIG" "$file" > "$TMP_ORIG.diff"; then
        echo -e "\n\033[31m❌ Header reorganization needed in: $file\033[0m"
        # Print diff cleanly, abstracting away the temporary paths
        sed "s|--- $TMP_ORIG.*|--- a/$file|g; s|+++ $file.*|+++ b/$file|g" "$TMP_ORIG.diff"
        FAILED=1

        # Revert changes and restore original timestamp
        cat "$TMP_ORIG" > "$file"
        touch -r "$TMP_ORIG" "$file"
    fi

    rm -f "$TMP_ORIG" "$TMP_ORIG.diff"
done

if [ "$FAILED" -eq 1 ]; then
    echo -e "\n\033[31mPlease run tools/reorganize-headers/run.py on the above files to fix the issues.\033[0m"
    exit 1
else
    echo -e "\033[32m✔ All checked headers are correctly organized.\033[0m"
    exit 0
fi
