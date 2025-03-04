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

# The file contains Docker container identifiers currently used by test scripts.
# Test scripts should source this file to get the identifiers.

readonly LINUX_ALPINE_CONTAINER="gcr.io/google.com/absl-177019/alpine:20230612"
readonly LINUX_CLANG_LATEST_CONTAINER="gcr.io/google.com/absl-177019/linux_hybrid-latest:20231218"
readonly LINUX_ARM_CLANG_LATEST_CONTAINER="gcr.io/google.com/absl-177019/linux_arm_hybrid-latest:20231219"
readonly LINUX_GCC_LATEST_CONTAINER="gcr.io/google.com/absl-177019/linux_hybrid-latest:20231218"
readonly LINUX_GCC_FLOOR_CONTAINER="gcr.io/google.com/absl-177019/linux_gcc-floor:20230120"
