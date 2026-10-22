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

set(EXPAT_CFLAGS "${CMAKE_C_FLAGS} ${FUZZING_FLAGS}")
set(EXPAT_CXXFLAGS "${CMAKE_CXX_FLAGS} ${FUZZING_FLAGS}")
include(expat)

add_executable(expat_example
               expat_example.cc)
target_link_libraries(expat_example
                      protobuf-mutator-xml
                      ${LIB_PROTO_MUTATOR_FUZZER_LIBRARIES}
                      ${EXPAT_LIBRARIES})
set_property(TARGET expat_example
             PROPERTY COMPILE_FLAGS ${FUZZING_FLAGS})
set_property(TARGET expat_example
             PROPERTY LINK_FLAGS ${FUZZING_FLAGS_BINARY})

add_executable(expat_example_test
               expat_example_test.cc)
add_dependencies(expat_example_test expat_example)
target_link_libraries(expat_example_test
                      ${GTEST_BOTH_LIBRARIES}
                      ${CMAKE_THREAD_LIBS_INIT})
add_test(test.expat_example_test expat_example_test --gtest_color=yes AUTO)
if(LIB_PROTO_MUTATOR_TESTING)
  add_dependencies(check expat_example_test)
endif()
