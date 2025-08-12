LOCAL_PATH := $(call my-dir)
SPVTOOLS_OUT_PATH=$(if $(call host-path-is-absolute,$(TARGET_OUT)),$(TARGET_OUT),$(abspath $(TARGET_OUT)))

ifeq ($(SPVHEADERS_LOCAL_PATH),)
	SPVHEADERS_LOCAL_PATH := $(LOCAL_PATH)/external/spirv-headers
endif

SPVTOOLS_SRC_FILES := \
		source/assembly_grammar.cpp \
		source/binary.cpp \
		source/diagnostic.cpp \
		source/disassemble.cpp \
		source/ext_inst.cpp \
		source/extensions.cpp \
		source/libspirv.cpp \
		source/name_mapper.cpp \
		source/opcode.cpp \
		source/operand.cpp \
		source/parsed_operand.cpp \
		source/print.cpp \
		source/software_version.cpp \
		source/spirv_endian.cpp \
		source/spirv_optimizer_options.cpp \
		source/spirv_target_env.cpp \
		source/spirv_validator_options.cpp \
		source/table.cpp \
		source/table2.cpp \
		source/text.cpp \
		source/text_handler.cpp \
		source/to_string.cpp \
		source/util/bit_vector.cpp \
		source/util/parse_number.cpp \
		source/util/string_utils.cpp \
		source/util/timer.cpp \
		source/val/basic_block.cpp \
		source/val/construct.cpp \
		source/val/function.cpp \
		source/val/instruction.cpp \
		source/val/validation_state.cpp \
		source/val/validate.cpp \
		source/val/validate_adjacency.cpp \
		source/val/validate_annotation.cpp \
		source/val/validate_arithmetics.cpp \
		source/val/validate_atomics.cpp \
		source/val/validate_barriers.cpp \
		source/val/validate_bitwise.cpp \
		source/val/validate_builtins.cpp \
		source/val/validate_capability.cpp \
		source/val/validate_cfg.cpp \
		source/val/validate_composites.cpp \
		source/val/validate_constants.cpp \
		source/val/validate_conversion.cpp \
		source/val/validate_debug.cpp \
		source/val/validate_decorations.cpp \
		source/val/validate_derivatives.cpp \
		source/val/validate_extensions.cpp \
		source/val/validate_execution_limitations.cpp \
		source/val/validate_function.cpp \
		source/val/validate_id.cpp \
		source/val/validate_image.cpp \
		source/val/validate_interfaces.cpp \
		source/val/validate_instruction.cpp \
		source/val/validate_memory.cpp \
		source/val/validate_memory_semantics.cpp \
		source/val/validate_mesh_shading.cpp \
		source/val/validate_misc.cpp \
		source/val/validate_mode_setting.cpp \
		source/val/validate_layout.cpp \
		source/val/validate_literals.cpp \
		source/val/validate_logicals.cpp \
		source/val/validate_non_uniform.cpp \
		source/val/validate_primitives.cpp \
		source/val/validate_ray_query.cpp \
		source/val/validate_ray_tracing.cpp \
		source/val/validate_ray_tracing_reorder.cpp \
		source/val/validate_scopes.cpp \
		source/val/validate_small_type_uses.cpp \
		source/val/validate_tensor.cpp \
		source/val/validate_tensor_layout.cpp \
		source/val/validate_type.cpp\
		source/val/validate_invalid_type.cpp

