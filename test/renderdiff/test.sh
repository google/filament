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

#!/usr/bin/bash

source `dirname $0`/src/preamble.sh

start_ && \
    bash `dirname $0`/generate.sh && \
    python3 ${RENDERDIFF_TEST_DIR}/src/golden_manager.py --output=${GOLDEN_OUTPUT_DIR} && \
    python3 ${RENDERDIFF_TEST_DIR}/src/compare.py \
            --src=${GOLDEN_OUTPUT_DIR} \
            --dest=${RENDER_OUTPUT_DIR} \
            --out=${DIFF_OUTPUT_DIR} && \
    end_
