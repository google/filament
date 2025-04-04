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

#!/usr/bin/bash

CODE_CORRECTNESS_TEST_DIR="$(pwd)/test/code-correctness"

# Check if the clang-tidy command exists and is executable
if ! command -v clang-tidy > /dev/null 2>&1; then
  # If command -v fails (returns a non-zero exit status), clang-tidy is not found
  echo "Error: clang-tidy command not found." >&2
  echo "Please install clang-tidy or ensure it is in your system's PATH." >&2
  exit 1 # Exit the script with an error code (conventionally non-zero for failure)
fi

set -e && set -x && \
    python3 ${CODE_CORRECTNESS_TEST_DIR}/src/run.py
