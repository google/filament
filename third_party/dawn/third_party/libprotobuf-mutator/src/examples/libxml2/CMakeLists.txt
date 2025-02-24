# Copyright 2017 Google Inc. All rights reserved.
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

set(LIBXML2_CFLAGS "${CMAKE_C_FLAGS} ${FUZZING_FLAGS} -w")
set(LIBXML2_CXXFLAGS "${CMAKE_CXX_FLAGS} ${FUZZING_FLAGS} -w")
include(libxml2)
add_executable(libxml2_example
               libxml2_example.cc)
target_link_libraries(libxml2_example
                      protobuf-mutator-xml
                      ${LIB_PROTO_MUTATOR_FUZZER_LIBRARIES}
                      ${LIBXML2_LIBRARIES}
                      ${ZLIB_LIBRARIES}
                      ${LIBLZMA_LIBRARIES})
set_property(TARGET libxml2_example
             PROPERTY COMPILE_FLAGS ${FUZZING_FLAGS})
set_property(TARGET libxml2_example
             PROPERTY LINK_FLAGS ${FUZZING_FLAGS_BINARY})

add_executable(libxml2_example_test
               libxml2_example_test.cc)
add_dependencies(libxml2_example_test libxml2_example)
target_link_libraries(libxml2_example_test
                      ${GTEST_BOTH_LIBRARIES}
                      ${CMAKE_THREAD_LIBS_INIT})
add_test(test.libxml2_example_test libxml2_example_test --gtest_color=yes AUTO)
if (LIB_PROTO_MUTATOR_TESTING)
  add_dependencies(check libxml2_example_test)
endif()
