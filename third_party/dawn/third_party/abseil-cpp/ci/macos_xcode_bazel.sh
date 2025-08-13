#!/bin/bash
#
# Copyright 2019 The Abseil Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# This script is invoked on Kokoro to test Abseil on macOS.
# It is not hermetic and may break when Kokoro is updated.

set -euox pipefail

# Use Xcode 16.3
sudo xcode-select -s /Applications/Xcode_16.3.app/Contents/Developer

if [[ -z ${ABSEIL_ROOT:-} ]]; then
  ABSEIL_ROOT="$(realpath $(dirname ${0})/..)"
fi

# If we are running on Kokoro, check for a versioned Bazel binary.
KOKORO_GFILE_BAZEL_BIN="bazel-8.2.1-darwin-x86_64"
if [[ ${KOKORO_GFILE_DIR:-} ]] && [[ -f ${KOKORO_GFILE_DIR}/${KOKORO_GFILE_BAZEL_BIN} ]]; then
  BAZEL_BIN="${KOKORO_GFILE_DIR}/${KOKORO_GFILE_BAZEL_BIN}"
  chmod +x ${BAZEL_BIN}
else
  BAZEL_BIN="bazel"
fi

# Use Bazel Vendor mode to reduce reliance on external dependencies.
if [[ ${KOKORO_GFILE_DIR:-} ]] && [[ -f "${KOKORO_GFILE_DIR}/distdir/abseil-cpp_vendor.tar.gz" ]]; then
  tar -xf "${KOKORO_GFILE_DIR}/distdir/abseil-cpp_vendor.tar.gz" -C "${TMP}/"
  BAZEL_EXTRA_ARGS="--vendor_dir=\"${TMP}/abseil-cpp_vendor\" ${BAZEL_EXTRA_ARGS:-}"
fi

# Print the compiler and Bazel versions.
echo "---------------"
gcc -v
echo "---------------"
${BAZEL_BIN} version
echo "---------------"

cd ${ABSEIL_ROOT}

if [[ -n "${ALTERNATE_OPTIONS:-}" ]]; then
  cp ${ALTERNATE_OPTIONS:-} absl/base/options.h || exit 1
fi

${BAZEL_BIN} test ... \
  --copt="-DGTEST_REMOVE_LEGACY_TEST_CASEAPI_=1" \
  --copt="-Werror" \
  --cxxopt="-std=c++17" \
  --enable_bzlmod=true \
  --features=external_include_paths \
  --keep_going \
  --show_timestamps \
  --test_env="TZDIR=${ABSEIL_ROOT}/absl/time/internal/cctz/testdata/zoneinfo" \
  --test_output=errors \
  --test_tag_filters=-benchmark \
  ${BAZEL_EXTRA_ARGS:-}
