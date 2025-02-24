# Copyright (c) 2018 Google LLC.
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

add_test(NAME spirv-tools_expect_unittests
         COMMAND Python3::Interpreter -m unittest expect_unittest.py
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME spirv-tools_spirv_test_framework_unittests
         COMMAND Python3::Interpreter -m unittest spirv_test_framework_unittest.py
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_spvtools_unittest(
  TARGET spirv_unit_test_tools_util
  SRCS flags_test.cpp ${spirv-tools_SOURCE_DIR}/tools/util/flags.cpp
  LIBS ${SPIRV_TOOLS_FULL_VISIBILITY}
  DEFINES TESTING=1)

add_subdirectory(opt)
if(NOT (${CMAKE_SYSTEM_NAME} STREQUAL "Android"))
  add_subdirectory(objdump)
endif ()
