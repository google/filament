/*
 * Copyright 2016-2018 The Brenwill Workshop Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "spirv_msl.hpp"
#include "GLSL.std.450.h"

#include <algorithm>
#include <assert.h>
#include <numeric>

using namespace spv;
using namespace spirv_cross;
using namespace std;

static const uint32_t k_unknown_location = ~0u;

CompilerMSL::CompilerMSL(vector<uint32_t> spirv_, vector<MSLVertexAttr> *p_vtx_attrs,
                         vector<MSLResourceBinding> *p_res_bindings)
    : CompilerGLSL(move(spirv_))
{
	if (p_vtx_attrs)
		for (auto &va : *p_vtx_attrs)
			vtx_attrs_by_location[va.location] = &va;

	if (p_res_bindings)
		for (auto &rb : *p_res_bindings)
			resource_bindings.push_back(&rb);
}

CompilerMSL::CompilerMSL(const uint32_t *ir, size_t word_count, MSLVertexAttr *p_vtx_attrs, size_t vtx_attrs_count,
                         MSLResourceBinding *p_res_bindings, size_t res_bindings_count)
    : CompilerGLSL(ir, word_count)
{
	if (p_vtx_attrs)
		for (size_t i = 0; i < vtx_attrs_count; i++)
			vtx_attrs_by_location[p_vtx_attrs[i].location] = &p_vtx_attrs[i];

	if (p_res_bindings)
		for (size_t i = 0; i < res_bindings_count; i++)
			resource_bindings.push_back(&p_res_bindings[i]);
}

void CompilerMSL::build_implicit_builtins()
{
	if (need_subpass_input)
	{
		bool has_frag_coord = false;

		for (auto &id : ids)
		{
			if (id.get_type() != TypeVariable)
				continue;

			auto &var = id.get<SPIRVariable>();

			if (var.storage == StorageClassInput && meta[var.self].decoration.builtin &&
			    meta[var.self].decoration.builtin_type == BuiltInFragCoord)
			{
				builtin_frag_coord_id = var.self;
				has_frag_coord = true;
				break;
			}
		}

		if (!has_frag_coord)
		{
			uint32_t offset = increase_bound_by(3);
			uint32_t type_id = offset;
			uint32_t type_ptr_id = offset + 1;
			uint32_t var_id = offset + 2;

			// Create gl_FragCoord.
			SPIRType vec4_type;
			vec4_type.basetype = SPIRType::Float;
			vec4_type.width = 32;
			vec4_type.vecsize = 4;
			set<SPIRType>(type_id, vec4_type);

			SPIRType vec4_type_ptr;
			vec4_type_ptr = vec4_type;
			vec4_type_ptr.pointer = true;
			vec4_type_ptr.parent_type = type_id;
			vec4_type_ptr.storage = StorageClassInput;
			auto &ptr_type = set<SPIRType>(type_ptr_id, vec4_type_ptr);
			ptr_type.self = type_id;

			set<SPIRVariable>(var_id, type_ptr_id, StorageClassInput);
			set_decoration(var_id, DecorationBuiltIn, BuiltInFragCoord);
			builtin_frag_coord_id = var_id;
		}
	}
}

static string create_sampler_address(const char *prefix, MSLSamplerAddress addr)
{
	switch (addr)
	{
	case MSL_SAMPLER_ADDRESS_CLAMP_TO_EDGE:
		return join(prefix, "address::clamp_to_edge");
	case MSL_SAMPLER_ADDRESS_CLAMP_TO_ZERO:
		return join(prefix, "address::clamp_to_zero");
	case MSL_SAMPLER_ADDRESS_CLAMP_TO_BORDER:
		return join(prefix, "address::clamp_to_border");
	case MSL_SAMPLER_ADDRESS_REPEAT:
		return join(prefix, "address::repeat");
	case MSL_SAMPLER_ADDRESS_MIRRORED_REPEAT:
		return join(prefix, "address::mirrored_repeat");
	default:
		SPIRV_CROSS_THROW("Invalid sampler addressing mode.");
	}
}

void CompilerMSL::emit_entry_point_declarations()
{
	// FIXME: Get test coverage here ...

	// Emit constexpr samplers here.
	for (auto &samp : constexpr_samplers)
	{
		auto &var = get<SPIRVariable>(samp.first);
		auto &type = get<SPIRType>(var.basetype);
		if (type.basetype == SPIRType::Sampler)
			add_resource_name(samp.first);

		vector<string> args;
		auto &s = samp.second;

		if (s.coord != MSL_SAMPLER_COORD_NORMALIZED)
			args.push_back("coord::pixel");

		if (s.min_filter == s.mag_filter)
		{
			if (s.min_filter != MSL_SAMPLER_FILTER_NEAREST)
				args.push_back("filter::linear");
		}
		else
		{
			if (s.min_filter != MSL_SAMPLER_FILTER_NEAREST)
				args.push_back("min_filter::linear");
			if (s.mag_filter != MSL_SAMPLER_FILTER_NEAREST)
				args.push_back("mag_filter::linear");
		}

		switch (s.mip_filter)
		{
		case MSL_SAMPLER_MIP_FILTER_NONE:
			// Default
			break;
		case MSL_SAMPLER_MIP_FILTER_NEAREST:
			args.push_back("mip_filter::nearest");
			break;
		case MSL_SAMPLER_MIP_FILTER_LINEAR:
			args.push_back("mip_filter::linear");
			break;
		default:
			SPIRV_CROSS_THROW("Invalid mip filter.");
		}

		if (s.s_address == s.t_address && s.s_address == s.r_address)
		{
			if (s.s_address != MSL_SAMPLER_ADDRESS_CLAMP_TO_EDGE)
				args.push_back(create_sampler_address("", s.s_address));
		}
		else
		{
			if (s.s_address != MSL_SAMPLER_ADDRESS_CLAMP_TO_EDGE)
				args.push_back(create_sampler_address("s_", s.s_address));
			if (s.t_address != MSL_SAMPLER_ADDRESS_CLAMP_TO_EDGE)
				args.push_back(create_sampler_address("t_", s.t_address));
			if (s.r_address != MSL_SAMPLER_ADDRESS_CLAMP_TO_EDGE)
				args.push_back(create_sampler_address("r_", s.r_address));
		}

		if (s.compare_enable)
		{
			switch (s.compare_func)
			{
			case MSL_SAMPLER_COMPARE_FUNC_ALWAYS:
				args.push_back("compare_func::always");
				break;
			case MSL_SAMPLER_COMPARE_FUNC_NEVER:
				args.push_back("compare_func::never");
				break;
			case MSL_SAMPLER_COMPARE_FUNC_EQUAL:
				args.push_back("compare_func::equal");
				break;
			case MSL_SAMPLER_COMPARE_FUNC_NOT_EQUAL:
				args.push_back("compare_func::not_equal");
				break;
			case MSL_SAMPLER_COMPARE_FUNC_LESS:
				args.push_back("compare_func::less");
				break;
			case MSL_SAMPLER_COMPARE_FUNC_LESS_EQUAL:
				args.push_back("compare_func::less_equal");
				break;
			case MSL_SAMPLER_COMPARE_FUNC_GREATER:
				args.push_back("compare_func::greater");
				break;
			case MSL_SAMPLER_COMPARE_FUNC_GREATER_EQUAL:
				args.push_back("compare_func::greater_equal");
				break;
			default:
				SPIRV_CROSS_THROW("Invalid sampler compare function.");
			}
		}

		if (s.s_address == MSL_SAMPLER_ADDRESS_CLAMP_TO_BORDER || s.t_address == MSL_SAMPLER_ADDRESS_CLAMP_TO_BORDER ||
		    s.r_address == MSL_SAMPLER_ADDRESS_CLAMP_TO_BORDER)
		{
			switch (s.border_color)
			{
			case MSL_SAMPLER_BORDER_COLOR_OPAQUE_BLACK:
				args.push_back("border_color::opaque_black");
				break;
			case MSL_SAMPLER_BORDER_COLOR_OPAQUE_WHITE:
				args.push_back("border_color::opaque_white");
				break;
			case MSL_SAMPLER_BORDER_COLOR_TRANSPARENT_BLACK:
				args.push_back("border_color::transparent_black");
				break;
			default:
				SPIRV_CROSS_THROW("Invalid sampler border color.");
			}
		}

		if (s.anisotropy_enable)
			args.push_back(join("max_anisotropy(", s.max_anisotropy, ")"));
		if (s.lod_clamp_enable)
		{
			args.push_back(
			    join("lod_clamp(", convert_to_string(s.lod_clamp_min), ", ", convert_to_string(s.lod_clamp_max), ")"));
		}

		statement("constexpr sampler ",
		          type.basetype == SPIRType::SampledImage ? to_sampler_expression(samp.first) : to_name(samp.first),
		          "(", merge(args), ");");
	}
}

string CompilerMSL::compile()
{
	// Force a classic "C" locale, reverts when function returns
	ClassicLocale classic_locale;

	// Do not deal with GLES-isms like precision, older extensions and such.
	options.vulkan_semantics = true;
	options.es = false;
	options.version = 450;
	backend.float_literal_suffix = false;
	backend.half_literal_suffix = "h";
	backend.uint32_t_literal_suffix = true;
	backend.basic_int_type = "int";
	backend.basic_uint_type = "uint";
	backend.discard_literal = "discard_fragment()";
	backend.swizzle_is_function = false;
	backend.shared_is_implied = false;
	backend.use_initializer_list = true;
	backend.use_typed_initializer_list = true;
	backend.native_row_major_matrix = false;
	backend.flexible_member_array_supported = false;
	backend.can_declare_arrays_inline = false;
	backend.can_return_array = false;
	backend.boolean_mix_support = false;
	backend.allow_truncated_access_chain = true;

	is_rasterization_disabled = msl_options.disable_rasterization;

	replace_illegal_names();

	struct_member_padding.clear();

	build_function_control_flow_graphs_and_analyze();
	update_active_builtins();
	analyze_image_and_sampler_usage();
	build_implicit_builtins();

	fixup_image_load_store_access();

	set_enabled_interface_variables(get_active_interface_variables());

	// Preprocess OpCodes to extract the need to output additional header content
	preprocess_op_codes();

	// Create structs to hold input, output and uniform variables.
	// Do output first to ensure out. is declared at top of entry function.
	qual_pos_var_name = "";
	stage_out_var_id = add_interface_block(StorageClassOutput);
	stage_in_var_id = add_interface_block(StorageClassInput);
	stage_uniforms_var_id = add_interface_block(StorageClassUniformConstant);

	// Metal vertex functions that define no output must disable rasterization and return void.
	if (!stage_out_var_id)
		is_rasterization_disabled = true;

	// Convert the use of global variables to recursively-passed function parameters
	localize_global_variables();
	extract_global_variables_from_functions();

	// Mark any non-stage-in structs to be tightly packed.
	mark_packable_structs();

	// Metal does not allow dynamic array lengths.
	// Resolve any specialization constants that are used for array lengths.
	if (msl_options.resolve_specialized_array_lengths)
		resolve_specialized_array_lengths();

	uint32_t pass_count = 0;
	do
	{
		if (pass_count >= 3)
			SPIRV_CROSS_THROW("Over 3 compilation loops detected. Must be a bug!");

		reset();

		next_metal_resource_index = MSLResourceBinding(); // Start bindings at zero

		// Move constructor for this type is broken on GCC 4.9 ...
		buffer = unique_ptr<ostringstream>(new ostringstream());

		emit_header();
		emit_specialization_constants();
		emit_resources();
		emit_custom_functions();
		emit_function(get<SPIRFunction>(entry_point), Bitset());

		pass_count++;
	} while (force_recompile);

	return buffer->str();
}

string CompilerMSL::compile(vector<MSLVertexAttr> *p_vtx_attrs, vector<MSLResourceBinding> *p_res_bindings)
{
	if (p_vtx_attrs)
	{
		vtx_attrs_by_location.clear();
		for (auto &va : *p_vtx_attrs)
			vtx_attrs_by_location[va.location] = &va;
	}

	if (p_res_bindings)
	{
		resource_bindings.clear();
		for (auto &rb : *p_res_bindings)
			resource_bindings.push_back(&rb);
	}

	return compile();
}

string CompilerMSL::compile(MSLConfiguration &msl_cfg, vector<MSLVertexAttr> *p_vtx_attrs,
                            vector<MSLResourceBinding> *p_res_bindings)
{
	msl_options = msl_cfg;
	return compile(p_vtx_attrs, p_res_bindings);
}

// Register the need to output any custom functions.
void CompilerMSL::preprocess_op_codes()
{
	OpCodePreprocessor preproc(*this);
	traverse_all_reachable_opcodes(get<SPIRFunction>(entry_point), preproc);

	if (preproc.suppress_missing_prototypes)
		add_pragma_line("#pragma clang diagnostic ignored \"-Wmissing-prototypes\"");

	if (preproc.uses_atomics)
	{
		add_header_line("#include <metal_atomic>");
		add_pragma_line("#pragma clang diagnostic ignored \"-Wunused-variable\"");
	}

	// Metal vertex functions that write to resources must disable rasterization and return void.
	if (preproc.uses_resource_write)
		is_rasterization_disabled = true;
}

// Move the Private and Workgroup global variables to the entry function.
// Non-constant variables cannot have global scope in Metal.
void CompilerMSL::localize_global_variables()
{
	auto &entry_func = get<SPIRFunction>(entry_point);
	auto iter = global_variables.begin();
	while (iter != global_variables.end())
	{
		uint32_t v_id = *iter;
		auto &var = get<SPIRVariable>(v_id);
		if (var.storage == StorageClassPrivate || var.storage == StorageClassWorkgroup)
		{
			entry_func.add_local_variable(v_id);
			iter = global_variables.erase(iter);
		}
		else
			iter++;
	}
}

// Metal does not allow dynamic array lengths.
// Turn off specialization of any constants that are used for array lengths.
void CompilerMSL::resolve_specialized_array_lengths()
{
	for (auto &id : ids)
	{
		if (id.get_type() == TypeConstant)
		{
			auto &c = id.get<SPIRConstant>();
			if (c.is_used_as_array_length)
				c.specialization = false;
		}
	}
}

// For any global variable accessed directly by a function,
// extract that variable and add it as an argument to that function.
void CompilerMSL::extract_global_variables_from_functions()
{
	// Uniforms
	unordered_set<uint32_t> global_var_ids;
	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();
			if (var.storage == StorageClassInput || var.storage == StorageClassOutput ||
			    var.storage == StorageClassUniform || var.storage == StorageClassUniformConstant ||
			    var.storage == StorageClassPushConstant || var.storage == StorageClassStorageBuffer)
			{
				global_var_ids.insert(var.self);
			}
		}
	}

	// Local vars that are declared in the main function and accessed directly by a function
	auto &entry_func = get<SPIRFunction>(entry_point);
	for (auto &var : entry_func.local_variables)
		if (get<SPIRVariable>(var).storage != StorageClassFunction)
			global_var_ids.insert(var);

	std::set<uint32_t> added_arg_ids;
	unordered_set<uint32_t> processed_func_ids;
	extract_global_variables_from_function(entry_point, added_arg_ids, global_var_ids, processed_func_ids);
}

// MSL does not support the use of global variables for shader input content.
// For any global variable accessed directly by the specified function, extract that variable,
// add it as an argument to that function, and the arg to the added_arg_ids collection.
void CompilerMSL::extract_global_variables_from_function(uint32_t func_id, std::set<uint32_t> &added_arg_ids,
                                                         unordered_set<uint32_t> &global_var_ids,
                                                         unordered_set<uint32_t> &processed_func_ids)
{
	// Avoid processing a function more than once
	if (processed_func_ids.find(func_id) != processed_func_ids.end())
	{
		// Return function global variables
		added_arg_ids = function_global_vars[func_id];
		return;
	}

	processed_func_ids.insert(func_id);

	auto &func = get<SPIRFunction>(func_id);

	// Recursively establish global args added to functions on which we depend.
	for (auto block : func.blocks)
	{
		auto &b = get<SPIRBlock>(block);
		for (auto &i : b.ops)
		{
			auto ops = stream(i);
			auto op = static_cast<Op>(i.op);

			switch (op)
			{
			case OpLoad:
			case OpInBoundsAccessChain:
			case OpAccessChain:
			{
				uint32_t base_id = ops[2];
				if (global_var_ids.find(base_id) != global_var_ids.end())
					added_arg_ids.insert(base_id);

				auto &type = get<SPIRType>(ops[0]);
				if (type.basetype == SPIRType::Image && type.image.dim == DimSubpassData)
				{
					// Implicitly reads gl_FragCoord.
					assert(builtin_frag_coord_id != 0);
					added_arg_ids.insert(builtin_frag_coord_id);
				}

				break;
			}

			case OpFunctionCall:
			{
				// First see if any of the function call args are globals
				for (uint32_t arg_idx = 3; arg_idx < i.length; arg_idx++)
				{
					uint32_t arg_id = ops[arg_idx];
					if (global_var_ids.find(arg_id) != global_var_ids.end())
						added_arg_ids.insert(arg_id);
				}

				// Then recurse into the function itself to extract globals used internally in the function
				uint32_t inner_func_id = ops[2];
				std::set<uint32_t> inner_func_args;
				extract_global_variables_from_function(inner_func_id, inner_func_args, global_var_ids,
				                                       processed_func_ids);
				added_arg_ids.insert(inner_func_args.begin(), inner_func_args.end());
				break;
			}

			case OpStore:
			{
				uint32_t base_id = ops[0];
				if (global_var_ids.find(base_id) != global_var_ids.end())
					added_arg_ids.insert(base_id);
				break;
			}

			default:
				break;
			}

			// TODO: Add all other operations which can affect memory.
			// We should consider a more unified system here to reduce boiler-plate.
			// This kind of analysis is done in several places ...
		}
	}

	function_global_vars[func_id] = added_arg_ids;

	// Add the global variables as arguments to the function
	if (func_id != entry_point)
	{
		for (uint32_t arg_id : added_arg_ids)
		{
			auto &var = get<SPIRVariable>(arg_id);
			uint32_t type_id = var.basetype;
			auto *p_type = &get<SPIRType>(type_id);

			if (is_builtin_variable(var) && p_type->basetype == SPIRType::Struct)
			{
				// Get the non-pointer type
				type_id = get_non_pointer_type_id(type_id);
				p_type = &get<SPIRType>(type_id);

				uint32_t mbr_idx = 0;
				for (auto &mbr_type_id : p_type->member_types)
				{
					BuiltIn builtin;
					bool is_builtin = is_member_builtin(*p_type, mbr_idx, &builtin);
					if (is_builtin && has_active_builtin(builtin, var.storage))
					{
						// Add a arg variable with the same type and decorations as the member
						uint32_t next_ids = increase_bound_by(2);
						uint32_t ptr_type_id = next_ids + 0;
						uint32_t var_id = next_ids + 1;

						// Make sure we have an actual pointer type,
						// so that we will get the appropriate address space when declaring these builtins.
						auto &ptr = set<SPIRType>(ptr_type_id, get<SPIRType>(mbr_type_id));
						ptr.self = mbr_type_id;
						ptr.storage = var.storage;
						ptr.pointer = true;

						func.add_parameter(mbr_type_id, var_id, true);
						set<SPIRVariable>(var_id, ptr_type_id, StorageClassFunction);
						meta[var_id].decoration = meta[type_id].members[mbr_idx];
					}
					mbr_idx++;
				}
			}
			else
			{
				uint32_t next_id = increase_bound_by(1);
				func.add_parameter(type_id, next_id, true);
				set<SPIRVariable>(next_id, type_id, StorageClassFunction, 0, arg_id);

				// Ensure the existing variable has a valid name and the new variable has all the same meta info
				set_name(arg_id, ensure_valid_name(to_name(arg_id), "v"));
				meta[next_id] = meta[arg_id];
			}
		}
	}
}

// For all variables that are some form of non-input-output interface block, mark that all the structs
// that are recursively contained within the type referenced by that variable should be packed tightly.
void CompilerMSL::mark_packable_structs()
{
	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();
			if (var.storage != StorageClassFunction && !is_hidden_variable(var))
			{
				auto &type = get<SPIRType>(var.basetype);
				if (type.pointer &&
				    (type.storage == StorageClassUniform || type.storage == StorageClassUniformConstant ||
				     type.storage == StorageClassPushConstant || type.storage == StorageClassStorageBuffer) &&
				    (has_decoration(type.self, DecorationBlock) || has_decoration(type.self, DecorationBufferBlock)))
					mark_as_packable(type);
			}
		}
	}
}

// If the specified type is a struct, it and any nested structs
// are marked as packable with the DecorationCPacked decoration,
void CompilerMSL::mark_as_packable(SPIRType &type)
{
	// If this is not the base type (eg. it's a pointer or array), tunnel down
	if (type.parent_type)
	{
		mark_as_packable(get<SPIRType>(type.parent_type));
		return;
	}

	if (type.basetype == SPIRType::Struct)
	{
		set_decoration(type.self, DecorationCPacked);

		// Recurse
		size_t mbr_cnt = type.member_types.size();
		for (uint32_t mbr_idx = 0; mbr_idx < mbr_cnt; mbr_idx++)
		{
			uint32_t mbr_type_id = type.member_types[mbr_idx];
			auto &mbr_type = get<SPIRType>(mbr_type_id);
			mark_as_packable(mbr_type);
			if (mbr_type.type_alias)
			{
				auto &mbr_type_alias = get<SPIRType>(mbr_type.type_alias);
				mark_as_packable(mbr_type_alias);
			}
		}
	}
}

// If a vertex attribute exists at the location, it is marked as being used by this shader
void CompilerMSL::mark_location_as_used_by_shader(uint32_t location, StorageClass storage)
{
	MSLVertexAttr *p_va;
	if ((get_entry_point().model == ExecutionModelVertex) && (storage == StorageClassInput) &&
	    (p_va = vtx_attrs_by_location[location]))
		p_va->used_by_shader = true;
}

// Add an interface structure for the type of storage, which is either StorageClassInput or StorageClassOutput.
// Returns the ID of the newly added variable, or zero if no variable was added.
uint32_t CompilerMSL::add_interface_block(StorageClass storage)
{
	// Accumulate the variables that should appear in the interface struct
	vector<SPIRVariable *> vars;
	bool incl_builtins = (storage == StorageClassOutput);
	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();
			auto &type = get<SPIRType>(var.basetype);
			if (var.storage == storage && interface_variable_exists_in_entry_point(var.self) &&
			    !is_hidden_variable(var, incl_builtins) && type.pointer)
			{
				vars.push_back(&var);
			}
		}
	}

	// If no variables qualify, leave
	if (vars.empty())
		return 0;

	// Add a new typed variable for this interface structure.
	// The initializer expression is allocated here, but populated when the function
	// declaraion is emitted, because it is cleared after each compilation pass.
	uint32_t next_id = increase_bound_by(3);
	uint32_t ib_type_id = next_id++;
	auto &ib_type = set<SPIRType>(ib_type_id);
	ib_type.basetype = SPIRType::Struct;
	ib_type.storage = storage;
	set_decoration(ib_type_id, DecorationBlock);

	uint32_t ib_var_id = next_id++;
	auto &var = set<SPIRVariable>(ib_var_id, ib_type_id, storage, 0);
	var.initializer = next_id++;

	string ib_var_ref;
	switch (storage)
	{
	case StorageClassInput:
		ib_var_ref = stage_in_var_name;
		break;

	case StorageClassOutput:
	{
		ib_var_ref = stage_out_var_name;

		// Add the output interface struct as a local variable to the entry function.
		// If the entry point should return the output struct, set the entry function
		// to return the output interface struct, otherwise to return nothing.
		// Indicate the output var requires early initialization.
		bool ep_should_return_output = !get_is_rasterization_disabled();
		uint32_t rtn_id = ep_should_return_output ? ib_var_id : 0;
		auto &entry_func = get<SPIRFunction>(entry_point);
		entry_func.add_local_variable(ib_var_id);
		for (auto &blk_id : entry_func.blocks)
		{
			auto &blk = get<SPIRBlock>(blk_id);
			if (blk.terminator == SPIRBlock::Return)
				blk.return_value = rtn_id;
		}
		vars_needing_early_declaration.push_back(ib_var_id);
		break;
	}

	case StorageClassUniformConstant:
	{
		ib_var_ref = stage_uniform_var_name;
		active_interface_variables.insert(ib_var_id); // Ensure will be emitted
		break;
	}

	default:
		break;
	}

	set_name(ib_type_id, to_name(entry_point) + "_" + ib_var_ref);
	set_name(ib_var_id, ib_var_ref);

	for (auto p_var : vars)
	{
		uint32_t type_id = p_var->basetype;
		auto &type = get<SPIRType>(type_id);
		if (type.basetype == SPIRType::Struct)
		{
			// Flatten the struct members into the interface struct
			uint32_t mbr_idx = 0;
			for (auto &mbr_type_id : type.member_types)
			{
				BuiltIn builtin;
				bool is_builtin = is_member_builtin(type, mbr_idx, &builtin);

				if (!is_builtin || has_active_builtin(builtin, storage))
				{
					// Add a reference to the member to the interface struct.
					uint32_t ib_mbr_idx = uint32_t(ib_type.member_types.size());
					mbr_type_id = ensure_correct_builtin_type(mbr_type_id, builtin);
					type.member_types[mbr_idx] = mbr_type_id;
					ib_type.member_types.push_back(mbr_type_id);

					// Give the member a name
					string mbr_name = ensure_valid_name(to_qualified_member_name(type, mbr_idx), "m");
					set_member_name(ib_type_id, ib_mbr_idx, mbr_name);

					// Update the original variable reference to include the structure reference
					string qual_var_name = ib_var_ref + "." + mbr_name;
					set_member_qualified_name(type_id, mbr_idx, qual_var_name);

					// Copy the variable location from the original variable to the member
					if (has_member_decoration(type_id, mbr_idx, DecorationLocation))
					{
						uint32_t locn = get_member_decoration(type_id, mbr_idx, DecorationLocation);
						set_member_decoration(ib_type_id, ib_mbr_idx, DecorationLocation, locn);
						mark_location_as_used_by_shader(locn, storage);
					}
					else if (has_decoration(p_var->self, DecorationLocation))
					{
						// The block itself might have a location and in this case, all members of the block
						// receive incrementing locations.
						uint32_t locn = get_decoration(p_var->self, DecorationLocation) + mbr_idx;
						set_member_decoration(ib_type_id, ib_mbr_idx, DecorationLocation, locn);
						mark_location_as_used_by_shader(locn, storage);
					}

					// Mark the member as builtin if needed
					if (is_builtin)
					{
						set_member_decoration(ib_type_id, ib_mbr_idx, DecorationBuiltIn, builtin);
						if (builtin == BuiltInPosition)
							qual_pos_var_name = qual_var_name;
					}
				}
				mbr_idx++;
			}
		}
		else if (type.basetype == SPIRType::Boolean || type.basetype == SPIRType::Char ||
		         type.basetype == SPIRType::Int || type.basetype == SPIRType::UInt ||
		         type.basetype == SPIRType::Int64 || type.basetype == SPIRType::UInt64 ||
		         type_is_floating_point(type) || type.basetype == SPIRType::Boolean)
		{
			bool is_builtin = is_builtin_variable(*p_var);
			BuiltIn builtin = BuiltIn(get_decoration(p_var->self, DecorationBuiltIn));

			if (!is_builtin || has_active_builtin(builtin, storage))
			{
				// MSL does not allow matrices or arrays in input or output variables, so need to handle it specially.
				if (!is_builtin && (storage == StorageClassInput || storage == StorageClassOutput) &&
				    (is_matrix(type) || is_array(type)))
				{
					uint32_t elem_cnt = 0;

					if (is_matrix(type))
					{
						if (is_array(type))
							SPIRV_CROSS_THROW("MSL cannot emit arrays-of-matrices in input and output variables.");

						elem_cnt = type.columns;
					}
					else if (is_array(type))
					{
						if (type.array.size() != 1)
							SPIRV_CROSS_THROW("MSL cannot emit arrays-of-arrays in input and output variables.");

						elem_cnt = type.array_size_literal.back() ? type.array.back() :
						                                            get<SPIRConstant>(type.array.back()).scalar();
					}

					auto *usable_type = &type;
					while (is_array(*usable_type) || is_matrix(*usable_type))
						usable_type = &get<SPIRType>(usable_type->parent_type);

					auto &entry_func = get<SPIRFunction>(entry_point);
					entry_func.add_local_variable(p_var->self);

					// We need to declare the variable early and at entry-point scope.
					vars_needing_early_declaration.push_back(p_var->self);

					for (uint32_t i = 0; i < elem_cnt; i++)
					{
						// Add a reference to the variable type to the interface struct.
						uint32_t ib_mbr_idx = uint32_t(ib_type.member_types.size());
						ib_type.member_types.push_back(usable_type->self);

						// Give the member a name
						string mbr_name = ensure_valid_name(join(to_expression(p_var->self), "_", i), "m");
						set_member_name(ib_type_id, ib_mbr_idx, mbr_name);

						// There is no qualified alias since we need to flatten the internal array on return.
						if (get_decoration_bitset(p_var->self).get(DecorationLocation))
						{
							uint32_t locn = get_decoration(p_var->self, DecorationLocation) + i;
							set_member_decoration(ib_type_id, ib_mbr_idx, DecorationLocation, locn);
							mark_location_as_used_by_shader(locn, storage);
						}

						if (get_decoration_bitset(p_var->self).get(DecorationIndex))
						{
							uint32_t index = get_decoration(p_var->self, DecorationIndex);
							set_member_decoration(ib_type_id, ib_mbr_idx, DecorationIndex, index);
						}

						switch (storage)
						{
						case StorageClassInput:
							entry_func.fixup_statements_in.push_back(
							    join(to_name(p_var->self), "[", i, "] = ", ib_var_ref, ".", mbr_name, ";"));
							break;

						case StorageClassOutput:
							entry_func.fixup_statements_out.push_back(
							    join(ib_var_ref, ".", mbr_name, " = ", to_name(p_var->self), "[", i, "];"));
							break;

						default:
							break;
						}
					}
				}
				else
				{
					// Add a reference to the variable type to the interface struct.
					uint32_t ib_mbr_idx = uint32_t(ib_type.member_types.size());
					type_id = ensure_correct_builtin_type(type_id, builtin);
					p_var->basetype = type_id;
					ib_type.member_types.push_back(type_id);

					// Give the member a name
					string mbr_name = ensure_valid_name(to_expression(p_var->self), "m");
					set_member_name(ib_type_id, ib_mbr_idx, mbr_name);

					// Update the original variable reference to include the structure reference
					string qual_var_name = ib_var_ref + "." + mbr_name;
					meta[p_var->self].decoration.qualified_alias = qual_var_name;

					// Copy the variable location from the original variable to the member
					if (get_decoration_bitset(p_var->self).get(DecorationLocation))
					{
						uint32_t locn = get_decoration(p_var->self, DecorationLocation);
						set_member_decoration(ib_type_id, ib_mbr_idx, DecorationLocation, locn);
						mark_location_as_used_by_shader(locn, storage);
					}

					if (get_decoration_bitset(p_var->self).get(DecorationIndex))
					{
						uint32_t index = get_decoration(p_var->self, DecorationIndex);
						set_member_decoration(ib_type_id, ib_mbr_idx, DecorationIndex, index);
					}

					// Mark the member as builtin if needed
					if (is_builtin)
					{
						set_member_decoration(ib_type_id, ib_mbr_idx, DecorationBuiltIn, builtin);
						if (builtin == BuiltInPosition)
							qual_pos_var_name = qual_var_name;
					}
				}
			}
		}
	}

	// Sort the members of the structure by their locations.
	MemberSorter member_sorter(ib_type, meta[ib_type_id], MemberSorter::Location);
	member_sorter.sort();

	return ib_var_id;
}

// Ensure that the type is compatible with the builtin.
// If it is, simply return the given type ID.
// Otherwise, create a new type, and return it's ID.
uint32_t CompilerMSL::ensure_correct_builtin_type(uint32_t type_id, BuiltIn builtin)
{
	auto &type = get<SPIRType>(type_id);

	if (builtin == BuiltInSampleMask && is_array(type))
	{
		uint32_t next_id = increase_bound_by(type.pointer ? 2 : 1);
		uint32_t base_type_id = next_id++;
		auto &base_type = set<SPIRType>(base_type_id);
		base_type.basetype = SPIRType::UInt;
		base_type.width = 32;

		if (!type.pointer)
			return base_type_id;

		uint32_t ptr_type_id = next_id++;
		auto &ptr_type = set<SPIRType>(ptr_type_id);
		ptr_type = base_type;
		ptr_type.pointer = true;
		ptr_type.storage = type.storage;
		ptr_type.parent_type = base_type_id;
		return ptr_type_id;
	}

	return type_id;
}

// Sort the members of the struct type by offset, and pack and then pad members where needed
// to align MSL members with SPIR-V offsets. The struct members are iterated twice. Packing
// occurs first, followed by padding, because packing a member reduces both its size and its
// natural alignment, possibly requiring a padding member to be added ahead of it.
void CompilerMSL::align_struct(SPIRType &ib_type)
{
	uint32_t &ib_type_id = ib_type.self;

	// Sort the members of the interface structure by their offset.
	// They should already be sorted per SPIR-V spec anyway.
	MemberSorter member_sorter(ib_type, meta[ib_type_id], MemberSorter::Offset);
	member_sorter.sort();

	uint32_t curr_offset;
	uint32_t mbr_cnt = uint32_t(ib_type.member_types.size());

	// Test the alignment of each member, and if a member should be closer to the previous
	// member than the default spacing expects, it is likely that the previous member is in
	// a packed format. If so, and the previous member is packable, pack it.
	// For example...this applies to any 3-element vector that is followed by a scalar.
	curr_offset = 0;
	for (uint32_t mbr_idx = 0; mbr_idx < mbr_cnt; mbr_idx++)
	{
		if (is_member_packable(ib_type, mbr_idx))
			set_member_decoration(ib_type_id, mbr_idx, DecorationCPacked);

		// Align current offset to the current member's default alignment.
		size_t align_mask = get_declared_struct_member_alignment(ib_type, mbr_idx) - 1;
		curr_offset = uint32_t((curr_offset + align_mask) & ~align_mask);

		// Fetch the member offset as declared in the SPIRV.
		uint32_t mbr_offset = get_member_decoration(ib_type_id, mbr_idx, DecorationOffset);
		if (mbr_offset > curr_offset)
		{
			// Since MSL and SPIR-V have slightly different struct member alignment and
			// size rules, we'll pad to standard C-packing rules. If the member is farther
			// away than C-packing, expects, add an inert padding member before the the member.
			MSLStructMemberKey key = get_struct_member_key(ib_type_id, mbr_idx);
			struct_member_padding[key] = mbr_offset - curr_offset;
		}

		// Increment the current offset to be positioned immediately after the current member.
		curr_offset = mbr_offset + uint32_t(get_declared_struct_member_size(ib_type, mbr_idx));
	}
}

// Returns whether the specified struct member supports a packable type
// variation that is smaller than the unpacked variation of that type.
bool CompilerMSL::is_member_packable(SPIRType &ib_type, uint32_t index)
{
	// We've already marked it as packable
	if (has_member_decoration(ib_type.self, index, DecorationCPacked))
		return true;

	auto &mbr_type = get<SPIRType>(ib_type.member_types[index]);

	// Only 3-element vectors or 3-row matrices need to be packed.
	if (mbr_type.vecsize != 3)
		return false;

	// Only row-major matrices need to be packed.
	if (is_matrix(mbr_type) && !has_member_decoration(ib_type.self, index, DecorationRowMajor))
		return false;

	uint32_t component_size = mbr_type.width / 8;
	uint32_t unpacked_mbr_size = component_size * (mbr_type.vecsize + 1) * mbr_type.columns;
	if (is_array(mbr_type))
	{
		// If member is an array, and the array stride is larger than the type needs, don't pack it.
		// Take into consideration multi-dimentional arrays.
		uint32_t md_elem_cnt = 1;
		size_t last_elem_idx = mbr_type.array.size() - 1;
		for (uint32_t i = 0; i < last_elem_idx; i++)
			md_elem_cnt *= max(to_array_size_literal(mbr_type, i), 1U);

		uint32_t unpacked_array_stride = unpacked_mbr_size * md_elem_cnt;
		uint32_t array_stride = type_struct_member_array_stride(ib_type, index);
		return unpacked_array_stride > array_stride;
	}
	else
	{
		// Pack if there is not enough space between this member and next.
		// If last member, only pack if it's a row-major matrix.
		if (index < ib_type.member_types.size() - 1)
		{
			uint32_t mbr_offset_curr = get_member_decoration(ib_type.self, index, DecorationOffset);
			uint32_t mbr_offset_next = get_member_decoration(ib_type.self, index + 1, DecorationOffset);
			return unpacked_mbr_size > mbr_offset_next - mbr_offset_curr;
		}
		else
			return is_matrix(mbr_type);
	}
}

// Returns a combination of type ID and member index for use as hash key
MSLStructMemberKey CompilerMSL::get_struct_member_key(uint32_t type_id, uint32_t index)
{
	MSLStructMemberKey k = type_id;
	k <<= 32;
	k += index;
	return k;
}

// Converts the format of the current expression from packed to unpacked,
// by wrapping the expression in a constructor of the appropriate type.
string CompilerMSL::unpack_expression_type(string expr_str, const SPIRType &type)
{
	return join(type_to_glsl(type), "(", expr_str, ")");
}

// Emits the file header info
void CompilerMSL::emit_header()
{
	for (auto &pragma : pragma_lines)
		statement(pragma);

	if (!pragma_lines.empty())
		statement("");

	statement("#include <metal_stdlib>");
	statement("#include <simd/simd.h>");

	for (auto &header : header_lines)
		statement(header);

	statement("");
	statement("using namespace metal;");
	statement("");

	for (auto &td : typedef_lines)
		statement(td);

	if (!typedef_lines.empty())
		statement("");
}

void CompilerMSL::add_pragma_line(const string &line)
{
	auto rslt = pragma_lines.insert(line);
	if (rslt.second)
		force_recompile = true;
}

void CompilerMSL::add_typedef_line(const string &line)
{
	auto rslt = typedef_lines.insert(line);
	if (rslt.second)
		force_recompile = true;
}

// Emits any needed custom function bodies.
void CompilerMSL::emit_custom_functions()
{
	for (auto &spv_func : spv_function_implementations)
	{
		switch (spv_func)
		{
		case SPVFuncImplMod:
			statement("// Implementation of the GLSL mod() function, which is slightly different than Metal fmod()");
			statement("template<typename Tx, typename Ty>");
			statement("Tx mod(Tx x, Ty y)");
			begin_scope();
			statement("return x - y * floor(x / y);");
			end_scope();
			statement("");
			break;

		case SPVFuncImplRadians:
			statement("// Implementation of the GLSL radians() function");
			statement("template<typename T>");
			statement("T radians(T d)");
			begin_scope();
			statement("return d * T(0.01745329251);");
			end_scope();
			statement("");
			break;

		case SPVFuncImplDegrees:
			statement("// Implementation of the GLSL degrees() function");
			statement("template<typename T>");
			statement("T degrees(T r)");
			begin_scope();
			statement("return r * T(57.2957795131);");
			end_scope();
			statement("");
			break;

		case SPVFuncImplFindILsb:
			statement("// Implementation of the GLSL findLSB() function");
			statement("template<typename T>");
			statement("T findLSB(T x)");
			begin_scope();
			statement("return select(ctz(x), T(-1), x == T(0));");
			end_scope();
			statement("");
			break;

		case SPVFuncImplFindUMsb:
			statement("// Implementation of the unsigned GLSL findMSB() function");
			statement("template<typename T>");
			statement("T findUMSB(T x)");
			begin_scope();
			statement("return select(clz(T(0)) - (clz(x) + T(1)), T(-1), x == T(0));");
			end_scope();
			statement("");
			break;

		case SPVFuncImplFindSMsb:
			statement("// Implementation of the signed GLSL findMSB() function");
			statement("template<typename T>");
			statement("T findSMSB(T x)");
			begin_scope();
			statement("T v = select(x, T(-1) - x, x < T(0));");
			statement("return select(clz(T(0)) - (clz(v) + T(1)), T(-1), v == T(0));");
			end_scope();
			statement("");
			break;

		case SPVFuncImplArrayCopy:
			statement("// Implementation of an array copy function to cover GLSL's ability to copy an array via "
			          "assignment.");
			statement("template<typename T, uint N>");
			statement("void spvArrayCopy(thread T (&dst)[N], thread const T (&src)[N])");
			begin_scope();
			statement("for (uint i = 0; i < N; dst[i] = src[i], i++);");
			end_scope();
			statement("");

			statement("// An overload for constant arrays.");
			statement("template<typename T, uint N>");
			statement("void spvArrayCopyConstant(thread T (&dst)[N], constant T (&src)[N])");
			begin_scope();
			statement("for (uint i = 0; i < N; dst[i] = src[i], i++);");
			end_scope();
			statement("");
			break;

		case SPVFuncImplTexelBufferCoords:
		{
			string tex_width_str = convert_to_string(msl_options.texel_buffer_texture_width);
			statement("// Returns 2D texture coords corresponding to 1D texel buffer coords");
			statement("uint2 spvTexelBufferCoord(uint tc)");
			begin_scope();
			statement(join("return uint2(tc % ", tex_width_str, ", tc / ", tex_width_str, ");"));
			end_scope();
			statement("");
			break;
		}

		case SPVFuncImplInverse4x4:
			statement("// Returns the determinant of a 2x2 matrix.");
			statement("inline float spvDet2x2(float a1, float a2, float b1, float b2)");
			begin_scope();
			statement("return a1 * b2 - b1 * a2;");
			end_scope();
			statement("");

			statement("// Returns the determinant of a 3x3 matrix.");
			statement("inline float spvDet3x3(float a1, float a2, float a3, float b1, float b2, float b3, float c1, "
			          "float c2, float c3)");
			begin_scope();
			statement("return a1 * spvDet2x2(b2, b3, c2, c3) - b1 * spvDet2x2(a2, a3, c2, c3) + c1 * spvDet2x2(a2, a3, "
			          "b2, b3);");
			end_scope();
			statement("");
			statement("// Returns the inverse of a matrix, by using the algorithm of calculating the classical");
			statement("// adjoint and dividing by the determinant. The contents of the matrix are changed.");
			statement("float4x4 spvInverse4x4(float4x4 m)");
			begin_scope();
			statement("float4x4 adj;	// The adjoint matrix (inverse after dividing by determinant)");
			statement_no_indent("");
			statement("// Create the transpose of the cofactors, as the classical adjoint of the matrix.");
			statement("adj[0][0] =  spvDet3x3(m[1][1], m[1][2], m[1][3], m[2][1], m[2][2], m[2][3], m[3][1], m[3][2], "
			          "m[3][3]);");
			statement("adj[0][1] = -spvDet3x3(m[0][1], m[0][2], m[0][3], m[2][1], m[2][2], m[2][3], m[3][1], m[3][2], "
			          "m[3][3]);");
			statement("adj[0][2] =  spvDet3x3(m[0][1], m[0][2], m[0][3], m[1][1], m[1][2], m[1][3], m[3][1], m[3][2], "
			          "m[3][3]);");
			statement("adj[0][3] = -spvDet3x3(m[0][1], m[0][2], m[0][3], m[1][1], m[1][2], m[1][3], m[2][1], m[2][2], "
			          "m[2][3]);");
			statement_no_indent("");
			statement("adj[1][0] = -spvDet3x3(m[1][0], m[1][2], m[1][3], m[2][0], m[2][2], m[2][3], m[3][0], m[3][2], "
			          "m[3][3]);");
			statement("adj[1][1] =  spvDet3x3(m[0][0], m[0][2], m[0][3], m[2][0], m[2][2], m[2][3], m[3][0], m[3][2], "
			          "m[3][3]);");
			statement("adj[1][2] = -spvDet3x3(m[0][0], m[0][2], m[0][3], m[1][0], m[1][2], m[1][3], m[3][0], m[3][2], "
			          "m[3][3]);");
			statement("adj[1][3] =  spvDet3x3(m[0][0], m[0][2], m[0][3], m[1][0], m[1][2], m[1][3], m[2][0], m[2][2], "
			          "m[2][3]);");
			statement_no_indent("");
			statement("adj[2][0] =  spvDet3x3(m[1][0], m[1][1], m[1][3], m[2][0], m[2][1], m[2][3], m[3][0], m[3][1], "
			          "m[3][3]);");
			statement("adj[2][1] = -spvDet3x3(m[0][0], m[0][1], m[0][3], m[2][0], m[2][1], m[2][3], m[3][0], m[3][1], "
			          "m[3][3]);");
			statement("adj[2][2] =  spvDet3x3(m[0][0], m[0][1], m[0][3], m[1][0], m[1][1], m[1][3], m[3][0], m[3][1], "
			          "m[3][3]);");
			statement("adj[2][3] = -spvDet3x3(m[0][0], m[0][1], m[0][3], m[1][0], m[1][1], m[1][3], m[2][0], m[2][1], "
			          "m[2][3]);");
			statement_no_indent("");
			statement("adj[3][0] = -spvDet3x3(m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2], m[3][0], m[3][1], "
			          "m[3][2]);");
			statement("adj[3][1] =  spvDet3x3(m[0][0], m[0][1], m[0][2], m[2][0], m[2][1], m[2][2], m[3][0], m[3][1], "
			          "m[3][2]);");
			statement("adj[3][2] = -spvDet3x3(m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[3][0], m[3][1], "
			          "m[3][2]);");
			statement("adj[3][3] =  spvDet3x3(m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], "
			          "m[2][2]);");
			statement_no_indent("");
			statement("// Calculate the determinant as a combination of the cofactors of the first row.");
			statement("float det = (adj[0][0] * m[0][0]) + (adj[0][1] * m[1][0]) + (adj[0][2] * m[2][0]) + (adj[0][3] "
			          "* m[3][0]);");
			statement_no_indent("");
			statement("// Divide the classical adjoint matrix by the determinant.");
			statement("// If determinant is zero, matrix is not invertable, so leave it unchanged.");
			statement("return (det != 0.0f) ? (adj * (1.0f / det)) : m;");
			end_scope();
			statement("");
			break;

		case SPVFuncImplInverse3x3:
			if (spv_function_implementations.count(SPVFuncImplInverse4x4) == 0)
			{
				statement("// Returns the determinant of a 2x2 matrix.");
				statement("inline float spvDet2x2(float a1, float a2, float b1, float b2)");
				begin_scope();
				statement("return a1 * b2 - b1 * a2;");
				end_scope();
				statement("");
			}

			statement("// Returns the inverse of a matrix, by using the algorithm of calculating the classical");
			statement("// adjoint and dividing by the determinant. The contents of the matrix are changed.");
			statement("float3x3 spvInverse3x3(float3x3 m)");
			begin_scope();
			statement("float3x3 adj;	// The adjoint matrix (inverse after dividing by determinant)");
			statement_no_indent("");
			statement("// Create the transpose of the cofactors, as the classical adjoint of the matrix.");
			statement("adj[0][0] =  spvDet2x2(m[1][1], m[1][2], m[2][1], m[2][2]);");
			statement("adj[0][1] = -spvDet2x2(m[0][1], m[0][2], m[2][1], m[2][2]);");
			statement("adj[0][2] =  spvDet2x2(m[0][1], m[0][2], m[1][1], m[1][2]);");
			statement_no_indent("");
			statement("adj[1][0] = -spvDet2x2(m[1][0], m[1][2], m[2][0], m[2][2]);");
			statement("adj[1][1] =  spvDet2x2(m[0][0], m[0][2], m[2][0], m[2][2]);");
			statement("adj[1][2] = -spvDet2x2(m[0][0], m[0][2], m[1][0], m[1][2]);");
			statement_no_indent("");
			statement("adj[2][0] =  spvDet2x2(m[1][0], m[1][1], m[2][0], m[2][1]);");
			statement("adj[2][1] = -spvDet2x2(m[0][0], m[0][1], m[2][0], m[2][1]);");
			statement("adj[2][2] =  spvDet2x2(m[0][0], m[0][1], m[1][0], m[1][1]);");
			statement_no_indent("");
			statement("// Calculate the determinant as a combination of the cofactors of the first row.");
			statement("float det = (adj[0][0] * m[0][0]) + (adj[0][1] * m[1][0]) + (adj[0][2] * m[2][0]);");
			statement_no_indent("");
			statement("// Divide the classical adjoint matrix by the determinant.");
			statement("// If determinant is zero, matrix is not invertable, so leave it unchanged.");
			statement("return (det != 0.0f) ? (adj * (1.0f / det)) : m;");
			end_scope();
			statement("");
			break;

		case SPVFuncImplInverse2x2:
			statement("// Returns the inverse of a matrix, by using the algorithm of calculating the classical");
			statement("// adjoint and dividing by the determinant. The contents of the matrix are changed.");
			statement("float2x2 spvInverse2x2(float2x2 m)");
			begin_scope();
			statement("float2x2 adj;	// The adjoint matrix (inverse after dividing by determinant)");
			statement_no_indent("");
			statement("// Create the transpose of the cofactors, as the classical adjoint of the matrix.");
			statement("adj[0][0] =  m[1][1];");
			statement("adj[0][1] = -m[0][1];");
			statement_no_indent("");
			statement("adj[1][0] = -m[1][0];");
			statement("adj[1][1] =  m[0][0];");
			statement_no_indent("");
			statement("// Calculate the determinant as a combination of the cofactors of the first row.");
			statement("float det = (adj[0][0] * m[0][0]) + (adj[0][1] * m[1][0]);");
			statement_no_indent("");
			statement("// Divide the classical adjoint matrix by the determinant.");
			statement("// If determinant is zero, matrix is not invertable, so leave it unchanged.");
			statement("return (det != 0.0f) ? (adj * (1.0f / det)) : m;");
			end_scope();
			statement("");
			break;

		case SPVFuncImplRowMajor2x3:
			statement("// Implementation of a conversion of matrix content from RowMajor to ColumnMajor organization.");
			statement("float2x3 spvConvertFromRowMajor2x3(float2x3 m)");
			begin_scope();
			statement("return float2x3(float3(m[0][0], m[0][2], m[1][1]), float3(m[0][1], m[1][0], m[1][2]));");
			end_scope();
			statement("");
			break;

		case SPVFuncImplRowMajor2x4:
			statement("// Implementation of a conversion of matrix content from RowMajor to ColumnMajor organization.");
			statement("float2x4 spvConvertFromRowMajor2x4(float2x4 m)");
			begin_scope();
			statement("return float2x4(float4(m[0][0], m[0][2], m[1][0], m[1][2]), float4(m[0][1], m[0][3], m[1][1], "
			          "m[1][3]));");
			end_scope();
			statement("");
			break;

		case SPVFuncImplRowMajor3x2:
			statement("// Implementation of a conversion of matrix content from RowMajor to ColumnMajor organization.");
			statement("float3x2 spvConvertFromRowMajor3x2(float3x2 m)");
			begin_scope();
			statement("return float3x2(float2(m[0][0], m[1][1]), float2(m[0][1], m[2][0]), float2(m[1][0], m[2][1]));");
			end_scope();
			statement("");
			break;

		case SPVFuncImplRowMajor3x4:
			statement("// Implementation of a conversion of matrix content from RowMajor to ColumnMajor organization.");
			statement("float3x4 spvConvertFromRowMajor3x4(float3x4 m)");
			begin_scope();
			statement("return float3x4(float4(m[0][0], m[0][3], m[1][2], m[2][1]), float4(m[0][1], m[1][0], m[1][3], "
			          "m[2][2]), float4(m[0][2], m[1][1], m[2][0], m[2][3]));");
			end_scope();
			statement("");
			break;

		case SPVFuncImplRowMajor4x2:
			statement("// Implementation of a conversion of matrix content from RowMajor to ColumnMajor organization.");
			statement("float4x2 spvConvertFromRowMajor4x2(float4x2 m)");
			begin_scope();
			statement("return float4x2(float2(m[0][0], m[2][0]), float2(m[0][1], m[2][1]), float2(m[1][0], m[3][0]), "
			          "float2(m[1][1], m[3][1]));");
			end_scope();
			statement("");
			break;

		case SPVFuncImplRowMajor4x3:
			statement("// Implementation of a conversion of matrix content from RowMajor to ColumnMajor organization.");
			statement("float4x3 spvConvertFromRowMajor4x3(float4x3 m)");
			begin_scope();
			statement("return float4x3(float3(m[0][0], m[1][1], m[2][2]), float3(m[0][1], m[1][2], m[3][0]), "
			          "float3(m[0][2], m[2][0], m[3][1]), float3(m[1][0], m[2][1], m[3][2]));");
			end_scope();
			statement("");
			break;

		default:
			break;
		}
	}
}

// Undefined global memory is not allowed in MSL.
// Declare constant and init to zeros. Use {}, as global constructors can break Metal.
void CompilerMSL::declare_undefined_values()
{
	bool emitted = false;
	for (auto &id : ids)
	{
		if (id.get_type() == TypeUndef)
		{
			auto &undef = id.get<SPIRUndef>();
			auto &type = get<SPIRType>(undef.basetype);
			statement("constant ", variable_decl(type, to_name(undef.self), undef.self), " = {};");
			emitted = true;
		}
	}

	if (emitted)
		statement("");
}

void CompilerMSL::declare_constant_arrays()
{
	// MSL cannot declare arrays inline (except when declaring a variable), so we must move them out to
	// global constants directly, so we are able to use constants as variable expressions.
	bool emitted = false;

	for (auto &id : ids)
	{
		if (id.get_type() == TypeConstant)
		{
			auto &c = id.get<SPIRConstant>();
			if (c.specialization)
				continue;

			auto &type = get<SPIRType>(c.constant_type);
			if (!type.array.empty())
			{
				auto name = to_name(c.self);
				statement("constant ", variable_decl(type, name), " = ", constant_expression(c), ";");
				emitted = true;
			}
		}
	}

	if (emitted)
		statement("");
}

void CompilerMSL::emit_resources()
{
	// Output non-interface structs. These include local function structs
	// and structs nested within uniform and read-write buffers.
	unordered_set<uint32_t> declared_structs;
	for (auto &id : ids)
	{
		if (id.get_type() == TypeType)
		{
			auto &type = id.get<SPIRType>();
			uint32_t type_id = type.self;

			bool is_struct = (type.basetype == SPIRType::Struct) && type.array.empty();
			bool is_block =
			    has_decoration(type.self, DecorationBlock) || has_decoration(type.self, DecorationBufferBlock);
			bool is_basic_struct = is_struct && !type.pointer && !is_block;

			bool is_interface = (type.storage == StorageClassInput || type.storage == StorageClassOutput ||
			                     type.storage == StorageClassUniformConstant);
			bool is_non_interface_block = is_struct && type.pointer && is_block && !is_interface;

			bool is_declarable_struct = is_basic_struct || is_non_interface_block;

			// Align and emit declarable structs...but avoid declaring each more than once.
			if (is_declarable_struct && declared_structs.count(type_id) == 0)
			{
				declared_structs.insert(type_id);

				if (has_decoration(type_id, DecorationCPacked))
					align_struct(type);

				emit_struct(type);
			}
		}
	}

	declare_constant_arrays();
	declare_undefined_values();

	// Output interface structs.
	emit_interface_block(stage_out_var_id);
	emit_interface_block(stage_in_var_id);
	emit_interface_block(stage_uniforms_var_id);
}

// Emit declarations for the specialization Metal function constants
void CompilerMSL::emit_specialization_constants()
{
	SpecializationConstant wg_x, wg_y, wg_z;
	uint32_t workgroup_size_id = get_work_group_size_specialization_constants(wg_x, wg_y, wg_z);
	bool emitted = false;

	for (auto &id : ids)
	{
		if (id.get_type() == TypeConstant)
		{
			auto &c = id.get<SPIRConstant>();
			if (!c.specialization)
				continue;

			// If WorkGroupSize is a specialization constant, it will be declared explicitly below.
			if (c.self == workgroup_size_id)
				continue;

			auto &type = get<SPIRType>(c.constant_type);
			string sc_type_name = type_to_glsl(type);
			string sc_name = to_name(c.self);
			string sc_tmp_name = sc_name + "_tmp";

			if (has_decoration(c.self, DecorationSpecId))
			{
				uint32_t constant_id = get_decoration(c.self, DecorationSpecId);
				// Only scalar, non-composite values can be function constants.
				statement("constant ", sc_type_name, " ", sc_tmp_name, " [[function_constant(", constant_id, ")]];");
				statement("constant ", sc_type_name, " ", sc_name, " = is_function_constant_defined(", sc_tmp_name,
				          ") ? ", sc_tmp_name, " : ", constant_expression(c), ";");
			}
			else
			{
				// Composite specialization constants must be built from other specialization constants.
				statement("constant ", sc_type_name, " ", sc_name, " = ", constant_expression(c), ";");
			}
			emitted = true;
		}
		else if (id.get_type() == TypeConstantOp)
		{
			auto &c = id.get<SPIRConstantOp>();
			auto &type = get<SPIRType>(c.basetype);
			auto name = to_name(c.self);
			statement("constant ", variable_decl(type, name), " = ", constant_op_expression(c), ";");
			emitted = true;
		}
	}

	// TODO: This can be expressed as a [[threads_per_threadgroup]] input semantic, but we need to know
	// the work group size at compile time in SPIR-V, and [[threads_per_threadgroup]] would need to be passed around as a global.
	// The work group size may be a specialization constant.
	if (workgroup_size_id)
	{
		statement("constant uint3 ", builtin_to_glsl(BuiltInWorkgroupSize, StorageClassWorkgroup), " = ",
		          constant_expression(get<SPIRConstant>(workgroup_size_id)), ";");
		emitted = true;
	}

	if (emitted)
		statement("");
}

// Override for MSL-specific syntax instructions
void CompilerMSL::emit_instruction(const Instruction &instruction)
{

#define MSL_BOP(op) emit_binary_op(ops[0], ops[1], ops[2], ops[3], #op)
#define MSL_BOP_CAST(op, type) \
	emit_binary_op_cast(ops[0], ops[1], ops[2], ops[3], #op, type, opcode_is_sign_invariant(opcode))
#define MSL_UOP(op) emit_unary_op(ops[0], ops[1], ops[2], #op)
#define MSL_QFOP(op) emit_quaternary_func_op(ops[0], ops[1], ops[2], ops[3], ops[4], ops[5], #op)
#define MSL_TFOP(op) emit_trinary_func_op(ops[0], ops[1], ops[2], ops[3], ops[4], #op)
#define MSL_BFOP(op) emit_binary_func_op(ops[0], ops[1], ops[2], ops[3], #op)
#define MSL_BFOP_CAST(op, type) \
	emit_binary_func_op_cast(ops[0], ops[1], ops[2], ops[3], #op, type, opcode_is_sign_invariant(opcode))
#define MSL_UFOP(op) emit_unary_func_op(ops[0], ops[1], ops[2], #op)

	auto ops = stream(instruction);
	auto opcode = static_cast<Op>(instruction.op);

	switch (opcode)
	{

	// Comparisons
	case OpIEqual:
	case OpLogicalEqual:
	case OpFOrdEqual:
		MSL_BOP(==);
		break;

	case OpINotEqual:
	case OpLogicalNotEqual:
	case OpFOrdNotEqual:
		MSL_BOP(!=);
		break;

	case OpUGreaterThan:
	case OpSGreaterThan:
	case OpFOrdGreaterThan:
		MSL_BOP(>);
		break;

	case OpUGreaterThanEqual:
	case OpSGreaterThanEqual:
	case OpFOrdGreaterThanEqual:
		MSL_BOP(>=);
		break;

	case OpULessThan:
	case OpSLessThan:
	case OpFOrdLessThan:
		MSL_BOP(<);
		break;

	case OpULessThanEqual:
	case OpSLessThanEqual:
	case OpFOrdLessThanEqual:
		MSL_BOP(<=);
		break;

	// Derivatives
	case OpDPdx:
	case OpDPdxFine:
	case OpDPdxCoarse:
		MSL_UFOP(dfdx);
		register_control_dependent_expression(ops[1]);
		break;

	case OpDPdy:
	case OpDPdyFine:
	case OpDPdyCoarse:
		MSL_UFOP(dfdy);
		register_control_dependent_expression(ops[1]);
		break;

	case OpFwidth:
	case OpFwidthCoarse:
	case OpFwidthFine:
		MSL_UFOP(fwidth);
		register_control_dependent_expression(ops[1]);
		break;

	// Bitfield
	case OpBitFieldInsert:
		MSL_QFOP(insert_bits);
		break;

	case OpBitFieldSExtract:
	case OpBitFieldUExtract:
		MSL_TFOP(extract_bits);
		break;

	case OpBitReverse:
		MSL_UFOP(reverse_bits);
		break;

	case OpBitCount:
		MSL_UFOP(popcount);
		break;

	case OpFRem:
		MSL_BFOP(fmod);
		break;

	// Atomics
	case OpAtomicExchange:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		uint32_t ptr = ops[2];
		uint32_t mem_sem = ops[4];
		uint32_t val = ops[5];
		emit_atomic_func_op(result_type, id, "atomic_exchange_explicit", mem_sem, mem_sem, false, ptr, val);
		break;
	}

	case OpAtomicCompareExchange:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		uint32_t ptr = ops[2];
		uint32_t mem_sem_pass = ops[4];
		uint32_t mem_sem_fail = ops[5];
		uint32_t val = ops[6];
		uint32_t comp = ops[7];
		emit_atomic_func_op(result_type, id, "atomic_compare_exchange_weak_explicit", mem_sem_pass, mem_sem_fail, true,
		                    ptr, comp, true, val);
		break;
	}

	case OpAtomicCompareExchangeWeak:
		SPIRV_CROSS_THROW("OpAtomicCompareExchangeWeak is only supported in kernel profile.");

	case OpAtomicLoad:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		uint32_t ptr = ops[2];
		uint32_t mem_sem = ops[4];
		emit_atomic_func_op(result_type, id, "atomic_load_explicit", mem_sem, mem_sem, false, ptr, 0);
		break;
	}

	case OpAtomicStore:
	{
		uint32_t result_type = expression_type(ops[0]).self;
		uint32_t id = ops[0];
		uint32_t ptr = ops[0];
		uint32_t mem_sem = ops[2];
		uint32_t val = ops[3];
		emit_atomic_func_op(result_type, id, "atomic_store_explicit", mem_sem, mem_sem, false, ptr, val);
		break;
	}

#define MSL_AFMO_IMPL(op, valsrc)                                                                                 \
	do                                                                                                            \
	{                                                                                                             \
		uint32_t result_type = ops[0];                                                                            \
		uint32_t id = ops[1];                                                                                     \
		uint32_t ptr = ops[2];                                                                                    \
		uint32_t mem_sem = ops[4];                                                                                \
		uint32_t val = valsrc;                                                                                    \
		emit_atomic_func_op(result_type, id, "atomic_fetch_" #op "_explicit", mem_sem, mem_sem, false, ptr, val); \
	} while (false)

#define MSL_AFMO(op) MSL_AFMO_IMPL(op, ops[5])
#define MSL_AFMIO(op) MSL_AFMO_IMPL(op, 1)

	case OpAtomicIIncrement:
		MSL_AFMIO(add);
		break;

	case OpAtomicIDecrement:
		MSL_AFMIO(sub);
		break;

	case OpAtomicIAdd:
		MSL_AFMO(add);
		break;

	case OpAtomicISub:
		MSL_AFMO(sub);
		break;

	case OpAtomicSMin:
	case OpAtomicUMin:
		MSL_AFMO(min);
		break;

	case OpAtomicSMax:
	case OpAtomicUMax:
		MSL_AFMO(max);
		break;

	case OpAtomicAnd:
		MSL_AFMO(and);
		break;

	case OpAtomicOr:
		MSL_AFMO(or);
		break;

	case OpAtomicXor:
		MSL_AFMO(xor);
		break;

	// Images

	// Reads == Fetches in Metal
	case OpImageRead:
	{
		// Mark that this shader reads from this image
		uint32_t img_id = ops[2];
		auto &type = expression_type(img_id);
		if (type.image.dim != DimSubpassData)
		{
			auto *p_var = maybe_get_backing_variable(img_id);
			if (p_var && has_decoration(p_var->self, DecorationNonReadable))
			{
				unset_decoration(p_var->self, DecorationNonReadable);
				force_recompile = true;
			}
		}

		emit_texture_op(instruction);
		break;
	}

	case OpImageWrite:
	{
		uint32_t img_id = ops[0];
		uint32_t coord_id = ops[1];
		uint32_t texel_id = ops[2];
		const uint32_t *opt = &ops[3];
		uint32_t length = instruction.length - 4;

		// Bypass pointers because we need the real image struct
		auto &type = expression_type(img_id);
		auto &img_type = get<SPIRType>(type.self);

		// Ensure this image has been marked as being written to and force a
		// recommpile so that the image type output will include write access
		auto *p_var = maybe_get_backing_variable(img_id);
		if (p_var && has_decoration(p_var->self, DecorationNonWritable))
		{
			unset_decoration(p_var->self, DecorationNonWritable);
			force_recompile = true;
		}

		bool forward = false;
		uint32_t bias = 0;
		uint32_t lod = 0;
		uint32_t flags = 0;

		if (length)
		{
			flags = *opt++;
			length--;
		}

		auto test = [&](uint32_t &v, uint32_t flag) {
			if (length && (flags & flag))
			{
				v = *opt++;
				length--;
			}
		};

		test(bias, ImageOperandsBiasMask);
		test(lod, ImageOperandsLodMask);

		statement(join(
		    to_expression(img_id), ".write(", to_expression(texel_id), ", ",
		    to_function_args(img_id, img_type, true, false, false, coord_id, 0, 0, 0, 0, lod, 0, 0, 0, 0, 0, &forward),
		    ");"));

		if (p_var && variable_storage_is_aliased(*p_var))
			flush_all_aliased_variables();

		break;
	}

	case OpImageQuerySize:
	case OpImageQuerySizeLod:
	{
		uint32_t rslt_type_id = ops[0];
		auto &rslt_type = get<SPIRType>(rslt_type_id);

		uint32_t id = ops[1];

		uint32_t img_id = ops[2];
		string img_exp = to_expression(img_id);
		auto &img_type = expression_type(img_id);
		Dim img_dim = img_type.image.dim;
		bool img_is_array = img_type.image.arrayed;

		if (img_type.basetype != SPIRType::Image)
			SPIRV_CROSS_THROW("Invalid type for OpImageQuerySize.");

		string lod;
		if (opcode == OpImageQuerySizeLod)
		{
			// LOD index defaults to zero, so don't bother outputing level zero index
			string decl_lod = to_expression(ops[3]);
			if (decl_lod != "0")
				lod = decl_lod;
		}

		string expr = type_to_glsl(rslt_type) + "(";
		expr += img_exp + ".get_width(" + lod + ")";

		if (img_dim == Dim2D || img_dim == DimCube || img_dim == Dim3D)
			expr += ", " + img_exp + ".get_height(" + lod + ")";

		if (img_dim == Dim3D)
			expr += ", " + img_exp + ".get_depth(" + lod + ")";

		if (img_is_array)
			expr += ", " + img_exp + ".get_array_size()";

		expr += ")";

		emit_op(rslt_type_id, id, expr, should_forward(img_id));

		break;
	}

#define MSL_ImgQry(qrytype)                                                                 \
	do                                                                                      \
	{                                                                                       \
		uint32_t rslt_type_id = ops[0];                                                     \
		auto &rslt_type = get<SPIRType>(rslt_type_id);                                      \
		uint32_t id = ops[1];                                                               \
		uint32_t img_id = ops[2];                                                           \
		string img_exp = to_expression(img_id);                                             \
		string expr = type_to_glsl(rslt_type) + "(" + img_exp + ".get_num_" #qrytype "())"; \
		emit_op(rslt_type_id, id, expr, should_forward(img_id));                            \
	} while (false)

	case OpImageQueryLevels:
		MSL_ImgQry(mip_levels);
		break;

	case OpImageQuerySamples:
		MSL_ImgQry(samples);
		break;

	// Casting
	case OpQuantizeToF16:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		uint32_t arg = ops[2];

		string exp;
		auto &type = get<SPIRType>(result_type);

		switch (type.vecsize)
		{
		case 1:
			exp = join("float(half(", to_expression(arg), "))");
			break;
		case 2:
			exp = join("float2(half2(", to_expression(arg), "))");
			break;
		case 3:
			exp = join("float3(half3(", to_expression(arg), "))");
			break;
		case 4:
			exp = join("float4(half4(", to_expression(arg), "))");
			break;
		default:
			SPIRV_CROSS_THROW("Illegal argument to OpQuantizeToF16.");
		}

		emit_op(result_type, id, exp, should_forward(arg));
		break;
	}

	case OpStore:
		if (maybe_emit_input_struct_assignment(ops[0], ops[1]))
			break;

		if (maybe_emit_array_assignment(ops[0], ops[1]))
			break;

		CompilerGLSL::emit_instruction(instruction);
		break;

	// Compute barriers
	case OpMemoryBarrier:
		emit_barrier(0, ops[0], ops[1]);
		break;

	case OpControlBarrier:
		// In GLSL a memory barrier is often followed by a control barrier.
		// But in MSL, memory barriers are also control barriers, so don't
		// emit a simple control barrier if a memory barrier has just been emitted.
		if (previous_instruction_opcode != OpMemoryBarrier)
			emit_barrier(ops[0], ops[1], ops[2]);
		break;

	case OpVectorTimesMatrix:
	case OpMatrixTimesVector:
	{
		// If the matrix needs transpose and it is square or packed, just flip the multiply order.
		uint32_t mtx_id = ops[opcode == OpMatrixTimesVector ? 2 : 3];
		auto *e = maybe_get<SPIRExpression>(mtx_id);
		auto &t = expression_type(mtx_id);
		bool is_packed = has_decoration(mtx_id, DecorationCPacked);
		if (e && e->need_transpose && (t.columns == t.vecsize || is_packed))
		{
			e->need_transpose = false;

			// This is important for matrices. Packed matrices
			// are generally transposed, so unpacking using a constructor argument
			// will result in an error.
			// The simplest solution for now is to just avoid unpacking the matrix in this operation.
			unset_decoration(mtx_id, DecorationCPacked);

			emit_binary_op(ops[0], ops[1], ops[3], ops[2], "*");
			if (is_packed)
				set_decoration(mtx_id, DecorationCPacked);
			e->need_transpose = true;
		}
		else
			MSL_BOP(*);
		break;
	}

		// OpOuterProduct

	default:
		CompilerGLSL::emit_instruction(instruction);
		break;
	}

	previous_instruction_opcode = opcode;
}

void CompilerMSL::emit_barrier(uint32_t id_exe_scope, uint32_t id_mem_scope, uint32_t id_mem_sem)
{
	if (get_entry_point().model != ExecutionModelGLCompute)
		return;

	string bar_stmt = "threadgroup_barrier(mem_flags::";

	uint32_t mem_sem = id_mem_sem ? get<SPIRConstant>(id_mem_sem).scalar() : uint32_t(MemorySemanticsMaskNone);

	if (mem_sem & MemorySemanticsCrossWorkgroupMemoryMask)
		bar_stmt += "mem_device";
	else if (mem_sem & (MemorySemanticsSubgroupMemoryMask | MemorySemanticsWorkgroupMemoryMask |
	                    MemorySemanticsAtomicCounterMemoryMask))
		bar_stmt += "mem_threadgroup";
	else if (mem_sem & MemorySemanticsImageMemoryMask)
		bar_stmt += "mem_texture";
	else
		bar_stmt += "mem_none";

	if (msl_options.is_ios() && msl_options.supports_msl_version(2))
	{
		bar_stmt += ", ";

		// Use the wider of the two scopes (smaller value)
		uint32_t exe_scope = id_exe_scope ? get<SPIRConstant>(id_exe_scope).scalar() : uint32_t(ScopeInvocation);
		uint32_t mem_scope = id_mem_scope ? get<SPIRConstant>(id_mem_scope).scalar() : uint32_t(ScopeInvocation);
		uint32_t scope = min(exe_scope, mem_scope);
		switch (scope)
		{
		case ScopeCrossDevice:
		case ScopeDevice:
			bar_stmt += "memory_scope_device";
			break;

		case ScopeSubgroup:
		case ScopeInvocation:
			bar_stmt += "memory_scope_simdgroup";
			break;

		case ScopeWorkgroup:
		default:
			bar_stmt += "memory_scope_threadgroup";
			break;
		}
	}

	bar_stmt += ");";

	statement(bar_stmt);

	assert(current_emitting_block);
	flush_control_dependent_expressions(current_emitting_block->self);
	flush_all_active_variables();
}

// Since MSL does not allow structs to be nested within the stage_in struct, the original input
// structs are flattened into a single stage_in struct by add_interface_block. As a result,
// if the LHS and RHS represent an assignment of an entire input struct, we must perform this
// member-by-member, mapping each RHS member to its name in the flattened stage_in struct.
// Returns whether the struct assignment was emitted.
bool CompilerMSL::maybe_emit_input_struct_assignment(uint32_t id_lhs, uint32_t id_rhs)
{
	// We only care about assignments of an entire struct
	uint32_t type_id = expression_type_id(id_rhs);
	auto &type = get<SPIRType>(type_id);
	if (type.basetype != SPIRType::Struct)
		return false;

	// We only care about assignments from Input variables
	auto *p_v_rhs = maybe_get_backing_variable(id_rhs);
	if (!(p_v_rhs && p_v_rhs->storage == StorageClassInput))
		return false;

	// Get the ID of the type of the underlying RHS variable.
	// This will be an Input OpTypePointer containing the qualified member names.
	uint32_t tid_v_rhs = p_v_rhs->basetype;

	// Ensure the LHS variable has been declared
	auto *p_v_lhs = maybe_get_backing_variable(id_lhs);
	if (p_v_lhs)
		flush_variable_declaration(p_v_lhs->self);

	size_t mbr_cnt = type.member_types.size();
	for (uint32_t mbr_idx = 0; mbr_idx < mbr_cnt; mbr_idx++)
	{
		string expr;

		//LHS
		expr += to_name(id_lhs);
		expr += ".";
		expr += to_member_name(type, mbr_idx);

		expr += " = ";

		//RHS
		string qual_mbr_name = get_member_qualified_name(tid_v_rhs, mbr_idx);
		if (qual_mbr_name.empty())
		{
			expr += to_name(id_rhs);
			expr += ".";
			expr += to_member_name(type, mbr_idx);
		}
		else
			expr += qual_mbr_name;

		statement(expr, ";");
	}

	return true;
}

void CompilerMSL::emit_array_copy(const string &lhs, uint32_t rhs_id)
{
	// Assignment from an array initializer is fine.
	if (ids[rhs_id].get_type() == TypeConstant)
		statement("spvArrayCopyConstant(", lhs, ", ", to_expression(rhs_id), ");");
	else
		statement("spvArrayCopy(", lhs, ", ", to_expression(rhs_id), ");");
}

// Since MSL does not allow arrays to be copied via simple variable assignment,
// if the LHS and RHS represent an assignment of an entire array, it must be
// implemented by calling an array copy function.
// Returns whether the struct assignment was emitted.
bool CompilerMSL::maybe_emit_array_assignment(uint32_t id_lhs, uint32_t id_rhs)
{
	// We only care about assignments of an entire array
	auto &type = expression_type(id_rhs);
	if (type.array.size() == 0)
		return false;

	auto *var = maybe_get<SPIRVariable>(id_lhs);

	// Is this a remapped, static constant? Don't do anything.
	if (var && var->remapped_variable && var->statically_assigned)
		return true;

	if (ids[id_rhs].get_type() == TypeConstant && var && var->deferred_declaration)
	{
		// Special case, if we end up declaring a variable when assigning the constant array,
		// we can avoid the copy by directly assigning the constant expression.
		// This is likely necessary to be able to use a variable as a true look-up table, as it is unlikely
		// the compiler will be able to optimize the spvArrayCopy() into a constant LUT.
		// After a variable has been declared, we can no longer assign constant arrays in MSL unfortunately.
		statement(to_expression(id_lhs), " = ", constant_expression(get<SPIRConstant>(id_rhs)), ";");
		return true;
	}

	// Ensure the LHS variable has been declared
	auto *p_v_lhs = maybe_get_backing_variable(id_lhs);
	if (p_v_lhs)
		flush_variable_declaration(p_v_lhs->self);

	emit_array_copy(to_expression(id_lhs), id_rhs);
	register_write(id_lhs);

	return true;
}

// Emits one of the atomic functions. In MSL, the atomic functions operate on pointers
void CompilerMSL::emit_atomic_func_op(uint32_t result_type, uint32_t result_id, const char *op, uint32_t mem_order_1,
                                      uint32_t mem_order_2, bool has_mem_order_2, uint32_t obj, uint32_t op1,
                                      bool op1_is_pointer, uint32_t op2)
{
	forced_temporaries.insert(result_id);

	string exp = string(op) + "(";

	auto &type = expression_type(obj);
	exp += "(volatile ";
	auto *var = maybe_get_backing_variable(obj);
	if (!var)
		SPIRV_CROSS_THROW("No backing variable for atomic operation.");
	exp += get_argument_address_space(*var);
	exp += " atomic_";
	exp += type_to_glsl(type);
	exp += "*)";

	exp += "&";
	exp += to_enclosed_expression(obj);

	bool is_atomic_compare_exchange_strong = op1_is_pointer && op1;

	if (is_atomic_compare_exchange_strong)
	{
		assert(strcmp(op, "atomic_compare_exchange_weak_explicit") == 0);
		assert(op2);
		assert(has_mem_order_2);
		exp += ", &";
		exp += to_name(result_id);
		exp += ", ";
		exp += to_expression(op2);
		exp += ", ";
		exp += get_memory_order(mem_order_1);
		exp += ", ";
		exp += get_memory_order(mem_order_2);
		exp += ")";

		// MSL only supports the weak atomic compare exchange,
		// so emit a CAS loop here.
		statement(variable_decl(type, to_name(result_id)), ";");
		statement("do");
		begin_scope();
		statement(to_name(result_id), " = ", to_expression(op1), ";");
		end_scope_decl(join("while (!", exp, ")"));
		set<SPIRExpression>(result_id, to_name(result_id), result_type, true);
	}
	else
	{
		assert(strcmp(op, "atomic_compare_exchange_weak_explicit") != 0);
		if (op1)
			exp += ", " + to_expression(op1);
		if (op2)
			exp += ", " + to_expression(op2);

		exp += string(", ") + get_memory_order(mem_order_1);
		if (has_mem_order_2)
			exp += string(", ") + get_memory_order(mem_order_2);

		exp += ")";
		emit_op(result_type, result_id, exp, false);
	}

	flush_all_atomic_capable_variables();
}

// Metal only supports relaxed memory order for now
const char *CompilerMSL::get_memory_order(uint32_t)
{
	return "memory_order_relaxed";
}

// Override for MSL-specific extension syntax instructions
void CompilerMSL::emit_glsl_op(uint32_t result_type, uint32_t id, uint32_t eop, const uint32_t *args, uint32_t count)
{
	GLSLstd450 op = static_cast<GLSLstd450>(eop);

	switch (op)
	{
	case GLSLstd450Atan2:
		emit_binary_func_op(result_type, id, args[0], args[1], "atan2");
		break;
	case GLSLstd450InverseSqrt:
		emit_unary_func_op(result_type, id, args[0], "rsqrt");
		break;
	case GLSLstd450RoundEven:
		emit_unary_func_op(result_type, id, args[0], "rint");
		break;

	case GLSLstd450FindSMsb:
		emit_unary_func_op(result_type, id, args[0], "findSMSB");
		break;
	case GLSLstd450FindUMsb:
		emit_unary_func_op(result_type, id, args[0], "findUMSB");
		break;

	case GLSLstd450PackSnorm4x8:
		emit_unary_func_op(result_type, id, args[0], "pack_float_to_snorm4x8");
		break;
	case GLSLstd450PackUnorm4x8:
		emit_unary_func_op(result_type, id, args[0], "pack_float_to_unorm4x8");
		break;
	case GLSLstd450PackSnorm2x16:
		emit_unary_func_op(result_type, id, args[0], "pack_float_to_snorm2x16");
		break;
	case GLSLstd450PackUnorm2x16:
		emit_unary_func_op(result_type, id, args[0], "pack_float_to_unorm2x16");
		break;

	case GLSLstd450PackHalf2x16:
	{
		auto expr = join("as_type<uint>(half2(", to_expression(args[0]), "))");
		emit_op(result_type, id, expr, should_forward(args[0]));
		inherit_expression_dependencies(id, args[0]);
		break;
	}

	case GLSLstd450UnpackSnorm4x8:
		emit_unary_func_op(result_type, id, args[0], "unpack_snorm4x8_to_float");
		break;
	case GLSLstd450UnpackUnorm4x8:
		emit_unary_func_op(result_type, id, args[0], "unpack_unorm4x8_to_float");
		break;
	case GLSLstd450UnpackSnorm2x16:
		emit_unary_func_op(result_type, id, args[0], "unpack_snorm2x16_to_float");
		break;
	case GLSLstd450UnpackUnorm2x16:
		emit_unary_func_op(result_type, id, args[0], "unpack_unorm2x16_to_float");
		break;

	case GLSLstd450UnpackHalf2x16:
	{
		auto expr = join("float2(as_type<half2>(", to_expression(args[0]), "))");
		emit_op(result_type, id, expr, should_forward(args[0]));
		inherit_expression_dependencies(id, args[0]);
		break;
	}

	case GLSLstd450PackDouble2x32:
		emit_unary_func_op(result_type, id, args[0], "unsupported_GLSLstd450PackDouble2x32"); // Currently unsupported
		break;
	case GLSLstd450UnpackDouble2x32:
		emit_unary_func_op(result_type, id, args[0], "unsupported_GLSLstd450UnpackDouble2x32"); // Currently unsupported
		break;

	case GLSLstd450MatrixInverse:
	{
		auto &mat_type = get<SPIRType>(result_type);
		switch (mat_type.columns)
		{
		case 2:
			emit_unary_func_op(result_type, id, args[0], "spvInverse2x2");
			break;
		case 3:
			emit_unary_func_op(result_type, id, args[0], "spvInverse3x3");
			break;
		case 4:
			emit_unary_func_op(result_type, id, args[0], "spvInverse4x4");
			break;
		default:
			break;
		}
		break;
	}

		// TODO:
		//        GLSLstd450InterpolateAtCentroid (centroid_no_perspective qualifier)
		//        GLSLstd450InterpolateAtSample (sample_no_perspective qualifier)
		//        GLSLstd450InterpolateAtOffset

	default:
		CompilerGLSL::emit_glsl_op(result_type, id, eop, args, count);
		break;
	}
}

// Emit a structure declaration for the specified interface variable.
void CompilerMSL::emit_interface_block(uint32_t ib_var_id)
{
	if (ib_var_id)
	{
		auto &ib_var = get<SPIRVariable>(ib_var_id);
		auto &ib_type = get<SPIRType>(ib_var.basetype);
		auto &m = meta.at(ib_type.self);
		if (m.members.size() > 0)
			emit_struct(ib_type);
	}
}

// Emits the declaration signature of the specified function.
// If this is the entry point function, Metal-specific return value and function arguments are added.
void CompilerMSL::emit_function_prototype(SPIRFunction &func, const Bitset &)
{
	if (func.self != entry_point)
		add_function_overload(func);

	local_variable_names = resource_names;
	string decl;

	processing_entry_point = (func.self == entry_point);

	auto &type = get<SPIRType>(func.return_type);

	if (type.array.empty())
	{
		decl += func_type_decl(type);
	}
	else
	{
		// We cannot return arrays in MSL, so "return" through an out variable.
		decl = "void";
	}

	decl += " ";
	decl += to_name(func.self);
	decl += "(";

	if (!type.array.empty())
	{
		// Fake arrays returns by writing to an out array instead.
		decl += "thread ";
		decl += type_to_glsl(type);
		decl += " (&SPIRV_Cross_return_value)";
		decl += type_to_array_glsl(type);
		if (!func.arguments.empty())
			decl += ", ";
	}

	if (processing_entry_point)
	{
		decl += entry_point_args(!func.arguments.empty());

		// If entry point function has variables that require early declaration,
		// ensure they each have an empty initializer, creating one if needed.
		// This is done at this late stage because the initialization expression
		// is cleared after each compilation pass.
		for (auto var_id : vars_needing_early_declaration)
		{
			auto &ed_var = get<SPIRVariable>(var_id);
			if (!ed_var.initializer)
				ed_var.initializer = increase_bound_by(1);

			set<SPIRExpression>(ed_var.initializer, "{}", ed_var.basetype, true);
		}
	}

	for (auto &arg : func.arguments)
	{
		add_local_variable_name(arg.id);

		string address_space;
		auto *var = maybe_get<SPIRVariable>(arg.id);
		if (var)
		{
			var->parameter = &arg; // Hold a pointer to the parameter so we can invalidate the readonly field if needed.
			address_space = get_argument_address_space(*var);
		}

		if (!address_space.empty())
			decl += address_space + " ";
		decl += argument_decl(arg);

		// Manufacture automatic sampler arg for SampledImage texture
		auto &arg_type = get<SPIRType>(arg.type);
		if (arg_type.basetype == SPIRType::SampledImage && arg_type.image.dim != DimBuffer)
			decl += join(", thread const ", sampler_type(arg_type), " ", to_sampler_expression(arg.id));

		if (&arg != &func.arguments.back())
			decl += ", ";
	}

	decl += ")";
	statement(decl);
}

// Returns the texture sampling function string for the specified image and sampling characteristics.
string CompilerMSL::to_function_name(uint32_t img, const SPIRType &, bool is_fetch, bool is_gather, bool, bool, bool,
                                     bool, bool has_dref, uint32_t)
{
	// Texture reference
	string fname = to_expression(img) + ".";

	// Texture function and sampler
	if (is_fetch)
		fname += "read";
	else if (is_gather)
		fname += "gather";
	else
		fname += "sample";

	if (has_dref)
		fname += "_compare";

	return fname;
}

// Returns the function args for a texture sampling function for the specified image and sampling characteristics.
string CompilerMSL::to_function_args(uint32_t img, const SPIRType &imgtype, bool is_fetch, bool, bool is_proj,
                                     uint32_t coord, uint32_t, uint32_t dref, uint32_t grad_x, uint32_t grad_y,
                                     uint32_t lod, uint32_t coffset, uint32_t offset, uint32_t bias, uint32_t comp,
                                     uint32_t sample, bool *p_forward)
{
	string farg_str;
	if (!is_fetch)
		farg_str += to_sampler_expression(img);

	// Texture coordinates
	bool forward = should_forward(coord);
	auto coord_expr = to_enclosed_expression(coord);
	auto &coord_type = expression_type(coord);
	bool coord_is_fp = type_is_floating_point(coord_type);
	bool is_cube_fetch = false;

	string tex_coords = coord_expr;
	const char *alt_coord = "";

	switch (imgtype.image.dim)
	{

	case Dim1D:
		if (coord_type.vecsize > 1)
			tex_coords += ".x";

		if (is_fetch)
			tex_coords = "uint(" + round_fp_tex_coords(tex_coords, coord_is_fp) + ")";

		alt_coord = ".y";

		break;

	case DimBuffer:
		if (coord_type.vecsize > 1)
			tex_coords += ".x";

		// Metal texel buffer textures are 2D, so convert 1D coord to 2D.
		if (is_fetch)
			tex_coords = "spvTexelBufferCoord(" + round_fp_tex_coords(tex_coords, coord_is_fp) + ")";

		alt_coord = ".y";

		break;

	case DimSubpassData:
		if (imgtype.image.ms)
			tex_coords = "uint2(gl_FragCoord.xy)";
		else
			tex_coords = join("uint2(gl_FragCoord.xy), 0");
		break;

	case Dim2D:
		if (coord_type.vecsize > 2)
			tex_coords += ".xy";

		if (is_fetch)
			tex_coords = "uint2(" + round_fp_tex_coords(tex_coords, coord_is_fp) + ")";

		alt_coord = ".z";

		break;

	case Dim3D:
		if (coord_type.vecsize > 3)
			tex_coords += ".xyz";

		if (is_fetch)
			tex_coords = "uint3(" + round_fp_tex_coords(tex_coords, coord_is_fp) + ")";

		alt_coord = ".w";

		break;

	case DimCube:
		if (is_fetch)
		{
			is_cube_fetch = true;
			tex_coords += ".xy";
			tex_coords = "uint2(" + round_fp_tex_coords(tex_coords, coord_is_fp) + ")";
		}
		else
		{
			if (coord_type.vecsize > 3)
				tex_coords += ".xyz";
		}

		alt_coord = ".w";

		break;

	default:
		break;
	}

	if (is_fetch && offset)
	{
		// Fetch offsets must be applied directly to the coordinate.
		forward = forward && should_forward(offset);
		auto &type = expression_type(offset);
		if (type.basetype != SPIRType::UInt)
			tex_coords += " + " + bitcast_expression(SPIRType::UInt, offset);
		else
			tex_coords += " + " + to_enclosed_expression(offset);
	}
	else if (is_fetch && coffset)
	{
		// Fetch offsets must be applied directly to the coordinate.
		forward = forward && should_forward(coffset);
		auto &type = expression_type(coffset);
		if (type.basetype != SPIRType::UInt)
			tex_coords += " + " + bitcast_expression(SPIRType::UInt, coffset);
		else
			tex_coords += " + " + to_enclosed_expression(coffset);
	}

	// If projection, use alt coord as divisor
	if (is_proj)
		tex_coords += " / " + coord_expr + alt_coord;

	if (!farg_str.empty())
		farg_str += ", ";
	farg_str += tex_coords;

	// If fetch from cube, add face explicitly
	if (is_cube_fetch)
	{
		// Special case for cube arrays, face and layer are packed in one dimension.
		if (imgtype.image.arrayed)
			farg_str += ", uint(" + join(coord_expr, ".z) % 6u");
		else
			farg_str += ", uint(" + round_fp_tex_coords(coord_expr + ".z", coord_is_fp) + ")";
	}

	// If array, use alt coord
	if (imgtype.image.arrayed)
	{
		// Special case for cube arrays, face and layer are packed in one dimension.
		if (imgtype.image.dim == DimCube && is_fetch)
			farg_str += ", uint(" + join(coord_expr, ".z) / 6u");
		else
			farg_str += ", uint(" + round_fp_tex_coords(coord_expr + alt_coord, coord_is_fp) + ")";
	}

	// Depth compare reference value
	if (dref)
	{
		forward = forward && should_forward(dref);
		farg_str += ", ";
		farg_str += to_expression(dref);
	}

	// LOD Options
	// Metal does not support LOD for 1D textures.
	if (bias && imgtype.image.dim != Dim1D)
	{
		forward = forward && should_forward(bias);
		farg_str += ", bias(" + to_expression(bias) + ")";
	}

	// Metal does not support LOD for 1D textures.
	if (lod && imgtype.image.dim != Dim1D)
	{
		forward = forward && should_forward(lod);
		if (is_fetch)
		{
			farg_str += ", " + to_expression(lod);
		}
		else
		{
			farg_str += ", level(" + to_expression(lod) + ")";
		}
	}
	else if (is_fetch && !lod && imgtype.image.dim != Dim1D && imgtype.image.dim != DimBuffer && !imgtype.image.ms &&
	         imgtype.image.sampled != 2)
	{
		// Lod argument is optional in OpImageFetch, but we require a LOD value, pick 0 as the default.
		// Check for sampled type as well, because is_fetch is also used for OpImageRead in MSL.
		farg_str += ", 0";
	}

	// Metal does not support LOD for 1D textures.
	if ((grad_x || grad_y) && imgtype.image.dim != Dim1D)
	{
		forward = forward && should_forward(grad_x);
		forward = forward && should_forward(grad_y);
		string grad_opt;
		switch (imgtype.image.dim)
		{
		case Dim2D:
			grad_opt = "2d";
			break;
		case Dim3D:
			grad_opt = "3d";
			break;
		case DimCube:
			grad_opt = "cube";
			break;
		default:
			grad_opt = "unsupported_gradient_dimension";
			break;
		}
		farg_str += ", gradient" + grad_opt + "(" + to_expression(grad_x) + ", " + to_expression(grad_y) + ")";
	}

	// Add offsets
	string offset_expr;
	if (coffset && !is_fetch)
	{
		forward = forward && should_forward(coffset);
		offset_expr = to_expression(coffset);
	}
	else if (offset && !is_fetch)
	{
		forward = forward && should_forward(offset);
		offset_expr = to_expression(offset);
	}

	if (!offset_expr.empty())
	{
		switch (imgtype.image.dim)
		{
		case Dim2D:
			if (coord_type.vecsize > 2)
				offset_expr = enclose_expression(offset_expr) + ".xy";

			farg_str += ", " + offset_expr;
			break;

		case Dim3D:
			if (coord_type.vecsize > 3)
				offset_expr = enclose_expression(offset_expr) + ".xyz";

			farg_str += ", " + offset_expr;
			break;

		default:
			break;
		}
	}

	if (comp)
	{
		// If 2D has gather component, ensure it also has an offset arg
		if (imgtype.image.dim == Dim2D && offset_expr.empty())
			farg_str += ", int2(0)";

		forward = forward && should_forward(comp);
		farg_str += ", " + to_component_argument(comp);
	}

	if (sample)
	{
		farg_str += ", ";
		farg_str += to_expression(sample);
	}

	*p_forward = forward;

	return farg_str;
}

// If the texture coordinates are floating point, invokes MSL round() function to round them.
string CompilerMSL::round_fp_tex_coords(string tex_coords, bool coord_is_fp)
{
	return coord_is_fp ? ("round(" + tex_coords + ")") : tex_coords;
}

// Returns a string to use in an image sampling function argument.
// The ID must be a scalar constant.
string CompilerMSL::to_component_argument(uint32_t id)
{
	if (ids[id].get_type() != TypeConstant)
	{
		SPIRV_CROSS_THROW("ID " + to_string(id) + " is not an OpConstant.");
		return "component::x";
	}

	uint32_t component_index = get<SPIRConstant>(id).scalar();
	switch (component_index)
	{
	case 0:
		return "component::x";
	case 1:
		return "component::y";
	case 2:
		return "component::z";
	case 3:
		return "component::w";

	default:
		SPIRV_CROSS_THROW("The value (" + to_string(component_index) + ") of OpConstant ID " + to_string(id) +
		                  " is not a valid Component index, which must be one of 0, 1, 2, or 3.");
		return "component::x";
	}
}

// Establish sampled image as expression object and assign the sampler to it.
void CompilerMSL::emit_sampled_image_op(uint32_t result_type, uint32_t result_id, uint32_t image_id, uint32_t samp_id)
{
	set<SPIRExpression>(result_id, to_expression(image_id), result_type, true);
	meta[result_id].sampler = samp_id;
}

// Returns a string representation of the ID, usable as a function arg.
// Manufacture automatic sampler arg for SampledImage texture.
string CompilerMSL::to_func_call_arg(uint32_t id)
{
	string arg_str = CompilerGLSL::to_func_call_arg(id);

	// Manufacture automatic sampler arg if the arg is a SampledImage texture.
	auto &type = expression_type(id);
	if (type.basetype == SPIRType::SampledImage && type.image.dim != DimBuffer)
		arg_str += ", " + to_sampler_expression(id);

	return arg_str;
}

// If the ID represents a sampled image that has been assigned a sampler already,
// generate an expression for the sampler, otherwise generate a fake sampler name
// by appending a suffix to the expression constructed from the ID.
string CompilerMSL::to_sampler_expression(uint32_t id)
{
	auto expr = to_expression(id);
	auto index = expr.find_first_of('[');
	uint32_t samp_id = meta[id].sampler;

	if (index == string::npos)
		return samp_id ? to_expression(samp_id) : expr + sampler_name_suffix;
	else
	{
		auto image_expr = expr.substr(0, index);
		auto array_expr = expr.substr(index);
		return samp_id ? to_expression(samp_id) : (image_expr + sampler_name_suffix + array_expr);
	}
}

// Checks whether the ID is a row_major matrix that requires conversion before use
bool CompilerMSL::is_non_native_row_major_matrix(uint32_t id)
{
	// Natively supported row-major matrices do not need to be converted.
	if (backend.native_row_major_matrix)
		return false;

	// Non-matrix or column-major matrix types do not need to be converted.
	if (!meta[id].decoration.decoration_flags.get(DecorationRowMajor))
		return false;

	// Generate a function that will swap matrix elements from row-major to column-major.
	// Packed row-matrix should just use transpose() function.
	if (!has_decoration(id, DecorationCPacked))
	{
		const auto type = expression_type(id);
		add_convert_row_major_matrix_function(type.columns, type.vecsize);
	}

	return true;
}

// Checks whether the member is a row_major matrix that requires conversion before use
bool CompilerMSL::member_is_non_native_row_major_matrix(const SPIRType &type, uint32_t index)
{
	// Natively supported row-major matrices do not need to be converted.
	if (backend.native_row_major_matrix)
		return false;

	// Non-matrix or column-major matrix types do not need to be converted.
	if (!combined_decoration_for_member(type, index).get(DecorationRowMajor))
		return false;

	// Generate a function that will swap matrix elements from row-major to column-major.
	// Packed row-matrix should just use transpose() function.
	if (!has_member_decoration(type.self, index, DecorationCPacked))
	{
		const auto mbr_type = get<SPIRType>(type.member_types[index]);
		add_convert_row_major_matrix_function(mbr_type.columns, mbr_type.vecsize);
	}

	return true;
}

// Adds a function suitable for converting a non-square row-major matrix to a column-major matrix.
void CompilerMSL::add_convert_row_major_matrix_function(uint32_t cols, uint32_t rows)
{
	SPVFuncImpl spv_func;
	if (cols == rows) // Square matrix...just use transpose() function
		return;
	else if (cols == 2 && rows == 3)
		spv_func = SPVFuncImplRowMajor2x3;
	else if (cols == 2 && rows == 4)
		spv_func = SPVFuncImplRowMajor2x4;
	else if (cols == 3 && rows == 2)
		spv_func = SPVFuncImplRowMajor3x2;
	else if (cols == 3 && rows == 4)
		spv_func = SPVFuncImplRowMajor3x4;
	else if (cols == 4 && rows == 2)
		spv_func = SPVFuncImplRowMajor4x2;
	else if (cols == 4 && rows == 3)
		spv_func = SPVFuncImplRowMajor4x3;
	else
		SPIRV_CROSS_THROW("Could not convert row-major matrix.");

	auto rslt = spv_function_implementations.insert(spv_func);
	if (rslt.second)
	{
		add_pragma_line("#pragma clang diagnostic ignored \"-Wmissing-prototypes\"");
		force_recompile = true;
	}
}

// Wraps the expression string in a function call that converts the
// row_major matrix result of the expression to a column_major matrix.
string CompilerMSL::convert_row_major_matrix(string exp_str, const SPIRType &exp_type, bool is_packed)
{
	strip_enclosed_expression(exp_str);

	string func_name;

	// Square and packed matrices can just use transpose
	if (exp_type.columns == exp_type.vecsize || is_packed)
		func_name = "transpose";
	else
		func_name = string("spvConvertFromRowMajor") + to_string(exp_type.columns) + "x" + to_string(exp_type.vecsize);

	return join(func_name, "(", exp_str, ")");
}

// Called automatically at the end of the entry point function
void CompilerMSL::emit_fixup()
{
	if ((get_entry_point().model == ExecutionModelVertex) && stage_out_var_id && !qual_pos_var_name.empty())
	{
		if (options.vertex.fixup_clipspace)
			statement(qual_pos_var_name, ".z = (", qual_pos_var_name, ".z + ", qual_pos_var_name,
			          ".w) * 0.5;       // Adjust clip-space for Metal");

		if (options.vertex.flip_vert_y)
			statement(qual_pos_var_name, ".y = -(", qual_pos_var_name, ".y);", "    // Invert Y-axis for Metal");
	}
}

// Emit a structure member, padding and packing to maintain the correct memeber alignments.
void CompilerMSL::emit_struct_member(const SPIRType &type, uint32_t member_type_id, uint32_t index,
                                     const string &qualifier, uint32_t)
{
	auto &membertype = get<SPIRType>(member_type_id);

	// If this member requires padding to maintain alignment, emit a dummy padding member.
	MSLStructMemberKey key = get_struct_member_key(type.self, index);
	uint32_t pad_len = struct_member_padding[key];
	if (pad_len > 0)
		statement("char pad", to_string(index), "[", to_string(pad_len), "];");

	// If this member is packed, mark it as so.
	string pack_pfx = "";
	if (member_is_packed_type(type, index))
	{
		pack_pfx = "packed_";

		// If we're packing a matrix, output an appropriate typedef
		if (membertype.vecsize > 1 && membertype.columns > 1)
		{
			string base_type = membertype.width == 16 ? "half" : "float";
			string td_line = "typedef ";
			td_line += base_type + to_string(membertype.vecsize) + "x" + to_string(membertype.columns);
			td_line += " " + pack_pfx;
			td_line += base_type + to_string(membertype.columns) + "x" + to_string(membertype.vecsize);
			td_line += ";";
			add_typedef_line(td_line);
		}
	}

	statement(pack_pfx, type_to_glsl(membertype), " ", qualifier, to_member_name(type, index),
	          member_attribute_qualifier(type, index), type_to_array_glsl(membertype), ";");
}

// Return a MSL qualifier for the specified function attribute member
string CompilerMSL::member_attribute_qualifier(const SPIRType &type, uint32_t index)
{
	auto &execution = get_entry_point();

	uint32_t mbr_type_id = type.member_types[index];
	auto &mbr_type = get<SPIRType>(mbr_type_id);

	BuiltIn builtin;
	bool is_builtin = is_member_builtin(type, index, &builtin);

	// Vertex function inputs
	if (execution.model == ExecutionModelVertex && type.storage == StorageClassInput)
	{
		if (is_builtin)
		{
			switch (builtin)
			{
			case BuiltInVertexId:
			case BuiltInVertexIndex:
			case BuiltInInstanceId:
			case BuiltInInstanceIndex:
				return string(" [[") + builtin_qualifier(builtin) + "]]";

			default:
				return "";
			}
		}
		uint32_t locn = get_ordered_member_location(type.self, index);
		if (locn != k_unknown_location)
			return string(" [[attribute(") + convert_to_string(locn) + ")]]";
	}

	// Vertex function outputs
	if (execution.model == ExecutionModelVertex && type.storage == StorageClassOutput)
	{
		if (is_builtin)
		{
			switch (builtin)
			{
			case BuiltInPointSize:
				// Only mark the PointSize builtin if really rendering points.
				// Some shaders may include a PointSize builtin even when used to render
				// non-point topologies, and Metal will reject this builtin when compiling
				// the shader into a render pipeline that uses a non-point topology.
				return msl_options.enable_point_size_builtin ? (string(" [[") + builtin_qualifier(builtin) + "]]") : "";

			case BuiltInPosition:
			case BuiltInLayer:
			case BuiltInClipDistance:
				return string(" [[") + builtin_qualifier(builtin) + "]]" + (mbr_type.array.empty() ? "" : " ");

			default:
				return "";
			}
		}
		uint32_t locn = get_ordered_member_location(type.self, index);
		if (locn != k_unknown_location)
			return string(" [[user(locn") + convert_to_string(locn) + ")]]";
	}

	// Fragment function inputs
	if (execution.model == ExecutionModelFragment && type.storage == StorageClassInput)
	{
		if (is_builtin)
		{
			switch (builtin)
			{
			case BuiltInFrontFacing:
			case BuiltInPointCoord:
			case BuiltInFragCoord:
			case BuiltInSampleId:
			case BuiltInSampleMask:
			case BuiltInLayer:
				return string(" [[") + builtin_qualifier(builtin) + "]]";

			default:
				return "";
			}
		}
		uint32_t locn = get_ordered_member_location(type.self, index);
		if (locn != k_unknown_location)
			return string(" [[user(locn") + convert_to_string(locn) + ")]]";
	}

	// Fragment function outputs
	if (execution.model == ExecutionModelFragment && type.storage == StorageClassOutput)
	{
		if (is_builtin)
		{
			switch (builtin)
			{
			case BuiltInSampleMask:
			case BuiltInFragDepth:
				return string(" [[") + builtin_qualifier(builtin) + "]]";

			default:
				return "";
			}
		}
		uint32_t locn = get_ordered_member_location(type.self, index);
		if (locn != k_unknown_location && has_member_decoration(type.self, index, DecorationIndex))
			return join(" [[color(", locn, "), index(", get_member_decoration(type.self, index, DecorationIndex),
			            ")]]");
		else if (locn != k_unknown_location)
			return join(" [[color(", locn, ")]]");
		else if (has_member_decoration(type.self, index, DecorationIndex))
			return join(" [[index(", get_member_decoration(type.self, index, DecorationIndex), ")]]");
		else
			return "";
	}

	// Compute function inputs
	if (execution.model == ExecutionModelGLCompute && type.storage == StorageClassInput)
	{
		if (is_builtin)
		{
			switch (builtin)
			{
			case BuiltInGlobalInvocationId:
			case BuiltInWorkgroupId:
			case BuiltInNumWorkgroups:
			case BuiltInLocalInvocationId:
			case BuiltInLocalInvocationIndex:
				return string(" [[") + builtin_qualifier(builtin) + "]]";

			default:
				return "";
			}
		}
	}

	return "";
}

// Returns the location decoration of the member with the specified index in the specified type.
// If the location of the member has been explicitly set, that location is used. If not, this
// function assumes the members are ordered in their location order, and simply returns the
// index as the location.
uint32_t CompilerMSL::get_ordered_member_location(uint32_t type_id, uint32_t index)
{
	auto &m = meta.at(type_id);
	if (index < m.members.size())
	{
		auto &dec = m.members[index];
		if (dec.decoration_flags.get(DecorationLocation))
			return dec.location;
	}

	return index;
}

string CompilerMSL::constant_expression(const SPIRConstant &c)
{
	if (!c.subconstants.empty())
	{
		// Handles Arrays and structures.
		string res = "{";
		for (auto &elem : c.subconstants)
		{
			res += constant_expression(get<SPIRConstant>(elem));
			if (&elem != &c.subconstants.back())
				res += ", ";
		}
		res += "}";
		return res;
	}
	else if (c.columns() == 1)
	{
		return constant_expression_vector(c, 0);
	}
	else
	{
		string res = type_to_glsl(get<SPIRType>(c.constant_type)) + "(";
		for (uint32_t col = 0; col < c.columns(); col++)
		{
			res += constant_expression_vector(c, col);
			if (col + 1 < c.columns())
				res += ", ";
		}
		res += ")";
		return res;
	}
}

// Returns the type declaration for a function, including the
// entry type if the current function is the entry point function
string CompilerMSL::func_type_decl(SPIRType &type)
{
	// The regular function return type. If not processing the entry point function, that's all we need
	string return_type = type_to_glsl(type) + type_to_array_glsl(type);
	if (!processing_entry_point)
		return return_type;

	// If an outgoing interface block has been defined, and it should be returned, override the entry point return type
	bool ep_should_return_output = !get_is_rasterization_disabled();
	if (stage_out_var_id && ep_should_return_output)
	{
		auto &so_var = get<SPIRVariable>(stage_out_var_id);
		auto &so_type = get<SPIRType>(so_var.basetype);
		return_type = type_to_glsl(so_type) + type_to_array_glsl(type);
	}

	// Prepend a entry type, based on the execution model
	string entry_type;
	auto &execution = get_entry_point();
	switch (execution.model)
	{
	case ExecutionModelVertex:
		entry_type = "vertex";
		break;
	case ExecutionModelFragment:
		entry_type =
		    execution.flags.get(ExecutionModeEarlyFragmentTests) ? "fragment [[ early_fragment_tests ]]" : "fragment";
		break;
	case ExecutionModelGLCompute:
	case ExecutionModelKernel:
		entry_type = "kernel";
		break;
	default:
		entry_type = "unknown";
		break;
	}

	return entry_type + " " + return_type;
}

// In MSL, address space qualifiers are required for all pointer or reference arguments
string CompilerMSL::get_argument_address_space(const SPIRVariable &argument)
{
	const auto &type = get<SPIRType>(argument.basetype);

	switch (type.storage)
	{
	case StorageClassWorkgroup:
		return "threadgroup";

	case StorageClassStorageBuffer:
	{
		bool readonly = get_buffer_block_flags(argument).get(DecorationNonWritable);
		return readonly ? "const device" : "device";
	}

	case StorageClassUniform:
	case StorageClassUniformConstant:
	case StorageClassPushConstant:
		if (type.basetype == SPIRType::Struct)
		{
			bool ssbo = has_decoration(type.self, DecorationBufferBlock);
			if (ssbo)
			{
				bool readonly = get_buffer_block_flags(argument).get(DecorationNonWritable);
				return readonly ? "const device" : "device";
			}
			else
				return "constant";
		}
		break;

	case StorageClassFunction:
	case StorageClassGeneric:
		// No address space for plain values.
		return type.pointer ? "thread" : "";

	default:
		break;
	}

	return "thread";
}

// Returns a string containing a comma-delimited list of args for the entry point function
string CompilerMSL::entry_point_args(bool append_comma)
{
	string ep_args;

	// Stage-in structure
	if (stage_in_var_id)
	{
		auto &var = get<SPIRVariable>(stage_in_var_id);
		auto &type = get<SPIRType>(var.basetype);

		if (!ep_args.empty())
			ep_args += ", ";

		ep_args += type_to_glsl(type) + " " + to_name(var.self) + " [[stage_in]]";
	}

	// Output resources, sorted by resource index & type
	// We need to sort to work around a bug on macOS 10.13 with NVidia drivers where switching between shaders
	// with different order of buffers can result in issues with buffer assignments inside the driver.
	struct Resource
	{
		Variant *id;
		string name;
		SPIRType::BaseType basetype;
		uint32_t index;
	};

	vector<Resource> resources;

	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();
			auto &type = get<SPIRType>(var.basetype);

			uint32_t var_id = var.self;

			if ((var.storage == StorageClassUniform || var.storage == StorageClassUniformConstant ||
			     var.storage == StorageClassPushConstant || var.storage == StorageClassStorageBuffer) &&
			    !is_hidden_variable(var))
			{
				if (type.basetype == SPIRType::SampledImage)
				{
					resources.push_back(
					    { &id, to_name(var_id), SPIRType::Image, get_metal_resource_index(var, SPIRType::Image) });

					if (type.image.dim != DimBuffer && constexpr_samplers.count(var_id) == 0)
					{
						resources.push_back({ &id, to_sampler_expression(var_id), SPIRType::Sampler,
						                      get_metal_resource_index(var, SPIRType::Sampler) });
					}
				}
				else if (constexpr_samplers.count(var_id) == 0)
				{
					// constexpr samplers are not declared as resources.
					resources.push_back(
					    { &id, to_name(var_id), type.basetype, get_metal_resource_index(var, type.basetype) });
				}
			}
		}
	}

	std::sort(resources.begin(), resources.end(), [](const Resource &lhs, const Resource &rhs) {
		return tie(lhs.basetype, lhs.index) < tie(rhs.basetype, rhs.index);
	});

	for (auto &r : resources)
	{
		auto &var = r.id->get<SPIRVariable>();
		auto &type = get<SPIRType>(var.basetype);

		uint32_t var_id = var.self;

		switch (r.basetype)
		{
		case SPIRType::Struct:
		{
			auto &m = meta.at(type.self);
			if (m.members.size() == 0)
				break;
			if (!ep_args.empty())
				ep_args += ", ";
			ep_args += get_argument_address_space(var) + " " + type_to_glsl(type) + "& " + r.name;
			ep_args += " [[buffer(" + convert_to_string(r.index) + ")]]";
			break;
		}
		case SPIRType::Sampler:
			if (!ep_args.empty())
				ep_args += ", ";
			ep_args += sampler_type(type) + " " + r.name;
			ep_args += " [[sampler(" + convert_to_string(r.index) + ")]]";
			break;
		case SPIRType::Image:
			if (!ep_args.empty())
				ep_args += ", ";
			ep_args += image_type_glsl(type, var_id) + " " + r.name;
			ep_args += " [[texture(" + convert_to_string(r.index) + ")]]";
			break;
		default:
			SPIRV_CROSS_THROW("Unexpected resource type");
			break;
		}
	}

	// Builtin variables
	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();

			uint32_t var_id = var.self;

			if (var.storage == StorageClassInput && is_builtin_variable(var))
			{
				if (!ep_args.empty())
					ep_args += ", ";

				BuiltIn bi_type = meta[var_id].decoration.builtin_type;
				ep_args += builtin_type_decl(bi_type) + " " + to_expression(var_id);
				ep_args += " [[" + builtin_qualifier(bi_type) + "]]";
			}
		}
	}

	// Vertex and instance index built-ins
	if (needs_vertex_idx_arg)
		ep_args += built_in_func_arg(BuiltInVertexIndex, !ep_args.empty());

	if (needs_instance_idx_arg)
		ep_args += built_in_func_arg(BuiltInInstanceIndex, !ep_args.empty());

	if (!ep_args.empty() && append_comma)
		ep_args += ", ";

	return ep_args;
}

// Returns the Metal index of the resource of the specified type as used by the specified variable.
uint32_t CompilerMSL::get_metal_resource_index(SPIRVariable &var, SPIRType::BaseType basetype)
{
	auto &execution = get_entry_point();
	auto &var_dec = meta[var.self].decoration;
	uint32_t var_desc_set = (var.storage == StorageClassPushConstant) ? kPushConstDescSet : var_dec.set;
	uint32_t var_binding = (var.storage == StorageClassPushConstant) ? kPushConstBinding : var_dec.binding;

	// If a matching binding has been specified, find and use it
	for (auto p_res_bind : resource_bindings)
	{
		if (p_res_bind->stage == execution.model && p_res_bind->desc_set == var_desc_set &&
		    p_res_bind->binding == var_binding)
		{

			p_res_bind->used_by_shader = true;
			switch (basetype)
			{
			case SPIRType::Struct:
				return p_res_bind->msl_buffer;
			case SPIRType::Image:
				return p_res_bind->msl_texture;
			case SPIRType::Sampler:
				return p_res_bind->msl_sampler;
			default:
				return 0;
			}
		}
	}

	// If there is no explicit mapping of bindings to MSL, use the declared binding.
	if (has_decoration(var.self, DecorationBinding))
		return get_decoration(var.self, DecorationBinding);

	uint32_t binding_stride = 1;
	auto &type = get<SPIRType>(var.basetype);
	for (uint32_t i = 0; i < uint32_t(type.array.size()); i++)
		binding_stride *= type.array_size_literal[i] ? type.array[i] : get<SPIRConstant>(type.array[i]).scalar();

	// If a binding has not been specified, revert to incrementing resource indices
	uint32_t resource_index;
	switch (basetype)
	{
	case SPIRType::Struct:
		resource_index = next_metal_resource_index.msl_buffer;
		next_metal_resource_index.msl_buffer += binding_stride;
		break;
	case SPIRType::Image:
		resource_index = next_metal_resource_index.msl_texture;
		next_metal_resource_index.msl_texture += binding_stride;
		break;
	case SPIRType::Sampler:
		resource_index = next_metal_resource_index.msl_sampler;
		next_metal_resource_index.msl_sampler += binding_stride;
		break;
	default:
		resource_index = 0;
		break;
	}
	return resource_index;
}

string CompilerMSL::argument_decl(const SPIRFunction::Parameter &arg)
{
	auto &var = get<SPIRVariable>(arg.id);
	auto &type = expression_type(arg.id);

	bool constref = !arg.alias_global_variable && type.pointer && arg.write_count == 0;

	bool type_is_image = type.basetype == SPIRType::Image || type.basetype == SPIRType::SampledImage ||
	                     type.basetype == SPIRType::Sampler;

	// Arrays of images/samplers in MSL are always const.
	if (!type.array.empty() && type_is_image)
		constref = true;

	string decl;
	if (constref)
		decl += "const ";

	bool builtin = is_builtin_variable(var);
	if (builtin)
		decl += builtin_type_decl(static_cast<BuiltIn>(get_decoration(arg.id, DecorationBuiltIn)));
	else
		decl += type_to_glsl(type, arg.id);

	bool opaque_handle = type.storage == StorageClassUniformConstant;

	if (!builtin && !opaque_handle && !type.pointer &&
	    (type.storage == StorageClassFunction || type.storage == StorageClassGeneric))
	{
		// If the argument is a pure value and not an opaque type, we will pass by value.
		decl += " ";
		decl += to_expression(var.self);
	}
	else if (is_array(type) && !type_is_image)
	{
		// Arrays of images and samplers are special cased.
		decl += " (&";
		decl += to_expression(var.self);
		decl += ")";
		decl += type_to_array_glsl(type);
	}
	else if (!opaque_handle)
	{
		decl += "&";
		decl += " ";
		decl += to_expression(var.self);
	}
	else
	{
		decl += " ";
		decl += to_expression(var.self);
	}

	return decl;
}

// If we're currently in the entry point function, and the object
// has a qualified name, use it, otherwise use the standard name.
string CompilerMSL::to_name(uint32_t id, bool allow_alias) const
{
	if (current_function && (current_function->self == entry_point))
	{
		string qual_name = meta.at(id).decoration.qualified_alias;
		if (!qual_name.empty())
			return qual_name;
	}
	return Compiler::to_name(id, allow_alias);
}

// Returns a name that combines the name of the struct with the name of the member, except for Builtins
string CompilerMSL::to_qualified_member_name(const SPIRType &type, uint32_t index)
{
	// Don't qualify Builtin names because they are unique and are treated as such when building expressions
	BuiltIn builtin;
	if (is_member_builtin(type, index, &builtin))
		return builtin_to_glsl(builtin, type.storage);

	// Strip any underscore prefix from member name
	string mbr_name = to_member_name(type, index);
	size_t startPos = mbr_name.find_first_not_of("_");
	mbr_name = (startPos != string::npos) ? mbr_name.substr(startPos) : "";
	return join(to_name(type.self), "_", mbr_name);
}

// Ensures that the specified name is permanently usable by prepending a prefix
// if the first chars are _ and a digit, which indicate a transient name.
string CompilerMSL::ensure_valid_name(string name, string pfx)
{
	return (name.size() >= 2 && name[0] == '_' && isdigit(name[1])) ? (pfx + name) : name;
}

// Replace all names that match MSL keywords or Metal Standard Library functions.
void CompilerMSL::replace_illegal_names()
{
	// FIXME: MSL and GLSL are doing two different things here.
	// Agree on convention and remove this override.
	static const unordered_set<string> keywords = {
		"kernel", "vertex", "fragment", "compute", "bias",
	};

	static const unordered_set<string> illegal_func_names = {
		"main",
		"saturate",
	};

	for (auto &id : ids)
	{
		switch (id.get_type())
		{
		case TypeVariable:
		{
			auto &dec = meta[id.get_id()].decoration;
			if (keywords.find(dec.alias) != end(keywords))
				dec.alias += "0";

			break;
		}

		case TypeFunction:
		{
			auto &dec = meta[id.get_id()].decoration;
			if (illegal_func_names.find(dec.alias) != end(illegal_func_names))
				dec.alias += "0";

			break;
		}

		case TypeType:
		{
			for (auto &mbr_dec : meta[id.get_id()].members)
				if (keywords.find(mbr_dec.alias) != end(keywords))
					mbr_dec.alias += "0";

			break;
		}

		default:
			break;
		}
	}

	for (auto &entry : entry_points)
	{
		// Change both the entry point name and the alias, to keep them synced.
		string &ep_name = entry.second.name;
		if (illegal_func_names.find(ep_name) != end(illegal_func_names))
			ep_name += "0";

		// Always write this because entry point might have been renamed earlier.
		meta[entry.first].decoration.alias = ep_name;
	}

	CompilerGLSL::replace_illegal_names();
}

string CompilerMSL::to_qualifiers_glsl(uint32_t id)
{
	string quals;

	auto &type = expression_type(id);
	if (type.storage == StorageClassWorkgroup)
		quals += "threadgroup ";

	return quals;
}

// The optional id parameter indicates the object whose type we are trying
// to find the description for. It is optional. Most type descriptions do not
// depend on a specific object's use of that type.
string CompilerMSL::type_to_glsl(const SPIRType &type, uint32_t id)
{
	// Ignore the pointer type since GLSL doesn't have pointers.

	string type_name;

	switch (type.basetype)
	{
	case SPIRType::Struct:
		// Need OpName lookup here to get a "sensible" name for a struct.
		return to_name(type.self);

	case SPIRType::Image:
	case SPIRType::SampledImage:
		return image_type_glsl(type, id);

	case SPIRType::Sampler:
		return sampler_type(type);

	case SPIRType::Void:
		return "void";

	case SPIRType::AtomicCounter:
		return "atomic_uint";

	// Scalars
	case SPIRType::Boolean:
		type_name = "bool";
		break;
	case SPIRType::Char:
		type_name = "char";
		break;
	case SPIRType::Int:
		type_name = (type.width == 16 ? "short" : "int");
		break;
	case SPIRType::UInt:
		type_name = (type.width == 16 ? "ushort" : "uint");
		break;
	case SPIRType::Int64:
		type_name = "long"; // Currently unsupported
		break;
	case SPIRType::UInt64:
		type_name = "size_t";
		break;
	case SPIRType::Half:
		type_name = "half";
		break;
	case SPIRType::Float:
		type_name = "float";
		break;
	case SPIRType::Double:
		type_name = "double"; // Currently unsupported
		break;

	default:
		return "unknown_type";
	}

	// Matrix?
	if (type.columns > 1)
		type_name += to_string(type.columns) + "x";

	// Vector or Matrix?
	if (type.vecsize > 1)
		type_name += to_string(type.vecsize);

	return type_name;
}

std::string CompilerMSL::sampler_type(const SPIRType &type)
{
	if (!type.array.empty())
	{
		if (!msl_options.supports_msl_version(2))
			SPIRV_CROSS_THROW("MSL 2.0 or greater is required for arrays of samplers.");

		// Arrays of samplers in MSL must be declared with a special array<T, N> syntax ala C++11 std::array.
		uint32_t array_size =
		    type.array_size_literal.back() ? type.array.back() : get<SPIRConstant>(type.array.back()).scalar();
		if (array_size == 0)
			SPIRV_CROSS_THROW("Unsized array of samplers is not supported in MSL.");

		auto &parent = get<SPIRType>(get_non_pointer_type(type).parent_type);
		return join("array<", sampler_type(parent), ", ", array_size, ">");
	}
	else
		return "sampler";
}

// Returns an MSL string describing the SPIR-V image type
string CompilerMSL::image_type_glsl(const SPIRType &type, uint32_t id)
{
	auto *var = maybe_get<SPIRVariable>(id);
	if (var && var->basevariable)
	{
		// For comparison images, check against the base variable,
		// and not the fake ID which might have been generated for this variable.
		id = var->basevariable;
	}

	if (!type.array.empty())
	{
		if (!msl_options.supports_msl_version(2))
			SPIRV_CROSS_THROW("MSL 2.0 or greater is required for arrays of textures.");

		// Arrays of images in MSL must be declared with a special array<T, N> syntax ala C++11 std::array.
		uint32_t array_size =
		    type.array_size_literal.back() ? type.array.back() : get<SPIRConstant>(type.array.back()).scalar();
		if (array_size == 0)
			SPIRV_CROSS_THROW("Unsized array of images is not supported in MSL.");

		auto &parent = get<SPIRType>(get_non_pointer_type(type).parent_type);
		return join("array<", image_type_glsl(parent, id), ", ", array_size, ">");
	}

	string img_type_name;

	// Bypass pointers because we need the real image struct
	auto &img_type = get<SPIRType>(type.self).image;
	if (image_is_comparison(type, id))
	{
		switch (img_type.dim)
		{
		case Dim1D:
			img_type_name += "depth1d_unsupported_by_metal";
			break;
		case Dim2D:
			img_type_name += (img_type.ms ? "depth2d_ms" : (img_type.arrayed ? "depth2d_array" : "depth2d"));
			break;
		case Dim3D:
			img_type_name += "depth3d_unsupported_by_metal";
			break;
		case DimCube:
			img_type_name += (img_type.arrayed ? "depthcube_array" : "depthcube");
			break;
		default:
			img_type_name += "unknown_depth_texture_type";
			break;
		}
	}
	else
	{
		switch (img_type.dim)
		{
		case Dim1D:
			img_type_name += (img_type.arrayed ? "texture1d_array" : "texture1d");
			break;
		case DimBuffer:
		case Dim2D:
		case DimSubpassData:
			img_type_name += (img_type.ms ? "texture2d_ms" : (img_type.arrayed ? "texture2d_array" : "texture2d"));
			break;
		case Dim3D:
			img_type_name += "texture3d";
			break;
		case DimCube:
			img_type_name += (img_type.arrayed ? "texturecube_array" : "texturecube");
			break;
		default:
			img_type_name += "unknown_texture_type";
			break;
		}
	}

	// Append the pixel type
	img_type_name += "<";
	img_type_name += type_to_glsl(get<SPIRType>(img_type.type));

	// For unsampled images, append the sample/read/write access qualifier.
	// For kernel images, the access qualifier my be supplied directly by SPIR-V.
	// Otherwise it may be set based on whether the image is read from or written to within the shader.
	if (type.basetype == SPIRType::Image && type.image.sampled == 2 && type.image.dim != DimSubpassData)
	{
		switch (img_type.access)
		{
		case AccessQualifierReadOnly:
			img_type_name += ", access::read";
			break;

		case AccessQualifierWriteOnly:
			img_type_name += ", access::write";
			break;

		case AccessQualifierReadWrite:
			img_type_name += ", access::read_write";
			break;

		default:
		{
			auto *p_var = maybe_get_backing_variable(id);
			if (p_var && p_var->basevariable)
				p_var = maybe_get<SPIRVariable>(p_var->basevariable);
			if (p_var && !has_decoration(p_var->self, DecorationNonWritable))
			{
				img_type_name += ", access::";

				if (!has_decoration(p_var->self, DecorationNonReadable))
					img_type_name += "read_";

				img_type_name += "write";
			}
			break;
		}
		}
	}

	img_type_name += ">";

	return img_type_name;
}

string CompilerMSL::bitcast_glsl_op(const SPIRType &out_type, const SPIRType &in_type)
{
	if ((out_type.basetype == SPIRType::UInt && in_type.basetype == SPIRType::Int) ||
	    (out_type.basetype == SPIRType::Int && in_type.basetype == SPIRType::UInt) ||
	    (out_type.basetype == SPIRType::UInt64 && in_type.basetype == SPIRType::Int64) ||
	    (out_type.basetype == SPIRType::Int64 && in_type.basetype == SPIRType::UInt64))
		return type_to_glsl(out_type);

	if ((out_type.basetype == SPIRType::UInt && in_type.basetype == SPIRType::Float) ||
	    (out_type.basetype == SPIRType::Int && in_type.basetype == SPIRType::Float) ||
	    (out_type.basetype == SPIRType::Float && in_type.basetype == SPIRType::UInt) ||
	    (out_type.basetype == SPIRType::Float && in_type.basetype == SPIRType::Int) ||
	    (out_type.basetype == SPIRType::Int64 && in_type.basetype == SPIRType::Double) ||
	    (out_type.basetype == SPIRType::UInt64 && in_type.basetype == SPIRType::Double) ||
	    (out_type.basetype == SPIRType::Double && in_type.basetype == SPIRType::Int64) ||
	    (out_type.basetype == SPIRType::Double && in_type.basetype == SPIRType::UInt64) ||
	    (out_type.basetype == SPIRType::Half && in_type.basetype == SPIRType::UInt) ||
	    (out_type.basetype == SPIRType::UInt && in_type.basetype == SPIRType::Half))
		return "as_type<" + type_to_glsl(out_type) + ">";

	return "";
}

// Returns an MSL string identifying the name of a SPIR-V builtin.
// Output builtins are qualified with the name of the stage out structure.
string CompilerMSL::builtin_to_glsl(BuiltIn builtin, StorageClass storage)
{
	switch (builtin)
	{

	// Override GLSL compiler strictness
	case BuiltInVertexId:
		return "gl_VertexID";
	case BuiltInInstanceId:
		return "gl_InstanceID";
	case BuiltInVertexIndex:
		return "gl_VertexIndex";
	case BuiltInInstanceIndex:
		return "gl_InstanceIndex";

	// When used in the entry function, output builtins are qualified with output struct name.
	// Test storage class as NOT Input, as output builtins might be part of generic type.
	case BuiltInPosition:
	case BuiltInPointSize:
	case BuiltInClipDistance:
	case BuiltInCullDistance:
	case BuiltInLayer:
	case BuiltInFragDepth:
	case BuiltInSampleMask:
		if (storage != StorageClassInput && current_function && (current_function->self == entry_point))
			return stage_out_var_name + "." + CompilerGLSL::builtin_to_glsl(builtin, storage);

		break;

	default:
		break;
	}

	return CompilerGLSL::builtin_to_glsl(builtin, storage);
}

// Returns an MSL string attribute qualifer for a SPIR-V builtin
string CompilerMSL::builtin_qualifier(BuiltIn builtin)
{
	auto &execution = get_entry_point();

	switch (builtin)
	{
	// Vertex function in
	case BuiltInVertexId:
		return "vertex_id";
	case BuiltInVertexIndex:
		return "vertex_id";
	case BuiltInInstanceId:
		return "instance_id";
	case BuiltInInstanceIndex:
		return "instance_id";

	// Vertex function out
	case BuiltInClipDistance:
		return "clip_distance";
	case BuiltInPointSize:
		return "point_size";
	case BuiltInPosition:
		return "position";
	case BuiltInLayer:
		return "render_target_array_index";

	// Fragment function in
	case BuiltInFrontFacing:
		return "front_facing";
	case BuiltInPointCoord:
		return "point_coord";
	case BuiltInFragCoord:
		return "position";
	case BuiltInSampleId:
		return "sample_id";
	case BuiltInSampleMask:
		return "sample_mask";

	// Fragment function out
	case BuiltInFragDepth:
		if (execution.flags.get(ExecutionModeDepthGreater))
			return "depth(greater)";
		else if (execution.flags.get(ExecutionModeDepthLess))
			return "depth(less)";
		else
			return "depth(any)";

	// Compute function in
	case BuiltInGlobalInvocationId:
		return "thread_position_in_grid";

	case BuiltInWorkgroupId:
		return "threadgroup_position_in_grid";

	case BuiltInNumWorkgroups:
		return "threadgroups_per_grid";

	case BuiltInLocalInvocationId:
		return "thread_position_in_threadgroup";

	case BuiltInLocalInvocationIndex:
		return "thread_index_in_threadgroup";

	default:
		return "unsupported-built-in";
	}
}

// Returns an MSL string type declaration for a SPIR-V builtin
string CompilerMSL::builtin_type_decl(BuiltIn builtin)
{
	switch (builtin)
	{
	// Vertex function in
	case BuiltInVertexId:
		return "uint";
	case BuiltInVertexIndex:
		return "uint";
	case BuiltInInstanceId:
		return "uint";
	case BuiltInInstanceIndex:
		return "uint";

	// Vertex function out
	case BuiltInClipDistance:
		return "float";
	case BuiltInPointSize:
		return "float";
	case BuiltInPosition:
		return "float4";
	case BuiltInLayer:
		return "uint";

	// Fragment function in
	case BuiltInFrontFacing:
		return "bool";
	case BuiltInPointCoord:
		return "float2";
	case BuiltInFragCoord:
		return "float4";
	case BuiltInSampleId:
		return "uint";
	case BuiltInSampleMask:
		return "uint";

	// Compute function in
	case BuiltInGlobalInvocationId:
	case BuiltInLocalInvocationId:
	case BuiltInNumWorkgroups:
	case BuiltInWorkgroupId:
		return "uint3";
	case BuiltInLocalInvocationIndex:
		return "uint";

	default:
		return "unsupported-built-in-type";
	}
}

// Returns the declaration of a built-in argument to a function
string CompilerMSL::built_in_func_arg(BuiltIn builtin, bool prefix_comma)
{
	string bi_arg;
	if (prefix_comma)
		bi_arg += ", ";

	bi_arg += builtin_type_decl(builtin);
	bi_arg += " " + builtin_to_glsl(builtin, StorageClassInput);
	bi_arg += " [[" + builtin_qualifier(builtin) + "]]";

	return bi_arg;
}

// Returns the byte size of a struct member.
size_t CompilerMSL::get_declared_struct_member_size(const SPIRType &struct_type, uint32_t index) const
{
	auto &type = get<SPIRType>(struct_type.member_types[index]);

	switch (type.basetype)
	{
	case SPIRType::Unknown:
	case SPIRType::Void:
	case SPIRType::AtomicCounter:
	case SPIRType::Image:
	case SPIRType::SampledImage:
	case SPIRType::Sampler:
		SPIRV_CROSS_THROW("Querying size of opaque object.");

	default:
	{
		// For arrays, we can use ArrayStride to get an easy check.
		// Runtime arrays will have zero size so force to min of one.
		if (!type.array.empty())
		{
			bool array_size_literal = type.array_size_literal.back();
			uint32_t array_size =
			    array_size_literal ? type.array.back() : get<SPIRConstant>(type.array.back()).scalar();
			return type_struct_member_array_stride(struct_type, index) * max(array_size, 1u);
		}

		if (type.basetype == SPIRType::Struct)
			return get_declared_struct_size(type);

		uint32_t component_size = type.width / 8;
		uint32_t vecsize = type.vecsize;
		uint32_t columns = type.columns;

		// An unpacked 3-element vector or matrix column is the same memory size as a 4-element.
		if (vecsize == 3 && !has_member_decoration(struct_type.self, index, DecorationCPacked))
			vecsize = 4;

		return component_size * vecsize * columns;
	}
	}
}

// Returns the byte alignment of a struct member.
size_t CompilerMSL::get_declared_struct_member_alignment(const SPIRType &struct_type, uint32_t index) const
{
	auto &type = get<SPIRType>(struct_type.member_types[index]);

	switch (type.basetype)
	{
	case SPIRType::Unknown:
	case SPIRType::Void:
	case SPIRType::AtomicCounter:
	case SPIRType::Image:
	case SPIRType::SampledImage:
	case SPIRType::Sampler:
		SPIRV_CROSS_THROW("Querying alignment of opaque object.");

	case SPIRType::Struct:
		return 16; // Per Vulkan spec section 14.5.4

	default:
	{
		// Alignment of packed type is the same as the underlying component or column size.
		// Alignment of unpacked type is the same as the vector size.
		// Alignment of 3-elements vector is the same as 4-elements (including packed using column).
		if (member_is_packed_type(struct_type, index))
			return (type.width / 8) * (type.columns == 3 ? 4 : type.columns);
		else
			return (type.width / 8) * (type.vecsize == 3 ? 4 : type.vecsize);
	}
	}
}

bool CompilerMSL::skip_argument(uint32_t) const
{
	return false;
}

bool CompilerMSL::OpCodePreprocessor::handle(Op opcode, const uint32_t *args, uint32_t length)
{
	// Since MSL exists in a single execution scope, function prototype declarations are not
	// needed, and clutter the output. If secondary functions are output (either as a SPIR-V
	// function implementation or as indicated by the presence of OpFunctionCall), then set
	// suppress_missing_prototypes to suppress compiler warnings of missing function prototypes.

	// Mark if the input requires the implementation of an SPIR-V function that does not exist in Metal.
	SPVFuncImpl spv_func = get_spv_func_impl(opcode, args);
	if (spv_func != SPVFuncImplNone)
	{
		compiler.spv_function_implementations.insert(spv_func);
		suppress_missing_prototypes = true;
	}

	switch (opcode)
	{

	case OpFunctionCall:
		suppress_missing_prototypes = true;
		break;

	case OpImageWrite:
		uses_resource_write = true;
		break;

	case OpStore:
		check_resource_write(args[0]);
		break;

	case OpAtomicExchange:
	case OpAtomicCompareExchange:
	case OpAtomicCompareExchangeWeak:
	case OpAtomicIIncrement:
	case OpAtomicIDecrement:
	case OpAtomicIAdd:
	case OpAtomicISub:
	case OpAtomicSMin:
	case OpAtomicUMin:
	case OpAtomicSMax:
	case OpAtomicUMax:
	case OpAtomicAnd:
	case OpAtomicOr:
	case OpAtomicXor:
		uses_atomics = true;
		check_resource_write(args[2]);
		break;

	case OpAtomicLoad:
		uses_atomics = true;
		break;

	default:
		break;
	}

	// If it has one, keep track of the instruction's result type, mapped by ID
	uint32_t result_type, result_id;
	if (compiler.instruction_to_result_type(result_type, result_id, opcode, args, length))
		result_types[result_id] = result_type;

	return true;
}

// If the variable is a Uniform or StorageBuffer, mark that a resource has been written to.
void CompilerMSL::OpCodePreprocessor::check_resource_write(uint32_t var_id)
{
	auto *p_var = compiler.maybe_get_backing_variable(var_id);
	StorageClass sc = p_var ? p_var->storage : StorageClassMax;
	if (sc == StorageClassUniform || sc == StorageClassStorageBuffer)
		uses_resource_write = true;
}

// Returns an enumeration of a SPIR-V function that needs to be output for certain Op codes.
CompilerMSL::SPVFuncImpl CompilerMSL::OpCodePreprocessor::get_spv_func_impl(Op opcode, const uint32_t *args)
{
	switch (opcode)
	{
	case OpFMod:
		return SPVFuncImplMod;

	case OpFunctionCall:
	{
		auto &return_type = compiler.get<SPIRType>(args[0]);
		if (!return_type.array.empty())
			return SPVFuncImplArrayCopy;

		break;
	}

	case OpStore:
	{
		// Get the result type of the RHS. Since this is run as a pre-processing stage,
		// we must extract the result type directly from the Instruction, rather than the ID.
		uint32_t id_lhs = args[0];
		uint32_t id_rhs = args[1];

		const SPIRType *type = nullptr;
		if (compiler.ids[id_rhs].get_type() != TypeNone)
		{
			// Could be a constant, or similar.
			type = &compiler.expression_type(id_rhs);
		}
		else
		{
			// Or ... an expression.
			uint32_t tid = result_types[id_rhs];
			if (tid)
				type = &compiler.get<SPIRType>(tid);
		}

		auto *var = compiler.maybe_get<SPIRVariable>(id_lhs);

		// Are we simply assigning to a statically assigned variable which takes a constant?
		// Don't bother emitting this function.
		bool static_expression_lhs =
		    var && var->storage == StorageClassFunction && var->statically_assigned && var->remapped_variable;
		if (type && compiler.is_array(*type) && !static_expression_lhs)
			return SPVFuncImplArrayCopy;

		break;
	}

	case OpImageFetch:
	{
		// Retrieve the image type, and if it's a Buffer, emit a texel coordinate function
		uint32_t tid = result_types[args[2]];
		if (tid && compiler.get<SPIRType>(tid).image.dim == DimBuffer)
			return SPVFuncImplTexelBufferCoords;

		break;
	}

	case OpExtInst:
	{
		uint32_t extension_set = args[2];
		if (compiler.get<SPIRExtension>(extension_set).ext == SPIRExtension::GLSL)
		{
			GLSLstd450 op_450 = static_cast<GLSLstd450>(args[3]);
			switch (op_450)
			{
			case GLSLstd450Radians:
				return SPVFuncImplRadians;
			case GLSLstd450Degrees:
				return SPVFuncImplDegrees;
			case GLSLstd450FindILsb:
				return SPVFuncImplFindILsb;
			case GLSLstd450FindSMsb:
				return SPVFuncImplFindSMsb;
			case GLSLstd450FindUMsb:
				return SPVFuncImplFindUMsb;
			case GLSLstd450MatrixInverse:
			{
				auto &mat_type = compiler.get<SPIRType>(args[0]);
				switch (mat_type.columns)
				{
				case 2:
					return SPVFuncImplInverse2x2;
				case 3:
					return SPVFuncImplInverse3x3;
				case 4:
					return SPVFuncImplInverse4x4;
				default:
					break;
				}
				break;
			}
			default:
				break;
			}
		}
		break;
	}

	default:
		break;
	}
	return SPVFuncImplNone;
}

// Sort both type and meta member content based on builtin status (put builtins at end),
// then by the required sorting aspect.
void CompilerMSL::MemberSorter::sort()
{
	// Create a temporary array of consecutive member indices and sort it based on how
	// the members should be reordered, based on builtin and sorting aspect meta info.
	size_t mbr_cnt = type.member_types.size();
	vector<uint32_t> mbr_idxs(mbr_cnt);
	iota(mbr_idxs.begin(), mbr_idxs.end(), 0); // Fill with consecutive indices
	std::sort(mbr_idxs.begin(), mbr_idxs.end(), *this); // Sort member indices based on sorting aspect

	// Move type and meta member info to the order defined by the sorted member indices.
	// This is done by creating temporary copies of both member types and meta, and then
	// copying back to the original content at the sorted indices.
	auto mbr_types_cpy = type.member_types;
	auto mbr_meta_cpy = meta.members;
	for (uint32_t mbr_idx = 0; mbr_idx < mbr_cnt; mbr_idx++)
	{
		type.member_types[mbr_idx] = mbr_types_cpy[mbr_idxs[mbr_idx]];
		meta.members[mbr_idx] = mbr_meta_cpy[mbr_idxs[mbr_idx]];
	}
}

// Sort first by builtin status (put builtins at end), then by the sorting aspect.
bool CompilerMSL::MemberSorter::operator()(uint32_t mbr_idx1, uint32_t mbr_idx2)
{
	auto &mbr_meta1 = meta.members[mbr_idx1];
	auto &mbr_meta2 = meta.members[mbr_idx2];
	if (mbr_meta1.builtin != mbr_meta2.builtin)
		return mbr_meta2.builtin;
	else
		switch (sort_aspect)
		{
		case Location:
			return mbr_meta1.location < mbr_meta2.location;
		case LocationReverse:
			return mbr_meta1.location > mbr_meta2.location;
		case Offset:
			return mbr_meta1.offset < mbr_meta2.offset;
		case OffsetThenLocationReverse:
			return (mbr_meta1.offset < mbr_meta2.offset) ||
			       ((mbr_meta1.offset == mbr_meta2.offset) && (mbr_meta1.location > mbr_meta2.location));
		case Alphabetical:
			return mbr_meta1.alias < mbr_meta2.alias;
		default:
			return false;
		}
}

CompilerMSL::MemberSorter::MemberSorter(SPIRType &t, Meta &m, SortAspect sa)
    : type(t)
    , meta(m)
    , sort_aspect(sa)
{
	// Ensure enough meta info is available
	meta.members.resize(max(type.member_types.size(), meta.members.size()));
}

void CompilerMSL::remap_constexpr_sampler(uint32_t id, const MSLConstexprSampler &sampler)
{
	auto &type = get<SPIRType>(get<SPIRVariable>(id).basetype);
	if (type.basetype != SPIRType::SampledImage && type.basetype != SPIRType::Sampler)
		SPIRV_CROSS_THROW("Can only remap SampledImage and Sampler type.");
	if (!type.array.empty())
		SPIRV_CROSS_THROW("Can not remap array of samplers.");
	constexpr_samplers[id] = sampler;
}

// MSL always declares builtins with their SPIR-V type.
void CompilerMSL::bitcast_from_builtin_load(uint32_t, std::string &, const SPIRType &)
{
}

void CompilerMSL::bitcast_to_builtin_store(uint32_t, std::string &, const SPIRType &)
{
}

std::string CompilerMSL::to_initializer_expression(const SPIRVariable &var)
{
	// We risk getting an array initializer here with MSL. If we have an array.
	// FIXME: We cannot handle non-constant arrays being initialized.
	// We will need to inject spvArrayCopy here somehow ...
	auto &type = get<SPIRType>(var.basetype);
	if (ids[var.initializer].get_type() == TypeConstant && (!type.array.empty() || type.basetype == SPIRType::Struct))
		return constant_expression(get<SPIRConstant>(var.initializer));
	else
		return CompilerGLSL::to_initializer_expression(var);
}
