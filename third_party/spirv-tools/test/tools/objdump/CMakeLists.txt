# Copyright (c) 2023 Google LLC.
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

add_spvtools_unittest(
  TARGET spirv_unit_test_tools_objdump
  SRCS
    extract_source_test.cpp
    ${spirv-tools_SOURCE_DIR}/tools/util/flags.cpp
    ${spirv-tools_SOURCE_DIR}/tools/util/cli_consumer.cpp
    ${spirv-tools_SOURCE_DIR}/tools/objdump/extract_source.cpp
    LIBS ${SPIRV_TOOLS_FULL_VISIBILITY} SPIRV-Tools-opt
  DEFINES TESTING=1)