SPVTOOLS_OPT_SRC_FILES := \
		source/opt/aggressive_dead_code_elim_pass.cpp \
		source/opt/amd_ext_to_khr.cpp \
		source/opt/analyze_live_input_pass.cpp \
		source/opt/basic_block.cpp \
		source/opt/block_merge_pass.cpp \
		source/opt/block_merge_util.cpp \
		source/opt/build_module.cpp \
		source/opt/cfg.cpp \
		source/opt/cfg_cleanup_pass.cpp \
		source/opt/ccp_pass.cpp \
		source/opt/code_sink.cpp \
		source/opt/combine_access_chains.cpp \
		source/opt/compact_ids_pass.cpp \
		source/opt/composite.cpp \
		source/opt/const_folding_rules.cpp \
		source/opt/constants.cpp \
		source/opt/control_dependence.cpp \
		source/opt/convert_to_sampled_image_pass.cpp \
		source/opt/convert_to_half_pass.cpp \
		source/opt/copy_prop_arrays.cpp \
		source/opt/dataflow.cpp \
		source/opt/dead_branch_elim_pass.cpp \
		source/opt/dead_insert_elim_pass.cpp \
		source/opt/dead_variable_elimination.cpp \
		source/opt/decoration_manager.cpp \
		source/opt/debug_info_manager.cpp \
		source/opt/def_use_manager.cpp \
		source/opt/desc_sroa.cpp \
		source/opt/desc_sroa_util.cpp \
		source/opt/dominator_analysis.cpp \
		source/opt/dominator_tree.cpp \
		source/opt/eliminate_dead_constant_pass.cpp \
		source/opt/eliminate_dead_functions_pass.cpp \
		source/opt/eliminate_dead_functions_util.cpp \
		source/opt/eliminate_dead_io_components_pass.cpp \
		source/opt/eliminate_dead_members_pass.cpp \
		source/opt/eliminate_dead_output_stores_pass.cpp \
		source/opt/feature_manager.cpp \
		source/opt/fix_func_call_arguments.cpp \
		source/opt/fix_storage_class.cpp \
		source/opt/flatten_decoration_pass.cpp \
		source/opt/fold.cpp \
		source/opt/folding_rules.cpp \
		source/opt/fold_spec_constant_op_and_composite_pass.cpp \
		source/opt/freeze_spec_constant_value_pass.cpp \
		source/opt/function.cpp \
		source/opt/graphics_robust_access_pass.cpp \
		source/opt/if_conversion.cpp \
		source/opt/inline_pass.cpp \
		source/opt/inline_exhaustive_pass.cpp \
		source/opt/inline_opaque_pass.cpp \
		source/opt/instruction.cpp \
		source/opt/instruction_list.cpp \
		source/opt/interface_var_sroa.cpp \
		source/opt/interp_fixup_pass.cpp \
		source/opt/invocation_interlock_placement_pass.cpp \
		source/opt/ir_context.cpp \
		source/opt/ir_loader.cpp \
		source/opt/licm_pass.cpp \
		source/opt/liveness.cpp \
		source/opt/local_access_chain_convert_pass.cpp \
		source/opt/local_redundancy_elimination.cpp \
		source/opt/local_single_block_elim_pass.cpp \
		source/opt/local_single_store_elim_pass.cpp \
		source/opt/loop_dependence.cpp \
		source/opt/loop_dependence_helpers.cpp \
		source/opt/loop_descriptor.cpp \
		source/opt/loop_fission.cpp \
		source/opt/loop_fusion.cpp \
		source/opt/loop_fusion_pass.cpp \
		source/opt/loop_peeling.cpp \
		source/opt/loop_unroller.cpp \
		source/opt/loop_unswitch_pass.cpp \
		source/opt/loop_utils.cpp \
		source/opt/mem_pass.cpp \
		source/opt/merge_return_pass.cpp \
		source/opt/modify_maximal_reconvergence.cpp \
		source/opt/module.cpp \
		source/opt/opextinst_forward_ref_fixup_pass.cpp \
		source/opt/optimizer.cpp \
		source/opt/pass.cpp \
		source/opt/pass_manager.cpp \
		source/opt/private_to_local_pass.cpp \
		source/opt/propagator.cpp \
		source/opt/reduce_load_size.cpp \
		source/opt/redundancy_elimination.cpp \
		source/opt/register_pressure.cpp \
		source/opt/relax_float_ops_pass.cpp \
		source/opt/canonicalize_ids_pass.cpp \
		source/opt/remove_dontinline_pass.cpp \
		source/opt/remove_duplicates_pass.cpp \
		source/opt/remove_unused_interface_variables_pass.cpp \
		source/opt/replace_desc_array_access_using_var_index.cpp \
		source/opt/replace_invalid_opc.cpp \
		source/opt/resolve_binding_conflicts_pass.cpp \
		source/opt/scalar_analysis.cpp \
		source/opt/scalar_analysis_simplification.cpp \
		source/opt/scalar_replacement_pass.cpp \
		source/opt/set_spec_constant_default_value_pass.cpp \
		source/opt/simplification_pass.cpp \
		source/opt/split_combined_image_sampler_pass.cpp \
		source/opt/spread_volatile_semantics.cpp \
		source/opt/ssa_rewrite_pass.cpp \
		source/opt/strength_reduction_pass.cpp \
		source/opt/strip_debug_info_pass.cpp \
		source/opt/strip_nonsemantic_info_pass.cpp \
		source/opt/struct_cfg_analysis.cpp \
		source/opt/struct_packing_pass.cpp \
		source/opt/switch_descriptorset_pass.cpp \
		source/opt/trim_capabilities_pass.cpp \
		source/opt/type_manager.cpp \
		source/opt/types.cpp \
		source/opt/unify_const_pass.cpp \
		source/opt/upgrade_memory_model.cpp \
		source/opt/value_number_table.cpp \
		source/opt/vector_dce.cpp \
		source/opt/workaround1209.cpp \
		source/opt/wrap_opkill.cpp

