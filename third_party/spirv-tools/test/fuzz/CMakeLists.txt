# Copyright (c) 2019 Google LLC
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

if (${SPIRV_BUILD_FUZZER})

  set(SOURCES
          fuzz_test_util.h

          data_synonym_transformation_test.cpp
          equivalence_relation_test.cpp
          fact_manager_test.cpp
          fuzz_test_util.cpp
          fuzzer_pass_add_useful_constructs_test.cpp
          fuzzer_pass_donate_modules_test.cpp
          instruction_descriptor_test.cpp
          transformation_add_constant_boolean_test.cpp
          transformation_add_constant_composite_test.cpp
          transformation_add_constant_scalar_test.cpp
          transformation_add_dead_block_test.cpp
          transformation_add_dead_break_test.cpp
          transformation_add_dead_continue_test.cpp
          transformation_add_function_test.cpp
          transformation_add_global_undef_test.cpp
          transformation_add_global_variable_test.cpp
          transformation_add_no_contraction_decoration_test.cpp
          transformation_add_type_array_test.cpp
          transformation_add_type_boolean_test.cpp
          transformation_add_type_float_test.cpp
          transformation_add_type_function_test.cpp
          transformation_add_type_int_test.cpp
          transformation_add_type_matrix_test.cpp
          transformation_add_type_pointer_test.cpp
          transformation_add_type_struct_test.cpp
          transformation_add_type_vector_test.cpp
          transformation_composite_construct_test.cpp
          transformation_composite_extract_test.cpp
          transformation_copy_object_test.cpp
          transformation_merge_blocks_test.cpp
          transformation_move_block_down_test.cpp
          transformation_outline_function_test.cpp
          transformation_replace_boolean_constant_with_constant_binary_test.cpp
          transformation_replace_constant_with_uniform_test.cpp
          transformation_replace_id_with_synonym_test.cpp
          transformation_set_function_control_test.cpp
          transformation_set_loop_control_test.cpp
          transformation_set_memory_operands_mask_test.cpp
          transformation_set_selection_control_test.cpp
          transformation_split_block_test.cpp
          transformation_vector_shuffle_test.cpp
          uniform_buffer_element_descriptor_test.cpp)

  if (${SPIRV_ENABLE_LONG_FUZZER_TESTS})
    # These are long-running tests that depend on random seeds.  We do not want
    # to run them during regular whole-project CI because they may reveal
    # spirv-fuzz bugs in changes that are totally unrelated to spirv-fuzz,
    # which would be counfounding.  Instead, they should be run regularly but
    # separately.
    set(SOURCES ${SOURCES}
            fuzzer_replayer_test.cpp
            fuzzer_shrinker_test.cpp)
  endif()

  add_spvtools_unittest(TARGET fuzz
        SRCS ${SOURCES}
        LIBS SPIRV-Tools-fuzz
        )
endif()
