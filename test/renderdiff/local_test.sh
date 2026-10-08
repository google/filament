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

start_

if [[ "$GITHUB_WORKFLOW" ]]; then
    echo "This is meant to run locally (not part of the CI)"
    exit 1
else
    GOLDEN_BRANCH=$(git log -1 | python3 test/renderdiff/src/commit_msg.py)
fi

# The goldens are rendered by presubmit on Linux/aarch64, and the golden path records only the
# platform and the backend, not the host. A different host produces different pixels for reasons
# that have nothing to do with the change under test: a different compiler, a different Mesa build,
# a different libm. Rendering locally is still the fastest way to see what a change does, so this
# warns rather than refusing to run.
if [[ "$(uname -s)" != "Linux" ]] || [[ "$(uname -m)" != "aarch64" ]]; then
    echo ""
    echo "############################################################################"
    echo "# WARNING: this host is $(uname -s)/$(uname -m), but the goldens are"
    echo "# generated on Linux/aarch64. Comparison results below are advisory:"
    echo "# differences are expected and are not necessarily regressions."
    echo "#"
    echo "# To judge a change against the goldens, read the presubmit result, or use"
    echo "# the 'Renderdiff Goldens' workflow. See test/renderdiff/README.md."
    echo "############################################################################"
    echo ""
fi

TEST_CONFIG="${RENDERDIFF_TEST_DIR}/tests/presubmit.json"
PASSTHROUGH_ARGS=()

for i in "$@"
do
case $i in
    --test=*)
    TEST_CONFIG="${i#*=}"
    ;;
    *)
    PASSTHROUGH_ARGS+=("$i")
    ;;
esac
done

# generate.sh builds diffimg along with the renderers, so there is no separate build step here.
bash `dirname $0`/generate.sh --test="${TEST_CONFIG}" "${PASSTHROUGH_ARGS[@]}" && \
    python3 ${RENDERDIFF_TEST_DIR}/src/golden_manager.py \
            --branch=${GOLDEN_BRANCH} \
            --output=${GOLDEN_OUTPUT_DIR} && \
    python3 ${RENDERDIFF_TEST_DIR}/src/compare.py \
            --src=${GOLDEN_OUTPUT_DIR} \
            --dest=${RENDER_OUTPUT_DIR} \
            --out=${DIFF_OUTPUT_DIR} \
            --diffimg="${DIFFIMG_PATH}" \
            --test="${TEST_CONFIG}" "${PASSTHROUGH_ARGS[@]}"

end_