# Locations of grammar files.
GRAMMAR_DIR=$(SPVHEADERS_LOCAL_PATH)/include/spirv/unified1

define gen_spvtools_grammar_tables
# $1 is the output directory, which is unique per ABI.
# Rules for creating grammar tables. They are statically compiled
# into the SPIRV-Tools code.
$(call generate-file-dir,$(1)/core_tables_body.inc)
$(1)/core_tables_body.inc \
$(1)/core_tables_header.inc \
: \
        $(LOCAL_PATH)/utils/ggt.py \
	$(GRAMMAR_DIR)/extinst.debuginfo.grammar.json \
	$(GRAMMAR_DIR)/extinst.glsl.std.450.grammar.json \
	$(GRAMMAR_DIR)/extinst.nonsemantic.clspvreflection.grammar.json \
	$(GRAMMAR_DIR)/extinst.nonsemantic.shader.debuginfo.100.grammar.json \
	$(GRAMMAR_DIR)/extinst.nonsemantic.vkspreflection.grammar.json \
	$(GRAMMAR_DIR)/extinst.opencl.debuginfo.100.grammar.json \
	$(GRAMMAR_DIR)/extinst.opencl.std.100.grammar.json \
	$(GRAMMAR_DIR)/extinst.spv-amd-gcn-shader.grammar.json \
	$(GRAMMAR_DIR)/extinst.spv-amd-shader-ballot.grammar.json \
	$(GRAMMAR_DIR)/extinst.spv-amd-shader-explicit-vertex-parameter.grammar.json \
	$(GRAMMAR_DIR)/extinst.spv-amd-shader-trinary-minmax.grammar.json \
	$(GRAMMAR_DIR)/spirv.core.grammar.json
	@$(HOST_PYTHON) $(LOCAL_PATH)/utils/ggt.py \
		--core-tables-body-output=$(1)/core_tables_body.inc \
		--core-tables-header-output=$(1)/core_tables_header.inc \
		--spirv-core-grammar=$(GRAMMAR_DIR)/spirv.core.grammar.json \
		--extinst=,$(GRAMMAR_DIR)/extinst.debuginfo.grammar.json \
		--extinst=,$(GRAMMAR_DIR)/extinst.glsl.std.450.grammar.json \
		--extinst=,$(GRAMMAR_DIR)/extinst.nonsemantic.clspvreflection.grammar.json \
		--extinst=SHDEBUG100_,$(GRAMMAR_DIR)/extinst.nonsemantic.shader.debuginfo.100.grammar.json \
		--extinst=,$(GRAMMAR_DIR)/extinst.nonsemantic.vkspreflection.grammar.json \
		--extinst=CLDEBUG100_,$(GRAMMAR_DIR)/extinst.opencl.debuginfo.100.grammar.json \
		--extinst=,$(GRAMMAR_DIR)/extinst.opencl.std.100.grammar.json \
		--extinst=,$(GRAMMAR_DIR)/extinst.spv-amd-gcn-shader.grammar.json \
		--extinst=,$(GRAMMAR_DIR)/extinst.spv-amd-shader-ballot.grammar.json \
		--extinst=,$(GRAMMAR_DIR)/extinst.spv-amd-shader-explicit-vertex-parameter.grammar.json \
		--extinst=,$(GRAMMAR_DIR)/extinst.spv-amd-shader-trinary-minmax.grammar.json
	@echo "[$(TARGET_ARCH_ABI)] Grammar tables <= grammar JSON files"
# Make all source files depend on the generated core tables
$(foreach F,$(SPVTOOLS_SRC_FILES) $(SPVTOOLS_OPT_SRC_FILES),$(LOCAL_PATH)/$F ) \
  : $(1)/core_tables_body.inc \
    $(1)/core_tables_header.inc
endef
$(eval $(call gen_spvtools_grammar_tables,$(SPVTOOLS_OUT_PATH)))


