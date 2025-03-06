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

add_executable(libfuzzer_example_test
               libfuzzer_example_test.cc)
target_link_libraries(libfuzzer_example_test
                      ${GTEST_BOTH_LIBRARIES}
                      ${CMAKE_THREAD_LIBS_INIT})
add_test(test.libfuzzer_example_test libfuzzer_example_test --gtest_color=yes AUTO)
if(LIB_PROTO_MUTATOR_TESTING)
  add_dependencies(check libfuzzer_example_test)
endif()

protobuf_generate_cpp(LIB_FUZZER_EXAMPLE_PROTO_SRCS
                      LIB_FUZZER_EXAMPLE_PROTO_HDRS
                      libfuzzer_example.proto)

add_library(fuzzer-example-proto
            ${LIB_FUZZER_EXAMPLE_PROTO_SRCS})
set_property(TARGET fuzzer-example-proto
             PROPERTY COMPILE_FLAGS ${NO_FUZZING_FLAGS})

foreach(fuzzer libfuzzer_example libfuzzer_bin_example)
  add_executable(${fuzzer} ${fuzzer}.cc)
  target_link_libraries(${fuzzer}
                        fuzzer-example-proto
                        protobuf-mutator-libfuzzer
                        ${LIB_PROTO_MUTATOR_FUZZER_LIBRARIES})
  set_property(TARGET ${fuzzer}
               PROPERTY COMPILE_FLAGS ${FUZZING_FLAGS})
  set_property(TARGET ${fuzzer}
               PROPERTY LINK_FLAGS ${FUZZING_FLAGS_BINARY})
  if(LIB_PROTO_MUTATOR_TESTING)
    add_dependencies(libfuzzer_example_test ${fuzzer})
  endif()
endforeach(fuzzer)
