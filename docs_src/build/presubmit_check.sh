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

COMMIT_HASH=$1
python3 docs_src/build/checks.py --do-or="no_direct_edits,commit_docs_allow_direct_edits" $COMMIT_HASH ||\
    (echo "Direct edits to /docs are not allowed. Please see /docs_src/README.md if you need to bypass this check." && exit 1)
