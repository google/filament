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

add_spvtools_unittest(TARGET opt
  SRCS aggressive_dead_code_elim_test.cpp
       amd_ext_to_khr.cpp
       assembly_builder_test.cpp
       block_merge_test.cpp
       ccp_test.cpp
       cfg_cleanup_test.cpp
       cfg_test.cpp
       code_sink_test.cpp
       combine_access_chains_test.cpp
       compact_ids_test.cpp
       constants_test.cpp
       constant_manager_test.cpp
       control_dependence.cpp
       convert_relaxed_to_half_test.cpp
       convert_to_sampled_image_test.cpp
       copy_prop_array_test.cpp
       dataflow.cpp
       dead_branch_elim_test.cpp
       dead_insert_elim_test.cpp
       dead_variable_elim_test.cpp
       debug_info_manager_test.cpp
       decoration_manager_test.cpp
       def_use_test.cpp
       desc_sroa_test.cpp
       eliminate_dead_const_test.cpp
       eliminate_dead_functions_test.cpp
       eliminate_dead_input_components_test.cpp
       eliminate_dead_member_test.cpp
       feature_manager_test.cpp
       fix_func_call_arguments_test.cpp
       fix_storage_class_test.cpp
       flatten_decoration_test.cpp
       fold_spec_const_op_composite_test.cpp
       fold_test.cpp
       freeze_spec_const_test.cpp
       function_test.cpp
       graphics_robust_access_test.cpp
       if_conversion_test.cpp
       inline_opaque_test.cpp
       inline_test.cpp
       insert_extract_elim_test.cpp
       inst_bindless_check_test.cpp
       inst_buff_addr_check_test.cpp
       inst_debug_printf_test.cpp
       instruction_list_test.cpp
       instruction_test.cpp
       interface_var_sroa_test.cpp
       interp_fixup_test.cpp
       ir_builder.cpp
       ir_context_test.cpp
       ir_loader_test.cpp
       iterator_test.cpp
       line_debug_info_test.cpp
       local_access_chain_convert_test.cpp
       local_redundancy_elimination_test.cpp
       local_single_block_elim.cpp
       local_single_store_elim_test.cpp
       local_ssa_elim_test.cpp
       module_test.cpp
       module_utils.h
       optimizer_test.cpp
       pass_manager_test.cpp
       pass_merge_return_test.cpp
       pass_remove_duplicates_test.cpp
       pass_utils.cpp
       private_to_local_test.cpp
       propagator_test.cpp
       reduce_load_size_test.cpp
       redundancy_elimination_test.cpp
       remove_dontinline_test.cpp
       remove_unused_interface_variables_test.cpp
       register_liveness.cpp
       relax_float_ops_test.cpp
       replace_desc_array_access_using_var_index_test.cpp
       replace_invalid_opc_test.cpp
       scalar_analysis.cpp
       scalar_replacement_test.cpp
       set_spec_const_default_value_test.cpp
       simplification_test.cpp
       spread_volatile_semantics_test.cpp
       strength_reduction_test.cpp
       strip_debug_info_test.cpp
       strip_nonsemantic_info_test.cpp
       struct_cfg_analysis_test.cpp
       type_manager_test.cpp
       types_test.cpp
       unify_const_test.cpp
       upgrade_memory_model_test.cpp
       utils_test.cpp pass_utils.cpp
       value_table_test.cpp
       vector_dce_test.cpp
       workaround1209_test.cpp
       wrap_opkill_test.cpp
  LIBS SPIRV-Tools-opt
  PCH_FILE pch_test_opt
)
