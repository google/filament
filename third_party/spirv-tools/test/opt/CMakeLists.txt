# Copyright (c) 2016 Google Inc.
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

add_subdirectory(dominator_tree)
add_subdirectory(loop_optimizations)

add_spvtools_unittest(TARGET instruction
  SRCS instruction_test.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET instruction_list
  SRCS instruction_list_test.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET ir_loader
  SRCS ir_loader_test.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_manager
  SRCS module_utils.h
       pass_manager_test.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET optimizer
  SRCS optimizer_test.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_strip_debug_info
  SRCS strip_debug_info_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_strip_reflect_info
  SRCS strip_reflect_info_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_compact_ids
  SRCS compact_ids_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_flatten_decoration
  SRCS flatten_decoration_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_freeze_spec_const
  SRCS freeze_spec_const_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_block_merge
  SRCS block_merge_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_inline
  SRCS inline_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_inline_opaque
  SRCS inline_opaque_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_insert_extract_elim
  SRCS insert_extract_elim_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_dead_insert_elim
  SRCS dead_insert_elim_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_local_ssa_elim
  SRCS local_ssa_elim_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_local_single_block_elim
  SRCS local_single_block_elim.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_local_access_chain_convert
  SRCS local_access_chain_convert_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_local_single_store_elim
  SRCS local_single_store_elim_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_dead_branch_elim
  SRCS dead_branch_elim_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_dead_variable_elim
  SRCS dead_variable_elim_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_aggressive_dce
  SRCS aggressive_dead_code_elim_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_common_uniform_elim
  SRCS common_uniform_elim_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_eliminate_dead_const
  SRCS eliminate_dead_const_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_eliminate_dead_functions
  SRCS eliminate_dead_functions_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_pass
  SRCS pass_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_utils
  SRCS utils_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET def_use
  SRCS def_use_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET assembly_builder
  SRCS assembly_builder_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET types
  SRCS types_test.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET type_manager
  SRCS type_manager_test.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET iterator
  SRCS iterator_test.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET module
  SRCS module_utils.h
       module_test.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_fold_spec_const_op_composite
  SRCS fold_spec_const_op_composite_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_unify_const
  SRCS unify_const_test.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_set_spec_const_default_value
  SRCS set_spec_const_default_value_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET line_debug_info
  SRCS line_debug_info_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_strength_reduction
  SRCS strength_reduction_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_scalar_replacement
  SRCS scalar_replacement_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET cfg_cleanup
  SRCS cfg_cleanup_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET ir_context
  SRCS ir_context_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET feature_manager
  SRCS feature_manager_test.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_merge_return
  SRCS pass_merge_return_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET value_table
  SRCS value_table_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET local_redundancy_elimination
  SRCS local_redundancy_elimination_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET propagator
  SRCS propagator_test.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET redundancy_elimination
  SRCS redundancy_elimination_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET private_to_local
  SRCS private_to_local_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET decoration_manager
  SRCS decoration_manager_test.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_remove_duplicates
  SRCS pass_remove_duplicates_test.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET ccp
  SRCS ccp_test.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_workaround1209
  SRCS workaround1209_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET pass_if_conversion
  SRCS if_conversion_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET ir_builder
  SRCS ir_builder.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET instruction_folding
  SRCS fold_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET replace_invalid_opc
  SRCS replace_invalid_opc_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET register_liveness
  SRCS register_liveness.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET simplification
  SRCS simplification_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET copy_prop_array
  SRCS copy_prop_array_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET scalar_analysis
  SRCS scalar_analysis.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET vector_dce
  SRCS vector_dce_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET reduce_load_size
  SRCS reduce_load_size_test.cpp pass_utils.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET constant_manager
  SRCS constant_manager_test.cpp
  LIBS SPIRV-Tools-opt
)

add_spvtools_unittest(TARGET combine_access_chains
  SRCS combine_access_chains_test.cpp
  LIBS SPIRV-Tools-opt
)