define gen_spvtools_lang_headers
# Generate language-specific headers.  So far we only generate C headers
# $1 is the output directory.
# $2 is the base name of the header file, e.g. "DebugInfo".
# $3 is the grammar file containing token definitions.
$(call generate-file-dir,$(1)/$(2).h)
$(1)/$(2).h : \
        $(LOCAL_PATH)/utils/generate_language_headers.py \
        $(3)
		@$(HOST_PYTHON) $(LOCAL_PATH)/utils/generate_language_headers.py \
		    --extinst-grammar=$(3) \
		    --extinst-output-path=$(1)/$(2).h
		@echo "[$(TARGET_ARCH_ABI)] Generate language specific header for $(2): headers <= grammar"
$(foreach F,$(SPVTOOLS_SRC_FILES) $(SPVTOOLS_OPT_SRC_FILES),$(LOCAL_PATH)/$F ) \
	: $(1)/$(2).h
endef


# Generate C++ headers for some extended instruction sets.
$(eval $(call gen_spvtools_lang_headers,$(SPVTOOLS_OUT_PATH),DebugInfo,$(GRAMMAR_DIR)/extinst.debuginfo.grammar.json))
$(eval $(call gen_spvtools_lang_headers,$(SPVTOOLS_OUT_PATH),OpenCLDebugInfo100,$(GRAMMAR_DIR)/extinst.opencl.debuginfo.100.grammar.json))
$(eval $(call gen_spvtools_lang_headers,$(SPVTOOLS_OUT_PATH),NonSemanticShaderDebugInfo100,$(GRAMMAR_DIR)/extinst.nonsemantic.shader.debuginfo.100.grammar.json))


define gen_spvtools_build_version_inc
$(call generate-file-dir,$(1)/dummy_filename)
$(1)/build-version.inc: \
        $(LOCAL_PATH)/utils/update_build_version.py \
        $(LOCAL_PATH)/CHANGES
		@$(HOST_PYTHON) $(LOCAL_PATH)/utils/update_build_version.py \
		                $(LOCAL_PATH)/CHANGES $(1)/build-version.inc
		@echo "[$(TARGET_ARCH_ABI)] Generate       : build-version.inc <= CHANGES"
$(LOCAL_PATH)/source/software_version.cpp: $(1)/build-version.inc
endef
$(eval $(call gen_spvtools_build_version_inc,$(SPVTOOLS_OUT_PATH)))

define gen_spvtools_generators_inc
$(call generate-file-dir,$(1)/dummy_filename)
$(1)/generators.inc: \
        $(LOCAL_PATH)/utils/generate_registry_tables.py \
        $(SPVHEADERS_LOCAL_PATH)/include/spirv/spir-v.xml
		@$(HOST_PYTHON) $(LOCAL_PATH)/utils/generate_registry_tables.py \
		                --xml=$(SPVHEADERS_LOCAL_PATH)/include/spirv/spir-v.xml \
				--generator-output=$(1)/generators.inc
		@echo "[$(TARGET_ARCH_ABI)] Generate       : generators.inc <= spir-v.xml"
$(LOCAL_PATH)/source/opcode.cpp: $(1)/generators.inc
endef
$(eval $(call gen_spvtools_generators_inc,$(SPVTOOLS_OUT_PATH)))

include $(CLEAR_VARS)
LOCAL_MODULE := SPIRV-Tools
LOCAL_C_INCLUDES := \
		$(LOCAL_PATH)/include \
		$(SPVHEADERS_LOCAL_PATH)/include \
		$(SPVTOOLS_OUT_PATH)
LOCAL_EXPORT_C_INCLUDES := \
		$(LOCAL_PATH)/include
LOCAL_CXXFLAGS:=-std=c++17 -fno-exceptions -fno-rtti -Werror
LOCAL_SRC_FILES:= $(SPVTOOLS_SRC_FILES)
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := SPIRV-Tools-opt
LOCAL_C_INCLUDES := \
		$(LOCAL_PATH)/include \
		$(LOCAL_PATH)/source \
		$(SPVHEADERS_LOCAL_PATH)/include \
		$(SPVTOOLS_OUT_PATH)
LOCAL_CXXFLAGS:=-std=c++17 -fno-exceptions -fno-rtti -Werror
LOCAL_STATIC_LIBRARIES:=SPIRV-Tools
LOCAL_SRC_FILES:= $(SPVTOOLS_OPT_SRC_FILES)
include $(BUILD_STATIC_LIBRARY)
