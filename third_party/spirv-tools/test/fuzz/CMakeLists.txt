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

          available_instructions_test.cpp
          call_graph_test.cpp
          comparator_deep_blocks_first_test.cpp
          data_synonym_transformation_test.cpp
          equivalence_relation_test.cpp
          fact_manager/constant_uniform_facts_test.cpp
          fact_manager/data_synonym_and_id_equation_facts_test.cpp
          fact_manager/dead_block_facts_test.cpp
          fact_manager/irrelevant_value_facts_test.cpp
          fuzz_test_util.cpp
          fuzzer_pass_add_opphi_synonyms_test.cpp
          fuzzer_pass_construct_composites_test.cpp
          fuzzer_pass_donate_modules_test.cpp
          fuzzer_pass_outline_functions_test.cpp
          fuzzerutil_test.cpp
          instruction_descriptor_test.cpp
          fuzzer_pass_test.cpp
          replayer_test.cpp
          shrinker_test.cpp
          transformation_access_chain_test.cpp
          transformation_add_bit_instruction_synonym_test.cpp
          transformation_add_constant_boolean_test.cpp
          transformation_add_constant_composite_test.cpp
          transformation_add_constant_null_test.cpp
          transformation_add_constant_scalar_test.cpp
          transformation_add_copy_memory_test.cpp
          transformation_add_dead_block_test.cpp
          transformation_add_dead_break_test.cpp
          transformation_add_dead_continue_test.cpp
          transformation_add_early_terminator_wrapper_test.cpp
          transformation_add_function_test.cpp
          transformation_add_global_undef_test.cpp
          transformation_add_global_variable_test.cpp
          transformation_add_image_sample_unused_components_test.cpp
          transformation_add_local_variable_test.cpp
          transformation_add_loop_preheader_test.cpp
          transformation_add_loop_to_create_int_constant_synonym_test.cpp
          transformation_add_no_contraction_decoration_test.cpp
          transformation_add_opphi_synonym_test.cpp
          transformation_add_parameter_test.cpp
          transformation_add_relaxed_decoration_test.cpp
          transformation_add_synonym_test.cpp
          transformation_add_type_array_test.cpp
          transformation_add_type_boolean_test.cpp
          transformation_add_type_float_test.cpp
          transformation_add_type_function_test.cpp
          transformation_add_type_int_test.cpp
          transformation_add_type_matrix_test.cpp
          transformation_add_type_pointer_test.cpp
          transformation_add_type_struct_test.cpp
          transformation_add_type_vector_test.cpp
          transformation_adjust_branch_weights_test.cpp
          transformation_composite_construct_test.cpp
          transformation_composite_extract_test.cpp
          transformation_composite_insert_test.cpp
          transformation_compute_data_synonym_fact_closure_test.cpp
          transformation_duplicate_region_with_selection_test.cpp
          transformation_equation_instruction_test.cpp
          transformation_expand_vector_reduction_test.cpp
          transformation_flatten_conditional_branch_test.cpp
          transformation_function_call_test.cpp
          transformation_inline_function_test.cpp
          transformation_invert_comparison_operator_test.cpp
          transformation_load_test.cpp
          transformation_make_vector_operation_dynamic_test.cpp
          transformation_merge_blocks_test.cpp
          transformation_merge_function_returns_test.cpp
          transformation_move_block_down_test.cpp
          transformation_move_instruction_down_test.cpp
          transformation_mutate_pointer_test.cpp
          transformation_outline_function_test.cpp
          transformation_permute_function_parameters_test.cpp
          transformation_permute_phi_operands_test.cpp
          transformation_propagate_instruction_down_test.cpp
          transformation_propagate_instruction_up_test.cpp
          transformation_push_id_through_variable_test.cpp
          transformation_replace_add_sub_mul_with_carrying_extended_test.cpp
          transformation_replace_boolean_constant_with_constant_binary_test.cpp
          transformation_replace_branch_from_dead_block_with_exit_test.cpp
          transformation_replace_copy_object_with_store_load_test.cpp
          transformation_replace_constant_with_uniform_test.cpp
          transformation_replace_copy_memory_with_load_store_test.cpp
          transformation_replace_id_with_synonym_test.cpp
          transformation_replace_irrelevant_id_test.cpp
          transformation_replace_linear_algebra_instruction_test.cpp
          transformation_replace_load_store_with_copy_memory_test.cpp
          transformation_replace_opphi_id_from_dead_predecessor_test.cpp
          transformation_replace_opselect_with_conditional_branch_test.cpp
          transformation_replace_parameter_with_global_test.cpp
          transformation_replace_params_with_struct_test.cpp
          transformation_set_function_control_test.cpp
          transformation_set_loop_control_test.cpp
          transformation_set_memory_operands_mask_test.cpp
          transformation_set_selection_control_test.cpp
          transformation_split_block_test.cpp
          transformation_store_test.cpp
          transformation_swap_commutable_operands_test.cpp
          transformation_swap_conditional_branch_operands_test.cpp
          transformation_swap_function_variables_test.cpp
          transformation_swap_two_functions_test.cpp
          transformation_toggle_access_chain_instruction_test.cpp
          transformation_record_synonymous_constants_test.cpp
          transformation_vector_shuffle_test.cpp
          transformation_wrap_early_terminator_in_function_test.cpp
          transformation_wrap_region_in_selection_test.cpp
          transformation_wrap_vector_synonym_test.cpp
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
