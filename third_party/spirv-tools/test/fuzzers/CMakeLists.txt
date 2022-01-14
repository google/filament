# Copyright (c) 2021 Google LLC
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

function(add_spvtools_libfuzzer_target)
  set(one_value_args TARGET)
  set(multi_value_args SRCS LIBS)
  cmake_parse_arguments(
    ARG "" "${one_value_args}" "${multi_value_args}" ${ARGN})

  add_executable(${ARG_TARGET} ${ARG_SRCS})
  spvtools_default_compile_options(${ARG_TARGET})
  target_link_libraries(${ARG_TARGET} PRIVATE ${ARG_LIBS})
  target_include_directories(${ARG_TARGET} PRIVATE
    ${spirv-tools_SOURCE_DIR}
    ${spirv-tools_BINARY_DIR}
  )
  set_property(TARGET ${ARG_TARGET} PROPERTY FOLDER "SPIRV-Tools libFuzzer targets")
  if(NOT ${SPIRV_LIB_FUZZING_ENGINE_LINK_OPTIONS} STREQUAL "")
    # This is set when the fuzzers are being built by OSS-Fuzz. In this case the
    # variable provides the necessary linker flags, and OSS-Fuzz will take care
    # of passing suitable compiler flags.
    target_link_options(${ARG_TARGET} PRIVATE ${SPIRV_LIB_FUZZING_ENGINE_LINK_OPTIONS})
  else()
    # When the fuzzers are being built outside of OSS-Fuzz, standard libFuzzer
    # arguments to enable fuzzing are used.
    target_compile_options(${ARG_TARGET} PRIVATE "-fsanitize=fuzzer")
    target_link_options(${ARG_TARGET} PRIVATE "-fsanitize=fuzzer")    
  endif()
endfunction()

if (${SPIRV_BUILD_LIBFUZZER_TARGETS})
  if(NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    message(FATAL_ERROR "The libFuzzer targets are only supported with the Clang compiler. Compiler '${CMAKE_CXX_COMPILER_ID}' is not supported!")
  endif()
  add_spvtools_libfuzzer_target(TARGET spvtools_as_fuzzer SRCS spvtools_as_fuzzer.cpp random_generator.cpp LIBS ${SPIRV_TOOLS_FULL_VISIBILITY})
  add_spvtools_libfuzzer_target(TARGET spvtools_binary_parser_fuzzer SRCS spvtools_binary_parser_fuzzer.cpp random_generator.cpp LIBS ${SPIRV_TOOLS_FULL_VISIBILITY})
  add_spvtools_libfuzzer_target(TARGET spvtools_dis_fuzzer SRCS spvtools_dis_fuzzer.cpp random_generator.cpp LIBS ${SPIRV_TOOLS_FULL_VISIBILITY})
  add_spvtools_libfuzzer_target(TARGET spvtools_opt_legalization_fuzzer SRCS spvtools_opt_legalization_fuzzer.cpp random_generator.cpp LIBS SPIRV-Tools-opt ${SPIRV_TOOLS_FULL_VISIBILITY})
  add_spvtools_libfuzzer_target(TARGET spvtools_opt_performance_fuzzer SRCS spvtools_opt_performance_fuzzer.cpp random_generator.cpp LIBS SPIRV-Tools-opt ${SPIRV_TOOLS_FULL_VISIBILITY})
  add_spvtools_libfuzzer_target(TARGET spvtools_opt_size_fuzzer SRCS spvtools_opt_size_fuzzer.cpp random_generator.cpp LIBS SPIRV-Tools-opt ${SPIRV_TOOLS_FULL_VISIBILITY})
  add_spvtools_libfuzzer_target(TARGET spvtools_val_fuzzer SRCS spvtools_val_fuzzer.cpp random_generator.cpp LIBS ${SPIRV_TOOLS_FULL_VISIBILITY})
  if (${SPIRV_BUILD_FUZZER})
    add_spvtools_libfuzzer_target(TARGET spvtools_fuzz_fuzzer SRCS spvtools_fuzz_fuzzer.cpp random_generator.cpp LIBS SPIRV-Tools-fuzz ${SPIRV_TOOLS_FULL_VISIBILITY})
  endif()
endif()
