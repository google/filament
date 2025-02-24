# Copyright (c) 2022 Google LLC.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

include(diff_files/diff_test_files_autogen.cmake)

add_spvtools_unittest(TARGET lcs
  SRCS lcs_test.cpp
  LIBS SPIRV-Tools-diff
)

add_spvtools_unittest(TARGET diff
  SRCS diff_test.cpp diff_test_utils.h diff_test_utils.cpp
       ${DIFF_TEST_FILES} ${spirv-tools_SOURCE_DIR}/tools/util/cli_consumer.cpp
  LIBS SPIRV-Tools-diff
)
