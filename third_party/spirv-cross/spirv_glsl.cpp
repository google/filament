/*
 * Copyright 2015-2018 ARM Limited
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

#include "spirv_glsl.hpp"
#include "GLSL.std.450.h"
#include "spirv_common.hpp"
#include <algorithm>
#include <assert.h>
#include <cmath>
#include <limits>
#include <utility>

using namespace spv;
using namespace spirv_cross;
using namespace std;

static bool is_unsigned_opcode(Op op)
{
	// Don't have to be exhaustive, only relevant for legacy target checking ...
	switch (op)
	{
	case OpShiftRightLogical:
	case OpUGreaterThan:
	case OpUGreaterThanEqual:
	case OpULessThan:
	case OpULessThanEqual:
	case OpUConvert:
	case OpUDiv:
	case OpUMod:
	case OpUMulExtended:
	case OpConvertUToF:
	case OpConvertFToU:
		return true;

	default:
		return false;
	}
}

static bool is_unsigned_glsl_opcode(GLSLstd450 op)
{
	// Don't have to be exhaustive, only relevant for legacy target checking ...
	switch (op)
	{
	case GLSLstd450UClamp:
	case GLSLstd450UMin:
	case GLSLstd450UMax:
	case GLSLstd450FindUMsb:
		return true;

	default:
		return false;
	}
}

static bool packing_is_vec4_padded(BufferPackingStandard packing)
{
	switch (packing)
	{
	case BufferPackingHLSLCbuffer:
	case BufferPackingHLSLCbufferPackOffset:
	case BufferPackingStd140:
	case BufferPackingStd140EnhancedLayout:
		return true;

	default:
		return false;
	}
}

static bool packing_is_hlsl(BufferPackingStandard packing)
{
	switch (packing)
	{
	case BufferPackingHLSLCbuffer:
	case BufferPackingHLSLCbufferPackOffset:
		return true;

	default:
		return false;
	}
}

static bool packing_has_flexible_offset(BufferPackingStandard packing)
{
	switch (packing)
	{
	case BufferPackingStd140:
	case BufferPackingStd430:
	case BufferPackingHLSLCbuffer:
		return false;

	default:
		return true;
	}
}

static BufferPackingStandard packing_to_substruct_packing(BufferPackingStandard packing)
{
	switch (packing)
	{
	case BufferPackingStd140EnhancedLayout:
		return BufferPackingStd140;
	case BufferPackingStd430EnhancedLayout:
		return BufferPackingStd430;
	case BufferPackingHLSLCbufferPackOffset:
		return BufferPackingHLSLCbuffer;
	default:
		return packing;
	}
}

// Sanitizes underscores for GLSL where multiple underscores in a row are not allowed.
string CompilerGLSL::sanitize_underscores(const string &str)
{
	string res;
	res.reserve(str.size());

	bool last_underscore = false;
	for (auto c : str)
	{
		if (c == '_')
		{
			if (last_underscore)
				continue;

			res += c;
			last_underscore = true;
		}
		else
		{
			res += c;
			last_underscore = false;
		}
	}
	return res;
}

// Returns true if an arithmetic operation does not change behavior depending on signedness.
static bool glsl_opcode_is_sign_invariant(Op opcode)
{
	switch (opcode)
	{
	case OpIEqual:
	case OpINotEqual:
	case OpISub:
	case OpIAdd:
	case OpIMul:
	case OpShiftLeftLogical:
	case OpBitwiseOr:
	case OpBitwiseXor:
	case OpBitwiseAnd:
		return true;

	default:
		return false;
	}
}

static const char *to_pls_layout(PlsFormat format)
{
	switch (format)
	{
	case PlsR11FG11FB10F:
		return "layout(r11f_g11f_b10f) ";
	case PlsR32F:
		return "layout(r32f) ";
	case PlsRG16F:
		return "layout(rg16f) ";
	case PlsRGB10A2:
		return "layout(rgb10_a2) ";
	case PlsRGBA8:
		return "layout(rgba8) ";
	case PlsRG16:
		return "layout(rg16) ";
	case PlsRGBA8I:
		return "layout(rgba8i)";
	case PlsRG16I:
		return "layout(rg16i) ";
	case PlsRGB10A2UI:
		return "layout(rgb10_a2ui) ";
	case PlsRGBA8UI:
		return "layout(rgba8ui) ";
	case PlsRG16UI:
		return "layout(rg16ui) ";
	case PlsR32UI:
		return "layout(r32ui) ";
	default:
		return "";
	}
}

static SPIRType::BaseType pls_format_to_basetype(PlsFormat format)
{
	switch (format)
	{
	default:
	case PlsR11FG11FB10F:
	case PlsR32F:
	case PlsRG16F:
	case PlsRGB10A2:
	case PlsRGBA8:
	case PlsRG16:
		return SPIRType::Float;

	case PlsRGBA8I:
	case PlsRG16I:
		return SPIRType::Int;

	case PlsRGB10A2UI:
	case PlsRGBA8UI:
	case PlsRG16UI:
	case PlsR32UI:
		return SPIRType::UInt;
	}
}

static uint32_t pls_format_to_components(PlsFormat format)
{
	switch (format)
	{
	default:
	case PlsR32F:
	case PlsR32UI:
		return 1;

	case PlsRG16F:
	case PlsRG16:
	case PlsRG16UI:
	case PlsRG16I:
		return 2;

	case PlsR11FG11FB10F:
		return 3;

	case PlsRGB10A2:
	case PlsRGBA8:
	case PlsRGBA8I:
	case PlsRGB10A2UI:
	case PlsRGBA8UI:
		return 4;
	}
}

static const char *vector_swizzle(int vecsize, int index)
{
	static const char *swizzle[4][4] = {
		{ ".x", ".y", ".z", ".w" }, { ".xy", ".yz", ".zw" }, { ".xyz", ".yzw" }, { "" }
	};

	assert(vecsize >= 1 && vecsize <= 4);
	assert(index >= 0 && index < 4);
	assert(swizzle[vecsize - 1][index]);

	return swizzle[vecsize - 1][index];
}

void CompilerGLSL::reset()
{
	// We do some speculative optimizations which should pretty much always work out,
	// but just in case the SPIR-V is rather weird, recompile until it's happy.
	// This typically only means one extra pass.
	force_recompile = false;

	// Clear invalid expression tracking.
	invalid_expressions.clear();
	current_function = nullptr;

	// Clear temporary usage tracking.
	expression_usage_counts.clear();
	forwarded_temporaries.clear();

	resource_names.clear();
	function_overloads.clear();

	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			// Clear unflushed dependees.
			id.get<SPIRVariable>().dependees.clear();
		}
		else if (id.get_type() == TypeExpression)
		{
			// And remove all expressions.
			id.reset();
		}
		else if (id.get_type() == TypeFunction)
		{
			// Reset active state for all functions.
			id.get<SPIRFunction>().active = false;
			id.get<SPIRFunction>().flush_undeclared = true;
		}
	}

	statement_count = 0;
	indent = 0;
}

void CompilerGLSL::remap_pls_variables()
{
	for (auto &input : pls_inputs)
	{
		auto &var = get<SPIRVariable>(input.id);

		bool input_is_target = false;
		if (var.storage == StorageClassUniformConstant)
		{
			auto &type = get<SPIRType>(var.basetype);
			input_is_target = type.image.dim == DimSubpassData;
		}

		if (var.storage != StorageClassInput && !input_is_target)
			SPIRV_CROSS_THROW("Can only use in and target variables for PLS inputs.");
		var.remapped_variable = true;
	}

	for (auto &output : pls_outputs)
	{
		auto &var = get<SPIRVariable>(output.id);
		if (var.storage != StorageClassOutput)
			SPIRV_CROSS_THROW("Can only use out variables for PLS outputs.");
		var.remapped_variable = true;
	}
}

void CompilerGLSL::find_static_extensions()
{
	for (auto &id : ids)
	{
		if (id.get_type() == TypeType)
		{
			auto &type = id.get<SPIRType>();
			if (type.basetype == SPIRType::Double)
			{
				if (options.es)
					SPIRV_CROSS_THROW("FP64 not supported in ES profile.");
				if (!options.es && options.version < 400)
					require_extension_internal("GL_ARB_gpu_shader_fp64");
			}

			if (type.basetype == SPIRType::Int64 || type.basetype == SPIRType::UInt64)
			{
				if (options.es)
					SPIRV_CROSS_THROW("64-bit integers not supported in ES profile.");
				if (!options.es)
					require_extension_internal("GL_ARB_gpu_shader_int64");
			}

			if (type.basetype == SPIRType::Half)
				require_extension_internal("GL_AMD_gpu_shader_half_float");
		}
	}

	auto &execution = get_entry_point();
	switch (execution.model)
	{
	case ExecutionModelGLCompute:
		if (!options.es && options.version < 430)
			require_extension_internal("GL_ARB_compute_shader");
		if (options.es && options.version < 310)
			SPIRV_CROSS_THROW("At least ESSL 3.10 required for compute shaders.");
		break;

	case ExecutionModelGeometry:
		if (options.es && options.version < 320)
			require_extension_internal("GL_EXT_geometry_shader");
		if (!options.es && options.version < 150)
			require_extension_internal("GL_ARB_geometry_shader4");

		if (execution.flags.get(ExecutionModeInvocations) && execution.invocations != 1)
		{
			// Instanced GS is part of 400 core or this extension.
			if (!options.es && options.version < 400)
				require_extension_internal("GL_ARB_gpu_shader5");
		}
		break;

	case ExecutionModelTessellationEvaluation:
	case ExecutionModelTessellationControl:
		if (options.es && options.version < 320)
			require_extension_internal("GL_EXT_tessellation_shader");
		if (!options.es && options.version < 400)
			require_extension_internal("GL_ARB_tessellation_shader");
		break;

	default:
		break;
	}

	if (!pls_inputs.empty() || !pls_outputs.empty())
		require_extension_internal("GL_EXT_shader_pixel_local_storage");

	if (options.separate_shader_objects && !options.es && options.version < 410)
		require_extension_internal("GL_ARB_separate_shader_objects");
}

string CompilerGLSL::compile()
{
	// Force a classic "C" locale, reverts when function returns
	ClassicLocale classic_locale;

	if (options.vulkan_semantics)
		backend.allow_precision_qualifiers = true;
	backend.force_gl_in_out_block = true;
	backend.supports_extensions = true;

	// Scan the SPIR-V to find trivial uses of extensions.
	build_function_control_flow_graphs_and_analyze();
	find_static_extensions();
	fixup_image_load_store_access();
	update_active_builtins();
	analyze_image_and_sampler_usage();

	uint32_t pass_count = 0;
	do
	{
		if (pass_count >= 3)
			SPIRV_CROSS_THROW("Over 3 compilation loops detected. Must be a bug!");

		reset();

		// Move constructor for this type is broken on GCC 4.9 ...
		buffer = unique_ptr<ostringstream>(new ostringstream());

		emit_header();
		emit_resources();

		emit_function(get<SPIRFunction>(entry_point), Bitset());

		pass_count++;
	} while (force_recompile);

	// Entry point in GLSL is always main().
	get_entry_point().name = "main";

	return buffer->str();
}

std::string CompilerGLSL::get_partial_source()
{
	return buffer ? buffer->str() : "No compiled source available yet.";
}

void CompilerGLSL::emit_header()
{
	auto &execution = get_entry_point();
	statement("#version ", options.version, options.es && options.version > 100 ? " es" : "");

	if (!options.es && options.version < 420)
	{
		// Needed for binding = # on UBOs, etc.
		if (options.enable_420pack_extension)
		{
			statement("#ifdef GL_ARB_shading_language_420pack");
			statement("#extension GL_ARB_shading_language_420pack : require");
			statement("#endif");
		}
		// Needed for: layout(early_fragment_tests) in;
		if (execution.flags.get(ExecutionModeEarlyFragmentTests))
			require_extension_internal("GL_ARB_shader_image_load_store");
	}

	for (auto &ext : forced_extensions)
		statement("#extension ", ext, " : require");

	for (auto &header : header_lines)
		statement(header);

	vector<string> inputs;
	vector<string> outputs;

	switch (execution.model)
	{
	case ExecutionModelGeometry:
		outputs.push_back(join("max_vertices = ", execution.output_vertices));
		if ((execution.flags.get(ExecutionModeInvocations)) && execution.invocations != 1)
			inputs.push_back(join("invocations = ", execution.invocations));
		if (execution.flags.get(ExecutionModeInputPoints))
			inputs.push_back("points");
		if (execution.flags.get(ExecutionModeInputLines))
			inputs.push_back("lines");
		if (execution.flags.get(ExecutionModeInputLinesAdjacency))
			inputs.push_back("lines_adjacency");
		if (execution.flags.get(ExecutionModeTriangles))
			inputs.push_back("triangles");
		if (execution.flags.get(ExecutionModeInputTrianglesAdjacency))
			inputs.push_back("triangles_adjacency");
		if (execution.flags.get(ExecutionModeOutputTriangleStrip))
			outputs.push_back("triangle_strip");
		if (execution.flags.get(ExecutionModeOutputPoints))
			outputs.push_back("points");
		if (execution.flags.get(ExecutionModeOutputLineStrip))
			outputs.push_back("line_strip");
		break;

	case ExecutionModelTessellationControl:
		if (execution.flags.get(ExecutionModeOutputVertices))
			outputs.push_back(join("vertices = ", execution.output_vertices));
		break;

	case ExecutionModelTessellationEvaluation:
		if (execution.flags.get(ExecutionModeQuads))
			inputs.push_back("quads");
		if (execution.flags.get(ExecutionModeTriangles))
			inputs.push_back("triangles");
		if (execution.flags.get(ExecutionModeIsolines))
			inputs.push_back("isolines");
		if (execution.flags.get(ExecutionModePointMode))
			inputs.push_back("point_mode");

		if (!execution.flags.get(ExecutionModeIsolines))
		{
			if (execution.flags.get(ExecutionModeVertexOrderCw))
				inputs.push_back("cw");
			if (execution.flags.get(ExecutionModeVertexOrderCcw))
				inputs.push_back("ccw");
		}

		if (execution.flags.get(ExecutionModeSpacingFractionalEven))
			inputs.push_back("fractional_even_spacing");
		if (execution.flags.get(ExecutionModeSpacingFractionalOdd))
			inputs.push_back("fractional_odd_spacing");
		if (execution.flags.get(ExecutionModeSpacingEqual))
			inputs.push_back("equal_spacing");
		break;

	case ExecutionModelGLCompute:
	{
		if (execution.workgroup_size.constant != 0)
		{
			SpecializationConstant wg_x, wg_y, wg_z;
			get_work_group_size_specialization_constants(wg_x, wg_y, wg_z);

			if (wg_x.id)
			{
				if (options.vulkan_semantics)
					inputs.push_back(join("local_size_x_id = ", wg_x.constant_id));
				else
					inputs.push_back(join("local_size_x = ", get<SPIRConstant>(wg_x.id).scalar()));
			}
			else
				inputs.push_back(join("local_size_x = ", execution.workgroup_size.x));

			if (wg_y.id)
			{
				if (options.vulkan_semantics)
					inputs.push_back(join("local_size_y_id = ", wg_y.constant_id));
				else
					inputs.push_back(join("local_size_y = ", get<SPIRConstant>(wg_y.id).scalar()));
			}
			else
				inputs.push_back(join("local_size_y = ", execution.workgroup_size.y));

			if (wg_z.id)
			{
				if (options.vulkan_semantics)
					inputs.push_back(join("local_size_z_id = ", wg_z.constant_id));
				else
					inputs.push_back(join("local_size_z = ", get<SPIRConstant>(wg_z.id).scalar()));
			}
			else
				inputs.push_back(join("local_size_z = ", execution.workgroup_size.z));
		}
		else
		{
			inputs.push_back(join("local_size_x = ", execution.workgroup_size.x));
			inputs.push_back(join("local_size_y = ", execution.workgroup_size.y));
			inputs.push_back(join("local_size_z = ", execution.workgroup_size.z));
		}
		break;
	}

	case ExecutionModelFragment:
		if (options.es)
		{
			switch (options.fragment.default_float_precision)
			{
			case Options::Lowp:
				statement("precision lowp float;");
				break;

			case Options::Mediump:
				statement("precision mediump float;");
				break;

			case Options::Highp:
				statement("precision highp float;");
				break;

			default:
				break;
			}

			switch (options.fragment.default_int_precision)
			{
			case Options::Lowp:
				statement("precision lowp int;");
				break;

			case Options::Mediump:
				statement("precision mediump int;");
				break;

			case Options::Highp:
				statement("precision highp int;");
				break;

			default:
				break;
			}
		}

		if (execution.flags.get(ExecutionModeEarlyFragmentTests))
			inputs.push_back("early_fragment_tests");
		if (execution.flags.get(ExecutionModeDepthGreater))
			inputs.push_back("depth_greater");
		if (execution.flags.get(ExecutionModeDepthLess))
			inputs.push_back("depth_less");

		break;

	default:
		break;
	}

	if (!inputs.empty())
		statement("layout(", merge(inputs), ") in;");
	if (!outputs.empty())
		statement("layout(", merge(outputs), ") out;");

	statement("");
}

bool CompilerGLSL::type_is_empty(const SPIRType &type)
{
	return type.basetype == SPIRType::Struct && type.member_types.empty();
}

void CompilerGLSL::emit_struct(SPIRType &type)
{
	// Struct types can be stamped out multiple times
	// with just different offsets, matrix layouts, etc ...
	// Type-punning with these types is legal, which complicates things
	// when we are storing struct and array types in an SSBO for example.
	// If the type master is packed however, we can no longer assume that the struct declaration will be redundant.
	if (type.type_alias != 0 && !has_decoration(type.type_alias, DecorationCPacked))
		return;

	add_resource_name(type.self);
	auto name = type_to_glsl(type);

	statement(!backend.explicit_struct_type ? "struct " : "", name);
	begin_scope();

	type.member_name_cache.clear();

	uint32_t i = 0;
	bool emitted = false;
	for (auto &member : type.member_types)
	{
		add_member_name(type, i);
		emit_struct_member(type, member, i);
		i++;
		emitted = true;
	}

	// Don't declare empty structs in GLSL, this is not allowed.
	if (type_is_empty(type) && !backend.supports_empty_struct)
	{
		statement("int empty_struct_member;");
		emitted = true;
	}

	end_scope_decl();

	if (emitted)
		statement("");
}

string CompilerGLSL::to_interpolation_qualifiers(const Bitset &flags)
{
	string res;
	//if (flags & (1ull << DecorationSmooth))
	//    res += "smooth ";
	if (flags.get(DecorationFlat))
		res += "flat ";
	if (flags.get(DecorationNoPerspective))
		res += "noperspective ";
	if (flags.get(DecorationCentroid))
		res += "centroid ";
	if (flags.get(DecorationPatch))
		res += "patch ";
	if (flags.get(DecorationSample))
		res += "sample ";
	if (flags.get(DecorationInvariant))
		res += "invariant ";
	if (flags.get(DecorationExplicitInterpAMD))
		res += "__explicitInterpAMD ";

	return res;
}

string CompilerGLSL::layout_for_member(const SPIRType &type, uint32_t index)
{
	if (is_legacy())
		return "";

	bool is_block = meta[type.self].decoration.decoration_flags.get(DecorationBlock) ||
	                meta[type.self].decoration.decoration_flags.get(DecorationBufferBlock);
	if (!is_block)
		return "";

	auto &memb = meta[type.self].members;
	if (index >= memb.size())
		return "";
	auto &dec = memb[index];

	vector<string> attr;

	// We can only apply layouts on members in block interfaces.
	// This is a bit problematic because in SPIR-V decorations are applied on the struct types directly.
	// This is not supported on GLSL, so we have to make the assumption that if a struct within our buffer block struct
	// has a decoration, it was originally caused by a top-level layout() qualifier in GLSL.
	//
	// We would like to go from (SPIR-V style):
	//
	// struct Foo { layout(row_major) mat4 matrix; };
	// buffer UBO { Foo foo; };
	//
	// to
	//
	// struct Foo { mat4 matrix; }; // GLSL doesn't support any layout shenanigans in raw struct declarations.
	// buffer UBO { layout(row_major) Foo foo; }; // Apply the layout on top-level.
	auto flags = combined_decoration_for_member(type, index);

	if (flags.get(DecorationRowMajor))
		attr.push_back("row_major");
	// We don't emit any global layouts, so column_major is default.
	//if (flags & (1ull << DecorationColMajor))
	//    attr.push_back("column_major");

	if (dec.decoration_flags.get(DecorationLocation) && can_use_io_location(type.storage, true))
		attr.push_back(join("location = ", dec.location));

	// DecorationCPacked is set by layout_for_variable earlier to mark that we need to emit offset qualifiers.
	// This is only done selectively in GLSL as needed.
	if (has_decoration(type.self, DecorationCPacked) && dec.decoration_flags.get(DecorationOffset))
		attr.push_back(join("offset = ", dec.offset));

	if (attr.empty())
		return "";

	string res = "layout(";
	res += merge(attr);
	res += ") ";
	return res;
}

const char *CompilerGLSL::format_to_glsl(spv::ImageFormat format)
{
	if (options.es && is_desktop_only_format(format))
		SPIRV_CROSS_THROW("Attempting to use image format not supported in ES profile.");

	switch (format)
	{
	case ImageFormatRgba32f:
		return "rgba32f";
	case ImageFormatRgba16f:
		return "rgba16f";
	case ImageFormatR32f:
		return "r32f";
	case ImageFormatRgba8:
		return "rgba8";
	case ImageFormatRgba8Snorm:
		return "rgba8_snorm";
	case ImageFormatRg32f:
		return "rg32f";
	case ImageFormatRg16f:
		return "rg16f";
	case ImageFormatRgba32i:
		return "rgba32i";
	case ImageFormatRgba16i:
		return "rgba16i";
	case ImageFormatR32i:
		return "r32i";
	case ImageFormatRgba8i:
		return "rgba8i";
	case ImageFormatRg32i:
		return "rg32i";
	case ImageFormatRg16i:
		return "rg16i";
	case ImageFormatRgba32ui:
		return "rgba32ui";
	case ImageFormatRgba16ui:
		return "rgba16ui";
	case ImageFormatR32ui:
		return "r32ui";
	case ImageFormatRgba8ui:
		return "rgba8ui";
	case ImageFormatRg32ui:
		return "rg32ui";
	case ImageFormatRg16ui:
		return "rg16ui";
	case ImageFormatR11fG11fB10f:
		return "r11f_g11f_b10f";
	case ImageFormatR16f:
		return "r16f";
	case ImageFormatRgb10A2:
		return "rgb10_a2";
	case ImageFormatR8:
		return "r8";
	case ImageFormatRg8:
		return "rg8";
	case ImageFormatR16:
		return "r16";
	case ImageFormatRg16:
		return "rg16";
	case ImageFormatRgba16:
		return "rgba16";
	case ImageFormatR16Snorm:
		return "r16_snorm";
	case ImageFormatRg16Snorm:
		return "rg16_snorm";
	case ImageFormatRgba16Snorm:
		return "rgba16_snorm";
	case ImageFormatR8Snorm:
		return "r8_snorm";
	case ImageFormatRg8Snorm:
		return "rg8_snorm";
	case ImageFormatR8ui:
		return "r8ui";
	case ImageFormatRg8ui:
		return "rg8ui";
	case ImageFormatR16ui:
		return "r16ui";
	case ImageFormatRgb10a2ui:
		return "rgb10_a2ui";
	case ImageFormatR8i:
		return "r8i";
	case ImageFormatRg8i:
		return "rg8i";
	case ImageFormatR16i:
		return "r16i";
	default:
	case ImageFormatUnknown:
		return nullptr;
	}
}

uint32_t CompilerGLSL::type_to_packed_base_size(const SPIRType &type, BufferPackingStandard)
{
	switch (type.basetype)
	{
	case SPIRType::Double:
	case SPIRType::Int64:
	case SPIRType::UInt64:
		return 8;
	case SPIRType::Float:
	case SPIRType::Int:
	case SPIRType::UInt:
		return 4;
	case SPIRType::Half:
		return 2;

	default:
		SPIRV_CROSS_THROW("Unrecognized type in type_to_packed_base_size.");
	}
}

uint32_t CompilerGLSL::type_to_packed_alignment(const SPIRType &type, const Bitset &flags,
                                                BufferPackingStandard packing)
{
	if (!type.array.empty())
	{
		uint32_t minimum_alignment = 1;
		if (packing_is_vec4_padded(packing))
			minimum_alignment = 16;

		auto *tmp = &get<SPIRType>(type.parent_type);
		while (!tmp->array.empty())
			tmp = &get<SPIRType>(tmp->parent_type);

		// Get the alignment of the base type, then maybe round up.
		return max(minimum_alignment, type_to_packed_alignment(*tmp, flags, packing));
	}

	if (type.basetype == SPIRType::Struct)
	{
		// Rule 9. Structs alignments are maximum alignment of its members.
		uint32_t alignment = 0;
		for (uint32_t i = 0; i < type.member_types.size(); i++)
		{
			auto member_flags = meta[type.self].members.at(i).decoration_flags;
			alignment =
			    max(alignment, type_to_packed_alignment(get<SPIRType>(type.member_types[i]), member_flags, packing));
		}

		// In std140, struct alignment is rounded up to 16.
		if (packing_is_vec4_padded(packing))
			alignment = max(alignment, 16u);

		return alignment;
	}
	else
	{
		const uint32_t base_alignment = type_to_packed_base_size(type, packing);

		// Vectors are *not* aligned in HLSL, but there's an extra rule where vectors cannot straddle
		// a vec4, this is handled outside since that part knows our current offset.
		if (type.columns == 1 && packing_is_hlsl(packing))
			return base_alignment;

		// From 7.6.2.2 in GL 4.5 core spec.
		// Rule 1
		if (type.vecsize == 1 && type.columns == 1)
			return base_alignment;

		// Rule 2
		if ((type.vecsize == 2 || type.vecsize == 4) && type.columns == 1)
			return type.vecsize * base_alignment;

		// Rule 3
		if (type.vecsize == 3 && type.columns == 1)
			return 4 * base_alignment;

		// Rule 4 implied. Alignment does not change in std430.

		// Rule 5. Column-major matrices are stored as arrays of
		// vectors.
		if (flags.get(DecorationColMajor) && type.columns > 1)
		{
			if (packing_is_vec4_padded(packing))
				return 4 * base_alignment;
			else if (type.vecsize == 3)
				return 4 * base_alignment;
			else
				return type.vecsize * base_alignment;
		}

		// Rule 6 implied.

		// Rule 7.
		if (flags.get(DecorationRowMajor) && type.vecsize > 1)
		{
			if (packing_is_vec4_padded(packing))
				return 4 * base_alignment;
			else if (type.columns == 3)
				return 4 * base_alignment;
			else
				return type.columns * base_alignment;
		}

		// Rule 8 implied.
	}

	SPIRV_CROSS_THROW("Did not find suitable rule for type. Bogus decorations?");
}

uint32_t CompilerGLSL::type_to_packed_array_stride(const SPIRType &type, const Bitset &flags,
                                                   BufferPackingStandard packing)
{
	// Array stride is equal to aligned size of the underlying type.
	uint32_t parent = type.parent_type;
	assert(parent);

	auto &tmp = get<SPIRType>(parent);

	uint32_t size = type_to_packed_size(tmp, flags, packing);
	if (tmp.array.empty())
	{
		uint32_t alignment = type_to_packed_alignment(type, flags, packing);
		return (size + alignment - 1) & ~(alignment - 1);
	}
	else
	{
		// For multidimensional arrays, array stride always matches size of subtype.
		// The alignment cannot change because multidimensional arrays are basically N * M array elements.
		return size;
	}
}

uint32_t CompilerGLSL::type_to_packed_size(const SPIRType &type, const Bitset &flags, BufferPackingStandard packing)
{
	if (!type.array.empty())
	{
		return to_array_size_literal(type, uint32_t(type.array.size()) - 1) *
		       type_to_packed_array_stride(type, flags, packing);
	}

	uint32_t size = 0;

	if (type.basetype == SPIRType::Struct)
	{
		uint32_t pad_alignment = 1;

		for (uint32_t i = 0; i < type.member_types.size(); i++)
		{
			auto member_flags = meta[type.self].members.at(i).decoration_flags;
			auto &member_type = get<SPIRType>(type.member_types[i]);

			uint32_t packed_alignment = type_to_packed_alignment(member_type, member_flags, packing);
			uint32_t alignment = max(packed_alignment, pad_alignment);

			// The next member following a struct member is aligned to the base alignment of the struct that came before.
			// GL 4.5 spec, 7.6.2.2.
			if (member_type.basetype == SPIRType::Struct)
				pad_alignment = packed_alignment;
			else
				pad_alignment = 1;

			size = (size + alignment - 1) & ~(alignment - 1);
			size += type_to_packed_size(member_type, member_flags, packing);
		}
	}
	else
	{
		const uint32_t base_alignment = type_to_packed_base_size(type, packing);

		if (type.columns == 1)
			size = type.vecsize * base_alignment;

		if (flags.get(DecorationColMajor) && type.columns > 1)
		{
			if (packing_is_vec4_padded(packing))
				size = type.columns * 4 * base_alignment;
			else if (type.vecsize == 3)
				size = type.columns * 4 * base_alignment;
			else
				size = type.columns * type.vecsize * base_alignment;
		}

		if (flags.get(DecorationRowMajor) && type.vecsize > 1)
		{
			if (packing_is_vec4_padded(packing))
				size = type.vecsize * 4 * base_alignment;
			else if (type.columns == 3)
				size = type.vecsize * 4 * base_alignment;
			else
				size = type.vecsize * type.columns * base_alignment;
		}
	}

	return size;
}

bool CompilerGLSL::buffer_is_packing_standard(const SPIRType &type, BufferPackingStandard packing,
                                              uint32_t start_offset, uint32_t end_offset)
{
	// This is very tricky and error prone, but try to be exhaustive and correct here.
	// SPIR-V doesn't directly say if we're using std430 or std140.
	// SPIR-V communicates this using Offset and ArrayStride decorations (which is what really matters),
	// so we have to try to infer whether or not the original GLSL source was std140 or std430 based on this information.
	// We do not have to consider shared or packed since these layouts are not allowed in Vulkan SPIR-V (they are useless anyways, and custom offsets would do the same thing).
	//
	// It is almost certain that we're using std430, but it gets tricky with arrays in particular.
	// We will assume std430, but infer std140 if we can prove the struct is not compliant with std430.
	//
	// The only two differences between std140 and std430 are related to padding alignment/array stride
	// in arrays and structs. In std140 they take minimum vec4 alignment.
	// std430 only removes the vec4 requirement.

	uint32_t offset = 0;
	uint32_t pad_alignment = 1;

	for (uint32_t i = 0; i < type.member_types.size(); i++)
	{
		auto &memb_type = get<SPIRType>(type.member_types[i]);
		auto member_flags = meta[type.self].members.at(i).decoration_flags;

		// Verify alignment rules.
		uint32_t packed_alignment = type_to_packed_alignment(memb_type, member_flags, packing);
		uint32_t packed_size = type_to_packed_size(memb_type, member_flags, packing);

		if (packing_is_hlsl(packing))
		{
			// If a member straddles across a vec4 boundary, alignment is actually vec4.
			uint32_t begin_word = offset / 16;
			uint32_t end_word = (offset + packed_size - 1) / 16;
			if (begin_word != end_word)
				packed_alignment = max(packed_alignment, 16u);
		}

		uint32_t alignment = max(packed_alignment, pad_alignment);
		offset = (offset + alignment - 1) & ~(alignment - 1);

		// Field is not in the specified range anymore and we can ignore any further fields.
		if (offset >= end_offset)
			break;

		// The next member following a struct member is aligned to the base alignment of the struct that came before.
		// GL 4.5 spec, 7.6.2.2.
		if (memb_type.basetype == SPIRType::Struct)
			pad_alignment = packed_alignment;
		else
			pad_alignment = 1;

		// Only care about packing if we are in the given range
		if (offset >= start_offset)
		{
			// We only care about offsets in std140, std430, etc ...
			// For EnhancedLayout variants, we have the flexibility to choose our own offsets.
			if (!packing_has_flexible_offset(packing))
			{
				uint32_t actual_offset = type_struct_member_offset(type, i);
				if (actual_offset != offset) // This cannot be the packing we're looking for.
					return false;
			}

			// Verify array stride rules.
			if (!memb_type.array.empty() && type_to_packed_array_stride(memb_type, member_flags, packing) !=
			                                    type_struct_member_array_stride(type, i))
				return false;

			// Verify that sub-structs also follow packing rules.
			// We cannot use enhanced layouts on substructs, so they better be up to spec.
			auto substruct_packing = packing_to_substruct_packing(packing);

			if (!memb_type.member_types.empty() && !buffer_is_packing_standard(memb_type, substruct_packing))
				return false;
		}

		// Bump size.
		offset += packed_size;
	}

	return true;
}

bool CompilerGLSL::can_use_io_location(StorageClass storage, bool block)
{
	// Location specifiers are must have in SPIR-V, but they aren't really supported in earlier versions of GLSL.
	// Be very explicit here about how to solve the issue.
	if ((get_execution_model() != ExecutionModelVertex && storage == StorageClassInput) ||
	    (get_execution_model() != ExecutionModelFragment && storage == StorageClassOutput))
	{
		uint32_t minimum_desktop_version = block ? 440 : 410;
		// ARB_enhanced_layouts vs ARB_separate_shader_objects ...

		if (!options.es && options.version < minimum_desktop_version && !options.separate_shader_objects)
			return false;
		else if (options.es && options.version < 310)
			return false;
	}

	if ((get_execution_model() == ExecutionModelVertex && storage == StorageClassInput) ||
	    (get_execution_model() == ExecutionModelFragment && storage == StorageClassOutput))
	{
		if (options.es && options.version < 300)
			return false;
		else if (!options.es && options.version < 330)
			return false;
	}

	if (storage == StorageClassUniform || storage == StorageClassUniformConstant)
	{
		if (options.es && options.version < 310)
			return false;
		else if (!options.es && options.version < 430)
			return false;
	}

	return true;
}

string CompilerGLSL::layout_for_variable(const SPIRVariable &var)
{
	// FIXME: Come up with a better solution for when to disable layouts.
	// Having layouts depend on extensions as well as which types
	// of layouts are used. For now, the simple solution is to just disable
	// layouts for legacy versions.
	if (is_legacy())
		return "";

	vector<string> attr;

	auto &dec = meta[var.self].decoration;
	auto &type = get<SPIRType>(var.basetype);
	auto flags = dec.decoration_flags;
	auto typeflags = meta[type.self].decoration.decoration_flags;

	if (options.vulkan_semantics && var.storage == StorageClassPushConstant)
		attr.push_back("push_constant");

	if (flags.get(DecorationRowMajor))
		attr.push_back("row_major");
	if (flags.get(DecorationColMajor))
		attr.push_back("column_major");

	if (options.vulkan_semantics)
	{
		if (flags.get(DecorationInputAttachmentIndex))
			attr.push_back(join("input_attachment_index = ", dec.input_attachment));
	}

	bool is_block = has_decoration(type.self, DecorationBlock);
	if (flags.get(DecorationLocation) && can_use_io_location(var.storage, is_block))
	{
		Bitset combined_decoration;
		for (uint32_t i = 0; i < meta[type.self].members.size(); i++)
			combined_decoration.merge_or(combined_decoration_for_member(type, i));

		// If our members have location decorations, we don't need to
		// emit location decorations at the top as well (looks weird).
		if (!combined_decoration.get(DecorationLocation))
			attr.push_back(join("location = ", dec.location));
	}

	if (flags.get(DecorationIndex))
		attr.push_back(join("index = ", dec.index));

	// Do not emit set = decoration in regular GLSL output, but
	// we need to preserve it in Vulkan GLSL mode.
	if (var.storage != StorageClassPushConstant)
	{
		if (flags.get(DecorationDescriptorSet) && options.vulkan_semantics)
			attr.push_back(join("set = ", dec.set));
	}

	// GL 3.0/GLSL 1.30 is not considered legacy, but it doesn't have UBOs ...
	bool can_use_buffer_blocks = (options.es && options.version >= 300) || (!options.es && options.version >= 140);

	bool can_use_binding;
	if (options.es)
		can_use_binding = options.version >= 310;
	else
		can_use_binding = options.enable_420pack_extension || (options.version >= 420);

	// Make sure we don't emit binding layout for a classic uniform on GLSL 1.30.
	if (!can_use_buffer_blocks && var.storage == StorageClassUniform)
		can_use_binding = false;

	if (can_use_binding && flags.get(DecorationBinding))
		attr.push_back(join("binding = ", dec.binding));

	if (flags.get(DecorationOffset))
		attr.push_back(join("offset = ", dec.offset));

	bool push_constant_block = options.vulkan_semantics && var.storage == StorageClassPushConstant;
	bool ssbo_block = var.storage == StorageClassStorageBuffer ||
	                  (var.storage == StorageClassUniform && typeflags.get(DecorationBufferBlock));

	// Instead of adding explicit offsets for every element here, just assume we're using std140 or std430.
	// If SPIR-V does not comply with either layout, we cannot really work around it.
	if (can_use_buffer_blocks && var.storage == StorageClassUniform && typeflags.get(DecorationBlock))
	{
		if (buffer_is_packing_standard(type, BufferPackingStd140))
			attr.push_back("std140");
		else if (buffer_is_packing_standard(type, BufferPackingStd140EnhancedLayout))
		{
			attr.push_back("std140");
			// Fallback time. We might be able to use the ARB_enhanced_layouts to deal with this difference,
			// however, we can only use layout(offset) on the block itself, not any substructs, so the substructs better be the appropriate layout.
			// Enhanced layouts seem to always work in Vulkan GLSL, so no need for extensions there.
			if (options.es && !options.vulkan_semantics)
				SPIRV_CROSS_THROW("Push constant block cannot be expressed as neither std430 nor std140. ES-targets do "
				                  "not support GL_ARB_enhanced_layouts.");
			if (!options.es && !options.vulkan_semantics && options.version < 440)
				require_extension_internal("GL_ARB_enhanced_layouts");

			// This is a very last minute to check for this, but use this unused decoration to mark that we should emit
			// explicit offsets for this block type.
			// layout_for_variable() will be called before the actual buffer emit.
			// The alternative is a full pass before codegen where we deduce this decoration,
			// but then we are just doing the exact same work twice, and more complexity.
			set_decoration(type.self, DecorationCPacked);
		}
		else
		{
			SPIRV_CROSS_THROW("Uniform buffer cannot be expressed as std140, even with enhanced layouts. You can try "
			                  "flattening this block to "
			                  "support a more flexible layout.");
		}
	}
	else if (can_use_buffer_blocks && (push_constant_block || ssbo_block))
	{
		if (buffer_is_packing_standard(type, BufferPackingStd430))
			attr.push_back("std430");
		else if (buffer_is_packing_standard(type, BufferPackingStd140))
			attr.push_back("std140");
		else if (buffer_is_packing_standard(type, BufferPackingStd140EnhancedLayout))
		{
			attr.push_back("std140");

			// Fallback time. We might be able to use the ARB_enhanced_layouts to deal with this difference,
			// however, we can only use layout(offset) on the block itself, not any substructs, so the substructs better be the appropriate layout.
			// Enhanced layouts seem to always work in Vulkan GLSL, so no need for extensions there.
			if (options.es && !options.vulkan_semantics)
				SPIRV_CROSS_THROW("Push constant block cannot be expressed as neither std430 nor std140. ES-targets do "
				                  "not support GL_ARB_enhanced_layouts.");
			if (!options.es && !options.vulkan_semantics && options.version < 440)
				require_extension_internal("GL_ARB_enhanced_layouts");

			set_decoration(type.self, DecorationCPacked);
		}
		else if (buffer_is_packing_standard(type, BufferPackingStd430EnhancedLayout))
		{
			attr.push_back("std430");
			if (options.es && !options.vulkan_semantics)
				SPIRV_CROSS_THROW("Push constant block cannot be expressed as neither std430 nor std140. ES-targets do "
				                  "not support GL_ARB_enhanced_layouts.");
			if (!options.es && !options.vulkan_semantics && options.version < 440)
				require_extension_internal("GL_ARB_enhanced_layouts");

			set_decoration(type.self, DecorationCPacked);
		}
		else
		{
			SPIRV_CROSS_THROW("Buffer block cannot be expressed as neither std430 nor std140, even with enhanced "
			                  "layouts. You can try flattening this block to support a more flexible layout.");
		}
	}

	// For images, the type itself adds a layout qualifer.
	// Only emit the format for storage images.
	if (type.basetype == SPIRType::Image && type.image.sampled == 2)
	{
		const char *fmt = format_to_glsl(type.image.format);
		if (fmt)
			attr.push_back(fmt);
	}

	if (attr.empty())
		return "";

	string res = "layout(";
	res += merge(attr);
	res += ") ";
	return res;
}

void CompilerGLSL::emit_push_constant_block(const SPIRVariable &var)
{
	if (flattened_buffer_blocks.count(var.self))
		emit_buffer_block_flattened(var);
	else if (options.vulkan_semantics)
		emit_push_constant_block_vulkan(var);
	else
		emit_push_constant_block_glsl(var);
}

void CompilerGLSL::emit_push_constant_block_vulkan(const SPIRVariable &var)
{
	emit_buffer_block(var);
}

void CompilerGLSL::emit_push_constant_block_glsl(const SPIRVariable &var)
{
	// OpenGL has no concept of push constant blocks, implement it as a uniform struct.
	auto &type = get<SPIRType>(var.basetype);

	auto &flags = meta[var.self].decoration.decoration_flags;
	flags.clear(DecorationBinding);
	flags.clear(DecorationDescriptorSet);

#if 0
    if (flags & ((1ull << DecorationBinding) | (1ull << DecorationDescriptorSet)))
        SPIRV_CROSS_THROW("Push constant blocks cannot be compiled to GLSL with Binding or Set syntax. "
                            "Remap to location with reflection API first or disable these decorations.");
#endif

	// We're emitting the push constant block as a regular struct, so disable the block qualifier temporarily.
	// Otherwise, we will end up emitting layout() qualifiers on naked structs which is not allowed.
	auto &block_flags = meta[type.self].decoration.decoration_flags;
	bool block_flag = block_flags.get(DecorationBlock);
	block_flags.clear(DecorationBlock);

	emit_struct(type);

	if (block_flag)
		block_flags.set(DecorationBlock);

	emit_uniform(var);
	statement("");
}

void CompilerGLSL::emit_buffer_block(const SPIRVariable &var)
{
	if (flattened_buffer_blocks.count(var.self))
		emit_buffer_block_flattened(var);
	else if (is_legacy() || (!options.es && options.version == 130))
		emit_buffer_block_legacy(var);
	else
		emit_buffer_block_native(var);
}

void CompilerGLSL::emit_buffer_block_legacy(const SPIRVariable &var)
{
	auto &type = get<SPIRType>(var.basetype);
	bool ssbo = var.storage == StorageClassStorageBuffer ||
	            meta[type.self].decoration.decoration_flags.get(DecorationBufferBlock);
	if (ssbo)
		SPIRV_CROSS_THROW("SSBOs not supported in legacy targets.");

	// We're emitting the push constant block as a regular struct, so disable the block qualifier temporarily.
	// Otherwise, we will end up emitting layout() qualifiers on naked structs which is not allowed.
	auto &block_flags = meta[type.self].decoration.decoration_flags;
	bool block_flag = block_flags.get(DecorationBlock);
	block_flags.clear(DecorationBlock);
	emit_struct(type);
	if (block_flag)
		block_flags.set(DecorationBlock);
	emit_uniform(var);
	statement("");
}

void CompilerGLSL::emit_buffer_block_native(const SPIRVariable &var)
{
	auto &type = get<SPIRType>(var.basetype);

	Bitset flags = get_buffer_block_flags(var);
	bool ssbo = var.storage == StorageClassStorageBuffer ||
	            meta[type.self].decoration.decoration_flags.get(DecorationBufferBlock);
	bool is_restrict = ssbo && flags.get(DecorationRestrict);
	bool is_writeonly = ssbo && flags.get(DecorationNonReadable);
	bool is_readonly = ssbo && flags.get(DecorationNonWritable);
	bool is_coherent = ssbo && flags.get(DecorationCoherent);

	// Block names should never alias, but from HLSL input they kind of can because block types are reused for UAVs ...
	auto buffer_name = to_name(type.self, false);

	// Shaders never use the block by interface name, so we don't
	// have to track this other than updating name caches.
	if (meta[type.self].decoration.alias.empty() || resource_names.find(buffer_name) != end(resource_names))
		buffer_name = get_block_fallback_name(var.self);

	// Make sure we get something unique.
	add_variable(resource_names, buffer_name);

	// If for some reason buffer_name is an illegal name, make a final fallback to a workaround name.
	// This cannot conflict with anything else, so we're safe now.
	if (buffer_name.empty())
		buffer_name = join("_", get<SPIRType>(var.basetype).self, "_", var.self);

	// Save for post-reflection later.
	declared_block_names[var.self] = buffer_name;

	statement(layout_for_variable(var), is_coherent ? "coherent " : "", is_restrict ? "restrict " : "",
	          is_writeonly ? "writeonly " : "", is_readonly ? "readonly " : "", ssbo ? "buffer " : "uniform ",
	          buffer_name);

	begin_scope();

	type.member_name_cache.clear();

	uint32_t i = 0;
	for (auto &member : type.member_types)
	{
		add_member_name(type, i);
		emit_struct_member(type, member, i);
		i++;
	}

	add_resource_name(var.self);
	end_scope_decl(to_name(var.self) + type_to_array_glsl(type));
	statement("");
}

void CompilerGLSL::emit_buffer_block_flattened(const SPIRVariable &var)
{
	auto &type = get<SPIRType>(var.basetype);

	// Block names should never alias.
	auto buffer_name = to_name(type.self, false);
	size_t buffer_size = (get_declared_struct_size(type) + 15) / 16;

	SPIRType::BaseType basic_type;
	if (get_common_basic_type(type, basic_type))
	{
		SPIRType tmp;
		tmp.basetype = basic_type;
		tmp.vecsize = 4;
		if (basic_type != SPIRType::Float && basic_type != SPIRType::Int && basic_type != SPIRType::UInt)
			SPIRV_CROSS_THROW("Basic types in a flattened UBO must be float, int or uint.");

		auto flags = get_buffer_block_flags(var);
		statement("uniform ", flags_to_precision_qualifiers_glsl(tmp, flags), type_to_glsl(tmp), " ", buffer_name, "[",
		          buffer_size, "];");
	}
	else
		SPIRV_CROSS_THROW("All basic types in a flattened block must be the same.");
}

const char *CompilerGLSL::to_storage_qualifiers_glsl(const SPIRVariable &var)
{
	auto &execution = get_entry_point();

	if (var.storage == StorageClassInput || var.storage == StorageClassOutput)
	{
		if (is_legacy() && execution.model == ExecutionModelVertex)
			return var.storage == StorageClassInput ? "attribute " : "varying ";
		else if (is_legacy() && execution.model == ExecutionModelFragment)
			return "varying "; // Fragment outputs are renamed so they never hit this case.
		else
			return var.storage == StorageClassInput ? "in " : "out ";
	}
	else if (var.storage == StorageClassUniformConstant || var.storage == StorageClassUniform ||
	         var.storage == StorageClassPushConstant)
	{
		return "uniform ";
	}

	return "";
}

void CompilerGLSL::emit_flattened_io_block(const SPIRVariable &var, const char *qual)
{
	auto &type = get<SPIRType>(var.basetype);
	if (!type.array.empty())
		SPIRV_CROSS_THROW("Array of varying structs cannot be flattened to legacy-compatible varyings.");

	auto old_flags = meta[type.self].decoration.decoration_flags;
	// Emit the members as if they are part of a block to get all qualifiers.
	meta[type.self].decoration.decoration_flags.set(DecorationBlock);

	type.member_name_cache.clear();

	uint32_t i = 0;
	for (auto &member : type.member_types)
	{
		add_member_name(type, i);
		auto &membertype = get<SPIRType>(member);

		if (membertype.basetype == SPIRType::Struct)
			SPIRV_CROSS_THROW("Cannot flatten struct inside structs in I/O variables.");

		// Pass in the varying qualifier here so it will appear in the correct declaration order.
		// Replace member name while emitting it so it encodes both struct name and member name.
		// Sanitize underscores because joining the two identifiers might create more than 1 underscore in a row,
		// which is not allowed.
		auto backup_name = get_member_name(type.self, i);
		auto member_name = to_member_name(type, i);
		set_member_name(type.self, i, sanitize_underscores(join(to_name(var.self), "_", member_name)));
		emit_struct_member(type, member, i, qual);
		// Restore member name.
		set_member_name(type.self, i, member_name);
		i++;
	}

	meta[type.self].decoration.decoration_flags = old_flags;

	// Treat this variable as flattened from now on.
	flattened_structs.insert(var.self);
}

void CompilerGLSL::emit_interface_block(const SPIRVariable &var)
{
	auto &type = get<SPIRType>(var.basetype);

	// Either make it plain in/out or in/out blocks depending on what shader is doing ...
	bool block = meta[type.self].decoration.decoration_flags.get(DecorationBlock);
	const char *qual = to_storage_qualifiers_glsl(var);

	if (block)
	{
		// ESSL earlier than 310 and GLSL earlier than 150 did not support
		// I/O variables which are struct types.
		// To support this, flatten the struct into separate varyings instead.
		if ((options.es && options.version < 310) || (!options.es && options.version < 150))
		{
			// I/O blocks on ES require version 310 with Android Extension Pack extensions, or core version 320.
			// On desktop, I/O blocks were introduced with geometry shaders in GL 3.2 (GLSL 150).
			emit_flattened_io_block(var, qual);
		}
		else
		{
			if (options.es && options.version < 320)
			{
				// Geometry and tessellation extensions imply this extension.
				if (!has_extension("GL_EXT_geometry_shader") && !has_extension("GL_EXT_tessellation_shader"))
					require_extension_internal("GL_EXT_shader_io_blocks");
			}

			// Block names should never alias.
			auto block_name = to_name(type.self, false);

			// Shaders never use the block by interface name, so we don't
			// have to track this other than updating name caches.
			if (resource_names.find(block_name) != end(resource_names))
				block_name = get_fallback_name(type.self);
			else
				resource_names.insert(block_name);

			statement(layout_for_variable(var), qual, block_name);
			begin_scope();

			type.member_name_cache.clear();

			uint32_t i = 0;
			for (auto &member : type.member_types)
			{
				add_member_name(type, i);
				emit_struct_member(type, member, i);
				i++;
			}

			add_resource_name(var.self);
			end_scope_decl(join(to_name(var.self), type_to_array_glsl(type)));
			statement("");
		}
	}
	else
	{
		// ESSL earlier than 310 and GLSL earlier than 150 did not support
		// I/O variables which are struct types.
		// To support this, flatten the struct into separate varyings instead.
		if (type.basetype == SPIRType::Struct &&
		    ((options.es && options.version < 310) || (!options.es && options.version < 150)))
		{
			emit_flattened_io_block(var, qual);
		}
		else
		{
			add_resource_name(var.self);
			statement(layout_for_variable(var), variable_decl(var), ";");
		}
	}
}

void CompilerGLSL::emit_uniform(const SPIRVariable &var)
{
	auto &type = get<SPIRType>(var.basetype);
	if (type.basetype == SPIRType::Image && type.image.sampled == 2)
	{
		if (!options.es && options.version < 420)
			require_extension_internal("GL_ARB_shader_image_load_store");
		else if (options.es && options.version < 310)
			SPIRV_CROSS_THROW("At least ESSL 3.10 required for shader image load store.");
	}

	add_resource_name(var.self);
	statement(layout_for_variable(var), variable_decl(var), ";");
}

void CompilerGLSL::emit_specialization_constant_op(const SPIRConstantOp &constant)
{
	auto &type = get<SPIRType>(constant.basetype);
	auto name = to_name(constant.self);
	statement("const ", variable_decl(type, name), " = ", constant_op_expression(constant), ";");
}

void CompilerGLSL::emit_constant(const SPIRConstant &constant)
{
	auto &type = get<SPIRType>(constant.constant_type);
	auto name = to_name(constant.self);

	SpecializationConstant wg_x, wg_y, wg_z;
	uint32_t workgroup_size_id = get_work_group_size_specialization_constants(wg_x, wg_y, wg_z);

	if (constant.self == workgroup_size_id || constant.self == wg_x.id || constant.self == wg_y.id ||
	    constant.self == wg_z.id)
	{
		// These specialization constants are implicitly declared by emitting layout() in;
		return;
	}

	// Only scalars have constant IDs.
	if (has_decoration(constant.self, DecorationSpecId))
	{
		statement("layout(constant_id = ", get_decoration(constant.self, DecorationSpecId), ") const ",
		          variable_decl(type, name), " = ", constant_expression(constant), ";");
	}
	else
	{
		statement("const ", variable_decl(type, name), " = ", constant_expression(constant), ";");
	}
}

void CompilerGLSL::emit_entry_point_declarations()
{
}

void CompilerGLSL::replace_illegal_names()
{
	// clang-format off
	static const unordered_set<string> keywords = {
		"abs", "acos", "acosh", "all", "any", "asin", "asinh", "atan", "atanh",
		"atomicAdd", "atomicCompSwap", "atomicCounter", "atomicCounterDecrement", "atomicCounterIncrement",
		"atomicExchange", "atomicMax", "atomicMin", "atomicOr", "atomicXor",
		"bitCount", "bitfieldExtract", "bitfieldInsert", "bitfieldReverse",
		"ceil", "cos", "cosh", "cross", "degrees",
		"dFdx", "dFdxCoarse", "dFdxFine",
		"dFdy", "dFdyCoarse", "dFdyFine",
		"distance", "dot", "EmitStreamVertex", "EmitVertex", "EndPrimitive", "EndStreamPrimitive", "equal", "exp", "exp2",
		"faceforward", "findLSB", "findMSB", "floatBitsToInt", "floatBitsToUint", "floor", "fma", "fract", "frexp", "fwidth", "fwidthCoarse", "fwidthFine",
		"greaterThan", "greaterThanEqual", "groupMemoryBarrier",
		"imageAtomicAdd", "imageAtomicAnd", "imageAtomicCompSwap", "imageAtomicExchange", "imageAtomicMax", "imageAtomicMin", "imageAtomicOr", "imageAtomicXor",
		"imageLoad", "imageSamples", "imageSize", "imageStore", "imulExtended", "intBitsToFloat", "interpolateAtOffset", "interpolateAtCentroid", "interpolateAtSample",
		"inverse", "inversesqrt", "isinf", "isnan", "ldexp", "length", "lessThan", "lessThanEqual", "log", "log2",
		"matrixCompMult", "max", "memoryBarrier", "memoryBarrierAtomicCounter", "memoryBarrierBuffer", "memoryBarrierImage", "memoryBarrierShared",
		"min", "mix", "mod", "modf", "noise", "noise1", "noise2", "noise3", "noise4", "normalize", "not", "notEqual",
		"outerProduct", "packDouble2x32", "packHalf2x16", "packSnorm2x16", "packSnorm4x8", "packUnorm2x16", "packUnorm4x8", "pow",
		"radians", "reflect", "refract", "round", "roundEven", "sign", "sin", "sinh", "smoothstep", "sqrt", "step",
		"tan", "tanh", "texelFetch", "texelFetchOffset", "texture", "textureGather", "textureGatherOffset", "textureGatherOffsets",
		"textureGrad", "textureGradOffset", "textureLod", "textureLodOffset", "textureOffset", "textureProj", "textureProjGrad",
		"textureProjGradOffset", "textureProjLod", "textureProjLodOffset", "textureProjOffset", "textureQueryLevels", "textureQueryLod", "textureSamples", "textureSize",
		"transpose", "trunc", "uaddCarry", "uintBitsToFloat", "umulExtended", "unpackDouble2x32", "unpackHalf2x16", "unpackSnorm2x16", "unpackSnorm4x8",
		"unpackUnorm2x16", "unpackUnorm4x8", "usubBorrow",

		"active", "asm", "atomic_uint", "attribute", "bool", "break", "buffer",
		"bvec2", "bvec3", "bvec4", "case", "cast", "centroid", "class", "coherent", "common", "const", "continue", "default", "discard",
		"dmat2", "dmat2x2", "dmat2x3", "dmat2x4", "dmat3", "dmat3x2", "dmat3x3", "dmat3x4", "dmat4", "dmat4x2", "dmat4x3", "dmat4x4",
		"do", "double", "dvec2", "dvec3", "dvec4", "else", "enum", "extern", "external", "false", "filter", "fixed", "flat", "float",
		"for", "fvec2", "fvec3", "fvec4", "goto", "half", "highp", "hvec2", "hvec3", "hvec4", "if", "iimage1D", "iimage1DArray",
		"iimage2D", "iimage2DArray", "iimage2DMS", "iimage2DMSArray", "iimage2DRect", "iimage3D", "iimageBuffer", "iimageCube",
		"iimageCubeArray", "image1D", "image1DArray", "image2D", "image2DArray", "image2DMS", "image2DMSArray", "image2DRect",
		"image3D", "imageBuffer", "imageCube", "imageCubeArray", "in", "inline", "inout", "input", "int", "interface", "invariant",
		"isampler1D", "isampler1DArray", "isampler2D", "isampler2DArray", "isampler2DMS", "isampler2DMSArray", "isampler2DRect",
		"isampler3D", "isamplerBuffer", "isamplerCube", "isamplerCubeArray", "ivec2", "ivec3", "ivec4", "layout", "long", "lowp",
		"mat2", "mat2x2", "mat2x3", "mat2x4", "mat3", "mat3x2", "mat3x3", "mat3x4", "mat4", "mat4x2", "mat4x3", "mat4x4", "mediump",
		"namespace", "noinline", "noperspective", "out", "output", "packed", "partition", "patch", "precise", "precision", "public", "readonly",
		"resource", "restrict", "return", "sample", "sampler1D", "sampler1DArray", "sampler1DArrayShadow",
		"sampler1DShadow", "sampler2D", "sampler2DArray", "sampler2DArrayShadow", "sampler2DMS", "sampler2DMSArray",
		"sampler2DRect", "sampler2DRectShadow", "sampler2DShadow", "sampler3D", "sampler3DRect", "samplerBuffer",
		"samplerCube", "samplerCubeArray", "samplerCubeArrayShadow", "samplerCubeShadow", "shared", "short", "sizeof", "smooth", "static",
		"struct", "subroutine", "superp", "switch", "template", "this", "true", "typedef", "uimage1D", "uimage1DArray", "uimage2D",
		"uimage2DArray", "uimage2DMS", "uimage2DMSArray", "uimage2DRect", "uimage3D", "uimageBuffer", "uimageCube",
		"uimageCubeArray", "uint", "uniform", "union", "unsigned", "usampler1D", "usampler1DArray", "usampler2D", "usampler2DArray",
		"usampler2DMS", "usampler2DMSArray", "usampler2DRect", "usampler3D", "usamplerBuffer", "usamplerCube",
		"usamplerCubeArray", "using", "uvec2", "uvec3", "uvec4", "varying", "vec2", "vec3", "vec4", "void", "volatile",
		"while", "writeonly",
	};
	// clang-format on

	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();
			if (!is_hidden_variable(var))
			{
				auto &m = meta[var.self].decoration;
				if (m.alias.compare(0, 3, "gl_") == 0 || keywords.find(m.alias) != end(keywords))
					m.alias = join("_", m.alias);
			}
		}
	}
}

void CompilerGLSL::replace_fragment_output(SPIRVariable &var)
{
	auto &m = meta[var.self].decoration;
	uint32_t location = 0;
	if (m.decoration_flags.get(DecorationLocation))
		location = m.location;

	// If our variable is arrayed, we must not emit the array part of this as the SPIR-V will
	// do the access chain part of this for us.
	auto &type = get<SPIRType>(var.basetype);

	if (type.array.empty())
	{
		// Redirect the write to a specific render target in legacy GLSL.
		m.alias = join("gl_FragData[", location, "]");

		if (is_legacy_es() && location != 0)
			require_extension_internal("GL_EXT_draw_buffers");
	}
	else if (type.array.size() == 1)
	{
		// If location is non-zero, we probably have to add an offset.
		// This gets really tricky since we'd have to inject an offset in the access chain.
		// FIXME: This seems like an extremely odd-ball case, so it's probably fine to leave it like this for now.
		m.alias = "gl_FragData";
		if (location != 0)
			SPIRV_CROSS_THROW("Arrayed output variable used, but location is not 0. "
			                  "This is unimplemented in SPIRV-Cross.");

		if (is_legacy_es())
			require_extension_internal("GL_EXT_draw_buffers");
	}
	else
		SPIRV_CROSS_THROW("Array-of-array output variable used. This cannot be implemented in legacy GLSL.");

	var.compat_builtin = true; // We don't want to declare this variable, but use the name as-is.
}

void CompilerGLSL::replace_fragment_outputs()
{
	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();
			auto &type = get<SPIRType>(var.basetype);

			if (!is_builtin_variable(var) && !var.remapped_variable && type.pointer &&
			    var.storage == StorageClassOutput)
				replace_fragment_output(var);
		}
	}
}

string CompilerGLSL::remap_swizzle(const SPIRType &out_type, uint32_t input_components, const string &expr)
{
	if (out_type.vecsize == input_components)
		return expr;
	else if (input_components == 1 && !backend.can_swizzle_scalar)
		return join(type_to_glsl(out_type), "(", expr, ")");
	else
	{
		// FIXME: This will not work with packed expressions.
		auto e = enclose_expression(expr) + ".";
		// Just clamp the swizzle index if we have more outputs than inputs.
		for (uint32_t c = 0; c < out_type.vecsize; c++)
			e += index_to_swizzle(min(c, input_components - 1));
		if (backend.swizzle_is_function && out_type.vecsize > 1)
			e += "()";

		remove_duplicate_swizzle(e);
		return e;
	}
}

void CompilerGLSL::emit_pls()
{
	auto &execution = get_entry_point();
	if (execution.model != ExecutionModelFragment)
		SPIRV_CROSS_THROW("Pixel local storage only supported in fragment shaders.");

	if (!options.es)
		SPIRV_CROSS_THROW("Pixel local storage only supported in OpenGL ES.");

	if (options.version < 300)
		SPIRV_CROSS_THROW("Pixel local storage only supported in ESSL 3.0 and above.");

	if (!pls_inputs.empty())
	{
		statement("__pixel_local_inEXT _PLSIn");
		begin_scope();
		for (auto &input : pls_inputs)
			statement(pls_decl(input), ";");
		end_scope_decl();
		statement("");
	}

	if (!pls_outputs.empty())
	{
		statement("__pixel_local_outEXT _PLSOut");
		begin_scope();
		for (auto &output : pls_outputs)
			statement(pls_decl(output), ";");
		end_scope_decl();
		statement("");
	}
}

void CompilerGLSL::fixup_image_load_store_access()
{
	for (auto &id : ids)
	{
		if (id.get_type() != TypeVariable)
			continue;

		uint32_t var = id.get<SPIRVariable>().self;
		auto &vartype = expression_type(var);
		if (vartype.basetype == SPIRType::Image)
		{
			// Older glslangValidator does not emit required qualifiers here.
			// Solve this by making the image access as restricted as possible and loosen up if we need to.
			// If any no-read/no-write flags are actually set, assume that the compiler knows what it's doing.

			auto &flags = meta.at(var).decoration.decoration_flags;
			if (!flags.get(DecorationNonWritable) && !flags.get(DecorationNonReadable))
			{
				flags.set(DecorationNonWritable);
				flags.set(DecorationNonReadable);
			}
		}
	}
}

void CompilerGLSL::emit_declared_builtin_block(StorageClass storage, ExecutionModel model)
{
	Bitset emitted_builtins;
	Bitset global_builtins;
	const SPIRVariable *block_var = nullptr;
	bool emitted_block = false;
	bool builtin_array = false;

	// Need to use declared size in the type.
	// These variables might have been declared, but not statically used, so we haven't deduced their size yet.
	uint32_t cull_distance_size = 0;
	uint32_t clip_distance_size = 0;

	for (auto &id : ids)
	{
		if (id.get_type() != TypeVariable)
			continue;

		auto &var = id.get<SPIRVariable>();
		auto &type = get<SPIRType>(var.basetype);
		bool block = has_decoration(type.self, DecorationBlock);
		Bitset builtins;

		if (var.storage == storage && block && is_builtin_variable(var))
		{
			uint32_t index = 0;
			for (auto &m : meta[type.self].members)
			{
				if (m.builtin)
				{
					builtins.set(m.builtin_type);
					if (m.builtin_type == BuiltInCullDistance)
						cull_distance_size = get<SPIRType>(type.member_types[index]).array.front();
					else if (m.builtin_type == BuiltInClipDistance)
						clip_distance_size = get<SPIRType>(type.member_types[index]).array.front();
				}
				index++;
			}
		}
		else if (var.storage == storage && !block && is_builtin_variable(var))
		{
			// While we're at it, collect all declared global builtins (HLSL mostly ...).
			auto &m = meta[var.self].decoration;
			if (m.builtin)
			{
				global_builtins.set(m.builtin_type);
				if (m.builtin_type == BuiltInCullDistance)
					cull_distance_size = type.array.front();
				else if (m.builtin_type == BuiltInClipDistance)
					clip_distance_size = type.array.front();
			}
		}

		if (builtins.empty())
			continue;

		if (emitted_block)
			SPIRV_CROSS_THROW("Cannot use more than one builtin I/O block.");

		emitted_builtins = builtins;
		emitted_block = true;
		builtin_array = !type.array.empty();
		block_var = &var;
	}

	global_builtins =
	    Bitset(global_builtins.get_lower() & ((1ull << BuiltInPosition) | (1ull << BuiltInPointSize) |
	                                          (1ull << BuiltInClipDistance) | (1ull << BuiltInCullDistance)));

	// Try to collect all other declared builtins.
	if (!emitted_block)
		emitted_builtins = global_builtins;

	// Can't declare an empty interface block.
	if (emitted_builtins.empty())
		return;

	if (storage == StorageClassOutput)
		statement("out gl_PerVertex");
	else
		statement("in gl_PerVertex");

	begin_scope();
	if (emitted_builtins.get(BuiltInPosition))
		statement("vec4 gl_Position;");
	if (emitted_builtins.get(BuiltInPointSize))
		statement("float gl_PointSize;");
	if (emitted_builtins.get(BuiltInClipDistance))
		statement("float gl_ClipDistance[", clip_distance_size, "];");
	if (emitted_builtins.get(BuiltInCullDistance))
		statement("float gl_CullDistance[", cull_distance_size, "];");

	bool tessellation = model == ExecutionModelTessellationEvaluation || model == ExecutionModelTessellationControl;
	if (builtin_array)
	{
		// Make sure the array has a supported name in the code.
		if (storage == StorageClassOutput)
			set_name(block_var->self, "gl_out");
		else if (storage == StorageClassInput)
			set_name(block_var->self, "gl_in");

		if (model == ExecutionModelTessellationControl && storage == StorageClassOutput)
			end_scope_decl(join(to_name(block_var->self), "[", get_entry_point().output_vertices, "]"));
		else
			end_scope_decl(join(to_name(block_var->self), tessellation ? "[gl_MaxPatchVertices]" : "[]"));
	}
	else
		end_scope_decl();
	statement("");
}

void CompilerGLSL::declare_undefined_values()
{
	bool emitted = false;
	for (auto &id : ids)
	{
		if (id.get_type() != TypeUndef)
			continue;

		auto &undef = id.get<SPIRUndef>();
		statement(variable_decl(get<SPIRType>(undef.basetype), to_name(undef.self), undef.self), ";");
		emitted = true;
	}

	if (emitted)
		statement("");
}

void CompilerGLSL::emit_resources()
{
	auto &execution = get_entry_point();

	replace_illegal_names();

	// Legacy GL uses gl_FragData[], redeclare all fragment outputs
	// with builtins.
	if (execution.model == ExecutionModelFragment && is_legacy())
		replace_fragment_outputs();

	// Emit PLS blocks if we have such variables.
	if (!pls_inputs.empty() || !pls_outputs.empty())
		emit_pls();

	// Emit custom gl_PerVertex for SSO compatibility.
	if (options.separate_shader_objects && !options.es && execution.model != ExecutionModelFragment)
	{
		switch (execution.model)
		{
		case ExecutionModelGeometry:
		case ExecutionModelTessellationControl:
		case ExecutionModelTessellationEvaluation:
			emit_declared_builtin_block(StorageClassInput, execution.model);
			emit_declared_builtin_block(StorageClassOutput, execution.model);
			break;

		case ExecutionModelVertex:
			emit_declared_builtin_block(StorageClassOutput, execution.model);
			break;

		default:
			break;
		}
	}
	else
	{
		// Need to redeclare clip/cull distance with explicit size to use them.
		// SPIR-V mandates these builtins have a size declared.
		const char *storage = execution.model == ExecutionModelFragment ? "in" : "out";
		if (clip_distance_count != 0)
			statement(storage, " float gl_ClipDistance[", clip_distance_count, "];");
		if (cull_distance_count != 0)
			statement(storage, " float gl_CullDistance[", cull_distance_count, "];");
		if (clip_distance_count != 0 || cull_distance_count != 0)
			statement("");
	}

	if (position_invariant)
	{
		statement("invariant gl_Position;");
		statement("");
	}

	bool emitted = false;

	// If emitted Vulkan GLSL,
	// emit specialization constants as actual floats,
	// spec op expressions will redirect to the constant name.
	//
	// TODO: If we have the fringe case that we create a spec constant which depends on a struct type,
	// we'll have to deal with that, but there's currently no known way to express that.
	for (auto &id : ids)
	{
		if (id.get_type() == TypeConstant)
		{
			auto &c = id.get<SPIRConstant>();

			bool needs_declaration = (c.specialization && options.vulkan_semantics) || c.is_used_as_lut;

			if (needs_declaration)
			{
				emit_constant(c);
				emitted = true;
			}
		}
		else if (options.vulkan_semantics && id.get_type() == TypeConstantOp)
		{
			emit_specialization_constant_op(id.get<SPIRConstantOp>());
			emitted = true;
		}
	}

	if (emitted)
		statement("");
	emitted = false;

	// Output all basic struct types which are not Block or BufferBlock as these are declared inplace
	// when such variables are instantiated.
	for (auto &id : ids)
	{
		if (id.get_type() == TypeType)
		{
			auto &type = id.get<SPIRType>();
			if (type.basetype == SPIRType::Struct && type.array.empty() && !type.pointer &&
			    (!meta[type.self].decoration.decoration_flags.get(DecorationBlock) &&
			     !meta[type.self].decoration.decoration_flags.get(DecorationBufferBlock)))
			{
				emit_struct(type);
			}
		}
	}

	// Output UBOs and SSBOs
	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();
			auto &type = get<SPIRType>(var.basetype);

			bool is_block_storage = type.storage == StorageClassStorageBuffer || type.storage == StorageClassUniform;
			bool has_block_flags = meta[type.self].decoration.decoration_flags.get(DecorationBlock) ||
			                       meta[type.self].decoration.decoration_flags.get(DecorationBufferBlock);

			if (var.storage != StorageClassFunction && type.pointer && is_block_storage && !is_hidden_variable(var) &&
			    has_block_flags)
			{
				emit_buffer_block(var);
			}
		}
	}

	// Output push constant blocks
	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();
			auto &type = get<SPIRType>(var.basetype);
			if (var.storage != StorageClassFunction && type.pointer && type.storage == StorageClassPushConstant &&
			    !is_hidden_variable(var))
			{
				emit_push_constant_block(var);
			}
		}
	}

	bool skip_separate_image_sampler = !combined_image_samplers.empty() || !options.vulkan_semantics;

	// Output Uniform Constants (values, samplers, images, etc).
	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();
			auto &type = get<SPIRType>(var.basetype);

			// If we're remapping separate samplers and images, only emit the combined samplers.
			if (skip_separate_image_sampler)
			{
				// Sampler buffers are always used without a sampler, and they will also work in regular GL.
				bool sampler_buffer = type.basetype == SPIRType::Image && type.image.dim == DimBuffer;
				bool separate_image = type.basetype == SPIRType::Image && type.image.sampled == 1;
				bool separate_sampler = type.basetype == SPIRType::Sampler;
				if (!sampler_buffer && (separate_image || separate_sampler))
					continue;
			}

			if (var.storage != StorageClassFunction && type.pointer &&
			    (type.storage == StorageClassUniformConstant || type.storage == StorageClassAtomicCounter) &&
			    !is_hidden_variable(var))
			{
				emit_uniform(var);
				emitted = true;
			}
		}
	}

	if (emitted)
		statement("");
	emitted = false;

	// Output in/out interfaces.
	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();
			auto &type = get<SPIRType>(var.basetype);

			if (var.storage != StorageClassFunction && type.pointer &&
			    (var.storage == StorageClassInput || var.storage == StorageClassOutput) &&
			    interface_variable_exists_in_entry_point(var.self) && !is_hidden_variable(var))
			{
				emit_interface_block(var);
				emitted = true;
			}
			else if (is_builtin_variable(var))
			{
				// For gl_InstanceIndex emulation on GLES, the API user needs to
				// supply this uniform.
				if (options.vertex.support_nonzero_base_instance &&
				    meta[var.self].decoration.builtin_type == BuiltInInstanceIndex && !options.vulkan_semantics)
				{
					statement("uniform int SPIRV_Cross_BaseInstance;");
					emitted = true;
				}
			}
		}
	}

	// Global variables.
	for (auto global : global_variables)
	{
		auto &var = get<SPIRVariable>(global);
		if (var.storage != StorageClassOutput)
		{
			add_resource_name(var.self);
			statement(variable_decl(var), ";");
			emitted = true;
		}
	}

	if (emitted)
		statement("");

	declare_undefined_values();
}

// Returns a string representation of the ID, usable as a function arg.
// Default is to simply return the expression representation fo the arg ID.
// Subclasses may override to modify the return value.
string CompilerGLSL::to_func_call_arg(uint32_t id)
{
	return to_expression(id);
}

void CompilerGLSL::handle_invalid_expression(uint32_t id)
{
	// We tried to read an invalidated expression.
	// This means we need another pass at compilation, but next time, force temporary variables so that they cannot be invalidated.
	forced_temporaries.insert(id);
	force_recompile = true;
}

// Converts the format of the current expression from packed to unpacked,
// by wrapping the expression in a constructor of the appropriate type.
// GLSL does not support packed formats, so simply return the expression.
// Subclasses that do will override
string CompilerGLSL::unpack_expression_type(string expr_str, const SPIRType &)
{
	return expr_str;
}

// Sometimes we proactively enclosed an expression where it turns out we might have not needed it after all.
void CompilerGLSL::strip_enclosed_expression(string &expr)
{
	if (expr.size() < 2 || expr.front() != '(' || expr.back() != ')')
		return;

	// Have to make sure that our first and last parens actually enclose everything inside it.
	uint32_t paren_count = 0;
	for (auto &c : expr)
	{
		if (c == '(')
			paren_count++;
		else if (c == ')')
		{
			paren_count--;

			// If we hit 0 and this is not the final char, our first and final parens actually don't
			// enclose the expression, and we cannot strip, e.g.: (a + b) * (c + d).
			if (paren_count == 0 && &c != &expr.back())
				return;
		}
	}
	expr.erase(expr.size() - 1, 1);
	expr.erase(begin(expr));
}

string CompilerGLSL::enclose_expression(const string &expr)
{
	bool need_parens = false;

	// If the expression starts with a unary we need to enclose to deal with cases where we have back-to-back
	// unary expressions.
	if (!expr.empty())
	{
		auto c = expr.front();
		if (c == '-' || c == '+' || c == '!' || c == '~')
			need_parens = true;
	}

	if (!need_parens)
	{
		uint32_t paren_count = 0;
		for (auto c : expr)
		{
			if (c == '(' || c == '[')
				paren_count++;
			else if (c == ')' || c == ']')
			{
				assert(paren_count);
				paren_count--;
			}
			else if (c == ' ' && paren_count == 0)
			{
				need_parens = true;
				break;
			}
		}
		assert(paren_count == 0);
	}

	// If this expression contains any spaces which are not enclosed by parentheses,
	// we need to enclose it so we can treat the whole string as an expression.
	// This happens when two expressions have been part of a binary op earlier.
	if (need_parens)
		return join('(', expr, ')');
	else
		return expr;
}

// Just like to_expression except that we enclose the expression inside parentheses if needed.
string CompilerGLSL::to_enclosed_expression(uint32_t id)
{
	return enclose_expression(to_expression(id));
}

string CompilerGLSL::to_unpacked_expression(uint32_t id)
{
	// If we need to transpose, it will also take care of unpacking rules.
	auto *e = maybe_get<SPIRExpression>(id);
	bool need_transpose = e && e->need_transpose;
	if (!need_transpose && has_decoration(id, DecorationCPacked))
		return unpack_expression_type(to_expression(id), expression_type(id));
	else
		return to_expression(id);
}

string CompilerGLSL::to_enclosed_unpacked_expression(uint32_t id)
{
	// If we need to transpose, it will also take care of unpacking rules.
	auto *e = maybe_get<SPIRExpression>(id);
	bool need_transpose = e && e->need_transpose;
	if (!need_transpose && has_decoration(id, DecorationCPacked))
		return unpack_expression_type(to_expression(id), expression_type(id));
	else
		return to_enclosed_expression(id);
}

string CompilerGLSL::to_extract_component_expression(uint32_t id, uint32_t index)
{
	auto expr = to_enclosed_expression(id);
	if (has_decoration(id, DecorationCPacked))
		return join(expr, "[", index, "]");
	else
		return join(expr, ".", index_to_swizzle(index));
}

string CompilerGLSL::to_expression(uint32_t id)
{
	auto itr = invalid_expressions.find(id);
	if (itr != end(invalid_expressions))
		handle_invalid_expression(id);

	if (ids[id].get_type() == TypeExpression)
	{
		// We might have a more complex chain of dependencies.
		// A possible scenario is that we
		//
		// %1 = OpLoad
		// %2 = OpDoSomething %1 %1. here %2 will have a dependency on %1.
		// %3 = OpDoSomethingAgain %2 %2. Here %3 will lose the link to %1 since we don't propagate the dependencies like that.
		// OpStore %1 %foo // Here we can invalidate %1, and hence all expressions which depend on %1. Only %2 will know since it's part of invalid_expressions.
		// %4 = OpDoSomethingAnotherTime %3 %3 // If we forward all expressions we will see %1 expression after store, not before.
		//
		// However, we can propagate up a list of depended expressions when we used %2, so we can check if %2 is invalid when reading %3 after the store,
		// and see that we should not forward reads of the original variable.
		auto &expr = get<SPIRExpression>(id);
		for (uint32_t dep : expr.expression_dependencies)
			if (invalid_expressions.find(dep) != end(invalid_expressions))
				handle_invalid_expression(dep);
	}

	track_expression_read(id);

	switch (ids[id].get_type())
	{
	case TypeExpression:
	{
		auto &e = get<SPIRExpression>(id);
		if (e.base_expression)
			return to_enclosed_expression(e.base_expression) + e.expression;
		else if (e.need_transpose)
		{
			bool is_packed = has_decoration(id, DecorationCPacked);
			return convert_row_major_matrix(e.expression, get<SPIRType>(e.expression_type), is_packed);
		}
		else
		{
			if (force_recompile)
			{
				// During first compilation phase, certain expression patterns can trigger exponential growth of memory.
				// Avoid this by returning dummy expressions during this phase.
				// Do not use empty expressions here, because those are sentinels for other cases.
				return "_";
			}
			else
				return e.expression;
		}
	}

	case TypeConstant:
	{
		auto &c = get<SPIRConstant>(id);
		auto &type = get<SPIRType>(c.constant_type);

		// WorkGroupSize may be a constant.
		auto &dec = meta[c.self].decoration;
		if (dec.builtin)
			return builtin_to_glsl(dec.builtin_type, StorageClassGeneric);
		else if (c.specialization && options.vulkan_semantics)
			return to_name(id);
		else if (c.is_used_as_lut)
			return to_name(id);
		else if (type.basetype == SPIRType::Struct && !backend.can_declare_struct_inline)
			return to_name(id);
		else if (!type.array.empty() && !backend.can_declare_arrays_inline)
			return to_name(id);
		else
			return constant_expression(c);
	}

	case TypeConstantOp:
		if (options.vulkan_semantics)
			return to_name(id);
		else
			return constant_op_expression(get<SPIRConstantOp>(id));

	case TypeVariable:
	{
		auto &var = get<SPIRVariable>(id);
		// If we try to use a loop variable before the loop header, we have to redirect it to the static expression,
		// the variable has not been declared yet.
		if (var.statically_assigned || (var.loop_variable && !var.loop_variable_enable))
			return to_expression(var.static_expression);
		else if (var.deferred_declaration)
		{
			var.deferred_declaration = false;
			return variable_decl(var);
		}
		else if (flattened_structs.count(id))
		{
			return load_flattened_struct(var);
		}
		else
		{
			auto &dec = meta[var.self].decoration;
			if (dec.builtin)
				return builtin_to_glsl(dec.builtin_type, var.storage);
			else
				return to_name(id);
		}
	}

	case TypeCombinedImageSampler:
		// This type should never be taken the expression of directly.
		// The intention is that texture sampling functions will extract the image and samplers
		// separately and take their expressions as needed.
		// GLSL does not use this type because OpSampledImage immediately creates a combined image sampler
		// expression ala sampler2D(texture, sampler).
		SPIRV_CROSS_THROW("Combined image samplers have no default expression representation.");

	case TypeAccessChain:
		// We cannot express this type. They only have meaning in other OpAccessChains, OpStore or OpLoad.
		SPIRV_CROSS_THROW("Access chains have no default expression representation.");

	default:
		return to_name(id);
	}
}

string CompilerGLSL::constant_op_expression(const SPIRConstantOp &cop)
{
	auto &type = get<SPIRType>(cop.basetype);
	bool binary = false;
	bool unary = false;
	string op;

	if (is_legacy() && is_unsigned_opcode(cop.opcode))
		SPIRV_CROSS_THROW("Unsigned integers are not supported on legacy targets.");

	// TODO: Find a clean way to reuse emit_instruction.
	switch (cop.opcode)
	{
	case OpSConvert:
	case OpUConvert:
	case OpFConvert:
		op = type_to_glsl_constructor(type);
		break;

#define GLSL_BOP(opname, x) \
	case Op##opname:        \
		binary = true;      \
		op = x;             \
		break

#define GLSL_UOP(opname, x) \
	case Op##opname:        \
		unary = true;       \
		op = x;             \
		break

		GLSL_UOP(SNegate, "-");
		GLSL_UOP(Not, "~");
		GLSL_BOP(IAdd, "+");
		GLSL_BOP(ISub, "-");
		GLSL_BOP(IMul, "*");
		GLSL_BOP(SDiv, "/");
		GLSL_BOP(UDiv, "/");
		GLSL_BOP(UMod, "%");
		GLSL_BOP(SMod, "%");
		GLSL_BOP(ShiftRightLogical, ">>");
		GLSL_BOP(ShiftRightArithmetic, ">>");
		GLSL_BOP(ShiftLeftLogical, "<<");
		GLSL_BOP(BitwiseOr, "|");
		GLSL_BOP(BitwiseXor, "^");
		GLSL_BOP(BitwiseAnd, "&");
		GLSL_BOP(LogicalOr, "||");
		GLSL_BOP(LogicalAnd, "&&");
		GLSL_UOP(LogicalNot, "!");
		GLSL_BOP(LogicalEqual, "==");
		GLSL_BOP(LogicalNotEqual, "!=");
		GLSL_BOP(IEqual, "==");
		GLSL_BOP(INotEqual, "!=");
		GLSL_BOP(ULessThan, "<");
		GLSL_BOP(SLessThan, "<");
		GLSL_BOP(ULessThanEqual, "<=");
		GLSL_BOP(SLessThanEqual, "<=");
		GLSL_BOP(UGreaterThan, ">");
		GLSL_BOP(SGreaterThan, ">");
		GLSL_BOP(UGreaterThanEqual, ">=");
		GLSL_BOP(SGreaterThanEqual, ">=");

	case OpSelect:
	{
		if (cop.arguments.size() < 3)
			SPIRV_CROSS_THROW("Not enough arguments to OpSpecConstantOp.");

		// This one is pretty annoying. It's triggered from
		// uint(bool), int(bool) from spec constants.
		// In order to preserve its compile-time constness in Vulkan GLSL,
		// we need to reduce the OpSelect expression back to this simplified model.
		// If we cannot, fail.
		if (to_trivial_mix_op(type, op, cop.arguments[2], cop.arguments[1], cop.arguments[0]))
		{
			// Implement as a simple cast down below.
		}
		else
		{
			// Implement a ternary and pray the compiler understands it :)
			return to_ternary_expression(type, cop.arguments[0], cop.arguments[1], cop.arguments[2]);
		}
		break;
	}

	case OpVectorShuffle:
	{
		string expr = type_to_glsl_constructor(type);
		expr += "(";

		uint32_t left_components = expression_type(cop.arguments[0]).vecsize;
		string left_arg = to_enclosed_expression(cop.arguments[0]);
		string right_arg = to_enclosed_expression(cop.arguments[1]);

		for (uint32_t i = 2; i < uint32_t(cop.arguments.size()); i++)
		{
			uint32_t index = cop.arguments[i];
			if (index >= left_components)
				expr += right_arg + "." + "xyzw"[index - left_components];
			else
				expr += left_arg + "." + "xyzw"[index];

			if (i + 1 < uint32_t(cop.arguments.size()))
				expr += ", ";
		}

		expr += ")";
		return expr;
	}

	case OpCompositeExtract:
	{
		auto expr =
		    access_chain_internal(cop.arguments[0], &cop.arguments[1], uint32_t(cop.arguments.size() - 1), true, false);
		return expr;
	}

	case OpCompositeInsert:
		SPIRV_CROSS_THROW("OpCompositeInsert spec constant op is not supported.");

	default:
		// Some opcodes are unimplemented here, these are currently not possible to test from glslang.
		SPIRV_CROSS_THROW("Unimplemented spec constant op.");
	}

	SPIRType::BaseType input_type;
	bool skip_cast_if_equal_type = glsl_opcode_is_sign_invariant(cop.opcode);

	switch (cop.opcode)
	{
	case OpIEqual:
	case OpINotEqual:
		input_type = SPIRType::Int;
		break;

	default:
		input_type = type.basetype;
		break;
	}

#undef GLSL_BOP
#undef GLSL_UOP
	if (binary)
	{
		if (cop.arguments.size() < 2)
			SPIRV_CROSS_THROW("Not enough arguments to OpSpecConstantOp.");

		string cast_op0;
		string cast_op1;
		auto expected_type = binary_op_bitcast_helper(cast_op0, cast_op1, input_type, cop.arguments[0],
		                                              cop.arguments[1], skip_cast_if_equal_type);

		if (type.basetype != input_type && type.basetype != SPIRType::Boolean)
		{
			expected_type.basetype = input_type;
			auto expr = bitcast_glsl_op(type, expected_type);
			expr += '(';
			expr += join(cast_op0, " ", op, " ", cast_op1);
			expr += ')';
			return expr;
		}
		else
			return join("(", cast_op0, " ", op, " ", cast_op1, ")");
	}
	else if (unary)
	{
		if (cop.arguments.size() < 1)
			SPIRV_CROSS_THROW("Not enough arguments to OpSpecConstantOp.");

		// Auto-bitcast to result type as needed.
		// Works around various casting scenarios in glslang as there is no OpBitcast for specialization constants.
		return join("(", op, bitcast_glsl(type, cop.arguments[0]), ")");
	}
	else
	{
		if (cop.arguments.size() < 1)
			SPIRV_CROSS_THROW("Not enough arguments to OpSpecConstantOp.");
		return join(op, "(", to_expression(cop.arguments[0]), ")");
	}
}

string CompilerGLSL::constant_expression(const SPIRConstant &c)
{
	if (!c.subconstants.empty())
	{
		// Handles Arrays and structures.
		string res;
		if (backend.use_initializer_list)
			res = "{ ";
		else
			res = type_to_glsl_constructor(get<SPIRType>(c.constant_type)) + "(";

		for (auto &elem : c.subconstants)
		{
			auto &subc = get<SPIRConstant>(elem);
			if (subc.specialization && options.vulkan_semantics)
				res += to_name(elem);
			else
				res += constant_expression(subc);

			if (&elem != &c.subconstants.back())
				res += ", ";
		}

		res += backend.use_initializer_list ? " }" : ")";
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
			if (options.vulkan_semantics && c.specialization_constant_id(col) != 0)
				res += to_name(c.specialization_constant_id(col));
			else
				res += constant_expression_vector(c, col);

			if (col + 1 < c.columns())
				res += ", ";
		}
		res += ")";
		return res;
	}
}

#ifdef _MSC_VER
// sprintf warning.
// We cannot rely on snprintf existing because, ..., MSVC.
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

string CompilerGLSL::convert_half_to_string(const SPIRConstant &c, uint32_t col, uint32_t row)
{
	string res;
	float float_value = c.scalar_f16(col, row);

	if (std::isnan(float_value) || std::isinf(float_value))
	{
		if (backend.half_literal_suffix)
		{
			// There is no uintBitsToFloat for 16-bit, so have to rely on legacy fallback here.
			if (float_value == numeric_limits<float>::infinity())
				res = join("(1.0", backend.half_literal_suffix, " / 0.0", backend.half_literal_suffix, ")");
			else if (float_value == -numeric_limits<float>::infinity())
				res = join("(-1.0", backend.half_literal_suffix, " / 0.0", backend.half_literal_suffix, ")");
			else if (std::isnan(float_value))
				res = join("(0.0", backend.half_literal_suffix, " / 0.0", backend.half_literal_suffix, ")");
			else
				SPIRV_CROSS_THROW("Cannot represent non-finite floating point constant.");
		}
		else
		{
			SPIRType type;
			type.basetype = SPIRType::Half;
			type.vecsize = 1;
			type.columns = 1;

			if (float_value == numeric_limits<float>::infinity())
				res = join(type_to_glsl(type), "(1.0 / 0.0)");
			else if (float_value == -numeric_limits<float>::infinity())
				res = join(type_to_glsl(type), "(-1.0 / 0.0)");
			else if (std::isnan(float_value))
				res = join(type_to_glsl(type), "(0.0 / 0.0)");
			else
				SPIRV_CROSS_THROW("Cannot represent non-finite floating point constant.");
		}
	}
	else
	{
		if (backend.half_literal_suffix)
			res = convert_to_string(float_value) + backend.half_literal_suffix;
		else
		{
			// In HLSL (FXC), it's important to cast the literals to half precision right away.
			// There is no literal for it.
			SPIRType type;
			type.basetype = SPIRType::Half;
			type.vecsize = 1;
			type.columns = 1;
			res = join(type_to_glsl(type), "(", convert_to_string(float_value), ")");
		}
	}

	return res;
}

string CompilerGLSL::convert_float_to_string(const SPIRConstant &c, uint32_t col, uint32_t row)
{
	string res;
	float float_value = c.scalar_f32(col, row);

	if (std::isnan(float_value) || std::isinf(float_value))
	{
		// Use special representation.
		if (!is_legacy())
		{
			SPIRType out_type;
			SPIRType in_type;
			out_type.basetype = SPIRType::Float;
			in_type.basetype = SPIRType::UInt;
			out_type.vecsize = 1;
			in_type.vecsize = 1;
			out_type.width = 32;
			in_type.width = 32;

			char print_buffer[32];
			sprintf(print_buffer, "0x%xu", c.scalar(col, row));
			res = join(bitcast_glsl_op(out_type, in_type), "(", print_buffer, ")");
		}
		else
		{
			if (float_value == numeric_limits<float>::infinity())
			{
				if (backend.float_literal_suffix)
					res = "(1.0f / 0.0f)";
				else
					res = "(1.0 / 0.0)";
			}
			else if (float_value == -numeric_limits<float>::infinity())
			{
				if (backend.float_literal_suffix)
					res = "(-1.0f / 0.0f)";
				else
					res = "(-1.0 / 0.0)";
			}
			else if (std::isnan(float_value))
			{
				if (backend.float_literal_suffix)
					res = "(0.0f / 0.0f)";
				else
					res = "(0.0 / 0.0)";
			}
			else
				SPIRV_CROSS_THROW("Cannot represent non-finite floating point constant.");
		}
	}
	else
	{
		res = convert_to_string(float_value);
		if (backend.float_literal_suffix)
			res += "f";
	}

	return res;
}

std::string CompilerGLSL::convert_double_to_string(const SPIRConstant &c, uint32_t col, uint32_t row)
{
	string res;
	double double_value = c.scalar_f64(col, row);

	if (std::isnan(double_value) || std::isinf(double_value))
	{
		// Use special representation.
		if (!is_legacy())
		{
			SPIRType out_type;
			SPIRType in_type;
			out_type.basetype = SPIRType::Double;
			in_type.basetype = SPIRType::UInt64;
			out_type.vecsize = 1;
			in_type.vecsize = 1;
			out_type.width = 64;
			in_type.width = 64;

			uint64_t u64_value = c.scalar_u64(col, row);

			if (options.es)
				SPIRV_CROSS_THROW("64-bit integers/float not supported in ES profile.");
			require_extension_internal("GL_ARB_gpu_shader_int64");

			char print_buffer[64];
			sprintf(print_buffer, "0x%llx%s", static_cast<unsigned long long>(u64_value),
			        backend.long_long_literal_suffix ? "ull" : "ul");
			res = join(bitcast_glsl_op(out_type, in_type), "(", print_buffer, ")");
		}
		else
		{
			if (options.es)
				SPIRV_CROSS_THROW("FP64 not supported in ES profile.");
			if (options.version < 400)
				require_extension_internal("GL_ARB_gpu_shader_fp64");

			if (double_value == numeric_limits<double>::infinity())
			{
				if (backend.double_literal_suffix)
					res = "(1.0lf / 0.0lf)";
				else
					res = "(1.0 / 0.0)";
			}
			else if (double_value == -numeric_limits<double>::infinity())
			{
				if (backend.double_literal_suffix)
					res = "(-1.0lf / 0.0lf)";
				else
					res = "(-1.0 / 0.0)";
			}
			else if (std::isnan(double_value))
			{
				if (backend.double_literal_suffix)
					res = "(0.0lf / 0.0lf)";
				else
					res = "(0.0 / 0.0)";
			}
			else
				SPIRV_CROSS_THROW("Cannot represent non-finite floating point constant.");
		}
	}
	else
	{
		res = convert_to_string(double_value);
		if (backend.double_literal_suffix)
			res += "lf";
	}

	return res;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

string CompilerGLSL::constant_expression_vector(const SPIRConstant &c, uint32_t vector)
{
	auto type = get<SPIRType>(c.constant_type);
	type.columns = 1;

	string res;
	bool splat = backend.use_constructor_splatting && c.vector_size() > 1;
	bool swizzle_splat = backend.can_swizzle_scalar && c.vector_size() > 1;

	if (!type_is_floating_point(type))
	{
		// Cannot swizzle literal integers as a special case.
		swizzle_splat = false;
	}

	if (splat || swizzle_splat)
	{
		// Cannot use constant splatting if we have specialization constants somewhere in the vector.
		for (uint32_t i = 0; i < c.vector_size(); i++)
		{
			if (options.vulkan_semantics && c.specialization_constant_id(vector, i) != 0)
			{
				splat = false;
				swizzle_splat = false;
				break;
			}
		}
	}

	if (splat || swizzle_splat)
	{
		if (type.width == 64)
		{
			uint64_t ident = c.scalar_u64(vector, 0);
			for (uint32_t i = 1; i < c.vector_size(); i++)
			{
				if (ident != c.scalar_u64(vector, i))
				{
					splat = false;
					swizzle_splat = false;
					break;
				}
			}
		}
		else
		{
			uint32_t ident = c.scalar(vector, 0);
			for (uint32_t i = 1; i < c.vector_size(); i++)
			{
				if (ident != c.scalar(vector, i))
				{
					splat = false;
					swizzle_splat = false;
				}
			}
		}
	}

	if (c.vector_size() > 1 && !swizzle_splat)
		res += type_to_glsl(type) + "(";

	switch (type.basetype)
	{
	case SPIRType::Half:
		if (splat || swizzle_splat)
		{
			res += convert_half_to_string(c, vector, 0);
			if (swizzle_splat)
				res = remap_swizzle(get<SPIRType>(c.constant_type), 1, res);
		}
		else
		{
			for (uint32_t i = 0; i < c.vector_size(); i++)
			{
				if (options.vulkan_semantics && c.vector_size() > 1 && c.specialization_constant_id(vector, i) != 0)
					res += to_name(c.specialization_constant_id(vector, i));
				else
					res += convert_half_to_string(c, vector, i);

				if (i + 1 < c.vector_size())
					res += ", ";
			}
		}
		break;

	case SPIRType::Float:
		if (splat || swizzle_splat)
		{
			res += convert_float_to_string(c, vector, 0);
			if (swizzle_splat)
				res = remap_swizzle(get<SPIRType>(c.constant_type), 1, res);
		}
		else
		{
			for (uint32_t i = 0; i < c.vector_size(); i++)
			{
				if (options.vulkan_semantics && c.vector_size() > 1 && c.specialization_constant_id(vector, i) != 0)
					res += to_name(c.specialization_constant_id(vector, i));
				else
					res += convert_float_to_string(c, vector, i);

				if (i + 1 < c.vector_size())
					res += ", ";
			}
		}
		break;

	case SPIRType::Double:
		if (splat || swizzle_splat)
		{
			res += convert_double_to_string(c, vector, 0);
			if (swizzle_splat)
				res = remap_swizzle(get<SPIRType>(c.constant_type), 1, res);
		}
		else
		{
			for (uint32_t i = 0; i < c.vector_size(); i++)
			{
				if (options.vulkan_semantics && c.vector_size() > 1 && c.specialization_constant_id(vector, i) != 0)
					res += to_name(c.specialization_constant_id(vector, i));
				else
					res += convert_double_to_string(c, vector, i);

				if (i + 1 < c.vector_size())
					res += ", ";
			}
		}
		break;

	case SPIRType::Int64:
		if (splat)
		{
			res += convert_to_string(c.scalar_i64(vector, 0));
			if (backend.long_long_literal_suffix)
				res += "ll";
			else
				res += "l";
		}
		else
		{
			for (uint32_t i = 0; i < c.vector_size(); i++)
			{
				if (options.vulkan_semantics && c.vector_size() > 1 && c.specialization_constant_id(vector, i) != 0)
					res += to_name(c.specialization_constant_id(vector, i));
				else
				{
					res += convert_to_string(c.scalar_i64(vector, i));
					if (backend.long_long_literal_suffix)
						res += "ll";
					else
						res += "l";
				}

				if (i + 1 < c.vector_size())
					res += ", ";
			}
		}
		break;

	case SPIRType::UInt64:
		if (splat)
		{
			res += convert_to_string(c.scalar_u64(vector, 0));
			if (backend.long_long_literal_suffix)
				res += "ull";
			else
				res += "ul";
		}
		else
		{
			for (uint32_t i = 0; i < c.vector_size(); i++)
			{
				if (options.vulkan_semantics && c.vector_size() > 1 && c.specialization_constant_id(vector, i) != 0)
					res += to_name(c.specialization_constant_id(vector, i));
				else
				{
					res += convert_to_string(c.scalar_u64(vector, i));
					if (backend.long_long_literal_suffix)
						res += "ull";
					else
						res += "ul";
				}

				if (i + 1 < c.vector_size())
					res += ", ";
			}
		}
		break;

	case SPIRType::UInt:
		if (splat)
		{
			res += convert_to_string(c.scalar(vector, 0));
			if (is_legacy())
			{
				// Fake unsigned constant literals with signed ones if possible.
				// Things like array sizes, etc, tend to be unsigned even though they could just as easily be signed.
				if (c.scalar_i32(vector, 0) < 0)
					SPIRV_CROSS_THROW("Tried to convert uint literal into int, but this made the literal negative.");
			}
			else if (backend.uint32_t_literal_suffix)
				res += "u";
		}
		else
		{
			for (uint32_t i = 0; i < c.vector_size(); i++)
			{
				if (options.vulkan_semantics && c.vector_size() > 1 && c.specialization_constant_id(vector, i) != 0)
					res += to_name(c.specialization_constant_id(vector, i));
				else
				{
					res += convert_to_string(c.scalar(vector, i));
					if (is_legacy())
					{
						// Fake unsigned constant literals with signed ones if possible.
						// Things like array sizes, etc, tend to be unsigned even though they could just as easily be signed.
						if (c.scalar_i32(vector, i) < 0)
							SPIRV_CROSS_THROW(
							    "Tried to convert uint literal into int, but this made the literal negative.");
					}
					else if (backend.uint32_t_literal_suffix)
						res += "u";
				}

				if (i + 1 < c.vector_size())
					res += ", ";
			}
		}
		break;

	case SPIRType::Int:
		if (splat)
			res += convert_to_string(c.scalar_i32(vector, 0));
		else
		{
			for (uint32_t i = 0; i < c.vector_size(); i++)
			{
				if (options.vulkan_semantics && c.vector_size() > 1 && c.specialization_constant_id(vector, i) != 0)
					res += to_name(c.specialization_constant_id(vector, i));
				else
					res += convert_to_string(c.scalar_i32(vector, i));
				if (i + 1 < c.vector_size())
					res += ", ";
			}
		}
		break;

	case SPIRType::Boolean:
		if (splat)
			res += c.scalar(vector, 0) ? "true" : "false";
		else
		{
			for (uint32_t i = 0; i < c.vector_size(); i++)
			{
				if (options.vulkan_semantics && c.vector_size() > 1 && c.specialization_constant_id(vector, i) != 0)
					res += to_name(c.specialization_constant_id(vector, i));
				else
					res += c.scalar(vector, i) ? "true" : "false";

				if (i + 1 < c.vector_size())
					res += ", ";
			}
		}
		break;

	default:
		SPIRV_CROSS_THROW("Invalid constant expression basetype.");
	}

	if (c.vector_size() > 1 && !swizzle_splat)
		res += ")";

	return res;
}

string CompilerGLSL::declare_temporary(uint32_t result_type, uint32_t result_id)
{
	auto &type = get<SPIRType>(result_type);
	auto flags = meta[result_id].decoration.decoration_flags;

	// If we're declaring temporaries inside continue blocks,
	// we must declare the temporary in the loop header so that the continue block can avoid declaring new variables.
	if (current_continue_block && !hoisted_temporaries.count(result_id))
	{
		auto &header = get<SPIRBlock>(current_continue_block->loop_dominator);
		if (find_if(begin(header.declare_temporary), end(header.declare_temporary),
		            [result_type, result_id](const pair<uint32_t, uint32_t> &tmp) {
			            return tmp.first == result_type && tmp.second == result_id;
		            }) == end(header.declare_temporary))
		{
			header.declare_temporary.emplace_back(result_type, result_id);
			hoisted_temporaries.insert(result_id);
			force_recompile = true;
		}

		return join(to_name(result_id), " = ");
	}
	else if (hoisted_temporaries.count(result_id))
	{
		// The temporary has already been declared earlier, so just "declare" the temporary by writing to it.
		return join(to_name(result_id), " = ");
	}
	else
	{
		// The result_id has not been made into an expression yet, so use flags interface.
		add_local_variable_name(result_id);
		return join(flags_to_precision_qualifiers_glsl(type, flags), variable_decl(type, to_name(result_id)), " = ");
	}
}

bool CompilerGLSL::expression_is_forwarded(uint32_t id)
{
	return forwarded_temporaries.find(id) != end(forwarded_temporaries);
}

SPIRExpression &CompilerGLSL::emit_op(uint32_t result_type, uint32_t result_id, const string &rhs, bool forwarding,
                                      bool suppress_usage_tracking)
{
	if (forwarding && (forced_temporaries.find(result_id) == end(forced_temporaries)))
	{
		// Just forward it without temporary.
		// If the forward is trivial, we do not force flushing to temporary for this expression.
		if (!suppress_usage_tracking)
			forwarded_temporaries.insert(result_id);

		return set<SPIRExpression>(result_id, rhs, result_type, true);
	}
	else
	{
		// If expression isn't immutable, bind it to a temporary and make the new temporary immutable (they always are).
		statement(declare_temporary(result_type, result_id), rhs, ";");
		return set<SPIRExpression>(result_id, to_name(result_id), result_type, true);
	}
}

void CompilerGLSL::emit_unary_op(uint32_t result_type, uint32_t result_id, uint32_t op0, const char *op)
{
	bool forward = should_forward(op0);
	emit_op(result_type, result_id, join(op, to_enclosed_unpacked_expression(op0)), forward);
	inherit_expression_dependencies(result_id, op0);
}

void CompilerGLSL::emit_binary_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, const char *op)
{
	bool forward = should_forward(op0) && should_forward(op1);
	emit_op(result_type, result_id,
	        join(to_enclosed_unpacked_expression(op0), " ", op, " ", to_enclosed_unpacked_expression(op1)), forward);

	inherit_expression_dependencies(result_id, op0);
	inherit_expression_dependencies(result_id, op1);
}

void CompilerGLSL::emit_unrolled_unary_op(uint32_t result_type, uint32_t result_id, uint32_t operand, const char *op)
{
	auto &type = get<SPIRType>(result_type);
	auto expr = type_to_glsl_constructor(type);
	expr += '(';
	for (uint32_t i = 0; i < type.vecsize; i++)
	{
		// Make sure to call to_expression multiple times to ensure
		// that these expressions are properly flushed to temporaries if needed.
		expr += op;
		expr += to_extract_component_expression(operand, i);

		if (i + 1 < type.vecsize)
			expr += ", ";
	}
	expr += ')';
	emit_op(result_type, result_id, expr, should_forward(operand));

	inherit_expression_dependencies(result_id, operand);
}

void CompilerGLSL::emit_unrolled_binary_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1,
                                           const char *op)
{
	auto &type = get<SPIRType>(result_type);
	auto expr = type_to_glsl_constructor(type);
	expr += '(';
	for (uint32_t i = 0; i < type.vecsize; i++)
	{
		// Make sure to call to_expression multiple times to ensure
		// that these expressions are properly flushed to temporaries if needed.
		expr += to_extract_component_expression(op0, i);
		expr += ' ';
		expr += op;
		expr += ' ';
		expr += to_extract_component_expression(op1, i);

		if (i + 1 < type.vecsize)
			expr += ", ";
	}
	expr += ')';
	emit_op(result_type, result_id, expr, should_forward(op0) && should_forward(op1));

	inherit_expression_dependencies(result_id, op0);
	inherit_expression_dependencies(result_id, op1);
}

SPIRType CompilerGLSL::binary_op_bitcast_helper(string &cast_op0, string &cast_op1, SPIRType::BaseType &input_type,
                                                uint32_t op0, uint32_t op1, bool skip_cast_if_equal_type)
{
	auto &type0 = expression_type(op0);
	auto &type1 = expression_type(op1);

	// We have to bitcast if our inputs are of different type, or if our types are not equal to expected inputs.
	// For some functions like OpIEqual and INotEqual, we don't care if inputs are of different types than expected
	// since equality test is exactly the same.
	bool cast = (type0.basetype != type1.basetype) || (!skip_cast_if_equal_type && type0.basetype != input_type);

	// Create a fake type so we can bitcast to it.
	// We only deal with regular arithmetic types here like int, uints and so on.
	SPIRType expected_type;
	expected_type.basetype = input_type;
	expected_type.vecsize = type0.vecsize;
	expected_type.columns = type0.columns;
	expected_type.width = type0.width;

	if (cast)
	{
		cast_op0 = bitcast_glsl(expected_type, op0);
		cast_op1 = bitcast_glsl(expected_type, op1);
	}
	else
	{
		// If we don't cast, our actual input type is that of the first (or second) argument.
		cast_op0 = to_enclosed_unpacked_expression(op0);
		cast_op1 = to_enclosed_unpacked_expression(op1);
		input_type = type0.basetype;
	}

	return expected_type;
}

void CompilerGLSL::emit_binary_op_cast(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1,
                                       const char *op, SPIRType::BaseType input_type, bool skip_cast_if_equal_type)
{
	string cast_op0, cast_op1;
	auto expected_type = binary_op_bitcast_helper(cast_op0, cast_op1, input_type, op0, op1, skip_cast_if_equal_type);
	auto &out_type = get<SPIRType>(result_type);

	// We might have casted away from the result type, so bitcast again.
	// For example, arithmetic right shift with uint inputs.
	// Special case boolean outputs since relational opcodes output booleans instead of int/uint.
	string expr;
	if (out_type.basetype != input_type && out_type.basetype != SPIRType::Boolean)
	{
		expected_type.basetype = input_type;
		expr = bitcast_glsl_op(out_type, expected_type);
		expr += '(';
		expr += join(cast_op0, " ", op, " ", cast_op1);
		expr += ')';
	}
	else
		expr += join(cast_op0, " ", op, " ", cast_op1);

	emit_op(result_type, result_id, expr, should_forward(op0) && should_forward(op1));
	inherit_expression_dependencies(result_id, op0);
	inherit_expression_dependencies(result_id, op1);
}

void CompilerGLSL::emit_unary_func_op(uint32_t result_type, uint32_t result_id, uint32_t op0, const char *op)
{
	bool forward = should_forward(op0);
	emit_op(result_type, result_id, join(op, "(", to_unpacked_expression(op0), ")"), forward);
	inherit_expression_dependencies(result_id, op0);
}

void CompilerGLSL::emit_binary_func_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1,
                                       const char *op)
{
	bool forward = should_forward(op0) && should_forward(op1);
	emit_op(result_type, result_id, join(op, "(", to_unpacked_expression(op0), ", ", to_unpacked_expression(op1), ")"),
	        forward);
	inherit_expression_dependencies(result_id, op0);
	inherit_expression_dependencies(result_id, op1);
}

void CompilerGLSL::emit_binary_func_op_cast(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1,
                                            const char *op, SPIRType::BaseType input_type, bool skip_cast_if_equal_type)
{
	string cast_op0, cast_op1;
	auto expected_type = binary_op_bitcast_helper(cast_op0, cast_op1, input_type, op0, op1, skip_cast_if_equal_type);
	auto &out_type = get<SPIRType>(result_type);

	// Special case boolean outputs since relational opcodes output booleans instead of int/uint.
	string expr;
	if (out_type.basetype != input_type && out_type.basetype != SPIRType::Boolean)
	{
		expected_type.basetype = input_type;
		expr = bitcast_glsl_op(out_type, expected_type);
		expr += '(';
		expr += join(op, "(", cast_op0, ", ", cast_op1, ")");
		expr += ')';
	}
	else
	{
		expr += join(op, "(", cast_op0, ", ", cast_op1, ")");
	}

	emit_op(result_type, result_id, expr, should_forward(op0) && should_forward(op1));
	inherit_expression_dependencies(result_id, op0);
	inherit_expression_dependencies(result_id, op1);
}

void CompilerGLSL::emit_trinary_func_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1,
                                        uint32_t op2, const char *op)
{
	bool forward = should_forward(op0) && should_forward(op1) && should_forward(op2);
	emit_op(result_type, result_id,
	        join(op, "(", to_unpacked_expression(op0), ", ", to_unpacked_expression(op1), ", ",
	             to_unpacked_expression(op2), ")"),
	        forward);

	inherit_expression_dependencies(result_id, op0);
	inherit_expression_dependencies(result_id, op1);
	inherit_expression_dependencies(result_id, op2);
}

void CompilerGLSL::emit_quaternary_func_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1,
                                           uint32_t op2, uint32_t op3, const char *op)
{
	bool forward = should_forward(op0) && should_forward(op1) && should_forward(op2) && should_forward(op3);
	emit_op(result_type, result_id,
	        join(op, "(", to_unpacked_expression(op0), ", ", to_unpacked_expression(op1), ", ",
	             to_unpacked_expression(op2), ", ", to_unpacked_expression(op3), ")"),
	        forward);

	inherit_expression_dependencies(result_id, op0);
	inherit_expression_dependencies(result_id, op1);
	inherit_expression_dependencies(result_id, op2);
	inherit_expression_dependencies(result_id, op3);
}

// EXT_shader_texture_lod only concerns fragment shaders so lod tex functions
// are not allowed in ES 2 vertex shaders. But SPIR-V only supports lod tex
// functions in vertex shaders so we revert those back to plain calls when
// the lod is a constant value of zero.
bool CompilerGLSL::check_explicit_lod_allowed(uint32_t lod)
{
	auto &execution = get_entry_point();
	bool allowed = !is_legacy_es() || execution.model == ExecutionModelFragment;
	if (!allowed && lod != 0)
	{
		auto *lod_constant = maybe_get<SPIRConstant>(lod);
		if (!lod_constant || lod_constant->scalar_f32() != 0.0f)
		{
			SPIRV_CROSS_THROW("Explicit lod not allowed in legacy ES non-fragment shaders.");
		}
	}
	return allowed;
}

string CompilerGLSL::legacy_tex_op(const std::string &op, const SPIRType &imgtype, uint32_t lod, uint32_t tex)
{
	const char *type;
	switch (imgtype.image.dim)
	{
	case spv::Dim1D:
		type = (imgtype.image.arrayed && !options.es) ? "1DArray" : "1D";
		break;
	case spv::Dim2D:
		type = (imgtype.image.arrayed && !options.es) ? "2DArray" : "2D";
		break;
	case spv::Dim3D:
		type = "3D";
		break;
	case spv::DimCube:
		type = "Cube";
		break;
	case spv::DimBuffer:
		type = "Buffer";
		break;
	case spv::DimSubpassData:
		type = "2D";
		break;
	default:
		type = "";
		break;
	}

	bool use_explicit_lod = check_explicit_lod_allowed(lod);

	if (op == "textureLod" || op == "textureProjLod" || op == "textureGrad" || op == "textureProjGrad")
	{
		if (is_legacy_es())
		{
			if (use_explicit_lod)
				require_extension_internal("GL_EXT_shader_texture_lod");
		}
		else if (is_legacy())
			require_extension_internal("GL_ARB_shader_texture_lod");
	}

	if (op == "textureLodOffset" || op == "textureProjLodOffset")
	{
		if (is_legacy_es())
			SPIRV_CROSS_THROW(join(op, " not allowed in legacy ES"));

		require_extension_internal("GL_EXT_gpu_shader4");
	}

	// GLES has very limited support for shadow samplers.
	// Basically shadow2D and shadow2DProj work through EXT_shadow_samplers,
	// everything else can just throw
	if (image_is_comparison(imgtype, tex) && is_legacy_es())
	{
		if (op == "texture" || op == "textureProj")
			require_extension_internal("GL_EXT_shadow_samplers");
		else
			SPIRV_CROSS_THROW(join(op, " not allowed on depth samplers in legacy ES"));
	}

	bool is_es_and_depth = is_legacy_es() && image_is_comparison(imgtype, tex);
	std::string type_prefix = image_is_comparison(imgtype, tex) ? "shadow" : "texture";

	if (op == "texture")
		return is_es_and_depth ? join(type_prefix, type, "EXT") : join(type_prefix, type);
	else if (op == "textureLod")
	{
		if (use_explicit_lod)
			return join(type_prefix, type, is_legacy_es() ? "LodEXT" : "Lod");
		else
			return join(type_prefix, type);
	}
	else if (op == "textureProj")
		return join(type_prefix, type, is_es_and_depth ? "ProjEXT" : "Proj");
	else if (op == "textureGrad")
		return join(type_prefix, type, is_legacy_es() ? "GradEXT" : is_legacy_desktop() ? "GradARB" : "Grad");
	else if (op == "textureProjLod")
	{
		if (use_explicit_lod)
			return join(type_prefix, type, is_legacy_es() ? "ProjLodEXT" : "ProjLod");
		else
			return join(type_prefix, type, "Proj");
	}
	else if (op == "textureLodOffset")
	{
		if (use_explicit_lod)
			return join(type_prefix, type, "LodOffset");
		else
			return join(type_prefix, type);
	}
	else if (op == "textureProjGrad")
		return join(type_prefix, type,
		            is_legacy_es() ? "ProjGradEXT" : is_legacy_desktop() ? "ProjGradARB" : "ProjGrad");
	else if (op == "textureProjLodOffset")
	{
		if (use_explicit_lod)
			return join(type_prefix, type, "ProjLodOffset");
		else
			return join(type_prefix, type, "ProjOffset");
	}
	else
	{
		SPIRV_CROSS_THROW(join("Unsupported legacy texture op: ", op));
	}
}

bool CompilerGLSL::to_trivial_mix_op(const SPIRType &type, string &op, uint32_t left, uint32_t right, uint32_t lerp)
{
	auto *cleft = maybe_get<SPIRConstant>(left);
	auto *cright = maybe_get<SPIRConstant>(right);
	auto &lerptype = expression_type(lerp);

	// If our targets aren't constants, we cannot use construction.
	if (!cleft || !cright)
		return false;

	// If our targets are spec constants, we cannot use construction.
	if (cleft->specialization || cright->specialization)
		return false;

	// We can only use trivial construction if we have a scalar
	// (should be possible to do it for vectors as well, but that is overkill for now).
	if (lerptype.basetype != SPIRType::Boolean || lerptype.vecsize > 1)
		return false;

	// If our bool selects between 0 and 1, we can cast from bool instead, making our trivial constructor.
	bool ret = false;
	switch (type.basetype)
	{
	case SPIRType::Int:
	case SPIRType::UInt:
		ret = cleft->scalar() == 0 && cright->scalar() == 1;
		break;

	case SPIRType::Half:
		ret = cleft->scalar_f16() == 0.0f && cright->scalar_f16() == 1.0f;
		break;

	case SPIRType::Float:
		ret = cleft->scalar_f32() == 0.0f && cright->scalar_f32() == 1.0f;
		break;

	case SPIRType::Double:
		ret = cleft->scalar_f64() == 0.0 && cright->scalar_f64() == 1.0;
		break;

	case SPIRType::Int64:
	case SPIRType::UInt64:
		ret = cleft->scalar_u64() == 0 && cright->scalar_u64() == 1;
		break;

	default:
		break;
	}

	if (ret)
		op = type_to_glsl_constructor(type);
	return ret;
}

string CompilerGLSL::to_ternary_expression(const SPIRType &restype, uint32_t select, uint32_t true_value,
                                           uint32_t false_value)
{
	string expr;
	auto &lerptype = expression_type(select);

	if (lerptype.vecsize == 1)
		expr = join(to_enclosed_expression(select), " ? ", to_enclosed_expression(true_value), " : ",
		            to_enclosed_expression(false_value));
	else
	{
		auto swiz = [this](uint32_t expression, uint32_t i) { return to_extract_component_expression(expression, i); };

		expr = type_to_glsl_constructor(restype);
		expr += "(";
		for (uint32_t i = 0; i < restype.vecsize; i++)
		{
			expr += swiz(select, i);
			expr += " ? ";
			expr += swiz(true_value, i);
			expr += " : ";
			expr += swiz(false_value, i);
			if (i + 1 < restype.vecsize)
				expr += ", ";
		}
		expr += ")";
	}

	return expr;
}

void CompilerGLSL::emit_mix_op(uint32_t result_type, uint32_t id, uint32_t left, uint32_t right, uint32_t lerp)
{
	auto &lerptype = expression_type(lerp);
	auto &restype = get<SPIRType>(result_type);

	string mix_op;
	bool has_boolean_mix = backend.boolean_mix_support &&
	                       ((options.es && options.version >= 310) || (!options.es && options.version >= 450));
	bool trivial_mix = to_trivial_mix_op(restype, mix_op, left, right, lerp);

	// Cannot use boolean mix when the lerp argument is just one boolean,
	// fall back to regular trinary statements.
	if (lerptype.vecsize == 1)
		has_boolean_mix = false;

	// If we can reduce the mix to a simple cast, do so.
	// This helps for cases like int(bool), uint(bool) which is implemented with
	// OpSelect bool 1 0.
	if (trivial_mix)
	{
		emit_unary_func_op(result_type, id, lerp, mix_op.c_str());
	}
	else if (!has_boolean_mix && lerptype.basetype == SPIRType::Boolean)
	{
		// Boolean mix not supported on desktop without extension.
		// Was added in OpenGL 4.5 with ES 3.1 compat.
		//
		// Could use GL_EXT_shader_integer_mix on desktop at least,
		// but Apple doesn't support it. :(
		// Just implement it as ternary expressions.
		auto expr = to_ternary_expression(get<SPIRType>(result_type), lerp, right, left);
		emit_op(result_type, id, expr, should_forward(left) && should_forward(right) && should_forward(lerp));
		inherit_expression_dependencies(id, left);
		inherit_expression_dependencies(id, right);
		inherit_expression_dependencies(id, lerp);
	}
	else
		emit_trinary_func_op(result_type, id, left, right, lerp, "mix");
}

string CompilerGLSL::to_combined_image_sampler(uint32_t image_id, uint32_t samp_id)
{
	// Keep track of the array indices we have used to load the image.
	// We'll need to use the same array index into the combined image sampler array.
	auto image_expr = to_expression(image_id);
	string array_expr;
	auto array_index = image_expr.find_first_of('[');
	if (array_index != string::npos)
		array_expr = image_expr.substr(array_index, string::npos);

	auto &args = current_function->arguments;

	// For GLSL and ESSL targets, we must enumerate all possible combinations for sampler2D(texture2D, sampler) and redirect
	// all possible combinations into new sampler2D uniforms.
	auto *image = maybe_get_backing_variable(image_id);
	auto *samp = maybe_get_backing_variable(samp_id);
	if (image)
		image_id = image->self;
	if (samp)
		samp_id = samp->self;

	auto image_itr = find_if(begin(args), end(args),
	                         [image_id](const SPIRFunction::Parameter &param) { return param.id == image_id; });

	auto sampler_itr = find_if(begin(args), end(args),
	                           [samp_id](const SPIRFunction::Parameter &param) { return param.id == samp_id; });

	if (image_itr != end(args) || sampler_itr != end(args))
	{
		// If any parameter originates from a parameter, we will find it in our argument list.
		bool global_image = image_itr == end(args);
		bool global_sampler = sampler_itr == end(args);
		uint32_t iid = global_image ? image_id : uint32_t(image_itr - begin(args));
		uint32_t sid = global_sampler ? samp_id : uint32_t(sampler_itr - begin(args));

		auto &combined = current_function->combined_parameters;
		auto itr = find_if(begin(combined), end(combined), [=](const SPIRFunction::CombinedImageSamplerParameter &p) {
			return p.global_image == global_image && p.global_sampler == global_sampler && p.image_id == iid &&
			       p.sampler_id == sid;
		});

		if (itr != end(combined))
			return to_expression(itr->id) + array_expr;
		else
		{
			SPIRV_CROSS_THROW(
			    "Cannot find mapping for combined sampler parameter, was build_combined_image_samplers() used "
			    "before compile() was called?");
		}
	}
	else
	{
		// For global sampler2D, look directly at the global remapping table.
		auto &mapping = combined_image_samplers;
		auto itr = find_if(begin(mapping), end(mapping), [image_id, samp_id](const CombinedImageSampler &combined) {
			return combined.image_id == image_id && combined.sampler_id == samp_id;
		});

		if (itr != end(combined_image_samplers))
			return to_expression(itr->combined_id) + array_expr;
		else
		{
			SPIRV_CROSS_THROW("Cannot find mapping for combined sampler, was build_combined_image_samplers() used "
			                  "before compile() was called?");
		}
	}
}

void CompilerGLSL::emit_sampled_image_op(uint32_t result_type, uint32_t result_id, uint32_t image_id, uint32_t samp_id)
{
	if (options.vulkan_semantics && combined_image_samplers.empty())
	{
		emit_binary_func_op(result_type, result_id, image_id, samp_id,
		                    type_to_glsl(get<SPIRType>(result_type), result_id).c_str());

		// Make sure to suppress usage tracking. It is illegal to create temporaries of opaque types.
		forwarded_temporaries.erase(result_id);
	}
	else
	{
		// Make sure to suppress usage tracking. It is illegal to create temporaries of opaque types.
		emit_op(result_type, result_id, to_combined_image_sampler(image_id, samp_id), true, true);
	}
}

void CompilerGLSL::emit_texture_op(const Instruction &i)
{
	auto ops = stream(i);
	auto op = static_cast<Op>(i.op);
	uint32_t length = i.length;

	if (i.offset + length > spirv.size())
		SPIRV_CROSS_THROW("Compiler::parse() opcode out of range.");

	vector<uint32_t> inherited_expressions;

	uint32_t result_type = ops[0];
	uint32_t id = ops[1];
	uint32_t img = ops[2];
	uint32_t coord = ops[3];
	uint32_t dref = 0;
	uint32_t comp = 0;
	bool gather = false;
	bool proj = false;
	bool fetch = false;
	const uint32_t *opt = nullptr;

	inherited_expressions.push_back(coord);

	switch (op)
	{
	case OpImageSampleDrefImplicitLod:
	case OpImageSampleDrefExplicitLod:
		dref = ops[4];
		opt = &ops[5];
		length -= 5;
		break;

	case OpImageSampleProjDrefImplicitLod:
	case OpImageSampleProjDrefExplicitLod:
		dref = ops[4];
		opt = &ops[5];
		length -= 5;
		proj = true;
		break;

	case OpImageDrefGather:
		dref = ops[4];
		opt = &ops[5];
		length -= 5;
		gather = true;
		break;

	case OpImageGather:
		comp = ops[4];
		opt = &ops[5];
		length -= 5;
		gather = true;
		break;

	case OpImageFetch:
	case OpImageRead: // Reads == fetches in Metal (other langs will not get here)
		opt = &ops[4];
		length -= 4;
		fetch = true;
		break;

	case OpImageSampleProjImplicitLod:
	case OpImageSampleProjExplicitLod:
		opt = &ops[4];
		length -= 4;
		proj = true;
		break;

	default:
		opt = &ops[4];
		length -= 4;
		break;
	}

	// Bypass pointers because we need the real image struct
	auto &type = expression_type(img);
	auto &imgtype = get<SPIRType>(type.self);

	uint32_t coord_components = 0;
	switch (imgtype.image.dim)
	{
	case spv::Dim1D:
		coord_components = 1;
		break;
	case spv::Dim2D:
		coord_components = 2;
		break;
	case spv::Dim3D:
		coord_components = 3;
		break;
	case spv::DimCube:
		coord_components = 3;
		break;
	case spv::DimBuffer:
		coord_components = 1;
		break;
	default:
		coord_components = 2;
		break;
	}

	if (dref)
		inherited_expressions.push_back(dref);

	if (proj)
		coord_components++;
	if (imgtype.image.arrayed)
		coord_components++;

	uint32_t bias = 0;
	uint32_t lod = 0;
	uint32_t grad_x = 0;
	uint32_t grad_y = 0;
	uint32_t coffset = 0;
	uint32_t offset = 0;
	uint32_t coffsets = 0;
	uint32_t sample = 0;
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
			inherited_expressions.push_back(v);
			length--;
		}
	};

	test(bias, ImageOperandsBiasMask);
	test(lod, ImageOperandsLodMask);
	test(grad_x, ImageOperandsGradMask);
	test(grad_y, ImageOperandsGradMask);
	test(coffset, ImageOperandsConstOffsetMask);
	test(offset, ImageOperandsOffsetMask);
	test(coffsets, ImageOperandsConstOffsetsMask);
	test(sample, ImageOperandsSampleMask);

	string expr;
	bool forward = false;
	expr += to_function_name(img, imgtype, !!fetch, !!gather, !!proj, !!coffsets, (!!coffset || !!offset),
	                         (!!grad_x || !!grad_y), !!dref, lod);
	expr += "(";
	expr += to_function_args(img, imgtype, fetch, gather, proj, coord, coord_components, dref, grad_x, grad_y, lod,
	                         coffset, offset, bias, comp, sample, &forward);
	expr += ")";

	// texture(samplerXShadow) returns float. shadowX() returns vec4. Swizzle here.
	if (is_legacy() && image_is_comparison(imgtype, img))
		expr += ".r";

	emit_op(result_type, id, expr, forward);
	for (auto &inherit : inherited_expressions)
		inherit_expression_dependencies(id, inherit);

	switch (op)
	{
	case OpImageSampleDrefImplicitLod:
	case OpImageSampleImplicitLod:
	case OpImageSampleProjImplicitLod:
	case OpImageSampleProjDrefImplicitLod:
		register_control_dependent_expression(id);
		break;

	default:
		break;
	}
}

// Returns the function name for a texture sampling function for the specified image and sampling characteristics.
// For some subclasses, the function is a method on the specified image.
string CompilerGLSL::to_function_name(uint32_t tex, const SPIRType &imgtype, bool is_fetch, bool is_gather,
                                      bool is_proj, bool has_array_offsets, bool has_offset, bool has_grad, bool,
                                      uint32_t lod)
{
	string fname;

	// textureLod on sampler2DArrayShadow and samplerCubeShadow does not exist in GLSL for some reason.
	// To emulate this, we will have to use textureGrad with a constant gradient of 0.
	// The workaround will assert that the LOD is in fact constant 0, or we cannot emit correct code.
	// This happens for HLSL SampleCmpLevelZero on Texture2DArray and TextureCube.
	bool workaround_lod_array_shadow_as_grad = false;
	if (((imgtype.image.arrayed && imgtype.image.dim == Dim2D) || imgtype.image.dim == DimCube) &&
	    image_is_comparison(imgtype, tex) && lod)
	{
		auto *constant_lod = maybe_get<SPIRConstant>(lod);
		if (!constant_lod || constant_lod->scalar_f32() != 0.0f)
			SPIRV_CROSS_THROW(
			    "textureLod on sampler2DArrayShadow is not constant 0.0. This cannot be expressed in GLSL.");
		workaround_lod_array_shadow_as_grad = true;
	}

	if (is_fetch)
		fname += "texelFetch";
	else
	{
		fname += "texture";

		if (is_gather)
			fname += "Gather";
		if (has_array_offsets)
			fname += "Offsets";
		if (is_proj)
			fname += "Proj";
		if (has_grad || workaround_lod_array_shadow_as_grad)
			fname += "Grad";
		if (!!lod && !workaround_lod_array_shadow_as_grad)
			fname += "Lod";
	}

	if (has_offset)
		fname += "Offset";

	return is_legacy() ? legacy_tex_op(fname, imgtype, lod, tex) : fname;
}

std::string CompilerGLSL::convert_separate_image_to_combined(uint32_t id)
{
	auto &imgtype = expression_type(id);
	auto *var = maybe_get_backing_variable(id);

	// If we are fetching from a plain OpTypeImage, we must combine with a dummy sampler.
	if (var)
	{
		auto &type = get<SPIRType>(var->basetype);
		if (type.basetype == SPIRType::Image && type.image.sampled == 1 && type.image.dim != DimBuffer)
		{
			if (!dummy_sampler_id)
				SPIRV_CROSS_THROW(
				    "Cannot find dummy sampler ID. Was build_dummy_sampler_for_combined_images() called?");

			if (options.vulkan_semantics)
			{
				auto sampled_type = imgtype;
				sampled_type.basetype = SPIRType::SampledImage;
				return join(type_to_glsl(sampled_type), "(", to_expression(id), ", ", to_expression(dummy_sampler_id),
				            ")");
			}
			else
				return to_combined_image_sampler(id, dummy_sampler_id);
		}
	}

	return to_expression(id);
}

// Returns the function args for a texture sampling function for the specified image and sampling characteristics.
string CompilerGLSL::to_function_args(uint32_t img, const SPIRType &imgtype, bool is_fetch, bool is_gather,
                                      bool is_proj, uint32_t coord, uint32_t coord_components, uint32_t dref,
                                      uint32_t grad_x, uint32_t grad_y, uint32_t lod, uint32_t coffset, uint32_t offset,
                                      uint32_t bias, uint32_t comp, uint32_t sample, bool *p_forward)
{
	string farg_str;
	if (is_fetch)
		farg_str = convert_separate_image_to_combined(img);
	else
		farg_str = to_expression(img);

	bool swizz_func = backend.swizzle_is_function;
	auto swizzle = [swizz_func](uint32_t comps, uint32_t in_comps) -> const char * {
		if (comps == in_comps)
			return "";

		switch (comps)
		{
		case 1:
			return ".x";
		case 2:
			return swizz_func ? ".xy()" : ".xy";
		case 3:
			return swizz_func ? ".xyz()" : ".xyz";
		default:
			return "";
		}
	};

	bool forward = should_forward(coord);

	// The IR can give us more components than we need, so chop them off as needed.
	auto swizzle_expr = swizzle(coord_components, expression_type(coord).vecsize);
	// Only enclose the UV expression if needed.
	auto coord_expr = (*swizzle_expr == '\0') ? to_expression(coord) : (to_enclosed_expression(coord) + swizzle_expr);

	// texelFetch only takes int, not uint.
	auto &coord_type = expression_type(coord);
	if (coord_type.basetype == SPIRType::UInt)
	{
		auto expected_type = coord_type;
		expected_type.basetype = SPIRType::Int;
		coord_expr = bitcast_expression(expected_type, coord_type.basetype, coord_expr);
	}

	// textureLod on sampler2DArrayShadow and samplerCubeShadow does not exist in GLSL for some reason.
	// To emulate this, we will have to use textureGrad with a constant gradient of 0.
	// The workaround will assert that the LOD is in fact constant 0, or we cannot emit correct code.
	// This happens for HLSL SampleCmpLevelZero on Texture2DArray and TextureCube.
	bool workaround_lod_array_shadow_as_grad =
	    ((imgtype.image.arrayed && imgtype.image.dim == Dim2D) || imgtype.image.dim == DimCube) &&
	    image_is_comparison(imgtype, img) && lod;

	if (dref)
	{
		forward = forward && should_forward(dref);

		// SPIR-V splits dref and coordinate.
		if (is_gather || coord_components == 4) // GLSL also splits the arguments in two. Same for textureGather.
		{
			farg_str += ", ";
			farg_str += to_expression(coord);
			farg_str += ", ";
			farg_str += to_expression(dref);
		}
		else if (is_proj)
		{
			// Have to reshuffle so we get vec4(coord, dref, proj), special case.
			// Other shading languages splits up the arguments for coord and compare value like SPIR-V.
			// The coordinate type for textureProj shadow is always vec4 even for sampler1DShadow.
			farg_str += ", vec4(";

			if (imgtype.image.dim == Dim1D)
			{
				// Could reuse coord_expr, but we will mess up the temporary usage checking.
				farg_str += to_enclosed_expression(coord) + ".x";
				farg_str += ", ";
				farg_str += "0.0, ";
				farg_str += to_expression(dref);
				farg_str += ", ";
				farg_str += to_enclosed_expression(coord) + ".y)";
			}
			else if (imgtype.image.dim == Dim2D)
			{
				// Could reuse coord_expr, but we will mess up the temporary usage checking.
				farg_str += to_enclosed_expression(coord) + (swizz_func ? ".xy()" : ".xy");
				farg_str += ", ";
				farg_str += to_expression(dref);
				farg_str += ", ";
				farg_str += to_enclosed_expression(coord) + ".z)";
			}
			else
				SPIRV_CROSS_THROW("Invalid type for textureProj with shadow.");
		}
		else
		{
			// Create a composite which merges coord/dref into a single vector.
			auto type = expression_type(coord);
			type.vecsize = coord_components + 1;
			farg_str += ", ";
			farg_str += type_to_glsl_constructor(type);
			farg_str += "(";
			farg_str += coord_expr;
			farg_str += ", ";
			farg_str += to_expression(dref);
			farg_str += ")";
		}
	}
	else
	{
		farg_str += ", ";
		farg_str += coord_expr;
	}

	if (grad_x || grad_y)
	{
		forward = forward && should_forward(grad_x);
		forward = forward && should_forward(grad_y);
		farg_str += ", ";
		farg_str += to_expression(grad_x);
		farg_str += ", ";
		farg_str += to_expression(grad_y);
	}

	if (lod)
	{
		if (workaround_lod_array_shadow_as_grad)
		{
			// Implement textureGrad() instead. LOD == 0.0 is implemented as gradient of 0.0.
			// Implementing this as plain texture() is not safe on some implementations.
			if (imgtype.image.dim == Dim2D)
				farg_str += ", vec2(0.0), vec2(0.0)";
			else if (imgtype.image.dim == DimCube)
				farg_str += ", vec3(0.0), vec3(0.0)";
		}
		else
		{
			if (check_explicit_lod_allowed(lod))
			{
				forward = forward && should_forward(lod);
				farg_str += ", ";
				farg_str += to_expression(lod);
			}
		}
	}
	else if (is_fetch && imgtype.image.dim != DimBuffer && !imgtype.image.ms)
	{
		// Lod argument is optional in OpImageFetch, but we require a LOD value, pick 0 as the default.
		farg_str += ", 0";
	}

	if (coffset)
	{
		forward = forward && should_forward(coffset);
		farg_str += ", ";
		farg_str += to_expression(coffset);
	}
	else if (offset)
	{
		forward = forward && should_forward(offset);
		farg_str += ", ";
		farg_str += to_expression(offset);
	}

	if (bias)
	{
		forward = forward && should_forward(bias);
		farg_str += ", ";
		farg_str += to_expression(bias);
	}

	if (comp)
	{
		forward = forward && should_forward(comp);
		farg_str += ", ";
		farg_str += to_expression(comp);
	}

	if (sample)
	{
		farg_str += ", ";
		farg_str += to_expression(sample);
	}

	*p_forward = forward;

	return farg_str;
}

void CompilerGLSL::emit_glsl_op(uint32_t result_type, uint32_t id, uint32_t eop, const uint32_t *args, uint32_t)
{
	auto op = static_cast<GLSLstd450>(eop);

	if (is_legacy() && is_unsigned_glsl_opcode(op))
		SPIRV_CROSS_THROW("Unsigned integers are not supported on legacy GLSL targets.");

	switch (op)
	{
	// FP fiddling
	case GLSLstd450Round:
		emit_unary_func_op(result_type, id, args[0], "round");
		break;

	case GLSLstd450RoundEven:
		if ((options.es && options.version >= 300) || (!options.es && options.version >= 130))
			emit_unary_func_op(result_type, id, args[0], "roundEven");
		else
			SPIRV_CROSS_THROW("roundEven supported only in ESSL 300 and GLSL 130 and up.");
		break;

	case GLSLstd450Trunc:
		emit_unary_func_op(result_type, id, args[0], "trunc");
		break;
	case GLSLstd450SAbs:
	case GLSLstd450FAbs:
		emit_unary_func_op(result_type, id, args[0], "abs");
		break;
	case GLSLstd450SSign:
	case GLSLstd450FSign:
		emit_unary_func_op(result_type, id, args[0], "sign");
		break;
	case GLSLstd450Floor:
		emit_unary_func_op(result_type, id, args[0], "floor");
		break;
	case GLSLstd450Ceil:
		emit_unary_func_op(result_type, id, args[0], "ceil");
		break;
	case GLSLstd450Fract:
		emit_unary_func_op(result_type, id, args[0], "fract");
		break;
	case GLSLstd450Radians:
		emit_unary_func_op(result_type, id, args[0], "radians");
		break;
	case GLSLstd450Degrees:
		emit_unary_func_op(result_type, id, args[0], "degrees");
		break;
	case GLSLstd450Fma:
		emit_trinary_func_op(result_type, id, args[0], args[1], args[2], "fma");
		break;
	case GLSLstd450Modf:
		register_call_out_argument(args[1]);
		forced_temporaries.insert(id);
		emit_binary_func_op(result_type, id, args[0], args[1], "modf");
		break;

	case GLSLstd450ModfStruct:
	{
		forced_temporaries.insert(id);
		auto &type = get<SPIRType>(result_type);
		auto flags = meta[id].decoration.decoration_flags;
		statement(flags_to_precision_qualifiers_glsl(type, flags), variable_decl(type, to_name(id)), ";");
		set<SPIRExpression>(id, to_name(id), result_type, true);

		statement(to_expression(id), ".", to_member_name(type, 0), " = ", "modf(", to_expression(args[0]), ", ",
		          to_expression(id), ".", to_member_name(type, 1), ");");
		break;
	}

	// Minmax
	case GLSLstd450UMin:
	case GLSLstd450FMin:
	case GLSLstd450SMin:
		emit_binary_func_op(result_type, id, args[0], args[1], "min");
		break;
	case GLSLstd450FMax:
	case GLSLstd450UMax:
	case GLSLstd450SMax:
		emit_binary_func_op(result_type, id, args[0], args[1], "max");
		break;
	case GLSLstd450FClamp:
	case GLSLstd450UClamp:
	case GLSLstd450SClamp:
		emit_trinary_func_op(result_type, id, args[0], args[1], args[2], "clamp");
		break;

	// Trig
	case GLSLstd450Sin:
		emit_unary_func_op(result_type, id, args[0], "sin");
		break;
	case GLSLstd450Cos:
		emit_unary_func_op(result_type, id, args[0], "cos");
		break;
	case GLSLstd450Tan:
		emit_unary_func_op(result_type, id, args[0], "tan");
		break;
	case GLSLstd450Asin:
		emit_unary_func_op(result_type, id, args[0], "asin");
		break;
	case GLSLstd450Acos:
		emit_unary_func_op(result_type, id, args[0], "acos");
		break;
	case GLSLstd450Atan:
		emit_unary_func_op(result_type, id, args[0], "atan");
		break;
	case GLSLstd450Sinh:
		emit_unary_func_op(result_type, id, args[0], "sinh");
		break;
	case GLSLstd450Cosh:
		emit_unary_func_op(result_type, id, args[0], "cosh");
		break;
	case GLSLstd450Tanh:
		emit_unary_func_op(result_type, id, args[0], "tanh");
		break;
	case GLSLstd450Asinh:
		emit_unary_func_op(result_type, id, args[0], "asinh");
		break;
	case GLSLstd450Acosh:
		emit_unary_func_op(result_type, id, args[0], "acosh");
		break;
	case GLSLstd450Atanh:
		emit_unary_func_op(result_type, id, args[0], "atanh");
		break;
	case GLSLstd450Atan2:
		emit_binary_func_op(result_type, id, args[0], args[1], "atan");
		break;

	// Exponentials
	case GLSLstd450Pow:
		emit_binary_func_op(result_type, id, args[0], args[1], "pow");
		break;
	case GLSLstd450Exp:
		emit_unary_func_op(result_type, id, args[0], "exp");
		break;
	case GLSLstd450Log:
		emit_unary_func_op(result_type, id, args[0], "log");
		break;
	case GLSLstd450Exp2:
		emit_unary_func_op(result_type, id, args[0], "exp2");
		break;
	case GLSLstd450Log2:
		emit_unary_func_op(result_type, id, args[0], "log2");
		break;
	case GLSLstd450Sqrt:
		emit_unary_func_op(result_type, id, args[0], "sqrt");
		break;
	case GLSLstd450InverseSqrt:
		emit_unary_func_op(result_type, id, args[0], "inversesqrt");
		break;

	// Matrix math
	case GLSLstd450Determinant:
		emit_unary_func_op(result_type, id, args[0], "determinant");
		break;
	case GLSLstd450MatrixInverse:
		emit_unary_func_op(result_type, id, args[0], "inverse");
		break;

	// Lerping
	case GLSLstd450FMix:
	case GLSLstd450IMix:
	{
		emit_mix_op(result_type, id, args[0], args[1], args[2]);
		break;
	}
	case GLSLstd450Step:
		emit_binary_func_op(result_type, id, args[0], args[1], "step");
		break;
	case GLSLstd450SmoothStep:
		emit_trinary_func_op(result_type, id, args[0], args[1], args[2], "smoothstep");
		break;

	// Packing
	case GLSLstd450Frexp:
		register_call_out_argument(args[1]);
		forced_temporaries.insert(id);
		emit_binary_func_op(result_type, id, args[0], args[1], "frexp");
		break;

	case GLSLstd450FrexpStruct:
	{
		forced_temporaries.insert(id);
		auto &type = get<SPIRType>(result_type);
		auto flags = meta[id].decoration.decoration_flags;
		statement(flags_to_precision_qualifiers_glsl(type, flags), variable_decl(type, to_name(id)), ";");
		set<SPIRExpression>(id, to_name(id), result_type, true);

		statement(to_expression(id), ".", to_member_name(type, 0), " = ", "frexp(", to_expression(args[0]), ", ",
		          to_expression(id), ".", to_member_name(type, 1), ");");
		break;
	}

	case GLSLstd450Ldexp:
		emit_binary_func_op(result_type, id, args[0], args[1], "ldexp");
		break;
	case GLSLstd450PackSnorm4x8:
		emit_unary_func_op(result_type, id, args[0], "packSnorm4x8");
		break;
	case GLSLstd450PackUnorm4x8:
		emit_unary_func_op(result_type, id, args[0], "packUnorm4x8");
		break;
	case GLSLstd450PackSnorm2x16:
		emit_unary_func_op(result_type, id, args[0], "packSnorm2x16");
		break;
	case GLSLstd450PackUnorm2x16:
		emit_unary_func_op(result_type, id, args[0], "packUnorm2x16");
		break;
	case GLSLstd450PackHalf2x16:
		emit_unary_func_op(result_type, id, args[0], "packHalf2x16");
		break;
	case GLSLstd450UnpackSnorm4x8:
		emit_unary_func_op(result_type, id, args[0], "unpackSnorm4x8");
		break;
	case GLSLstd450UnpackUnorm4x8:
		emit_unary_func_op(result_type, id, args[0], "unpackUnorm4x8");
		break;
	case GLSLstd450UnpackSnorm2x16:
		emit_unary_func_op(result_type, id, args[0], "unpackSnorm2x16");
		break;
	case GLSLstd450UnpackUnorm2x16:
		emit_unary_func_op(result_type, id, args[0], "unpackUnorm2x16");
		break;
	case GLSLstd450UnpackHalf2x16:
		emit_unary_func_op(result_type, id, args[0], "unpackHalf2x16");
		break;

	case GLSLstd450PackDouble2x32:
		emit_unary_func_op(result_type, id, args[0], "packDouble2x32");
		break;
	case GLSLstd450UnpackDouble2x32:
		emit_unary_func_op(result_type, id, args[0], "unpackDouble2x32");
		break;

	// Vector math
	case GLSLstd450Length:
		emit_unary_func_op(result_type, id, args[0], "length");
		break;
	case GLSLstd450Distance:
		emit_binary_func_op(result_type, id, args[0], args[1], "distance");
		break;
	case GLSLstd450Cross:
		emit_binary_func_op(result_type, id, args[0], args[1], "cross");
		break;
	case GLSLstd450Normalize:
		emit_unary_func_op(result_type, id, args[0], "normalize");
		break;
	case GLSLstd450FaceForward:
		emit_trinary_func_op(result_type, id, args[0], args[1], args[2], "faceforward");
		break;
	case GLSLstd450Reflect:
		emit_binary_func_op(result_type, id, args[0], args[1], "reflect");
		break;
	case GLSLstd450Refract:
		emit_trinary_func_op(result_type, id, args[0], args[1], args[2], "refract");
		break;

	// Bit-fiddling
	case GLSLstd450FindILsb:
		emit_unary_func_op(result_type, id, args[0], "findLSB");
		break;
	case GLSLstd450FindSMsb:
	case GLSLstd450FindUMsb:
		emit_unary_func_op(result_type, id, args[0], "findMSB");
		break;

	// Multisampled varying
	case GLSLstd450InterpolateAtCentroid:
		emit_unary_func_op(result_type, id, args[0], "interpolateAtCentroid");
		break;
	case GLSLstd450InterpolateAtSample:
		emit_binary_func_op(result_type, id, args[0], args[1], "interpolateAtSample");
		break;
	case GLSLstd450InterpolateAtOffset:
		emit_binary_func_op(result_type, id, args[0], args[1], "interpolateAtOffset");
		break;

	case GLSLstd450NMin:
		emit_binary_func_op(result_type, id, args[0], args[1], "unsupported_glsl450_nmin");
		break;
	case GLSLstd450NMax:
		emit_binary_func_op(result_type, id, args[0], args[1], "unsupported_glsl450_nmax");
		break;
	case GLSLstd450NClamp:
		emit_binary_func_op(result_type, id, args[0], args[1], "unsupported_glsl450_nclamp");
		break;

	default:
		statement("// unimplemented GLSL op ", eop);
		break;
	}
}

void CompilerGLSL::emit_spv_amd_shader_ballot_op(uint32_t result_type, uint32_t id, uint32_t eop, const uint32_t *args,
                                                 uint32_t)
{
	require_extension_internal("GL_AMD_shader_ballot");

	enum AMDShaderBallot
	{
		SwizzleInvocationsAMD = 1,
		SwizzleInvocationsMaskedAMD = 2,
		WriteInvocationAMD = 3,
		MbcntAMD = 4
	};

	auto op = static_cast<AMDShaderBallot>(eop);

	switch (op)
	{
	case SwizzleInvocationsAMD:
		emit_binary_func_op(result_type, id, args[0], args[1], "swizzleInvocationsAMD");
		register_control_dependent_expression(id);
		break;

	case SwizzleInvocationsMaskedAMD:
		emit_binary_func_op(result_type, id, args[0], args[1], "swizzleInvocationsMaskedAMD");
		register_control_dependent_expression(id);
		break;

	case WriteInvocationAMD:
		emit_trinary_func_op(result_type, id, args[0], args[1], args[2], "writeInvocationAMD");
		register_control_dependent_expression(id);
		break;

	case MbcntAMD:
		emit_unary_func_op(result_type, id, args[0], "mbcntAMD");
		register_control_dependent_expression(id);
		break;

	default:
		statement("// unimplemented SPV AMD shader ballot op ", eop);
		break;
	}
}

void CompilerGLSL::emit_spv_amd_shader_explicit_vertex_parameter_op(uint32_t result_type, uint32_t id, uint32_t eop,
                                                                    const uint32_t *args, uint32_t)
{
	require_extension_internal("GL_AMD_shader_explicit_vertex_parameter");

	enum AMDShaderExplicitVertexParameter
	{
		InterpolateAtVertexAMD = 1
	};

	auto op = static_cast<AMDShaderExplicitVertexParameter>(eop);

	switch (op)
	{
	case InterpolateAtVertexAMD:
		emit_binary_func_op(result_type, id, args[0], args[1], "interpolateAtVertexAMD");
		break;

	default:
		statement("// unimplemented SPV AMD shader explicit vertex parameter op ", eop);
		break;
	}
}

void CompilerGLSL::emit_spv_amd_shader_trinary_minmax_op(uint32_t result_type, uint32_t id, uint32_t eop,
                                                         const uint32_t *args, uint32_t)
{
	require_extension_internal("GL_AMD_shader_trinary_minmax");

	enum AMDShaderTrinaryMinMax
	{
		FMin3AMD = 1,
		UMin3AMD = 2,
		SMin3AMD = 3,
		FMax3AMD = 4,
		UMax3AMD = 5,
		SMax3AMD = 6,
		FMid3AMD = 7,
		UMid3AMD = 8,
		SMid3AMD = 9
	};

	auto op = static_cast<AMDShaderTrinaryMinMax>(eop);

	switch (op)
	{
	case FMin3AMD:
	case UMin3AMD:
	case SMin3AMD:
		emit_trinary_func_op(result_type, id, args[0], args[1], args[2], "min3");
		break;

	case FMax3AMD:
	case UMax3AMD:
	case SMax3AMD:
		emit_trinary_func_op(result_type, id, args[0], args[1], args[2], "max3");
		break;

	case FMid3AMD:
	case UMid3AMD:
	case SMid3AMD:
		emit_trinary_func_op(result_type, id, args[0], args[1], args[2], "mid3");
		break;

	default:
		statement("// unimplemented SPV AMD shader trinary minmax op ", eop);
		break;
	}
}

void CompilerGLSL::emit_spv_amd_gcn_shader_op(uint32_t result_type, uint32_t id, uint32_t eop, const uint32_t *args,
                                              uint32_t)
{
	require_extension_internal("GL_AMD_gcn_shader");

	enum AMDGCNShader
	{
		CubeFaceIndexAMD = 1,
		CubeFaceCoordAMD = 2,
		TimeAMD = 3
	};

	auto op = static_cast<AMDGCNShader>(eop);

	switch (op)
	{
	case CubeFaceIndexAMD:
		emit_unary_func_op(result_type, id, args[0], "cubeFaceIndexAMD");
		break;
	case CubeFaceCoordAMD:
		emit_unary_func_op(result_type, id, args[0], "cubeFaceCoordAMD");
		break;
	case TimeAMD:
	{
		string expr = "timeAMD()";
		emit_op(result_type, id, expr, true);
		register_control_dependent_expression(id);
		break;
	}

	default:
		statement("// unimplemented SPV AMD gcn shader op ", eop);
		break;
	}
}

void CompilerGLSL::emit_subgroup_op(const Instruction &i)
{
	const uint32_t *ops = stream(i);
	auto op = static_cast<Op>(i.op);

	if (!options.vulkan_semantics)
		SPIRV_CROSS_THROW("Can only use subgroup operations in Vulkan semantics.");

	switch (op)
	{
	case OpGroupNonUniformElect:
		require_extension_internal("GL_KHR_shader_subgroup_basic");
		break;

	case OpGroupNonUniformBroadcast:
	case OpGroupNonUniformBroadcastFirst:
	case OpGroupNonUniformBallot:
	case OpGroupNonUniformInverseBallot:
	case OpGroupNonUniformBallotBitExtract:
	case OpGroupNonUniformBallotBitCount:
	case OpGroupNonUniformBallotFindLSB:
	case OpGroupNonUniformBallotFindMSB:
		require_extension_internal("GL_KHR_shader_subgroup_ballot");
		break;

	case OpGroupNonUniformShuffle:
	case OpGroupNonUniformShuffleXor:
		require_extension_internal("GL_KHR_shader_subgroup_shuffle");
		break;

	case OpGroupNonUniformShuffleUp:
	case OpGroupNonUniformShuffleDown:
		require_extension_internal("GL_KHR_shader_subgroup_shuffle_relative");
		break;

	case OpGroupNonUniformAll:
	case OpGroupNonUniformAny:
	case OpGroupNonUniformAllEqual:
		require_extension_internal("GL_KHR_shader_subgroup_vote");
		break;

	case OpGroupNonUniformFAdd:
	case OpGroupNonUniformFMul:
	case OpGroupNonUniformFMin:
	case OpGroupNonUniformFMax:
	case OpGroupNonUniformIAdd:
	case OpGroupNonUniformIMul:
	case OpGroupNonUniformSMin:
	case OpGroupNonUniformSMax:
	case OpGroupNonUniformUMin:
	case OpGroupNonUniformUMax:
	case OpGroupNonUniformBitwiseAnd:
	case OpGroupNonUniformBitwiseOr:
	case OpGroupNonUniformBitwiseXor:
	{
		auto operation = static_cast<GroupOperation>(ops[3]);
		if (operation == GroupOperationClusteredReduce)
		{
			require_extension_internal("GL_KHR_shader_subgroup_clustered");
		}
		else if (operation == GroupOperationExclusiveScan || operation == GroupOperationInclusiveScan ||
		         operation == GroupOperationReduce)
		{
			require_extension_internal("GL_KHR_shader_subgroup_arithmetic");
		}
		else
			SPIRV_CROSS_THROW("Invalid group operation.");
		break;
	}

	case OpGroupNonUniformQuadSwap:
	case OpGroupNonUniformQuadBroadcast:
		require_extension_internal("GL_KHR_shader_subgroup_quad");
		break;

	default:
		SPIRV_CROSS_THROW("Invalid opcode for subgroup.");
	}

	uint32_t result_type = ops[0];
	uint32_t id = ops[1];

	auto scope = static_cast<Scope>(get<SPIRConstant>(ops[2]).scalar());
	if (scope != ScopeSubgroup)
		SPIRV_CROSS_THROW("Only subgroup scope is supported.");

	switch (op)
	{
	case OpGroupNonUniformElect:
		emit_op(result_type, id, "subgroupElect()", true);
		break;

	case OpGroupNonUniformBroadcast:
		emit_binary_func_op(result_type, id, ops[3], ops[4], "subgroupBroadcast");
		break;

	case OpGroupNonUniformBroadcastFirst:
		emit_unary_func_op(result_type, id, ops[3], "subgroupBroadcastFirst");
		break;

	case OpGroupNonUniformBallot:
		emit_unary_func_op(result_type, id, ops[3], "subgroupBallot");
		break;

	case OpGroupNonUniformInverseBallot:
		emit_unary_func_op(result_type, id, ops[3], "subgroupInverseBallot");
		break;

	case OpGroupNonUniformBallotBitExtract:
		emit_binary_func_op(result_type, id, ops[3], ops[4], "subgroupBallotBitExtract");
		break;

	case OpGroupNonUniformBallotFindLSB:
		emit_unary_func_op(result_type, id, ops[3], "subgroupBallotFindLSB");
		break;

	case OpGroupNonUniformBallotFindMSB:
		emit_unary_func_op(result_type, id, ops[3], "subgroupBallotFindMSB");
		break;

	case OpGroupNonUniformBallotBitCount:
	{
		auto operation = static_cast<GroupOperation>(ops[3]);
		if (operation == GroupOperationReduce)
			emit_unary_func_op(result_type, id, ops[4], "subgroupBallotBitCount");
		else if (operation == GroupOperationInclusiveScan)
			emit_unary_func_op(result_type, id, ops[4], "subgroupBallotInclusiveBitCount");
		else if (operation == GroupOperationExclusiveScan)
			emit_unary_func_op(result_type, id, ops[4], "subgroupBallotExclusiveBitCount");
		else
			SPIRV_CROSS_THROW("Invalid BitCount operation.");
		break;
	}

	case OpGroupNonUniformShuffle:
		emit_binary_func_op(result_type, id, ops[3], ops[4], "subgroupShuffle");
		break;

	case OpGroupNonUniformShuffleXor:
		emit_binary_func_op(result_type, id, ops[3], ops[4], "subgroupShuffleXor");
		break;

	case OpGroupNonUniformShuffleUp:
		emit_binary_func_op(result_type, id, ops[3], ops[4], "subgroupShuffleUp");
		break;

	case OpGroupNonUniformShuffleDown:
		emit_binary_func_op(result_type, id, ops[3], ops[4], "subgroupShuffleDown");
		break;

	case OpGroupNonUniformAll:
		emit_unary_func_op(result_type, id, ops[3], "subgroupAll");
		break;

	case OpGroupNonUniformAny:
		emit_unary_func_op(result_type, id, ops[3], "subgroupAny");
		break;

	case OpGroupNonUniformAllEqual:
		emit_unary_func_op(result_type, id, ops[3], "subgroupAllEqual");
		break;

		// clang-format off
#define GLSL_GROUP_OP(op, glsl_op) \
case OpGroupNonUniform##op: \
	{ \
		auto operation = static_cast<GroupOperation>(ops[3]); \
		if (operation == GroupOperationReduce) \
			emit_unary_func_op(result_type, id, ops[4], "subgroup" #glsl_op); \
		else if (operation == GroupOperationInclusiveScan) \
			emit_unary_func_op(result_type, id, ops[4], "subgroupInclusive" #glsl_op); \
		else if (operation == GroupOperationExclusiveScan) \
			emit_unary_func_op(result_type, id, ops[4], "subgroupExclusive" #glsl_op); \
		else if (operation == GroupOperationClusteredReduce) \
			emit_binary_func_op(result_type, id, ops[4], ops[5], "subgroupClustered" #glsl_op); \
		else \
			SPIRV_CROSS_THROW("Invalid group operation."); \
		break; \
	}
	GLSL_GROUP_OP(FAdd, Add)
	GLSL_GROUP_OP(FMul, Mul)
	GLSL_GROUP_OP(FMin, Min)
	GLSL_GROUP_OP(FMax, Max)
	GLSL_GROUP_OP(IAdd, Add)
	GLSL_GROUP_OP(IMul, Mul)
	GLSL_GROUP_OP(SMin, Min)
	GLSL_GROUP_OP(SMax, Max)
	GLSL_GROUP_OP(UMin, Min)
	GLSL_GROUP_OP(UMax, Max)
	GLSL_GROUP_OP(BitwiseAnd, And)
	GLSL_GROUP_OP(BitwiseOr, Or)
	GLSL_GROUP_OP(BitwiseXor, Xor)
#undef GLSL_GROUP_OP
		// clang-format on

	case OpGroupNonUniformQuadSwap:
	{
		uint32_t direction = get<SPIRConstant>(ops[4]).scalar();
		if (direction == 0)
			emit_unary_func_op(result_type, id, ops[3], "subgroupQuadSwapHorizontal");
		else if (direction == 1)
			emit_unary_func_op(result_type, id, ops[3], "subgroupQuadSwapVertical");
		else if (direction == 2)
			emit_unary_func_op(result_type, id, ops[3], "subgroupQuadSwapDiagonal");
		else
			SPIRV_CROSS_THROW("Invalid quad swap direction.");
		break;
	}

	case OpGroupNonUniformQuadBroadcast:
	{
		emit_binary_func_op(result_type, id, ops[3], ops[4], "subgroupQuadBroadcast");
		break;
	}

	default:
		SPIRV_CROSS_THROW("Invalid opcode for subgroup.");
	}

	register_control_dependent_expression(id);
}

string CompilerGLSL::bitcast_glsl_op(const SPIRType &out_type, const SPIRType &in_type)
{
	if (out_type.basetype == SPIRType::UInt && in_type.basetype == SPIRType::Int)
		return type_to_glsl(out_type);
	else if (out_type.basetype == SPIRType::UInt64 && in_type.basetype == SPIRType::Int64)
		return type_to_glsl(out_type);
	else if (out_type.basetype == SPIRType::UInt && in_type.basetype == SPIRType::Float)
		return "floatBitsToUint";
	else if (out_type.basetype == SPIRType::Int && in_type.basetype == SPIRType::UInt)
		return type_to_glsl(out_type);
	else if (out_type.basetype == SPIRType::Int64 && in_type.basetype == SPIRType::UInt64)
		return type_to_glsl(out_type);
	else if (out_type.basetype == SPIRType::Int && in_type.basetype == SPIRType::Float)
		return "floatBitsToInt";
	else if (out_type.basetype == SPIRType::Float && in_type.basetype == SPIRType::UInt)
		return "uintBitsToFloat";
	else if (out_type.basetype == SPIRType::Float && in_type.basetype == SPIRType::Int)
		return "intBitsToFloat";
	else if (out_type.basetype == SPIRType::Int64 && in_type.basetype == SPIRType::Double)
		return "doubleBitsToInt64";
	else if (out_type.basetype == SPIRType::UInt64 && in_type.basetype == SPIRType::Double)
		return "doubleBitsToUint64";
	else if (out_type.basetype == SPIRType::Double && in_type.basetype == SPIRType::Int64)
		return "int64BitsToDouble";
	else if (out_type.basetype == SPIRType::Double && in_type.basetype == SPIRType::UInt64)
		return "uint64BitsToDouble";
	else if (out_type.basetype == SPIRType::UInt64 && in_type.basetype == SPIRType::UInt && in_type.vecsize == 2)
		return "packUint2x32";
	else if (out_type.basetype == SPIRType::Half && in_type.basetype == SPIRType::UInt && in_type.vecsize == 1)
		return "unpackFloat2x16";
	else if (out_type.basetype == SPIRType::UInt && in_type.basetype == SPIRType::Half && in_type.vecsize == 2)
		return "packFloat2x16";
	else
		return "";
}

string CompilerGLSL::bitcast_glsl(const SPIRType &result_type, uint32_t argument)
{
	auto op = bitcast_glsl_op(result_type, expression_type(argument));
	if (op.empty())
		return to_enclosed_expression(argument);
	else
		return join(op, "(", to_expression(argument), ")");
}

std::string CompilerGLSL::bitcast_expression(SPIRType::BaseType target_type, uint32_t arg)
{
	auto expr = to_expression(arg);
	auto &src_type = expression_type(arg);
	if (src_type.basetype != target_type)
	{
		auto target = src_type;
		target.basetype = target_type;
		expr = join(bitcast_glsl_op(target, src_type), "(", expr, ")");
	}

	return expr;
}

std::string CompilerGLSL::bitcast_expression(const SPIRType &target_type, SPIRType::BaseType expr_type,
                                             const std::string &expr)
{
	if (target_type.basetype == expr_type)
		return expr;

	auto src_type = target_type;
	src_type.basetype = expr_type;
	return join(bitcast_glsl_op(target_type, src_type), "(", expr, ")");
}

string CompilerGLSL::builtin_to_glsl(BuiltIn builtin, StorageClass storage)
{
	switch (builtin)
	{
	case BuiltInPosition:
		return "gl_Position";
	case BuiltInPointSize:
		return "gl_PointSize";
	case BuiltInClipDistance:
		return "gl_ClipDistance";
	case BuiltInCullDistance:
		return "gl_CullDistance";
	case BuiltInVertexId:
		if (options.vulkan_semantics)
			SPIRV_CROSS_THROW(
			    "Cannot implement gl_VertexID in Vulkan GLSL. This shader was created with GL semantics.");
		return "gl_VertexID";
	case BuiltInInstanceId:
		if (options.vulkan_semantics)
			SPIRV_CROSS_THROW(
			    "Cannot implement gl_InstanceID in Vulkan GLSL. This shader was created with GL semantics.");
		return "gl_InstanceID";
	case BuiltInVertexIndex:
		if (options.vulkan_semantics)
			return "gl_VertexIndex";
		else
			return "gl_VertexID"; // gl_VertexID already has the base offset applied.
	case BuiltInInstanceIndex:
		if (options.vulkan_semantics)
			return "gl_InstanceIndex";
		else if (options.vertex.support_nonzero_base_instance)
			return "(gl_InstanceID + SPIRV_Cross_BaseInstance)"; // ... but not gl_InstanceID.
		else
			return "gl_InstanceID";
	case BuiltInPrimitiveId:
		if (storage == StorageClassInput && get_entry_point().model == ExecutionModelGeometry)
			return "gl_PrimitiveIDIn";
		else
			return "gl_PrimitiveID";
	case BuiltInInvocationId:
		return "gl_InvocationID";
	case BuiltInLayer:
		return "gl_Layer";
	case BuiltInViewportIndex:
		return "gl_ViewportIndex";
	case BuiltInTessLevelOuter:
		return "gl_TessLevelOuter";
	case BuiltInTessLevelInner:
		return "gl_TessLevelInner";
	case BuiltInTessCoord:
		return "gl_TessCoord";
	case BuiltInFragCoord:
		return "gl_FragCoord";
	case BuiltInPointCoord:
		return "gl_PointCoord";
	case BuiltInFrontFacing:
		return "gl_FrontFacing";
	case BuiltInFragDepth:
		return "gl_FragDepth";
	case BuiltInNumWorkgroups:
		return "gl_NumWorkGroups";
	case BuiltInWorkgroupSize:
		return "gl_WorkGroupSize";
	case BuiltInWorkgroupId:
		return "gl_WorkGroupID";
	case BuiltInLocalInvocationId:
		return "gl_LocalInvocationID";
	case BuiltInGlobalInvocationId:
		return "gl_GlobalInvocationID";
	case BuiltInLocalInvocationIndex:
		return "gl_LocalInvocationIndex";

	case BuiltInSampleId:
		if (options.es && options.version < 320)
			require_extension_internal("GL_OES_sample_variables");
		if (!options.es && options.version < 400)
			SPIRV_CROSS_THROW("gl_SampleID not supported before GLSL 400.");
		return "gl_SampleID";

	case BuiltInSampleMask:
		if (options.es && options.version < 320)
			require_extension_internal("GL_OES_sample_variables");
		if (!options.es && options.version < 400)
			SPIRV_CROSS_THROW("gl_SampleMask/gl_SampleMaskIn not supported before GLSL 400.");

		if (storage == StorageClassInput)
			return "gl_SampleMaskIn";
		else
			return "gl_SampleMask";

	case BuiltInSamplePosition:
		if (options.es && options.version < 320)
			require_extension_internal("GL_OES_sample_variables");
		if (!options.es && options.version < 400)
			SPIRV_CROSS_THROW("gl_SamplePosition not supported before GLSL 400.");
		return "gl_SamplePosition";

	case BuiltInViewIndex:
		if (options.vulkan_semantics)
		{
			require_extension_internal("GL_EXT_multiview");
			return "gl_ViewIndex";
		}
		else
		{
			require_extension_internal("GL_OVR_multiview2");
			return "gl_ViewID_OVR";
		}

	case BuiltInNumSubgroups:
		if (!options.vulkan_semantics)
			SPIRV_CROSS_THROW("Need Vulkan semantics for subgroup.");
		require_extension_internal("GL_KHR_shader_subgroup_basic");
		return "gl_NumSubgroups";

	case BuiltInSubgroupId:
		if (!options.vulkan_semantics)
			SPIRV_CROSS_THROW("Need Vulkan semantics for subgroup.");
		require_extension_internal("GL_KHR_shader_subgroup_basic");
		return "gl_SubgroupID";

	case BuiltInSubgroupSize:
		if (!options.vulkan_semantics)
			SPIRV_CROSS_THROW("Need Vulkan semantics for subgroup.");
		require_extension_internal("GL_KHR_shader_subgroup_basic");
		return "gl_SubgroupSize";

	case BuiltInSubgroupLocalInvocationId:
		if (!options.vulkan_semantics)
			SPIRV_CROSS_THROW("Need Vulkan semantics for subgroup.");
		require_extension_internal("GL_KHR_shader_subgroup_basic");
		return "gl_SubgroupInvocationID";

	case BuiltInSubgroupEqMask:
		if (!options.vulkan_semantics)
			SPIRV_CROSS_THROW("Need Vulkan semantics for subgroup.");
		require_extension_internal("GL_KHR_shader_subgroup_ballot");
		return "gl_SubgroupEqMask";

	case BuiltInSubgroupGeMask:
		if (!options.vulkan_semantics)
			SPIRV_CROSS_THROW("Need Vulkan semantics for subgroup.");
		require_extension_internal("GL_KHR_shader_subgroup_ballot");
		return "gl_SubgroupGeMask";

	case BuiltInSubgroupGtMask:
		if (!options.vulkan_semantics)
			SPIRV_CROSS_THROW("Need Vulkan semantics for subgroup.");
		require_extension_internal("GL_KHR_shader_subgroup_ballot");
		return "gl_SubgroupGtMask";

	case BuiltInSubgroupLeMask:
		if (!options.vulkan_semantics)
			SPIRV_CROSS_THROW("Need Vulkan semantics for subgroup.");
		require_extension_internal("GL_KHR_shader_subgroup_ballot");
		return "gl_SubgroupLeMask";

	case BuiltInSubgroupLtMask:
		if (!options.vulkan_semantics)
			SPIRV_CROSS_THROW("Need Vulkan semantics for subgroup.");
		require_extension_internal("GL_KHR_shader_subgroup_ballot");
		return "gl_SubgroupLtMask";

	default:
		return join("gl_BuiltIn_", convert_to_string(builtin));
	}
}

const char *CompilerGLSL::index_to_swizzle(uint32_t index)
{
	switch (index)
	{
	case 0:
		return "x";
	case 1:
		return "y";
	case 2:
		return "z";
	case 3:
		return "w";
	default:
		SPIRV_CROSS_THROW("Swizzle index out of range");
	}
}

string CompilerGLSL::access_chain_internal(uint32_t base, const uint32_t *indices, uint32_t count,
                                           bool index_is_literal, bool chain_only, bool *need_transpose,
                                           bool *result_is_packed)
{
	string expr;
	if (!chain_only)
		expr = to_enclosed_expression(base);

	// Start traversing type hierarchy at the proper non-pointer types,
	// but keep type_id referencing the original pointer for use below.
	uint32_t type_id = expression_type_id(base);
	const auto *type = &get_non_pointer_type(type_id);

	bool access_chain_is_arrayed = expr.find_first_of('[') != string::npos;
	bool row_major_matrix_needs_conversion = is_non_native_row_major_matrix(base);
	bool is_packed = has_decoration(base, DecorationCPacked);
	bool pending_array_enclose = false;
	bool dimension_flatten = false;

	for (uint32_t i = 0; i < count; i++)
	{
		uint32_t index = indices[i];

		// Arrays
		if (!type->array.empty())
		{
			// If we are flattening multidimensional arrays, only create opening bracket on first
			// array index.
			if (options.flatten_multidimensional_arrays && !pending_array_enclose)
			{
				dimension_flatten = type->array.size() > 1;
				pending_array_enclose = dimension_flatten;
				if (pending_array_enclose)
					expr += "[";
			}

			assert(type->parent_type);

			const auto append_index = [&]() {
				expr += "[";
				if (index_is_literal)
					expr += convert_to_string(index);
				else
					expr += to_expression(index);
				expr += "]";
			};

			auto *var = maybe_get<SPIRVariable>(base);
			if (backend.force_gl_in_out_block && i == 0 && var && is_builtin_variable(*var) &&
			    !has_decoration(type->self, DecorationBlock))
			{
				// This deals with scenarios for tesc/geom where arrays of gl_Position[] are declared.
				// Normally, these variables live in blocks when compiled from GLSL,
				// but HLSL seems to just emit straight arrays here.
				// We must pretend this access goes through gl_in/gl_out arrays
				// to be able to access certain builtins as arrays.
				auto builtin = meta[base].decoration.builtin_type;
				switch (builtin)
				{
				// case BuiltInCullDistance: // These are already arrays, need to figure out rules for these in tess/geom.
				// case BuiltInClipDistance:
				case BuiltInPosition:
				case BuiltInPointSize:
					if (var->storage == StorageClassInput)
						expr = join("gl_in[", to_expression(index), "].", expr);
					else if (var->storage == StorageClassOutput)
						expr = join("gl_out[", to_expression(index), "].", expr);
					else
						append_index();
					break;

				default:
					append_index();
					break;
				}
			}
			else if (options.flatten_multidimensional_arrays && dimension_flatten)
			{
				// If we are flattening multidimensional arrays, do manual stride computation.
				auto &parent_type = get<SPIRType>(type->parent_type);

				if (index_is_literal)
					expr += convert_to_string(index);
				else
					expr += to_enclosed_expression(index);

				for (auto j = uint32_t(parent_type.array.size()); j; j--)
				{
					expr += " * ";
					expr += enclose_expression(to_array_size(parent_type, j - 1));
				}

				if (parent_type.array.empty())
					pending_array_enclose = false;
				else
					expr += " + ";

				if (!pending_array_enclose)
					expr += "]";
			}
			else
			{
				append_index();
			}

			type_id = type->parent_type;
			type = &get<SPIRType>(type_id);

			access_chain_is_arrayed = true;
		}
		// For structs, the index refers to a constant, which indexes into the members.
		// We also check if this member is a builtin, since we then replace the entire expression with the builtin one.
		else if (type->basetype == SPIRType::Struct)
		{
			if (!index_is_literal)
				index = get<SPIRConstant>(index).scalar();

			if (index >= type->member_types.size())
				SPIRV_CROSS_THROW("Member index is out of bounds!");

			BuiltIn builtin;
			if (is_member_builtin(*type, index, &builtin))
			{
				// FIXME: We rely here on OpName on gl_in/gl_out to make this work properly.
				// To make this properly work by omitting all OpName opcodes,
				// we need to infer gl_in or gl_out based on the builtin, and stage.
				if (access_chain_is_arrayed)
				{
					expr += ".";
					expr += builtin_to_glsl(builtin, type->storage);
				}
				else
					expr = builtin_to_glsl(builtin, type->storage);
			}
			else
			{
				// If the member has a qualified name, use it as the entire chain
				string qual_mbr_name = get_member_qualified_name(type_id, index);
				if (!qual_mbr_name.empty())
					expr = qual_mbr_name;
				else
				{
					expr += ".";
					expr += to_member_name(*type, index);
				}
			}

			is_packed = member_is_packed_type(*type, index);
			row_major_matrix_needs_conversion = member_is_non_native_row_major_matrix(*type, index);
			type = &get<SPIRType>(type->member_types[index]);
		}
		// Matrix -> Vector
		else if (type->columns > 1)
		{
			if (row_major_matrix_needs_conversion)
			{
				expr = convert_row_major_matrix(expr, *type, is_packed);
				row_major_matrix_needs_conversion = false;
				is_packed = false;
			}

			expr += "[";
			if (index_is_literal)
				expr += convert_to_string(index);
			else
				expr += to_expression(index);
			expr += "]";

			type_id = type->parent_type;
			type = &get<SPIRType>(type_id);
		}
		// Vector -> Scalar
		else if (type->vecsize > 1)
		{
			if (index_is_literal && !is_packed)
			{
				expr += ".";
				expr += index_to_swizzle(index);
			}
			else if (ids[index].get_type() == TypeConstant && !is_packed)
			{
				auto &c = get<SPIRConstant>(index);
				expr += ".";
				expr += index_to_swizzle(c.scalar());
			}
			else if (index_is_literal)
			{
				// For packed vectors, we can only access them as an array, not by swizzle.
				expr += join("[", index, "]");
			}
			else
			{
				expr += "[";
				expr += to_expression(index);
				expr += "]";
			}

			is_packed = false;
			type_id = type->parent_type;
			type = &get<SPIRType>(type_id);
		}
		else if (!backend.allow_truncated_access_chain)
			SPIRV_CROSS_THROW("Cannot subdivide a scalar value!");
	}

	if (pending_array_enclose)
	{
		SPIRV_CROSS_THROW("Flattening of multidimensional arrays were enabled, "
		                  "but the access chain was terminated in the middle of a multidimensional array. "
		                  "This is not supported.");
	}

	if (need_transpose)
		*need_transpose = row_major_matrix_needs_conversion;

	if (result_is_packed)
		*result_is_packed = is_packed;

	return expr;
}

string CompilerGLSL::to_flattened_struct_member(const SPIRVariable &var, uint32_t index)
{
	auto &type = get<SPIRType>(var.basetype);
	return sanitize_underscores(join(to_name(var.self), "_", to_member_name(type, index)));
}

string CompilerGLSL::access_chain(uint32_t base, const uint32_t *indices, uint32_t count, const SPIRType &target_type,
                                  bool *out_need_transpose, bool *result_is_packed)
{
	if (flattened_buffer_blocks.count(base))
	{
		uint32_t matrix_stride = 0;
		bool need_transpose = false;
		flattened_access_chain_offset(expression_type(base), indices, count, 0, 16, &need_transpose, &matrix_stride);

		if (out_need_transpose)
			*out_need_transpose = target_type.columns > 1 && need_transpose;
		if (result_is_packed)
			*result_is_packed = false;

		return flattened_access_chain(base, indices, count, target_type, 0, matrix_stride, need_transpose);
	}
	else if (flattened_structs.count(base) && count > 0)
	{
		auto chain = access_chain_internal(base, indices, count, false, true).substr(1);
		if (out_need_transpose)
			*out_need_transpose = false;
		if (result_is_packed)
			*result_is_packed = false;
		return sanitize_underscores(join(to_name(base), "_", chain));
	}
	else
	{
		return access_chain_internal(base, indices, count, false, false, out_need_transpose, result_is_packed);
	}
}

string CompilerGLSL::load_flattened_struct(SPIRVariable &var)
{
	auto expr = type_to_glsl_constructor(get<SPIRType>(var.basetype));
	expr += '(';

	auto &type = get<SPIRType>(var.basetype);
	for (uint32_t i = 0; i < uint32_t(type.member_types.size()); i++)
	{
		if (i)
			expr += ", ";

		// Flatten the varyings.
		// Apply name transformation for flattened I/O blocks.
		expr += to_flattened_struct_member(var, i);
	}
	expr += ')';
	return expr;
}

void CompilerGLSL::store_flattened_struct(SPIRVariable &var, uint32_t value)
{
	// We're trying to store a structure which has been flattened.
	// Need to copy members one by one.
	auto rhs = to_expression(value);

	// Store result locally.
	// Since we're declaring a variable potentially multiple times here,
	// store the variable in an isolated scope.
	begin_scope();
	statement(variable_decl_function_local(var), " = ", rhs, ";");

	auto &type = get<SPIRType>(var.basetype);
	for (uint32_t i = 0; i < uint32_t(type.member_types.size()); i++)
	{
		// Flatten the varyings.
		// Apply name transformation for flattened I/O blocks.

		auto lhs = sanitize_underscores(join(to_name(var.self), "_", to_member_name(type, i)));
		rhs = join(to_name(var.self), ".", to_member_name(type, i));
		statement(lhs, " = ", rhs, ";");
	}
	end_scope();
}

std::string CompilerGLSL::flattened_access_chain(uint32_t base, const uint32_t *indices, uint32_t count,
                                                 const SPIRType &target_type, uint32_t offset, uint32_t matrix_stride,
                                                 bool need_transpose)
{
	if (!target_type.array.empty())
		SPIRV_CROSS_THROW("Access chains that result in an array can not be flattened");
	else if (target_type.basetype == SPIRType::Struct)
		return flattened_access_chain_struct(base, indices, count, target_type, offset);
	else if (target_type.columns > 1)
		return flattened_access_chain_matrix(base, indices, count, target_type, offset, matrix_stride, need_transpose);
	else
		return flattened_access_chain_vector(base, indices, count, target_type, offset, matrix_stride, need_transpose);
}

std::string CompilerGLSL::flattened_access_chain_struct(uint32_t base, const uint32_t *indices, uint32_t count,
                                                        const SPIRType &target_type, uint32_t offset)
{
	std::string expr;

	expr += type_to_glsl_constructor(target_type);
	expr += "(";

	for (uint32_t i = 0; i < uint32_t(target_type.member_types.size()); ++i)
	{
		if (i != 0)
			expr += ", ";

		const SPIRType &member_type = get<SPIRType>(target_type.member_types[i]);
		uint32_t member_offset = type_struct_member_offset(target_type, i);

		// The access chain terminates at the struct, so we need to find matrix strides and row-major information
		// ahead of time.
		bool need_transpose = false;
		uint32_t matrix_stride = 0;
		if (member_type.columns > 1)
		{
			need_transpose = combined_decoration_for_member(target_type, i).get(DecorationRowMajor);
			matrix_stride = type_struct_member_matrix_stride(target_type, i);
		}

		auto tmp = flattened_access_chain(base, indices, count, member_type, offset + member_offset, matrix_stride,
		                                  need_transpose);

		// Cannot forward transpositions, so resolve them here.
		if (need_transpose)
			expr += convert_row_major_matrix(tmp, member_type, false);
		else
			expr += tmp;
	}

	expr += ")";

	return expr;
}

std::string CompilerGLSL::flattened_access_chain_matrix(uint32_t base, const uint32_t *indices, uint32_t count,
                                                        const SPIRType &target_type, uint32_t offset,
                                                        uint32_t matrix_stride, bool need_transpose)
{
	assert(matrix_stride);
	SPIRType tmp_type = target_type;
	if (need_transpose)
		swap(tmp_type.vecsize, tmp_type.columns);

	std::string expr;

	expr += type_to_glsl_constructor(tmp_type);
	expr += "(";

	for (uint32_t i = 0; i < tmp_type.columns; i++)
	{
		if (i != 0)
			expr += ", ";

		expr += flattened_access_chain_vector(base, indices, count, tmp_type, offset + i * matrix_stride, matrix_stride,
		                                      /* need_transpose= */ false);
	}

	expr += ")";

	return expr;
}

std::string CompilerGLSL::flattened_access_chain_vector(uint32_t base, const uint32_t *indices, uint32_t count,
                                                        const SPIRType &target_type, uint32_t offset,
                                                        uint32_t matrix_stride, bool need_transpose)
{
	auto result = flattened_access_chain_offset(expression_type(base), indices, count, offset, 16);

	auto buffer_name = to_name(expression_type(base).self);

	if (need_transpose)
	{
		std::string expr;

		if (target_type.vecsize > 1)
		{
			expr += type_to_glsl_constructor(target_type);
			expr += "(";
		}

		for (uint32_t i = 0; i < target_type.vecsize; ++i)
		{
			if (i != 0)
				expr += ", ";

			uint32_t component_offset = result.second + i * matrix_stride;

			assert(component_offset % (target_type.width / 8) == 0);
			uint32_t index = component_offset / (target_type.width / 8);

			expr += buffer_name;
			expr += "[";
			expr += result.first; // this is a series of N1 * k1 + N2 * k2 + ... that is either empty or ends with a +
			expr += convert_to_string(index / 4);
			expr += "]";

			expr += vector_swizzle(1, index % 4);
		}

		if (target_type.vecsize > 1)
		{
			expr += ")";
		}

		return expr;
	}
	else
	{
		assert(result.second % (target_type.width / 8) == 0);
		uint32_t index = result.second / (target_type.width / 8);

		std::string expr;

		expr += buffer_name;
		expr += "[";
		expr += result.first; // this is a series of N1 * k1 + N2 * k2 + ... that is either empty or ends with a +
		expr += convert_to_string(index / 4);
		expr += "]";

		expr += vector_swizzle(target_type.vecsize, index % 4);

		return expr;
	}
}

std::pair<std::string, uint32_t> CompilerGLSL::flattened_access_chain_offset(const SPIRType &basetype,
                                                                             const uint32_t *indices, uint32_t count,
                                                                             uint32_t offset, uint32_t word_stride,
                                                                             bool *need_transpose,
                                                                             uint32_t *out_matrix_stride)
{
	// Start traversing type hierarchy at the proper non-pointer types.
	const auto *type = &get_non_pointer_type(basetype);

	// This holds the type of the current pointer which we are traversing through.
	// We always start out from a struct type which is the block.
	// This is primarily used to reflect the array strides and matrix strides later.
	// For the first access chain index, type_id won't be needed, so just keep it as 0, it will be set
	// accordingly as members of structs are accessed.
	assert(type->basetype == SPIRType::Struct);
	uint32_t type_id = 0;

	std::string expr;

	// Inherit matrix information in case we are access chaining a vector which might have come from a row major layout.
	bool row_major_matrix_needs_conversion = need_transpose ? *need_transpose : false;
	uint32_t matrix_stride = out_matrix_stride ? *out_matrix_stride : 0;

	for (uint32_t i = 0; i < count; i++)
	{
		uint32_t index = indices[i];

		// Arrays
		if (!type->array.empty())
		{
			// Here, the type_id will be a type ID for the array type itself.
			uint32_t array_stride = get_decoration(type_id, DecorationArrayStride);
			if (!array_stride)
				SPIRV_CROSS_THROW("SPIR-V does not define ArrayStride for buffer block.");

			auto *constant = maybe_get<SPIRConstant>(index);
			if (constant)
			{
				// Constant array access.
				offset += constant->scalar() * array_stride;
			}
			else
			{
				// Dynamic array access.
				if (array_stride % word_stride)
				{
					SPIRV_CROSS_THROW(
					    "Array stride for dynamic indexing must be divisible by the size of a 4-component vector. "
					    "Likely culprit here is a float or vec2 array inside a push constant block which is std430. "
					    "This cannot be flattened. Try using std140 layout instead.");
				}

				expr += to_enclosed_expression(index);
				expr += " * ";
				expr += convert_to_string(array_stride / word_stride);
				expr += " + ";
			}

			uint32_t parent_type = type->parent_type;
			type = &get<SPIRType>(parent_type);
			type_id = parent_type;

			// Type ID now refers to the array type with one less dimension.
		}
		// For structs, the index refers to a constant, which indexes into the members.
		// We also check if this member is a builtin, since we then replace the entire expression with the builtin one.
		else if (type->basetype == SPIRType::Struct)
		{
			index = get<SPIRConstant>(index).scalar();

			if (index >= type->member_types.size())
				SPIRV_CROSS_THROW("Member index is out of bounds!");

			offset += type_struct_member_offset(*type, index);
			type_id = type->member_types[index];

			auto &struct_type = *type;
			type = &get<SPIRType>(type->member_types[index]);

			if (type->columns > 1)
			{
				matrix_stride = type_struct_member_matrix_stride(struct_type, index);
				row_major_matrix_needs_conversion =
				    combined_decoration_for_member(struct_type, index).get(DecorationRowMajor);
			}
			else
				row_major_matrix_needs_conversion = false;
		}
		// Matrix -> Vector
		else if (type->columns > 1)
		{
			auto *constant = maybe_get<SPIRConstant>(index);
			if (constant)
			{
				index = get<SPIRConstant>(index).scalar();
				offset += index * (row_major_matrix_needs_conversion ? (type->width / 8) : matrix_stride);
			}
			else
			{
				uint32_t indexing_stride = row_major_matrix_needs_conversion ? (type->width / 8) : matrix_stride;
				// Dynamic array access.
				if (indexing_stride % word_stride)
				{
					SPIRV_CROSS_THROW(
					    "Matrix stride for dynamic indexing must be divisible by the size of a 4-component vector. "
					    "Likely culprit here is a row-major matrix being accessed dynamically. "
					    "This cannot be flattened. Try using std140 layout instead.");
				}

				expr += to_enclosed_expression(index);
				expr += " * ";
				expr += convert_to_string(indexing_stride / word_stride);
				expr += " + ";
			}

			uint32_t parent_type = type->parent_type;
			type = &get<SPIRType>(type->parent_type);
			type_id = parent_type;
		}
		// Vector -> Scalar
		else if (type->vecsize > 1)
		{
			auto *constant = maybe_get<SPIRConstant>(index);
			if (constant)
			{
				index = get<SPIRConstant>(index).scalar();
				offset += index * (row_major_matrix_needs_conversion ? matrix_stride : (type->width / 8));
			}
			else
			{
				uint32_t indexing_stride = row_major_matrix_needs_conversion ? matrix_stride : (type->width / 8);

				// Dynamic array access.
				if (indexing_stride % word_stride)
				{
					SPIRV_CROSS_THROW(
					    "Stride for dynamic vector indexing must be divisible by the size of a 4-component vector. "
					    "This cannot be flattened in legacy targets.");
				}

				expr += to_enclosed_expression(index);
				expr += " * ";
				expr += convert_to_string(indexing_stride / word_stride);
				expr += " + ";
			}

			uint32_t parent_type = type->parent_type;
			type = &get<SPIRType>(type->parent_type);
			type_id = parent_type;
		}
		else
			SPIRV_CROSS_THROW("Cannot subdivide a scalar value!");
	}

	if (need_transpose)
		*need_transpose = row_major_matrix_needs_conversion;
	if (out_matrix_stride)
		*out_matrix_stride = matrix_stride;

	return std::make_pair(expr, offset);
}

bool CompilerGLSL::should_forward(uint32_t id)
{
	// Immutable expression can always be forwarded.
	// If not immutable, we can speculate about it by forwarding potentially mutable variables.
	auto *var = maybe_get<SPIRVariable>(id);
	bool forward = var ? var->forwardable : false;
	return (is_immutable(id) || forward) && !options.force_temporary;
}

void CompilerGLSL::track_expression_read(uint32_t id)
{
	// If we try to read a forwarded temporary more than once we will stamp out possibly complex code twice.
	// In this case, it's better to just bind the complex expression to the temporary and read that temporary twice.
	if (expression_is_forwarded(id))
	{
		auto &v = expression_usage_counts[id];
		v++;

		if (v >= 2)
		{
			//if (v == 2)
			//    fprintf(stderr, "ID %u was forced to temporary due to more than 1 expression use!\n", id);

			forced_temporaries.insert(id);
			// Force a recompile after this pass to avoid forwarding this variable.
			force_recompile = true;
		}
	}
}

bool CompilerGLSL::args_will_forward(uint32_t id, const uint32_t *args, uint32_t num_args, bool pure)
{
	if (forced_temporaries.find(id) != end(forced_temporaries))
		return false;

	for (uint32_t i = 0; i < num_args; i++)
		if (!should_forward(args[i]))
			return false;

	// We need to forward globals as well.
	if (!pure)
	{
		for (auto global : global_variables)
			if (!should_forward(global))
				return false;
		for (auto aliased : aliased_variables)
			if (!should_forward(aliased))
				return false;
	}

	return true;
}

void CompilerGLSL::register_impure_function_call()
{
	// Impure functions can modify globals and aliased variables, so invalidate them as well.
	for (auto global : global_variables)
		flush_dependees(get<SPIRVariable>(global));
	for (auto aliased : aliased_variables)
		flush_dependees(get<SPIRVariable>(aliased));
}

void CompilerGLSL::register_call_out_argument(uint32_t id)
{
	register_write(id);

	auto *var = maybe_get<SPIRVariable>(id);
	if (var)
		flush_variable_declaration(var->self);
}

string CompilerGLSL::variable_decl_function_local(SPIRVariable &var)
{
	// These variables are always function local,
	// so make sure we emit the variable without storage qualifiers.
	// Some backends will inject custom variables locally in a function
	// with a storage qualifier which is not function-local.
	auto old_storage = var.storage;
	var.storage = StorageClassFunction;
	auto expr = variable_decl(var);
	var.storage = old_storage;
	return expr;
}

void CompilerGLSL::flush_variable_declaration(uint32_t id)
{
	auto *var = maybe_get<SPIRVariable>(id);
	if (var && var->deferred_declaration)
	{
		statement(variable_decl_function_local(*var), ";");
		var->deferred_declaration = false;
	}
}

bool CompilerGLSL::remove_duplicate_swizzle(string &op)
{
	auto pos = op.find_last_of('.');
	if (pos == string::npos || pos == 0)
		return false;

	string final_swiz = op.substr(pos + 1, string::npos);

	if (backend.swizzle_is_function)
	{
		if (final_swiz.size() < 2)
			return false;

		if (final_swiz.substr(final_swiz.size() - 2, string::npos) == "()")
			final_swiz.erase(final_swiz.size() - 2, string::npos);
		else
			return false;
	}

	// Check if final swizzle is of form .x, .xy, .xyz, .xyzw or similar.
	// If so, and previous swizzle is of same length,
	// we can drop the final swizzle altogether.
	for (uint32_t i = 0; i < final_swiz.size(); i++)
	{
		static const char expected[] = { 'x', 'y', 'z', 'w' };
		if (i >= 4 || final_swiz[i] != expected[i])
			return false;
	}

	auto prevpos = op.find_last_of('.', pos - 1);
	if (prevpos == string::npos)
		return false;

	prevpos++;

	// Make sure there are only swizzles here ...
	for (auto i = prevpos; i < pos; i++)
	{
		if (op[i] < 'w' || op[i] > 'z')
		{
			// If swizzles are foo.xyz() like in C++ backend for example, check for that.
			if (backend.swizzle_is_function && i + 2 == pos && op[i] == '(' && op[i + 1] == ')')
				break;
			return false;
		}
	}

	// If original swizzle is large enough, just carve out the components we need.
	// E.g. foobar.wyx.xy will turn into foobar.wy.
	if (pos - prevpos >= final_swiz.size())
	{
		op.erase(prevpos + final_swiz.size(), string::npos);

		// Add back the function call ...
		if (backend.swizzle_is_function)
			op += "()";
	}
	return true;
}

// Optimizes away vector swizzles where we have something like
// vec3 foo;
// foo.xyz <-- swizzle expression does nothing.
// This is a very common pattern after OpCompositeCombine.
bool CompilerGLSL::remove_unity_swizzle(uint32_t base, string &op)
{
	auto pos = op.find_last_of('.');
	if (pos == string::npos || pos == 0)
		return false;

	string final_swiz = op.substr(pos + 1, string::npos);

	if (backend.swizzle_is_function)
	{
		if (final_swiz.size() < 2)
			return false;

		if (final_swiz.substr(final_swiz.size() - 2, string::npos) == "()")
			final_swiz.erase(final_swiz.size() - 2, string::npos);
		else
			return false;
	}

	// Check if final swizzle is of form .x, .xy, .xyz, .xyzw or similar.
	// If so, and previous swizzle is of same length,
	// we can drop the final swizzle altogether.
	for (uint32_t i = 0; i < final_swiz.size(); i++)
	{
		static const char expected[] = { 'x', 'y', 'z', 'w' };
		if (i >= 4 || final_swiz[i] != expected[i])
			return false;
	}

	auto &type = expression_type(base);

	// Sanity checking ...
	assert(type.columns == 1 && type.array.empty());

	if (type.vecsize == final_swiz.size())
		op.erase(pos, string::npos);
	return true;
}

string CompilerGLSL::build_composite_combiner(uint32_t return_type, const uint32_t *elems, uint32_t length)
{
	uint32_t base = 0;
	string op;
	string subop;

	// Can only merge swizzles for vectors.
	auto &type = get<SPIRType>(return_type);
	bool can_apply_swizzle_opt = type.basetype != SPIRType::Struct && type.array.empty() && type.columns == 1;
	bool swizzle_optimization = false;

	for (uint32_t i = 0; i < length; i++)
	{
		auto *e = maybe_get<SPIRExpression>(elems[i]);

		// If we're merging another scalar which belongs to the same base
		// object, just merge the swizzles to avoid triggering more than 1 expression read as much as possible!
		if (can_apply_swizzle_opt && e && e->base_expression && e->base_expression == base)
		{
			// Only supposed to be used for vector swizzle -> scalar.
			assert(!e->expression.empty() && e->expression.front() == '.');
			subop += e->expression.substr(1, string::npos);
			swizzle_optimization = true;
		}
		else
		{
			// We'll likely end up with duplicated swizzles, e.g.
			// foobar.xyz.xyz from patterns like
			// OpVectorShuffle
			// OpCompositeExtract x 3
			// OpCompositeConstruct 3x + other scalar.
			// Just modify op in-place.
			if (swizzle_optimization)
			{
				if (backend.swizzle_is_function)
					subop += "()";

				// Don't attempt to remove unity swizzling if we managed to remove duplicate swizzles.
				// The base "foo" might be vec4, while foo.xyz is vec3 (OpVectorShuffle) and looks like a vec3 due to the .xyz tacked on.
				// We only want to remove the swizzles if we're certain that the resulting base will be the same vecsize.
				// Essentially, we can only remove one set of swizzles, since that's what we have control over ...
				// Case 1:
				//  foo.yxz.xyz: Duplicate swizzle kicks in, giving foo.yxz, we are done.
				//               foo.yxz was the result of OpVectorShuffle and we don't know the type of foo.
				// Case 2:
				//  foo.xyz: Duplicate swizzle won't kick in.
				//           If foo is vec3, we can remove xyz, giving just foo.
				if (!remove_duplicate_swizzle(subop))
					remove_unity_swizzle(base, subop);

				// Strips away redundant parens if we created them during component extraction.
				strip_enclosed_expression(subop);
				swizzle_optimization = false;
				op += subop;
			}
			else
				op += subop;

			if (i)
				op += ", ";
			subop = to_expression(elems[i]);
		}

		base = e ? e->base_expression : 0;
	}

	if (swizzle_optimization)
	{
		if (backend.swizzle_is_function)
			subop += "()";

		if (!remove_duplicate_swizzle(subop))
			remove_unity_swizzle(base, subop);
		// Strips away redundant parens if we created them during component extraction.
		strip_enclosed_expression(subop);
	}

	op += subop;
	return op;
}

bool CompilerGLSL::skip_argument(uint32_t id) const
{
	if (!combined_image_samplers.empty() || !options.vulkan_semantics)
	{
		auto &type = expression_type(id);
		if (type.basetype == SPIRType::Sampler || (type.basetype == SPIRType::Image && type.image.sampled == 1))
			return true;
	}
	return false;
}

bool CompilerGLSL::optimize_read_modify_write(const SPIRType &type, const string &lhs, const string &rhs)
{
	// Do this with strings because we have a very clear pattern we can check for and it avoids
	// adding lots of special cases to the code emission.
	if (rhs.size() < lhs.size() + 3)
		return false;

	// Do not optimize matrices. They are a bit awkward to reason about in general
	// (in which order does operation happen?), and it does not work on MSL anyways.
	if (type.vecsize > 1 && type.columns > 1)
		return false;

	auto index = rhs.find(lhs);
	if (index != 0)
		return false;

	// TODO: Shift operators, but it's not important for now.
	auto op = rhs.find_first_of("+-/*%|&^", lhs.size() + 1);
	if (op != lhs.size() + 1)
		return false;

	// Check that the op is followed by space. This excludes && and ||.
	if (rhs[op + 1] != ' ')
		return false;

	char bop = rhs[op];
	auto expr = rhs.substr(lhs.size() + 3);
	// Try to find increments and decrements. Makes it look neater as += 1, -= 1 is fairly rare to see in real code.
	// Find some common patterns which are equivalent.
	if ((bop == '+' || bop == '-') && (expr == "1" || expr == "uint(1)" || expr == "1u" || expr == "int(1u)"))
		statement(lhs, bop, bop, ";");
	else
		statement(lhs, " ", bop, "= ", expr, ";");
	return true;
}

void CompilerGLSL::register_control_dependent_expression(uint32_t expr)
{
	if (forwarded_temporaries.find(expr) == end(forwarded_temporaries))
		return;

	assert(current_emitting_block);
	current_emitting_block->invalidate_expressions.push_back(expr);
}

void CompilerGLSL::emit_block_instructions(SPIRBlock &block)
{
	current_emitting_block = &block;
	for (auto &op : block.ops)
		emit_instruction(op);
	current_emitting_block = nullptr;
}

void CompilerGLSL::emit_instruction(const Instruction &instruction)
{
	auto ops = stream(instruction);
	auto opcode = static_cast<Op>(instruction.op);
	uint32_t length = instruction.length;

#define GLSL_BOP(op) emit_binary_op(ops[0], ops[1], ops[2], ops[3], #op)
#define GLSL_BOP_CAST(op, type) \
	emit_binary_op_cast(ops[0], ops[1], ops[2], ops[3], #op, type, glsl_opcode_is_sign_invariant(opcode))
#define GLSL_UOP(op) emit_unary_op(ops[0], ops[1], ops[2], #op)
#define GLSL_QFOP(op) emit_quaternary_func_op(ops[0], ops[1], ops[2], ops[3], ops[4], ops[5], #op)
#define GLSL_TFOP(op) emit_trinary_func_op(ops[0], ops[1], ops[2], ops[3], ops[4], #op)
#define GLSL_BFOP(op) emit_binary_func_op(ops[0], ops[1], ops[2], ops[3], #op)
#define GLSL_BFOP_CAST(op, type) \
	emit_binary_func_op_cast(ops[0], ops[1], ops[2], ops[3], #op, type, glsl_opcode_is_sign_invariant(opcode))
#define GLSL_BFOP(op) emit_binary_func_op(ops[0], ops[1], ops[2], ops[3], #op)
#define GLSL_UFOP(op) emit_unary_func_op(ops[0], ops[1], ops[2], #op)

	switch (opcode)
	{
	// Dealing with memory
	case OpLoad:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		uint32_t ptr = ops[2];

		flush_variable_declaration(ptr);

		// If we're loading from memory that cannot be changed by the shader,
		// just forward the expression directly to avoid needless temporaries.
		// If an expression is mutable and forwardable, we speculate that it is immutable.
		bool forward = should_forward(ptr) && forced_temporaries.find(id) == end(forced_temporaries);

		// If loading a non-native row-major matrix, mark the expression as need_transpose.
		bool need_transpose = false;
		bool old_need_transpose = false;

		auto *ptr_expression = maybe_get<SPIRExpression>(ptr);
		if (ptr_expression && ptr_expression->need_transpose)
		{
			old_need_transpose = true;
			ptr_expression->need_transpose = false;
			need_transpose = true;
		}
		else if (is_non_native_row_major_matrix(ptr))
			need_transpose = true;

		auto expr = to_expression(ptr);

		// We might need to bitcast in order to load from a builtin.
		bitcast_from_builtin_load(ptr, expr, get<SPIRType>(result_type));

		if (ptr_expression)
			ptr_expression->need_transpose = old_need_transpose;

		// By default, suppress usage tracking since using same expression multiple times does not imply any extra work.
		// However, if we try to load a complex, composite object from a flattened buffer,
		// we should avoid emitting the same code over and over and lower the result to a temporary.
		auto &type = get<SPIRType>(result_type);
		bool usage_tracking = ptr_expression && flattened_buffer_blocks.count(ptr_expression->loaded_from) != 0 &&
		                      (type.basetype == SPIRType::Struct || (type.columns > 1));

		auto &e = emit_op(result_type, id, expr, forward, !usage_tracking);
		e.need_transpose = need_transpose;
		register_read(id, ptr, forward);

		// Pass through whether the result is of a packed type.
		if (has_decoration(ptr, DecorationCPacked))
			set_decoration(id, DecorationCPacked);

		break;
	}

	case OpInBoundsAccessChain:
	case OpAccessChain:
	{
		auto *var = maybe_get<SPIRVariable>(ops[2]);
		if (var)
			flush_variable_declaration(var->self);

		// If the base is immutable, the access chain pointer must also be.
		// If an expression is mutable and forwardable, we speculate that it is immutable.
		bool need_transpose = false;
		bool result_is_packed = false;
		auto e = access_chain(ops[2], &ops[3], length - 3, get<SPIRType>(ops[0]), &need_transpose, &result_is_packed);

		auto &expr = set<SPIRExpression>(ops[1], move(e), ops[0], should_forward(ops[2]));

		auto *backing_variable = maybe_get_backing_variable(ops[2]);
		expr.loaded_from = backing_variable ? backing_variable->self : ops[2];
		expr.need_transpose = need_transpose;

		// Mark the result as being packed. Some platforms handled packed vectors differently than non-packed.
		if (result_is_packed)
			set_decoration(ops[1], DecorationCPacked);
		else
			unset_decoration(ops[1], DecorationCPacked);

		break;
	}

	case OpStore:
	{
		auto *var = maybe_get<SPIRVariable>(ops[0]);

		if (var && var->statically_assigned)
			var->static_expression = ops[1];
		else if (var && var->loop_variable && !var->loop_variable_enable)
			var->static_expression = ops[1];
		else if (var && var->remapped_variable)
		{
			// Skip the write.
		}
		else if (var && flattened_structs.count(ops[0]))
		{
			store_flattened_struct(*var, ops[1]);
			register_write(ops[0]);
		}
		else
		{
			auto rhs = to_expression(ops[1]);

			// Statements to OpStore may be empty if it is a struct with zero members. Just forward the store to /dev/null.
			if (!rhs.empty())
			{
				auto lhs = to_expression(ops[0]);

				// We might need to bitcast in order to store to a builtin.
				bitcast_to_builtin_store(ops[0], rhs, expression_type(ops[1]));

				// Tries to optimize assignments like "<lhs> = <lhs> op expr".
				// While this is purely cosmetic, this is important for legacy ESSL where loop
				// variable increments must be in either i++ or i += const-expr.
				// Without this, we end up with i = i + 1, which is correct GLSL, but not correct GLES 2.0.
				if (!optimize_read_modify_write(expression_type(ops[1]), lhs, rhs))
					statement(lhs, " = ", rhs, ";");
				register_write(ops[0]);
			}
		}
		break;
	}

	case OpArrayLength:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		auto e = access_chain_internal(ops[2], &ops[3], length - 3, true);
		set<SPIRExpression>(id, e + ".length()", result_type, true);
		break;
	}

	// Function calls
	case OpFunctionCall:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		uint32_t func = ops[2];
		const auto *arg = &ops[3];
		length -= 3;

		auto &callee = get<SPIRFunction>(func);
		auto &return_type = get<SPIRType>(callee.return_type);
		bool pure = function_is_pure(callee);

		bool callee_has_out_variables = false;
		bool emit_return_value_as_argument = false;

		// Invalidate out variables passed to functions since they can be OpStore'd to.
		for (uint32_t i = 0; i < length; i++)
		{
			if (callee.arguments[i].write_count)
			{
				register_call_out_argument(arg[i]);
				callee_has_out_variables = true;
			}

			flush_variable_declaration(arg[i]);
		}

		if (!return_type.array.empty() && !backend.can_return_array)
		{
			callee_has_out_variables = true;
			emit_return_value_as_argument = true;
		}

		if (!pure)
			register_impure_function_call();

		string funexpr;
		vector<string> arglist;
		funexpr += to_name(func) + "(";

		if (emit_return_value_as_argument)
		{
			statement(type_to_glsl(return_type), " ", to_name(id), type_to_array_glsl(return_type), ";");
			arglist.push_back(to_name(id));
		}

		for (uint32_t i = 0; i < length; i++)
		{
			// Do not pass in separate images or samplers if we're remapping
			// to combined image samplers.
			if (skip_argument(arg[i]))
				continue;

			arglist.push_back(to_func_call_arg(arg[i]));
		}

		for (auto &combined : callee.combined_parameters)
		{
			uint32_t image_id = combined.global_image ? combined.image_id : arg[combined.image_id];
			uint32_t sampler_id = combined.global_sampler ? combined.sampler_id : arg[combined.sampler_id];
			arglist.push_back(to_combined_image_sampler(image_id, sampler_id));
		}

		append_global_func_args(callee, length, arglist);

		funexpr += merge(arglist);
		funexpr += ")";

		// Check for function call constraints.
		check_function_call_constraints(arg, length);

		if (return_type.basetype != SPIRType::Void)
		{
			// If the function actually writes to an out variable,
			// take the conservative route and do not forward.
			// The problem is that we might not read the function
			// result (and emit the function) before an out variable
			// is read (common case when return value is ignored!
			// In order to avoid start tracking invalid variables,
			// just avoid the forwarding problem altogether.
			bool forward = args_will_forward(id, arg, length, pure) && !callee_has_out_variables && pure &&
			               (forced_temporaries.find(id) == end(forced_temporaries));

			if (emit_return_value_as_argument)
			{
				statement(funexpr, ";");
				set<SPIRExpression>(id, to_name(id), result_type, true);
			}
			else
				emit_op(result_type, id, funexpr, forward);

			// Function calls are implicit loads from all variables in question.
			// Set dependencies for them.
			for (uint32_t i = 0; i < length; i++)
				register_read(id, arg[i], forward);

			// If we're going to forward the temporary result,
			// put dependencies on every variable that must not change.
			if (forward)
				register_global_read_dependencies(callee, id);
		}
		else
			statement(funexpr, ";");

		break;
	}

	// Composite munging
	case OpCompositeConstruct:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		const auto *const elems = &ops[2];
		length -= 2;

		bool forward = true;
		for (uint32_t i = 0; i < length; i++)
			forward = forward && should_forward(elems[i]);

		auto &out_type = get<SPIRType>(result_type);
		auto *in_type = length > 0 ? &expression_type(elems[0]) : nullptr;

		// Only splat if we have vector constructors.
		// Arrays and structs must be initialized properly in full.
		bool composite = !out_type.array.empty() || out_type.basetype == SPIRType::Struct;

		bool splat = false;
		bool swizzle_splat = false;

		if (in_type)
		{
			splat = in_type->vecsize == 1 && in_type->columns == 1 && !composite && backend.use_constructor_splatting;
			swizzle_splat = in_type->vecsize == 1 && in_type->columns == 1 && backend.can_swizzle_scalar;

			if (ids[elems[0]].get_type() == TypeConstant && !type_is_floating_point(*in_type))
			{
				// Cannot swizzle literal integers as a special case.
				swizzle_splat = false;
			}
		}

		if (splat || swizzle_splat)
		{
			uint32_t input = elems[0];
			for (uint32_t i = 0; i < length; i++)
			{
				if (input != elems[i])
				{
					splat = false;
					swizzle_splat = false;
				}
			}
		}

		if (out_type.basetype == SPIRType::Struct && !backend.can_declare_struct_inline)
			forward = false;
		if (!out_type.array.empty() && !backend.can_declare_arrays_inline)
			forward = false;
		if (type_is_empty(out_type) && !backend.supports_empty_struct)
			forward = false;

		string constructor_op;
		if (backend.use_initializer_list && composite)
		{
			// Only use this path if we are building composites.
			// This path cannot be used for arithmetic.
			if (backend.use_typed_initializer_list && out_type.basetype == SPIRType::Struct)
				constructor_op += type_to_glsl_constructor(get<SPIRType>(result_type));
			constructor_op += "{ ";
			if (type_is_empty(out_type) && !backend.supports_empty_struct)
				constructor_op += "0";
			else if (splat)
				constructor_op += to_expression(elems[0]);
			else
				constructor_op += build_composite_combiner(result_type, elems, length);
			constructor_op += " }";
		}
		else if (swizzle_splat && !composite)
		{
			constructor_op = remap_swizzle(get<SPIRType>(result_type), 1, to_expression(elems[0]));
		}
		else
		{
			constructor_op = type_to_glsl_constructor(get<SPIRType>(result_type)) + "(";
			if (type_is_empty(out_type) && !backend.supports_empty_struct)
				constructor_op += "0";
			else if (splat)
				constructor_op += to_expression(elems[0]);
			else
				constructor_op += build_composite_combiner(result_type, elems, length);
			constructor_op += ")";
		}

		emit_op(result_type, id, constructor_op, forward);
		for (uint32_t i = 0; i < length; i++)
			inherit_expression_dependencies(id, elems[i]);
		break;
	}

	case OpVectorInsertDynamic:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		uint32_t vec = ops[2];
		uint32_t comp = ops[3];
		uint32_t index = ops[4];

		flush_variable_declaration(vec);

		// Make a copy, then use access chain to store the variable.
		statement(declare_temporary(result_type, id), to_expression(vec), ";");
		set<SPIRExpression>(id, to_name(id), result_type, true);
		auto chain = access_chain_internal(id, &index, 1, false);
		statement(chain, " = ", to_expression(comp), ";");
		break;
	}

	case OpVectorExtractDynamic:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];

		auto expr = access_chain_internal(ops[2], &ops[3], 1, false);
		emit_op(result_type, id, expr, should_forward(ops[2]));
		inherit_expression_dependencies(id, ops[2]);
		inherit_expression_dependencies(id, ops[3]);
		break;
	}

	case OpCompositeExtract:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		length -= 3;

		auto &type = get<SPIRType>(result_type);

		// We can only split the expression here if our expression is forwarded as a temporary.
		bool allow_base_expression = forced_temporaries.find(id) == end(forced_temporaries);

		// Do not allow base expression for struct members. We risk doing "swizzle" optimizations in this case.
		auto &composite_type = expression_type(ops[2]);
		if (composite_type.basetype == SPIRType::Struct || !composite_type.array.empty())
			allow_base_expression = false;

		// Packed expressions cannot be split up.
		if (has_decoration(ops[2], DecorationCPacked))
			allow_base_expression = false;

		// Only apply this optimization if result is scalar.
		if (allow_base_expression && should_forward(ops[2]) && type.vecsize == 1 && type.columns == 1 && length == 1)
		{
			// We want to split the access chain from the base.
			// This is so we can later combine different CompositeExtract results
			// with CompositeConstruct without emitting code like
			//
			// vec3 temp = texture(...).xyz
			// vec4(temp.x, temp.y, temp.z, 1.0).
			//
			// when we actually wanted to emit this
			// vec4(texture(...).xyz, 1.0).
			//
			// Including the base will prevent this and would trigger multiple reads
			// from expression causing it to be forced to an actual temporary in GLSL.
			auto expr = access_chain_internal(ops[2], &ops[3], length, true, true);
			auto &e = emit_op(result_type, id, expr, true, !expression_is_forwarded(ops[2]));
			inherit_expression_dependencies(id, ops[2]);
			e.base_expression = ops[2];
		}
		else
		{
			auto expr = access_chain_internal(ops[2], &ops[3], length, true);
			emit_op(result_type, id, expr, should_forward(ops[2]), !expression_is_forwarded(ops[2]));
			inherit_expression_dependencies(id, ops[2]);
		}
		break;
	}

	case OpCompositeInsert:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		uint32_t obj = ops[2];
		uint32_t composite = ops[3];
		const auto *elems = &ops[4];
		length -= 4;

		flush_variable_declaration(composite);

		// Make a copy, then use access chain to store the variable.
		statement(declare_temporary(result_type, id), to_expression(composite), ";");
		set<SPIRExpression>(id, to_name(id), result_type, true);
		auto chain = access_chain_internal(id, elems, length, true);
		statement(chain, " = ", to_expression(obj), ";");

		break;
	}

	case OpCopyMemory:
	{
		uint32_t lhs = ops[0];
		uint32_t rhs = ops[1];
		if (lhs != rhs)
		{
			flush_variable_declaration(lhs);
			flush_variable_declaration(rhs);
			statement(to_expression(lhs), " = ", to_expression(rhs), ";");
			register_write(lhs);
		}
		break;
	}

	case OpCopyObject:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		uint32_t rhs = ops[2];
		bool pointer = get<SPIRType>(result_type).pointer;

		if (expression_is_lvalue(rhs) && !pointer)
		{
			// Need a copy.
			// For pointer types, we copy the pointer itself.
			statement(declare_temporary(result_type, id), to_expression(rhs), ";");
			set<SPIRExpression>(id, to_name(id), result_type, true);
			inherit_expression_dependencies(id, rhs);
		}
		else
		{
			// RHS expression is immutable, so just forward it.
			// Copying these things really make no sense, but
			// seems to be allowed anyways.
			auto &e = set<SPIRExpression>(id, to_expression(rhs), result_type, true);
			if (pointer)
			{
				auto *var = maybe_get_backing_variable(rhs);
				e.loaded_from = var ? var->self : 0;
			}
		}
		break;
	}

	case OpVectorShuffle:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		uint32_t vec0 = ops[2];
		uint32_t vec1 = ops[3];
		const auto *elems = &ops[4];
		length -= 4;

		auto &type0 = expression_type(vec0);

		bool shuffle = false;
		for (uint32_t i = 0; i < length; i++)
			if (elems[i] >= type0.vecsize)
				shuffle = true;

		// Cannot use swizzles with packed expressions, force shuffle path.
		if (!shuffle && has_decoration(vec0, DecorationCPacked))
			shuffle = true;

		string expr;
		bool should_fwd, trivial_forward;

		if (shuffle)
		{
			should_fwd = should_forward(vec0) && should_forward(vec1);
			trivial_forward = !expression_is_forwarded(vec0) && !expression_is_forwarded(vec1);

			// Constructor style and shuffling from two different vectors.
			vector<string> args;
			for (uint32_t i = 0; i < length; i++)
			{
				if (elems[i] >= type0.vecsize)
					args.push_back(to_extract_component_expression(vec1, elems[i] - type0.vecsize));
				else
					args.push_back(to_extract_component_expression(vec0, elems[i]));
			}
			expr += join(type_to_glsl_constructor(get<SPIRType>(result_type)), "(", merge(args), ")");
		}
		else
		{
			should_fwd = should_forward(vec0);
			trivial_forward = !expression_is_forwarded(vec0);

			// We only source from first vector, so can use swizzle.
			// If the vector is packed, unpack it before applying a swizzle (needed for MSL)
			expr += to_enclosed_unpacked_expression(vec0);
			expr += ".";
			for (uint32_t i = 0; i < length; i++)
				expr += index_to_swizzle(elems[i]);

			if (backend.swizzle_is_function && length > 1)
				expr += "()";
		}

		// A shuffle is trivial in that it doesn't actually *do* anything.
		// We inherit the forwardedness from our arguments to avoid flushing out to temporaries when it's not really needed.

		emit_op(result_type, id, expr, should_fwd, trivial_forward);
		inherit_expression_dependencies(id, vec0);
		inherit_expression_dependencies(id, vec1);
		break;
	}

	// ALU
	case OpIsNan:
		GLSL_UFOP(isnan);
		break;

	case OpIsInf:
		GLSL_UFOP(isinf);
		break;

	case OpSNegate:
	case OpFNegate:
		GLSL_UOP(-);
		break;

	case OpIAdd:
	{
		// For simple arith ops, prefer the output type if there's a mismatch to avoid extra bitcasts.
		auto type = get<SPIRType>(ops[0]).basetype;
		GLSL_BOP_CAST(+, type);
		break;
	}

	case OpFAdd:
		GLSL_BOP(+);
		break;

	case OpISub:
	{
		auto type = get<SPIRType>(ops[0]).basetype;
		GLSL_BOP_CAST(-, type);
		break;
	}

	case OpFSub:
		GLSL_BOP(-);
		break;

	case OpIMul:
	{
		auto type = get<SPIRType>(ops[0]).basetype;
		GLSL_BOP_CAST(*, type);
		break;
	}

	case OpVectorTimesMatrix:
	case OpMatrixTimesVector:
	{
		// If the matrix needs transpose, just flip the multiply order.
		auto *e = maybe_get<SPIRExpression>(ops[opcode == OpMatrixTimesVector ? 2 : 3]);
		if (e && e->need_transpose)
		{
			e->need_transpose = false;
			emit_binary_op(ops[0], ops[1], ops[3], ops[2], "*");
			e->need_transpose = true;
		}
		else
			GLSL_BOP(*);
		break;
	}

	case OpFMul:
	case OpMatrixTimesScalar:
	case OpVectorTimesScalar:
	case OpMatrixTimesMatrix:
		GLSL_BOP(*);
		break;

	case OpOuterProduct:
		GLSL_BFOP(outerProduct);
		break;

	case OpDot:
		GLSL_BFOP(dot);
		break;

	case OpTranspose:
		GLSL_UFOP(transpose);
		break;

	case OpSRem:
	{
		uint32_t result_type = ops[0];
		uint32_t result_id = ops[1];
		uint32_t op0 = ops[2];
		uint32_t op1 = ops[3];

		// Needs special handling.
		bool forward = should_forward(op0) && should_forward(op1);
		auto expr = join(to_enclosed_expression(op0), " - ", to_enclosed_expression(op1), " * ", "(",
		                 to_enclosed_expression(op0), " / ", to_enclosed_expression(op1), ")");

		emit_op(result_type, result_id, expr, forward);
		inherit_expression_dependencies(result_id, op0);
		inherit_expression_dependencies(result_id, op1);
		break;
	}

	case OpSDiv:
		GLSL_BOP_CAST(/, SPIRType::Int);
		break;

	case OpUDiv:
		GLSL_BOP_CAST(/, SPIRType::UInt);
		break;

	case OpFDiv:
		GLSL_BOP(/);
		break;

	case OpShiftRightLogical:
		GLSL_BOP_CAST(>>, SPIRType::UInt);
		break;

	case OpShiftRightArithmetic:
		GLSL_BOP_CAST(>>, SPIRType::Int);
		break;

	case OpShiftLeftLogical:
	{
		auto type = get<SPIRType>(ops[0]).basetype;
		GLSL_BOP_CAST(<<, type);
		break;
	}

	case OpBitwiseOr:
	{
		auto type = get<SPIRType>(ops[0]).basetype;
		GLSL_BOP_CAST(|, type);
		break;
	}

	case OpBitwiseXor:
	{
		auto type = get<SPIRType>(ops[0]).basetype;
		GLSL_BOP_CAST (^, type);
		break;
	}

	case OpBitwiseAnd:
	{
		auto type = get<SPIRType>(ops[0]).basetype;
		GLSL_BOP_CAST(&, type);
		break;
	}

	case OpNot:
		GLSL_UOP(~);
		break;

	case OpUMod:
		GLSL_BOP_CAST(%, SPIRType::UInt);
		break;

	case OpSMod:
		GLSL_BOP_CAST(%, SPIRType::Int);
		break;

	case OpFMod:
		GLSL_BFOP(mod);
		break;

	case OpFRem:
	{
		if (is_legacy())
			SPIRV_CROSS_THROW("OpFRem requires trunc() and is only supported on non-legacy targets. A workaround is "
			                  "needed for legacy.");

		uint32_t result_type = ops[0];
		uint32_t result_id = ops[1];
		uint32_t op0 = ops[2];
		uint32_t op1 = ops[3];

		// Needs special handling.
		bool forward = should_forward(op0) && should_forward(op1);
		auto expr = join(to_enclosed_expression(op0), " - ", to_enclosed_expression(op1), " * ", "trunc(",
		                 to_enclosed_expression(op0), " / ", to_enclosed_expression(op1), ")");

		emit_op(result_type, result_id, expr, forward);
		inherit_expression_dependencies(result_id, op0);
		inherit_expression_dependencies(result_id, op1);
		break;
	}

	// Relational
	case OpAny:
		GLSL_UFOP(any);
		break;

	case OpAll:
		GLSL_UFOP(all);
		break;

	case OpSelect:
		emit_mix_op(ops[0], ops[1], ops[4], ops[3], ops[2]);
		break;

	case OpLogicalOr:
	{
		// No vector variant in GLSL for logical OR.
		auto result_type = ops[0];
		auto id = ops[1];
		auto &type = get<SPIRType>(result_type);

		if (type.vecsize > 1)
			emit_unrolled_binary_op(result_type, id, ops[2], ops[3], "||");
		else
			GLSL_BOP(||);
		break;
	}

	case OpLogicalAnd:
	{
		// No vector variant in GLSL for logical AND.
		auto result_type = ops[0];
		auto id = ops[1];
		auto &type = get<SPIRType>(result_type);

		if (type.vecsize > 1)
			emit_unrolled_binary_op(result_type, id, ops[2], ops[3], "&&");
		else
			GLSL_BOP(&&);
		break;
	}

	case OpLogicalNot:
	{
		auto &type = get<SPIRType>(ops[0]);
		if (type.vecsize > 1)
			GLSL_UFOP(not);
		else
			GLSL_UOP(!);
		break;
	}

	case OpIEqual:
	{
		if (expression_type(ops[2]).vecsize > 1)
			GLSL_BFOP_CAST(equal, SPIRType::Int);
		else
			GLSL_BOP_CAST(==, SPIRType::Int);
		break;
	}

	case OpLogicalEqual:
	case OpFOrdEqual:
	{
		if (expression_type(ops[2]).vecsize > 1)
			GLSL_BFOP(equal);
		else
			GLSL_BOP(==);
		break;
	}

	case OpINotEqual:
	{
		if (expression_type(ops[2]).vecsize > 1)
			GLSL_BFOP_CAST(notEqual, SPIRType::Int);
		else
			GLSL_BOP_CAST(!=, SPIRType::Int);
		break;
	}

	case OpLogicalNotEqual:
	case OpFOrdNotEqual:
	{
		if (expression_type(ops[2]).vecsize > 1)
			GLSL_BFOP(notEqual);
		else
			GLSL_BOP(!=);
		break;
	}

	case OpUGreaterThan:
	case OpSGreaterThan:
	{
		auto type = opcode == OpUGreaterThan ? SPIRType::UInt : SPIRType::Int;
		if (expression_type(ops[2]).vecsize > 1)
			GLSL_BFOP_CAST(greaterThan, type);
		else
			GLSL_BOP_CAST(>, type);
		break;
	}

	case OpFOrdGreaterThan:
	{
		if (expression_type(ops[2]).vecsize > 1)
			GLSL_BFOP(greaterThan);
		else
			GLSL_BOP(>);
		break;
	}

	case OpUGreaterThanEqual:
	case OpSGreaterThanEqual:
	{
		auto type = opcode == OpUGreaterThanEqual ? SPIRType::UInt : SPIRType::Int;
		if (expression_type(ops[2]).vecsize > 1)
			GLSL_BFOP_CAST(greaterThanEqual, type);
		else
			GLSL_BOP_CAST(>=, type);
		break;
	}

	case OpFOrdGreaterThanEqual:
	{
		if (expression_type(ops[2]).vecsize > 1)
			GLSL_BFOP(greaterThanEqual);
		else
			GLSL_BOP(>=);
		break;
	}

	case OpULessThan:
	case OpSLessThan:
	{
		auto type = opcode == OpULessThan ? SPIRType::UInt : SPIRType::Int;
		if (expression_type(ops[2]).vecsize > 1)
			GLSL_BFOP_CAST(lessThan, type);
		else
			GLSL_BOP_CAST(<, type);
		break;
	}

	case OpFOrdLessThan:
	{
		if (expression_type(ops[2]).vecsize > 1)
			GLSL_BFOP(lessThan);
		else
			GLSL_BOP(<);
		break;
	}

	case OpULessThanEqual:
	case OpSLessThanEqual:
	{
		auto type = opcode == OpULessThanEqual ? SPIRType::UInt : SPIRType::Int;
		if (expression_type(ops[2]).vecsize > 1)
			GLSL_BFOP_CAST(lessThanEqual, type);
		else
			GLSL_BOP_CAST(<=, type);
		break;
	}

	case OpFOrdLessThanEqual:
	{
		if (expression_type(ops[2]).vecsize > 1)
			GLSL_BFOP(lessThanEqual);
		else
			GLSL_BOP(<=);
		break;
	}

	// Conversion
	case OpConvertFToU:
	case OpConvertFToS:
	case OpConvertSToF:
	case OpConvertUToF:
	case OpUConvert:
	case OpSConvert:
	case OpFConvert:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];

		auto func = type_to_glsl_constructor(get<SPIRType>(result_type));
		emit_unary_func_op(result_type, id, ops[2], func.c_str());
		break;
	}

	case OpBitcast:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		uint32_t arg = ops[2];

		auto op = bitcast_glsl_op(get<SPIRType>(result_type), expression_type(arg));
		emit_unary_func_op(result_type, id, arg, op.c_str());
		break;
	}

	case OpQuantizeToF16:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		uint32_t arg = ops[2];

		string op;
		auto &type = get<SPIRType>(result_type);

		switch (type.vecsize)
		{
		case 1:
			op = join("unpackHalf2x16(packHalf2x16(vec2(", to_expression(arg), "))).x");
			break;
		case 2:
			op = join("unpackHalf2x16(packHalf2x16(", to_expression(arg), "))");
			break;
		case 3:
		{
			auto op0 = join("unpackHalf2x16(packHalf2x16(", to_expression(arg), ".xy))");
			auto op1 = join("unpackHalf2x16(packHalf2x16(", to_expression(arg), ".zz)).x");
			op = join("vec3(", op0, ", ", op1, ")");
			break;
		}
		case 4:
		{
			auto op0 = join("unpackHalf2x16(packHalf2x16(", to_expression(arg), ".xy))");
			auto op1 = join("unpackHalf2x16(packHalf2x16(", to_expression(arg), ".zw))");
			op = join("vec4(", op0, ", ", op1, ")");
			break;
		}
		default:
			SPIRV_CROSS_THROW("Illegal argument to OpQuantizeToF16.");
		}

		emit_op(result_type, id, op, should_forward(arg));
		inherit_expression_dependencies(id, arg);
		break;
	}

	// Derivatives
	case OpDPdx:
		GLSL_UFOP(dFdx);
		if (is_legacy_es())
			require_extension_internal("GL_OES_standard_derivatives");
		register_control_dependent_expression(ops[1]);
		break;

	case OpDPdy:
		GLSL_UFOP(dFdy);
		if (is_legacy_es())
			require_extension_internal("GL_OES_standard_derivatives");
		register_control_dependent_expression(ops[1]);
		break;

	case OpDPdxFine:
		GLSL_UFOP(dFdxFine);
		if (options.es)
		{
			SPIRV_CROSS_THROW("GL_ARB_derivative_control is unavailable in OpenGL ES.");
		}
		if (options.version < 450)
			require_extension_internal("GL_ARB_derivative_control");
		register_control_dependent_expression(ops[1]);
		break;

	case OpDPdyFine:
		GLSL_UFOP(dFdyFine);
		if (options.es)
		{
			SPIRV_CROSS_THROW("GL_ARB_derivative_control is unavailable in OpenGL ES.");
		}
		if (options.version < 450)
			require_extension_internal("GL_ARB_derivative_control");
		register_control_dependent_expression(ops[1]);
		break;

	case OpDPdxCoarse:
		if (options.es)
		{
			SPIRV_CROSS_THROW("GL_ARB_derivative_control is unavailable in OpenGL ES.");
		}
		GLSL_UFOP(dFdxCoarse);
		if (options.version < 450)
			require_extension_internal("GL_ARB_derivative_control");
		register_control_dependent_expression(ops[1]);
		break;

	case OpDPdyCoarse:
		GLSL_UFOP(dFdyCoarse);
		if (options.es)
		{
			SPIRV_CROSS_THROW("GL_ARB_derivative_control is unavailable in OpenGL ES.");
		}
		if (options.version < 450)
			require_extension_internal("GL_ARB_derivative_control");
		register_control_dependent_expression(ops[1]);
		break;

	case OpFwidth:
		GLSL_UFOP(fwidth);
		if (is_legacy_es())
			require_extension_internal("GL_OES_standard_derivatives");
		register_control_dependent_expression(ops[1]);
		break;

	case OpFwidthCoarse:
		GLSL_UFOP(fwidthCoarse);
		if (options.es)
		{
			SPIRV_CROSS_THROW("GL_ARB_derivative_control is unavailable in OpenGL ES.");
		}
		if (options.version < 450)
			require_extension_internal("GL_ARB_derivative_control");
		register_control_dependent_expression(ops[1]);
		break;

	case OpFwidthFine:
		GLSL_UFOP(fwidthFine);
		if (options.es)
		{
			SPIRV_CROSS_THROW("GL_ARB_derivative_control is unavailable in OpenGL ES.");
		}
		if (options.version < 450)
			require_extension_internal("GL_ARB_derivative_control");
		register_control_dependent_expression(ops[1]);
		break;

	// Bitfield
	case OpBitFieldInsert:
		// TODO: The signedness of inputs is strict in GLSL, but not in SPIR-V, bitcast if necessary.
		GLSL_QFOP(bitfieldInsert);
		break;

	case OpBitFieldSExtract:
	case OpBitFieldUExtract:
		// TODO: The signedness of inputs is strict in GLSL, but not in SPIR-V, bitcast if necessary.
		GLSL_TFOP(bitfieldExtract);
		break;

	case OpBitReverse:
		GLSL_UFOP(bitfieldReverse);
		break;

	case OpBitCount:
		GLSL_UFOP(bitCount);
		break;

	// Atomics
	case OpAtomicExchange:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		uint32_t ptr = ops[2];
		// Ignore semantics for now, probably only relevant to CL.
		uint32_t val = ops[5];
		const char *op = check_atomic_image(ptr) ? "imageAtomicExchange" : "atomicExchange";
		forced_temporaries.insert(id);
		emit_binary_func_op(result_type, id, ptr, val, op);
		flush_all_atomic_capable_variables();
		break;
	}

	case OpAtomicCompareExchange:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		uint32_t ptr = ops[2];
		uint32_t val = ops[6];
		uint32_t comp = ops[7];
		const char *op = check_atomic_image(ptr) ? "imageAtomicCompSwap" : "atomicCompSwap";

		forced_temporaries.insert(id);
		emit_trinary_func_op(result_type, id, ptr, comp, val, op);
		flush_all_atomic_capable_variables();
		break;
	}

	case OpAtomicLoad:
		flush_all_atomic_capable_variables();
		// FIXME: Image?
		// OpAtomicLoad seems to only be relevant for atomic counters.
		GLSL_UFOP(atomicCounter);
		register_read(ops[1], ops[2], should_forward(ops[2]));
		break;

	case OpAtomicStore:
		SPIRV_CROSS_THROW("Unsupported opcode OpAtomicStore.");

	case OpAtomicIIncrement:
		forced_temporaries.insert(ops[1]);
		// FIXME: Image?
		GLSL_UFOP(atomicCounterIncrement);
		flush_all_atomic_capable_variables();
		register_read(ops[1], ops[2], should_forward(ops[2]));
		break;

	case OpAtomicIDecrement:
		forced_temporaries.insert(ops[1]);
		// FIXME: Image?
		GLSL_UFOP(atomicCounterDecrement);
		flush_all_atomic_capable_variables();
		register_read(ops[1], ops[2], should_forward(ops[2]));
		break;

	case OpAtomicIAdd:
	{
		const char *op = check_atomic_image(ops[2]) ? "imageAtomicAdd" : "atomicAdd";
		forced_temporaries.insert(ops[1]);
		emit_binary_func_op(ops[0], ops[1], ops[2], ops[5], op);
		flush_all_atomic_capable_variables();
		register_read(ops[1], ops[2], should_forward(ops[2]));
		break;
	}

	case OpAtomicISub:
	{
		const char *op = check_atomic_image(ops[2]) ? "imageAtomicAdd" : "atomicAdd";
		forced_temporaries.insert(ops[1]);
		auto expr = join(op, "(", to_expression(ops[2]), ", -", to_enclosed_expression(ops[5]), ")");
		emit_op(ops[0], ops[1], expr, should_forward(ops[2]) && should_forward(ops[5]));
		flush_all_atomic_capable_variables();
		register_read(ops[1], ops[2], should_forward(ops[2]));
		break;
	}

	case OpAtomicSMin:
	case OpAtomicUMin:
	{
		const char *op = check_atomic_image(ops[2]) ? "imageAtomicMin" : "atomicMin";
		forced_temporaries.insert(ops[1]);
		emit_binary_func_op(ops[0], ops[1], ops[2], ops[5], op);
		flush_all_atomic_capable_variables();
		register_read(ops[1], ops[2], should_forward(ops[2]));
		break;
	}

	case OpAtomicSMax:
	case OpAtomicUMax:
	{
		const char *op = check_atomic_image(ops[2]) ? "imageAtomicMax" : "atomicMax";
		forced_temporaries.insert(ops[1]);
		emit_binary_func_op(ops[0], ops[1], ops[2], ops[5], op);
		flush_all_atomic_capable_variables();
		register_read(ops[1], ops[2], should_forward(ops[2]));
		break;
	}

	case OpAtomicAnd:
	{
		const char *op = check_atomic_image(ops[2]) ? "imageAtomicAnd" : "atomicAnd";
		forced_temporaries.insert(ops[1]);
		emit_binary_func_op(ops[0], ops[1], ops[2], ops[5], op);
		flush_all_atomic_capable_variables();
		register_read(ops[1], ops[2], should_forward(ops[2]));
		break;
	}

	case OpAtomicOr:
	{
		const char *op = check_atomic_image(ops[2]) ? "imageAtomicOr" : "atomicOr";
		forced_temporaries.insert(ops[1]);
		emit_binary_func_op(ops[0], ops[1], ops[2], ops[5], op);
		flush_all_atomic_capable_variables();
		register_read(ops[1], ops[2], should_forward(ops[2]));
		break;
	}

	case OpAtomicXor:
	{
		const char *op = check_atomic_image(ops[2]) ? "imageAtomicXor" : "atomicXor";
		forced_temporaries.insert(ops[1]);
		emit_binary_func_op(ops[0], ops[1], ops[2], ops[5], op);
		flush_all_atomic_capable_variables();
		register_read(ops[1], ops[2], should_forward(ops[2]));
		break;
	}

	// Geometry shaders
	case OpEmitVertex:
		statement("EmitVertex();");
		break;

	case OpEndPrimitive:
		statement("EndPrimitive();");
		break;

	case OpEmitStreamVertex:
		statement("EmitStreamVertex();");
		break;

	case OpEndStreamPrimitive:
		statement("EndStreamPrimitive();");
		break;

	// Textures
	case OpImageSampleExplicitLod:
	case OpImageSampleProjExplicitLod:
	case OpImageSampleDrefExplicitLod:
	case OpImageSampleProjDrefExplicitLod:
	case OpImageSampleImplicitLod:
	case OpImageSampleProjImplicitLod:
	case OpImageSampleDrefImplicitLod:
	case OpImageSampleProjDrefImplicitLod:
	case OpImageFetch:
	case OpImageGather:
	case OpImageDrefGather:
		// Gets a bit hairy, so move this to a separate instruction.
		emit_texture_op(instruction);
		break;

	case OpImage:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];

		// Suppress usage tracking.
		auto &e = emit_op(result_type, id, to_expression(ops[2]), true, true);

		// When using the image, we need to know which variable it is actually loaded from.
		auto *var = maybe_get_backing_variable(ops[2]);
		e.loaded_from = var ? var->self : 0;
		break;
	}

	case OpImageQueryLod:
	{
		if (!options.es && options.version < 400)
		{
			require_extension_internal("GL_ARB_texture_query_lod");
			// For some reason, the ARB spec is all-caps.
			GLSL_BFOP(textureQueryLOD);
		}
		else if (options.es)
			SPIRV_CROSS_THROW("textureQueryLod not supported in ES profile.");
		else
			GLSL_BFOP(textureQueryLod);
		register_control_dependent_expression(ops[1]);
		break;
	}

	case OpImageQueryLevels:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];

		if (!options.es && options.version < 430)
			require_extension_internal("GL_ARB_texture_query_levels");
		if (options.es)
			SPIRV_CROSS_THROW("textureQueryLevels not supported in ES profile.");

		auto expr = join("textureQueryLevels(", convert_separate_image_to_combined(ops[2]), ")");
		auto &restype = get<SPIRType>(ops[0]);
		expr = bitcast_expression(restype, SPIRType::Int, expr);
		emit_op(result_type, id, expr, true);
		break;
	}

	case OpImageQuerySamples:
	{
		auto &type = expression_type(ops[2]);
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];

		string expr;
		if (type.image.sampled == 2)
			expr = join("imageSamples(", to_expression(ops[2]), ")");
		else
			expr = join("textureSamples(", convert_separate_image_to_combined(ops[2]), ")");

		auto &restype = get<SPIRType>(ops[0]);
		expr = bitcast_expression(restype, SPIRType::Int, expr);
		emit_op(result_type, id, expr, true);
		break;
	}

	case OpSampledImage:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		emit_sampled_image_op(result_type, id, ops[2], ops[3]);
		break;
	}

	case OpImageQuerySizeLod:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];

		auto expr = join("textureSize(", convert_separate_image_to_combined(ops[2]), ", ",
		                 bitcast_expression(SPIRType::Int, ops[3]), ")");
		auto &restype = get<SPIRType>(ops[0]);
		expr = bitcast_expression(restype, SPIRType::Int, expr);
		emit_op(result_type, id, expr, true);
		break;
	}

	// Image load/store
	case OpImageRead:
	{
		// We added Nonreadable speculatively to the OpImage variable due to glslangValidator
		// not adding the proper qualifiers.
		// If it turns out we need to read the image after all, remove the qualifier and recompile.
		auto *var = maybe_get_backing_variable(ops[2]);
		if (var)
		{
			auto &flags = meta.at(var->self).decoration.decoration_flags;
			if (flags.get(DecorationNonReadable))
			{
				flags.clear(DecorationNonReadable);
				force_recompile = true;
			}
		}

		uint32_t result_type = ops[0];
		uint32_t id = ops[1];

		bool pure;
		string imgexpr;
		auto &type = expression_type(ops[2]);

		if (var && var->remapped_variable) // Remapped input, just read as-is without any op-code
		{
			if (type.image.ms)
				SPIRV_CROSS_THROW("Trying to remap multisampled image to variable, this is not possible.");

			auto itr =
			    find_if(begin(pls_inputs), end(pls_inputs), [var](const PlsRemap &pls) { return pls.id == var->self; });

			if (itr == end(pls_inputs))
			{
				// For non-PLS inputs, we rely on subpass type remapping information to get it right
				// since ImageRead always returns 4-component vectors and the backing type is opaque.
				if (!var->remapped_components)
					SPIRV_CROSS_THROW("subpassInput was remapped, but remap_components is not set correctly.");
				imgexpr = remap_swizzle(get<SPIRType>(result_type), var->remapped_components, to_expression(ops[2]));
			}
			else
			{
				// PLS input could have different number of components than what the SPIR expects, swizzle to
				// the appropriate vector size.
				uint32_t components = pls_format_to_components(itr->format);
				imgexpr = remap_swizzle(get<SPIRType>(result_type), components, to_expression(ops[2]));
			}
			pure = true;
		}
		else if (type.image.dim == DimSubpassData)
		{
			if (options.vulkan_semantics)
			{
				// With Vulkan semantics, use the proper Vulkan GLSL construct.
				if (type.image.ms)
				{
					uint32_t operands = ops[4];
					if (operands != ImageOperandsSampleMask || length != 6)
						SPIRV_CROSS_THROW(
						    "Multisampled image used in OpImageRead, but unexpected operand mask was used.");

					uint32_t samples = ops[5];
					imgexpr = join("subpassLoad(", to_expression(ops[2]), ", ", to_expression(samples), ")");
				}
				else
					imgexpr = join("subpassLoad(", to_expression(ops[2]), ")");
			}
			else
			{
				if (type.image.ms)
				{
					uint32_t operands = ops[4];
					if (operands != ImageOperandsSampleMask || length != 6)
						SPIRV_CROSS_THROW(
						    "Multisampled image used in OpImageRead, but unexpected operand mask was used.");

					uint32_t samples = ops[5];
					imgexpr = join("texelFetch(", to_expression(ops[2]), ", ivec2(gl_FragCoord.xy), ",
					               to_expression(samples), ")");
				}
				else
				{
					// Implement subpass loads via texture barrier style sampling.
					imgexpr = join("texelFetch(", to_expression(ops[2]), ", ivec2(gl_FragCoord.xy), 0)");
				}
			}
			imgexpr = remap_swizzle(get<SPIRType>(result_type), 4, imgexpr);
			pure = true;
		}
		else
		{
			// imageLoad only accepts int coords, not uint.
			auto coord_expr = to_expression(ops[3]);
			auto target_coord_type = expression_type(ops[3]);
			target_coord_type.basetype = SPIRType::Int;
			coord_expr = bitcast_expression(target_coord_type, expression_type(ops[3]).basetype, coord_expr);

			// Plain image load/store.
			if (type.image.ms)
			{
				uint32_t operands = ops[4];
				if (operands != ImageOperandsSampleMask || length != 6)
					SPIRV_CROSS_THROW("Multisampled image used in OpImageRead, but unexpected operand mask was used.");

				uint32_t samples = ops[5];
				imgexpr =
				    join("imageLoad(", to_expression(ops[2]), ", ", coord_expr, ", ", to_expression(samples), ")");
			}
			else
				imgexpr = join("imageLoad(", to_expression(ops[2]), ", ", coord_expr, ")");

			imgexpr = remap_swizzle(get<SPIRType>(result_type), 4, imgexpr);
			pure = false;
		}

		if (var && var->forwardable)
		{
			bool forward = forced_temporaries.find(id) == end(forced_temporaries);
			auto &e = emit_op(result_type, id, imgexpr, forward);

			// We only need to track dependencies if we're reading from image load/store.
			if (!pure)
			{
				e.loaded_from = var->self;
				if (forward)
					var->dependees.push_back(id);
			}
		}
		else
			emit_op(result_type, id, imgexpr, false);

		inherit_expression_dependencies(id, ops[2]);
		if (type.image.ms)
			inherit_expression_dependencies(id, ops[5]);
		break;
	}

	case OpImageTexelPointer:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		auto &e = set<SPIRExpression>(id, join(to_expression(ops[2]), ", ", to_expression(ops[3])), result_type, true);

		// When using the pointer, we need to know which variable it is actually loaded from.
		auto *var = maybe_get_backing_variable(ops[2]);
		e.loaded_from = var ? var->self : 0;
		break;
	}

	case OpImageWrite:
	{
		// We added Nonwritable speculatively to the OpImage variable due to glslangValidator
		// not adding the proper qualifiers.
		// If it turns out we need to write to the image after all, remove the qualifier and recompile.
		auto *var = maybe_get_backing_variable(ops[0]);
		if (var)
		{
			auto &flags = meta.at(var->self).decoration.decoration_flags;
			if (flags.get(DecorationNonWritable))
			{
				flags.clear(DecorationNonWritable);
				force_recompile = true;
			}
		}

		auto &type = expression_type(ops[0]);
		auto &value_type = expression_type(ops[2]);
		auto store_type = value_type;
		store_type.vecsize = 4;

		// imageStore only accepts int coords, not uint.
		auto coord_expr = to_expression(ops[1]);
		auto target_coord_type = expression_type(ops[1]);
		target_coord_type.basetype = SPIRType::Int;
		coord_expr = bitcast_expression(target_coord_type, expression_type(ops[1]).basetype, coord_expr);

		if (type.image.ms)
		{
			uint32_t operands = ops[3];
			if (operands != ImageOperandsSampleMask || length != 5)
				SPIRV_CROSS_THROW("Multisampled image used in OpImageWrite, but unexpected operand mask was used.");
			uint32_t samples = ops[4];
			statement("imageStore(", to_expression(ops[0]), ", ", coord_expr, ", ", to_expression(samples), ", ",
			          remap_swizzle(store_type, value_type.vecsize, to_expression(ops[2])), ");");
		}
		else
			statement("imageStore(", to_expression(ops[0]), ", ", coord_expr, ", ",
			          remap_swizzle(store_type, value_type.vecsize, to_expression(ops[2])), ");");

		if (var && variable_storage_is_aliased(*var))
			flush_all_aliased_variables();
		break;
	}

	case OpImageQuerySize:
	{
		auto &type = expression_type(ops[2]);
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];

		if (type.basetype == SPIRType::Image)
		{
			string expr;
			if (type.image.sampled == 2)
			{
				// The size of an image is always constant.
				expr = join("imageSize(", to_expression(ops[2]), ")");
			}
			else
			{
				// This path is hit for samplerBuffers and multisampled images which do not have LOD.
				expr = join("textureSize(", convert_separate_image_to_combined(ops[2]), ")");
			}

			auto &restype = get<SPIRType>(ops[0]);
			expr = bitcast_expression(restype, SPIRType::Int, expr);
			emit_op(result_type, id, expr, true);
		}
		else
			SPIRV_CROSS_THROW("Invalid type for OpImageQuerySize.");
		break;
	}

	// Compute
	case OpControlBarrier:
	case OpMemoryBarrier:
	{
		uint32_t execution_scope = 0;
		uint32_t memory;
		uint32_t semantics;

		if (opcode == OpMemoryBarrier)
		{
			memory = get<SPIRConstant>(ops[0]).scalar();
			semantics = get<SPIRConstant>(ops[1]).scalar();
		}
		else
		{
			execution_scope = get<SPIRConstant>(ops[0]).scalar();
			memory = get<SPIRConstant>(ops[1]).scalar();
			semantics = get<SPIRConstant>(ops[2]).scalar();
		}

		if (execution_scope == ScopeSubgroup || memory == ScopeSubgroup)
		{
			if (!options.vulkan_semantics)
				SPIRV_CROSS_THROW("Can only use subgroup operations in Vulkan semantics.");
			require_extension_internal("GL_KHR_shader_subgroup_basic");
		}

		if (execution_scope != ScopeSubgroup && get_entry_point().model == ExecutionModelTessellationControl)
		{
			// Control shaders only have barriers, and it implies memory barriers.
			if (opcode == OpControlBarrier)
				statement("barrier();");
			break;
		}

		// We only care about these flags, acquire/release and friends are not relevant to GLSL.
		semantics = mask_relevant_memory_semantics(semantics);

		if (opcode == OpMemoryBarrier)
		{
			// If we are a memory barrier, and the next instruction is a control barrier, check if that memory barrier
			// does what we need, so we avoid redundant barriers.
			const Instruction *next = get_next_instruction_in_block(instruction);
			if (next && next->op == OpControlBarrier)
			{
				auto *next_ops = stream(*next);
				uint32_t next_memory = get<SPIRConstant>(next_ops[1]).scalar();
				uint32_t next_semantics = get<SPIRConstant>(next_ops[2]).scalar();
				next_semantics = mask_relevant_memory_semantics(next_semantics);

				bool memory_scope_covered = false;
				if (next_memory == memory)
					memory_scope_covered = true;
				else if (next_semantics == MemorySemanticsWorkgroupMemoryMask)
				{
					// If we only care about workgroup memory, either Device or Workgroup scope is fine,
					// scope does not have to match.
					if ((next_memory == ScopeDevice || next_memory == ScopeWorkgroup) &&
					    (memory == ScopeDevice || memory == ScopeWorkgroup))
					{
						memory_scope_covered = true;
					}
				}
				else if (memory == ScopeWorkgroup && next_memory == ScopeDevice)
				{
					// The control barrier has device scope, but the memory barrier just has workgroup scope.
					memory_scope_covered = true;
				}

				// If we have the same memory scope, and all memory types are covered, we're good.
				if (memory_scope_covered && (semantics & next_semantics) == semantics)
					break;
			}
		}

		// We are synchronizing some memory or syncing execution,
		// so we cannot forward any loads beyond the memory barrier.
		if (semantics || opcode == OpControlBarrier)
		{
			assert(current_emitting_block);
			flush_control_dependent_expressions(current_emitting_block->self);
			flush_all_active_variables();
		}

		if (memory == ScopeWorkgroup) // Only need to consider memory within a group
		{
			if (semantics == MemorySemanticsWorkgroupMemoryMask)
				statement("memoryBarrierShared();");
			else if (semantics != 0)
				statement("groupMemoryBarrier();");
		}
		else if (memory == ScopeSubgroup)
		{
			const uint32_t all_barriers =
			    MemorySemanticsWorkgroupMemoryMask | MemorySemanticsUniformMemoryMask | MemorySemanticsImageMemoryMask;

			if (semantics & (MemorySemanticsCrossWorkgroupMemoryMask | MemorySemanticsSubgroupMemoryMask))
			{
				// These are not relevant for GLSL, but assume it means memoryBarrier().
				// memoryBarrier() does everything, so no need to test anything else.
				statement("subgroupMemoryBarrier();");
			}
			else if ((semantics & all_barriers) == all_barriers)
			{
				// Short-hand instead of emitting 3 barriers.
				statement("subgroupMemoryBarrier();");
			}
			else
			{
				// Pick out individual barriers.
				if (semantics & MemorySemanticsWorkgroupMemoryMask)
					statement("subgroupMemoryBarrierShared();");
				if (semantics & MemorySemanticsUniformMemoryMask)
					statement("subgroupMemoryBarrierBuffer();");
				if (semantics & MemorySemanticsImageMemoryMask)
					statement("subgroupMemoryBarrierImage();");
			}
		}
		else
		{
			const uint32_t all_barriers = MemorySemanticsWorkgroupMemoryMask | MemorySemanticsUniformMemoryMask |
			                              MemorySemanticsImageMemoryMask | MemorySemanticsAtomicCounterMemoryMask;

			if (semantics & (MemorySemanticsCrossWorkgroupMemoryMask | MemorySemanticsSubgroupMemoryMask))
			{
				// These are not relevant for GLSL, but assume it means memoryBarrier().
				// memoryBarrier() does everything, so no need to test anything else.
				statement("memoryBarrier();");
			}
			else if ((semantics & all_barriers) == all_barriers)
			{
				// Short-hand instead of emitting 4 barriers.
				statement("memoryBarrier();");
			}
			else
			{
				// Pick out individual barriers.
				if (semantics & MemorySemanticsWorkgroupMemoryMask)
					statement("memoryBarrierShared();");
				if (semantics & MemorySemanticsUniformMemoryMask)
					statement("memoryBarrierBuffer();");
				if (semantics & MemorySemanticsImageMemoryMask)
					statement("memoryBarrierImage();");
				if (semantics & MemorySemanticsAtomicCounterMemoryMask)
					statement("memoryBarrierAtomicCounter();");
			}
		}

		if (opcode == OpControlBarrier)
		{
			if (execution_scope == ScopeSubgroup)
				statement("subgroupBarrier();");
			else
				statement("barrier();");
		}
		break;
	}

	case OpExtInst:
	{
		uint32_t extension_set = ops[2];

		if (get<SPIRExtension>(extension_set).ext == SPIRExtension::GLSL)
		{
			emit_glsl_op(ops[0], ops[1], ops[3], &ops[4], length - 4);
		}
		else if (get<SPIRExtension>(extension_set).ext == SPIRExtension::SPV_AMD_shader_ballot)
		{
			emit_spv_amd_shader_ballot_op(ops[0], ops[1], ops[3], &ops[4], length - 4);
		}
		else if (get<SPIRExtension>(extension_set).ext == SPIRExtension::SPV_AMD_shader_explicit_vertex_parameter)
		{
			emit_spv_amd_shader_explicit_vertex_parameter_op(ops[0], ops[1], ops[3], &ops[4], length - 4);
		}
		else if (get<SPIRExtension>(extension_set).ext == SPIRExtension::SPV_AMD_shader_trinary_minmax)
		{
			emit_spv_amd_shader_trinary_minmax_op(ops[0], ops[1], ops[3], &ops[4], length - 4);
		}
		else if (get<SPIRExtension>(extension_set).ext == SPIRExtension::SPV_AMD_gcn_shader)
		{
			emit_spv_amd_gcn_shader_op(ops[0], ops[1], ops[3], &ops[4], length - 4);
		}
		else
		{
			statement("// unimplemented ext op ", instruction.op);
			break;
		}

		break;
	}

	// Legacy sub-group stuff ...
	case OpSubgroupBallotKHR:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		string expr;
		expr = join("uvec4(unpackUint2x32(ballotARB(" + to_expression(ops[2]) + ")), 0u, 0u)");
		emit_op(result_type, id, expr, should_forward(ops[2]));

		require_extension_internal("GL_ARB_shader_ballot");
		inherit_expression_dependencies(id, ops[2]);
		register_control_dependent_expression(ops[1]);
		break;
	}

	case OpSubgroupFirstInvocationKHR:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		emit_unary_func_op(result_type, id, ops[2], "readFirstInvocationARB");

		require_extension_internal("GL_ARB_shader_ballot");
		register_control_dependent_expression(ops[1]);
		break;
	}

	case OpSubgroupReadInvocationKHR:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		emit_binary_func_op(result_type, id, ops[2], ops[3], "readInvocationARB");

		require_extension_internal("GL_ARB_shader_ballot");
		register_control_dependent_expression(ops[1]);
		break;
	}

	case OpSubgroupAllKHR:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		emit_unary_func_op(result_type, id, ops[2], "allInvocationsARB");

		require_extension_internal("GL_ARB_shader_group_vote");
		register_control_dependent_expression(ops[1]);
		break;
	}

	case OpSubgroupAnyKHR:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		emit_unary_func_op(result_type, id, ops[2], "anyInvocationARB");

		require_extension_internal("GL_ARB_shader_group_vote");
		register_control_dependent_expression(ops[1]);
		break;
	}

	case OpSubgroupAllEqualKHR:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		emit_unary_func_op(result_type, id, ops[2], "allInvocationsEqualARB");

		require_extension_internal("GL_ARB_shader_group_vote");
		register_control_dependent_expression(ops[1]);
		break;
	}

	case OpGroupIAddNonUniformAMD:
	case OpGroupFAddNonUniformAMD:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		emit_unary_func_op(result_type, id, ops[4], "addInvocationsNonUniformAMD");

		require_extension_internal("GL_AMD_shader_ballot");
		register_control_dependent_expression(ops[1]);
		break;
	}

	case OpGroupFMinNonUniformAMD:
	case OpGroupUMinNonUniformAMD:
	case OpGroupSMinNonUniformAMD:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		emit_unary_func_op(result_type, id, ops[4], "minInvocationsNonUniformAMD");

		require_extension_internal("GL_AMD_shader_ballot");
		register_control_dependent_expression(ops[1]);
		break;
	}

	case OpGroupFMaxNonUniformAMD:
	case OpGroupUMaxNonUniformAMD:
	case OpGroupSMaxNonUniformAMD:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		emit_unary_func_op(result_type, id, ops[4], "maxInvocationsNonUniformAMD");

		require_extension_internal("GL_AMD_shader_ballot");
		register_control_dependent_expression(ops[1]);
		break;
	}

	case OpFragmentMaskFetchAMD:
	{
		auto &type = expression_type(ops[2]);
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];

		if (type.image.dim == spv::DimSubpassData)
		{
			emit_unary_func_op(result_type, id, ops[2], "fragmentMaskFetchAMD");
		}
		else
		{
			emit_binary_func_op(result_type, id, ops[2], ops[3], "fragmentMaskFetchAMD");
		}

		require_extension_internal("GL_AMD_shader_fragment_mask");
		break;
	}

	case OpFragmentFetchAMD:
	{
		auto &type = expression_type(ops[2]);
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];

		if (type.image.dim == spv::DimSubpassData)
		{
			emit_binary_func_op(result_type, id, ops[2], ops[4], "fragmentFetchAMD");
		}
		else
		{
			emit_trinary_func_op(result_type, id, ops[2], ops[3], ops[4], "fragmentFetchAMD");
		}

		require_extension_internal("GL_AMD_shader_fragment_mask");
		break;
	}

	// Vulkan 1.1 sub-group stuff ...
	case OpGroupNonUniformElect:
	case OpGroupNonUniformBroadcast:
	case OpGroupNonUniformBroadcastFirst:
	case OpGroupNonUniformBallot:
	case OpGroupNonUniformInverseBallot:
	case OpGroupNonUniformBallotBitExtract:
	case OpGroupNonUniformBallotBitCount:
	case OpGroupNonUniformBallotFindLSB:
	case OpGroupNonUniformBallotFindMSB:
	case OpGroupNonUniformShuffle:
	case OpGroupNonUniformShuffleXor:
	case OpGroupNonUniformShuffleUp:
	case OpGroupNonUniformShuffleDown:
	case OpGroupNonUniformAll:
	case OpGroupNonUniformAny:
	case OpGroupNonUniformAllEqual:
	case OpGroupNonUniformFAdd:
	case OpGroupNonUniformIAdd:
	case OpGroupNonUniformFMul:
	case OpGroupNonUniformIMul:
	case OpGroupNonUniformFMin:
	case OpGroupNonUniformFMax:
	case OpGroupNonUniformSMin:
	case OpGroupNonUniformSMax:
	case OpGroupNonUniformUMin:
	case OpGroupNonUniformUMax:
	case OpGroupNonUniformBitwiseAnd:
	case OpGroupNonUniformBitwiseOr:
	case OpGroupNonUniformBitwiseXor:
	case OpGroupNonUniformQuadSwap:
	case OpGroupNonUniformQuadBroadcast:
		emit_subgroup_op(instruction);
		break;

	case OpFUnordEqual:
		GLSL_BFOP(unsupported_FUnordEqual);
		break;

	case OpFUnordNotEqual:
		GLSL_BFOP(unsupported_FUnordNotEqual);
		break;

	case OpFUnordLessThan:
		GLSL_BFOP(unsupported_FUnordLessThan);
		break;

	case OpFUnordGreaterThan:
		GLSL_BFOP(unsupported_FUnordGreaterThan);
		break;

	case OpFUnordLessThanEqual:
		GLSL_BFOP(unsupported_FUnordLessThanEqual);
		break;

	case OpFUnordGreaterThanEqual:
		GLSL_BFOP(unsupported_FUnordGreaterThanEqual);
		break;

	default:
		statement("// unimplemented op ", instruction.op);
		break;
	}
}

// Appends function arguments, mapped from global variables, beyond the specified arg index.
// This is used when a function call uses fewer arguments than the function defines.
// This situation may occur if the function signature has been dynamically modified to
// extract global variables referenced from within the function, and convert them to
// function arguments. This is necessary for shader languages that do not support global
// access to shader input content from within a function (eg. Metal). Each additional
// function args uses the name of the global variable. Function nesting will modify the
// functions and function calls all the way up the nesting chain.
void CompilerGLSL::append_global_func_args(const SPIRFunction &func, uint32_t index, vector<string> &arglist)
{
	auto &args = func.arguments;
	uint32_t arg_cnt = uint32_t(args.size());
	for (uint32_t arg_idx = index; arg_idx < arg_cnt; arg_idx++)
	{
		auto &arg = args[arg_idx];
		assert(arg.alias_global_variable);
		arglist.push_back(to_func_call_arg(arg.id));

		// If the underlying variable needs to be declared
		// (ie. a local variable with deferred declaration), do so now.
		uint32_t var_id = get<SPIRVariable>(arg.id).basevariable;
		if (var_id)
			flush_variable_declaration(var_id);
	}
}

string CompilerGLSL::to_member_name(const SPIRType &type, uint32_t index)
{
	auto &memb = meta[type.self].members;
	if (index < memb.size() && !memb[index].alias.empty())
		return memb[index].alias;
	else
		return join("_m", index);
}

void CompilerGLSL::add_member_name(SPIRType &type, uint32_t index)
{
	auto &memb = meta[type.self].members;
	if (index < memb.size() && !memb[index].alias.empty())
	{
		auto &name = memb[index].alias;
		if (name.empty())
			return;

		// Reserved for temporaries.
		if (name[0] == '_' && name.size() >= 2 && isdigit(name[1]))
		{
			name.clear();
			return;
		}

		update_name_cache(type.member_name_cache, name);
	}
}

// Checks whether the ID is a row_major matrix that requires conversion before use
bool CompilerGLSL::is_non_native_row_major_matrix(uint32_t id)
{
	// Natively supported row-major matrices do not need to be converted.
	// Legacy targets do not support row major.
	if (backend.native_row_major_matrix && !is_legacy())
		return false;

	// Non-matrix or column-major matrix types do not need to be converted.
	if (!meta[id].decoration.decoration_flags.get(DecorationRowMajor))
		return false;

	// Only square row-major matrices can be converted at this time.
	// Converting non-square matrices will require defining custom GLSL function that
	// swaps matrix elements while retaining the original dimensional form of the matrix.
	const auto type = expression_type(id);
	if (type.columns != type.vecsize)
		SPIRV_CROSS_THROW("Row-major matrices must be square on this platform.");

	return true;
}

// Checks whether the member is a row_major matrix that requires conversion before use
bool CompilerGLSL::member_is_non_native_row_major_matrix(const SPIRType &type, uint32_t index)
{
	// Natively supported row-major matrices do not need to be converted.
	if (backend.native_row_major_matrix && !is_legacy())
		return false;

	// Non-matrix or column-major matrix types do not need to be converted.
	if (!combined_decoration_for_member(type, index).get(DecorationRowMajor))
		return false;

	// Only square row-major matrices can be converted at this time.
	// Converting non-square matrices will require defining custom GLSL function that
	// swaps matrix elements while retaining the original dimensional form of the matrix.
	const auto mbr_type = get<SPIRType>(type.member_types[index]);
	if (mbr_type.columns != mbr_type.vecsize)
		SPIRV_CROSS_THROW("Row-major matrices must be square on this platform.");

	return true;
}

// Checks whether the member is in packed data type, that might need to be unpacked.
// GLSL does not define packed data types, but certain subclasses do.
bool CompilerGLSL::member_is_packed_type(const SPIRType &type, uint32_t index) const
{
	return has_member_decoration(type.self, index, DecorationCPacked);
}

// Wraps the expression string in a function call that converts the
// row_major matrix result of the expression to a column_major matrix.
// Base implementation uses the standard library transpose() function.
// Subclasses may override to use a different function.
string CompilerGLSL::convert_row_major_matrix(string exp_str, const SPIRType & /*exp_type*/, bool /*is_packed*/)
{
	strip_enclosed_expression(exp_str);
	return join("transpose(", exp_str, ")");
}

string CompilerGLSL::variable_decl(const SPIRType &type, const string &name, uint32_t id)
{
	string type_name = type_to_glsl(type, id);
	remap_variable_type_name(type, name, type_name);
	return join(type_name, " ", name, type_to_array_glsl(type));
}

// Emit a structure member. Subclasses may override to modify output,
// or to dynamically add a padding member if needed.
void CompilerGLSL::emit_struct_member(const SPIRType &type, uint32_t member_type_id, uint32_t index,
                                      const string &qualifier, uint32_t)
{
	auto &membertype = get<SPIRType>(member_type_id);

	Bitset memberflags;
	auto &memb = meta[type.self].members;
	if (index < memb.size())
		memberflags = memb[index].decoration_flags;

	string qualifiers;
	bool is_block = meta[type.self].decoration.decoration_flags.get(DecorationBlock) ||
	                meta[type.self].decoration.decoration_flags.get(DecorationBufferBlock);

	if (is_block)
		qualifiers = to_interpolation_qualifiers(memberflags);

	statement(layout_for_member(type, index), qualifiers, qualifier,
	          flags_to_precision_qualifiers_glsl(membertype, memberflags),
	          variable_decl(membertype, to_member_name(type, index)), ";");
}

const char *CompilerGLSL::flags_to_precision_qualifiers_glsl(const SPIRType &type, const Bitset &flags)
{
	// Structs do not have precision qualifiers, neither do doubles (desktop only anyways, so no mediump/highp).
	if (type.basetype != SPIRType::Float && type.basetype != SPIRType::Int && type.basetype != SPIRType::UInt &&
	    type.basetype != SPIRType::Image && type.basetype != SPIRType::SampledImage &&
	    type.basetype != SPIRType::Sampler)
		return "";

	if (options.es)
	{
		auto &execution = get_entry_point();

		if (flags.get(DecorationRelaxedPrecision))
		{
			bool implied_fmediump = type.basetype == SPIRType::Float &&
			                        options.fragment.default_float_precision == Options::Mediump &&
			                        execution.model == ExecutionModelFragment;

			bool implied_imediump = (type.basetype == SPIRType::Int || type.basetype == SPIRType::UInt) &&
			                        options.fragment.default_int_precision == Options::Mediump &&
			                        execution.model == ExecutionModelFragment;

			return implied_fmediump || implied_imediump ? "" : "mediump ";
		}
		else
		{
			bool implied_fhighp =
			    type.basetype == SPIRType::Float && ((options.fragment.default_float_precision == Options::Highp &&
			                                          execution.model == ExecutionModelFragment) ||
			                                         (execution.model != ExecutionModelFragment));

			bool implied_ihighp = (type.basetype == SPIRType::Int || type.basetype == SPIRType::UInt) &&
			                      ((options.fragment.default_int_precision == Options::Highp &&
			                        execution.model == ExecutionModelFragment) ||
			                       (execution.model != ExecutionModelFragment));

			return implied_fhighp || implied_ihighp ? "" : "highp ";
		}
	}
	else if (backend.allow_precision_qualifiers)
	{
		// Vulkan GLSL supports precision qualifiers, even in desktop profiles, which is convenient.
		// The default is highp however, so only emit mediump in the rare case that a shader has these.
		if (flags.get(DecorationRelaxedPrecision))
			return "mediump ";
		else
			return "";
	}
	else
		return "";
}

const char *CompilerGLSL::to_precision_qualifiers_glsl(uint32_t id)
{
	return flags_to_precision_qualifiers_glsl(expression_type(id), meta[id].decoration.decoration_flags);
}

string CompilerGLSL::to_qualifiers_glsl(uint32_t id)
{
	auto flags = meta[id].decoration.decoration_flags;
	string res;

	auto *var = maybe_get<SPIRVariable>(id);

	if (var && var->storage == StorageClassWorkgroup && !backend.shared_is_implied)
		res += "shared ";

	res += to_interpolation_qualifiers(flags);
	if (var)
		res += to_storage_qualifiers_glsl(*var);

	auto &type = expression_type(id);
	if (type.image.dim != DimSubpassData && type.image.sampled == 2)
	{
		if (flags.get(DecorationCoherent))
			res += "coherent ";
		if (flags.get(DecorationRestrict))
			res += "restrict ";
		if (flags.get(DecorationNonWritable))
			res += "readonly ";
		if (flags.get(DecorationNonReadable))
			res += "writeonly ";
	}

	res += to_precision_qualifiers_glsl(id);

	return res;
}

string CompilerGLSL::argument_decl(const SPIRFunction::Parameter &arg)
{
	// glslangValidator seems to make all arguments pointer no matter what which is rather bizarre ...
	auto &type = expression_type(arg.id);
	const char *direction = "";

	if (type.pointer)
	{
		if (arg.write_count && arg.read_count)
			direction = "inout ";
		else if (arg.write_count)
			direction = "out ";
	}

	return join(direction, to_qualifiers_glsl(arg.id), variable_decl(type, to_name(arg.id), arg.id));
}

string CompilerGLSL::to_initializer_expression(const SPIRVariable &var)
{
	return to_expression(var.initializer);
}

string CompilerGLSL::variable_decl(const SPIRVariable &variable)
{
	// Ignore the pointer type since GLSL doesn't have pointers.
	auto &type = get<SPIRType>(variable.basetype);

	auto res = join(to_qualifiers_glsl(variable.self), variable_decl(type, to_name(variable.self), variable.self));

	if (variable.loop_variable && variable.static_expression)
	{
		uint32_t expr = variable.static_expression;
		if (ids[expr].get_type() != TypeUndef)
			res += join(" = ", to_expression(variable.static_expression));
	}
	else if (variable.initializer)
	{
		uint32_t expr = variable.initializer;
		if (ids[expr].get_type() != TypeUndef)
			res += join(" = ", to_initializer_expression(variable));
	}
	return res;
}

const char *CompilerGLSL::to_pls_qualifiers_glsl(const SPIRVariable &variable)
{
	auto flags = meta[variable.self].decoration.decoration_flags;
	if (flags.get(DecorationRelaxedPrecision))
		return "mediump ";
	else
		return "highp ";
}

string CompilerGLSL::pls_decl(const PlsRemap &var)
{
	auto &variable = get<SPIRVariable>(var.id);

	SPIRType type;
	type.vecsize = pls_format_to_components(var.format);
	type.basetype = pls_format_to_basetype(var.format);

	return join(to_pls_layout(var.format), to_pls_qualifiers_glsl(variable), type_to_glsl(type), " ",
	            to_name(variable.self));
}

uint32_t CompilerGLSL::to_array_size_literal(const SPIRType &type, uint32_t index) const
{
	assert(type.array.size() == type.array_size_literal.size());

	if (type.array_size_literal[index])
	{
		return type.array[index];
	}
	else
	{
		// Use the default spec constant value.
		// This is the best we can do.
		uint32_t array_size_id = type.array[index];
		uint32_t array_size = get<SPIRConstant>(array_size_id).scalar();
		return array_size;
	}
}

string CompilerGLSL::to_array_size(const SPIRType &type, uint32_t index)
{
	assert(type.array.size() == type.array_size_literal.size());

	// Tessellation control shaders must have either gl_MaxPatchVertices or unsized arrays for input arrays.
	// Opt for unsized as it's the more "correct" variant to use.
	if (type.storage == StorageClassInput && get_entry_point().model == ExecutionModelTessellationControl)
		return "";

	auto &size = type.array[index];
	if (!type.array_size_literal[index])
		return to_expression(size);
	else if (size)
		return convert_to_string(size);
	else if (!backend.flexible_member_array_supported)
	{
		// For runtime-sized arrays, we can work around
		// lack of standard support for this by simply having
		// a single element array.
		//
		// Runtime length arrays must always be the last element
		// in an interface block.
		return "1";
	}
	else
		return "";
}

string CompilerGLSL::type_to_array_glsl(const SPIRType &type)
{
	if (type.array.empty())
		return "";

	if (options.flatten_multidimensional_arrays)
	{
		string res;
		res += "[";
		for (auto i = uint32_t(type.array.size()); i; i--)
		{
			res += enclose_expression(to_array_size(type, i - 1));
			if (i > 1)
				res += " * ";
		}
		res += "]";
		return res;
	}
	else
	{
		if (type.array.size() > 1)
		{
			if (!options.es && options.version < 430)
				require_extension_internal("GL_ARB_arrays_of_arrays");
			else if (options.es && options.version < 310)
				SPIRV_CROSS_THROW("Arrays of arrays not supported before ESSL version 310. "
				                  "Try using --flatten-multidimensional-arrays or set "
				                  "options.flatten_multidimensional_arrays to true.");
		}

		string res;
		for (auto i = uint32_t(type.array.size()); i; i--)
		{
			res += "[";
			res += to_array_size(type, i - 1);
			res += "]";
		}
		return res;
	}
}

string CompilerGLSL::image_type_glsl(const SPIRType &type, uint32_t id)
{
	auto &imagetype = get<SPIRType>(type.image.type);
	string res;

	switch (imagetype.basetype)
	{
	case SPIRType::Int:
		res = "i";
		break;
	case SPIRType::UInt:
		res = "u";
		break;
	default:
		break;
	}

	if (type.basetype == SPIRType::Image && type.image.dim == DimSubpassData && options.vulkan_semantics)
		return res + "subpassInput" + (type.image.ms ? "MS" : "");

	// If we're emulating subpassInput with samplers, force sampler2D
	// so we don't have to specify format.
	if (type.basetype == SPIRType::Image && type.image.dim != DimSubpassData)
	{
		// Sampler buffers are always declared as samplerBuffer even though they might be separate images in the SPIR-V.
		if (type.image.dim == DimBuffer && type.image.sampled == 1)
			res += "sampler";
		else
			res += type.image.sampled == 2 ? "image" : "texture";
	}
	else
		res += "sampler";

	switch (type.image.dim)
	{
	case Dim1D:
		res += "1D";
		break;
	case Dim2D:
		res += "2D";
		break;
	case Dim3D:
		res += "3D";
		break;
	case DimCube:
		res += "Cube";
		break;

	case DimBuffer:
		if (options.es && options.version < 320)
			require_extension_internal("GL_OES_texture_buffer");
		else if (!options.es && options.version < 300)
			require_extension_internal("GL_EXT_texture_buffer_object");
		res += "Buffer";
		break;

	case DimSubpassData:
		res += "2D";
		break;
	default:
		SPIRV_CROSS_THROW("Only 1D, 2D, 3D, Buffer, InputTarget and Cube textures supported.");
	}

	if (type.image.ms)
		res += "MS";
	if (type.image.arrayed)
	{
		if (is_legacy_desktop())
			require_extension_internal("GL_EXT_texture_array");
		res += "Array";
	}

	// "Shadow" state in GLSL only exists for samplers and combined image samplers.
	if (((type.basetype == SPIRType::SampledImage) || (type.basetype == SPIRType::Sampler)) &&
	    image_is_comparison(type, id))
	{
		res += "Shadow";
	}

	return res;
}

string CompilerGLSL::type_to_glsl_constructor(const SPIRType &type)
{
	if (type.array.size() > 1)
	{
		if (options.flatten_multidimensional_arrays)
			SPIRV_CROSS_THROW("Cannot flatten constructors of multidimensional array constructors, e.g. float[][]().");
		else if (!options.es && options.version < 430)
			require_extension_internal("GL_ARB_arrays_of_arrays");
		else if (options.es && options.version < 310)
			SPIRV_CROSS_THROW("Arrays of arrays not supported before ESSL version 310.");
	}

	auto e = type_to_glsl(type);
	for (uint32_t i = 0; i < type.array.size(); i++)
		e += "[]";
	return e;
}

// The optional id parameter indicates the object whose type we are trying
// to find the description for. It is optional. Most type descriptions do not
// depend on a specific object's use of that type.
string CompilerGLSL::type_to_glsl(const SPIRType &type, uint32_t id)
{
	// Ignore the pointer type since GLSL doesn't have pointers.

	switch (type.basetype)
	{
	case SPIRType::Struct:
		// Need OpName lookup here to get a "sensible" name for a struct.
		if (backend.explicit_struct_type)
			return join("struct ", to_name(type.self));
		else
			return to_name(type.self);

	case SPIRType::Image:
	case SPIRType::SampledImage:
		return image_type_glsl(type, id);

	case SPIRType::Sampler:
		// The depth field is set by calling code based on the variable ID of the sampler, effectively reintroducing
		// this distinction into the type system.
		return comparison_ids.count(id) ? "samplerShadow" : "sampler";

	case SPIRType::Void:
		return "void";

	default:
		break;
	}

	if (type.basetype == SPIRType::UInt && is_legacy())
		SPIRV_CROSS_THROW("Unsigned integers are not supported on legacy targets.");

	if (type.vecsize == 1 && type.columns == 1) // Scalar builtin
	{
		switch (type.basetype)
		{
		case SPIRType::Boolean:
			return "bool";
		case SPIRType::Int:
			return backend.basic_int_type;
		case SPIRType::UInt:
			return backend.basic_uint_type;
		case SPIRType::AtomicCounter:
			return "atomic_uint";
		case SPIRType::Half:
			return "float16_t";
		case SPIRType::Float:
			return "float";
		case SPIRType::Double:
			return "double";
		case SPIRType::Int64:
			return "int64_t";
		case SPIRType::UInt64:
			return "uint64_t";
		default:
			return "???";
		}
	}
	else if (type.vecsize > 1 && type.columns == 1) // Vector builtin
	{
		switch (type.basetype)
		{
		case SPIRType::Boolean:
			return join("bvec", type.vecsize);
		case SPIRType::Int:
			return join("ivec", type.vecsize);
		case SPIRType::UInt:
			return join("uvec", type.vecsize);
		case SPIRType::Half:
			return join("f16vec", type.vecsize);
		case SPIRType::Float:
			return join("vec", type.vecsize);
		case SPIRType::Double:
			return join("dvec", type.vecsize);
		case SPIRType::Int64:
			return join("i64vec", type.vecsize);
		case SPIRType::UInt64:
			return join("u64vec", type.vecsize);
		default:
			return "???";
		}
	}
	else if (type.vecsize == type.columns) // Simple Matrix builtin
	{
		switch (type.basetype)
		{
		case SPIRType::Boolean:
			return join("bmat", type.vecsize);
		case SPIRType::Int:
			return join("imat", type.vecsize);
		case SPIRType::UInt:
			return join("umat", type.vecsize);
		case SPIRType::Half:
			return join("f16mat", type.vecsize);
		case SPIRType::Float:
			return join("mat", type.vecsize);
		case SPIRType::Double:
			return join("dmat", type.vecsize);
		// Matrix types not supported for int64/uint64.
		default:
			return "???";
		}
	}
	else
	{
		switch (type.basetype)
		{
		case SPIRType::Boolean:
			return join("bmat", type.columns, "x", type.vecsize);
		case SPIRType::Int:
			return join("imat", type.columns, "x", type.vecsize);
		case SPIRType::UInt:
			return join("umat", type.columns, "x", type.vecsize);
		case SPIRType::Half:
			return join("f16mat", type.columns, "x", type.vecsize);
		case SPIRType::Float:
			return join("mat", type.columns, "x", type.vecsize);
		case SPIRType::Double:
			return join("dmat", type.columns, "x", type.vecsize);
		// Matrix types not supported for int64/uint64.
		default:
			return "???";
		}
	}
}

void CompilerGLSL::add_variable(unordered_set<string> &variables, string &name)
{
	if (name.empty())
		return;

	// Reserved for temporaries.
	if (name[0] == '_' && name.size() >= 2 && isdigit(name[1]))
	{
		name.clear();
		return;
	}

	// Avoid double underscores.
	name = sanitize_underscores(name);

	update_name_cache(variables, name);
}

void CompilerGLSL::add_variable(unordered_set<string> &variables, uint32_t id)
{
	auto &name = meta[id].decoration.alias;
	add_variable(variables, name);
}

void CompilerGLSL::add_local_variable_name(uint32_t id)
{
	add_variable(local_variable_names, id);
}

void CompilerGLSL::add_resource_name(uint32_t id)
{
	add_variable(resource_names, id);
}

void CompilerGLSL::add_header_line(const std::string &line)
{
	header_lines.push_back(line);
}

bool CompilerGLSL::has_extension(const std::string &ext) const
{
	auto itr = find(begin(forced_extensions), end(forced_extensions), ext);
	return itr != end(forced_extensions);
}

void CompilerGLSL::require_extension(const std::string &ext)
{
	if (!has_extension(ext))
		forced_extensions.push_back(ext);
}

void CompilerGLSL::require_extension_internal(const string &ext)
{
	if (backend.supports_extensions && !has_extension(ext))
	{
		forced_extensions.push_back(ext);
		force_recompile = true;
	}
}

void CompilerGLSL::flatten_buffer_block(uint32_t id)
{
	auto &var = get<SPIRVariable>(id);
	auto &type = get<SPIRType>(var.basetype);
	auto name = to_name(type.self, false);
	auto flags = meta.at(type.self).decoration.decoration_flags;

	if (!type.array.empty())
		SPIRV_CROSS_THROW(name + " is an array of UBOs.");
	if (type.basetype != SPIRType::Struct)
		SPIRV_CROSS_THROW(name + " is not a struct.");
	if (!flags.get(DecorationBlock))
		SPIRV_CROSS_THROW(name + " is not a block.");
	if (type.member_types.empty())
		SPIRV_CROSS_THROW(name + " is an empty struct.");

	flattened_buffer_blocks.insert(id);
}

bool CompilerGLSL::check_atomic_image(uint32_t id)
{
	auto &type = expression_type(id);
	if (type.storage == StorageClassImage)
	{
		if (options.es && options.version < 320)
			require_extension_internal("GL_OES_shader_image_atomic");

		auto *var = maybe_get_backing_variable(id);
		if (var)
		{
			auto &flags = meta.at(var->self).decoration.decoration_flags;
			if (flags.get(DecorationNonWritable) || flags.get(DecorationNonReadable))
			{
				flags.clear(DecorationNonWritable);
				flags.clear(DecorationNonReadable);
				force_recompile = true;
			}
		}
		return true;
	}
	else
		return false;
}

void CompilerGLSL::add_function_overload(const SPIRFunction &func)
{
	Hasher hasher;
	for (auto &arg : func.arguments)
	{
		// Parameters can vary with pointer type or not,
		// but that will not change the signature in GLSL/HLSL,
		// so strip the pointer type before hashing.
		uint32_t type_id = get_non_pointer_type_id(arg.type);
		auto &type = get<SPIRType>(type_id);

		if (!combined_image_samplers.empty())
		{
			// If we have combined image samplers, we cannot really trust the image and sampler arguments
			// we pass down to callees, because they may be shuffled around.
			// Ignore these arguments, to make sure that functions need to differ in some other way
			// to be considered different overloads.
			if (type.basetype == SPIRType::SampledImage ||
			    (type.basetype == SPIRType::Image && type.image.sampled == 1) || type.basetype == SPIRType::Sampler)
			{
				continue;
			}
		}

		hasher.u32(type_id);
	}
	uint64_t types_hash = hasher.get();

	auto function_name = to_name(func.self);
	auto itr = function_overloads.find(function_name);
	if (itr != end(function_overloads))
	{
		// There exists a function with this name already.
		auto &overloads = itr->second;
		if (overloads.count(types_hash) != 0)
		{
			// Overload conflict, assign a new name.
			add_resource_name(func.self);
			function_overloads[to_name(func.self)].insert(types_hash);
		}
		else
		{
			// Can reuse the name.
			overloads.insert(types_hash);
		}
	}
	else
	{
		// First time we see this function name.
		add_resource_name(func.self);
		function_overloads[to_name(func.self)].insert(types_hash);
	}
}

void CompilerGLSL::emit_function_prototype(SPIRFunction &func, const Bitset &return_flags)
{
	if (func.self != entry_point)
		add_function_overload(func);

	// Avoid shadow declarations.
	local_variable_names = resource_names;

	string decl;

	auto &type = get<SPIRType>(func.return_type);
	decl += flags_to_precision_qualifiers_glsl(type, return_flags);
	decl += type_to_glsl(type);
	decl += type_to_array_glsl(type);
	decl += " ";

	if (func.self == entry_point)
	{
		decl += "main";
		processing_entry_point = true;
	}
	else
		decl += to_name(func.self);

	decl += "(";
	vector<string> arglist;
	for (auto &arg : func.arguments)
	{
		// Do not pass in separate images or samplers if we're remapping
		// to combined image samplers.
		if (skip_argument(arg.id))
			continue;

		// Might change the variable name if it already exists in this function.
		// SPIRV OpName doesn't have any semantic effect, so it's valid for an implementation
		// to use same name for variables.
		// Since we want to make the GLSL debuggable and somewhat sane, use fallback names for variables which are duplicates.
		add_local_variable_name(arg.id);

		arglist.push_back(argument_decl(arg));

		// Hold a pointer to the parameter so we can invalidate the readonly field if needed.
		auto *var = maybe_get<SPIRVariable>(arg.id);
		if (var)
			var->parameter = &arg;
	}

	for (auto &arg : func.shadow_arguments)
	{
		// Might change the variable name if it already exists in this function.
		// SPIRV OpName doesn't have any semantic effect, so it's valid for an implementation
		// to use same name for variables.
		// Since we want to make the GLSL debuggable and somewhat sane, use fallback names for variables which are duplicates.
		add_local_variable_name(arg.id);

		arglist.push_back(argument_decl(arg));

		// Hold a pointer to the parameter so we can invalidate the readonly field if needed.
		auto *var = maybe_get<SPIRVariable>(arg.id);
		if (var)
			var->parameter = &arg;
	}

	decl += merge(arglist);
	decl += ")";
	statement(decl);
}

void CompilerGLSL::emit_function(SPIRFunction &func, const Bitset &return_flags)
{
	// Avoid potential cycles.
	if (func.active)
		return;
	func.active = true;

	// If we depend on a function, emit that function before we emit our own function.
	for (auto block : func.blocks)
	{
		auto &b = get<SPIRBlock>(block);
		for (auto &i : b.ops)
		{
			auto ops = stream(i);
			auto op = static_cast<Op>(i.op);

			if (op == OpFunctionCall)
			{
				// Recursively emit functions which are called.
				uint32_t id = ops[2];
				emit_function(get<SPIRFunction>(id), meta[ops[1]].decoration.decoration_flags);
			}
		}
	}

	emit_function_prototype(func, return_flags);
	begin_scope();

	if (func.self == entry_point)
		emit_entry_point_declarations();

	current_function = &func;
	auto &entry_block = get<SPIRBlock>(func.entry_block);

	for (auto &v : func.local_variables)
	{
		auto &var = get<SPIRVariable>(v);
		if (var.storage == StorageClassWorkgroup)
		{
			// Special variable type which cannot have initializer,
			// need to be declared as standalone variables.
			// Comes from MSL which can push global variables as local variables in main function.
			add_local_variable_name(var.self);
			statement(variable_decl(var), ";");
			var.deferred_declaration = false;
		}
		else if (var.storage == StorageClassPrivate)
		{
			// These variables will not have had their CFG usage analyzed, so move it to the entry block.
			// Comes from MSL which can push global variables as local variables in main function.
			// We could just declare them right now, but we would miss out on an important initialization case which is
			// LUT declaration in MSL.
			// If we don't declare the variable when it is assigned we're forced to go through a helper function
			// which copies elements one by one.
			add_local_variable_name(var.self);
			auto &dominated = entry_block.dominated_variables;
			if (find(begin(dominated), end(dominated), var.self) == end(dominated))
				entry_block.dominated_variables.push_back(var.self);
			var.deferred_declaration = true;
		}
		else if (var.storage == StorageClassFunction && var.remapped_variable && var.static_expression)
		{
			// No need to declare this variable, it has a static expression.
			var.deferred_declaration = false;
		}
		else if (expression_is_lvalue(v))
		{
			add_local_variable_name(var.self);

			if (var.initializer)
				statement(variable_decl_function_local(var), ";");
			else
			{
				// Don't declare variable until first use to declutter the GLSL output quite a lot.
				// If we don't touch the variable before first branch,
				// declare it then since we need variable declaration to be in top scope.
				var.deferred_declaration = true;
			}
		}
		else
		{
			// HACK: SPIR-V in older glslang output likes to use samplers and images as local variables, but GLSL does not allow this.
			// For these types (non-lvalue), we enforce forwarding through a shadowed variable.
			// This means that when we OpStore to these variables, we just write in the expression ID directly.
			// This breaks any kind of branching, since the variable must be statically assigned.
			// Branching on samplers and images would be pretty much impossible to fake in GLSL.
			var.statically_assigned = true;
		}

		var.loop_variable_enable = false;

		// Loop variables are never declared outside their for-loop, so block any implicit declaration.
		if (var.loop_variable)
			var.deferred_declaration = false;
	}

	for (auto &line : current_function->fixup_statements_in)
		statement(line);

	entry_block.loop_dominator = SPIRBlock::NoDominator;
	emit_block_chain(entry_block);

	end_scope();
	processing_entry_point = false;
	statement("");
}

void CompilerGLSL::emit_fixup()
{
	auto &execution = get_entry_point();
	if (execution.model == ExecutionModelVertex)
	{
		if (options.vertex.fixup_clipspace)
		{
			const char *suffix = backend.float_literal_suffix ? "f" : "";
			statement("gl_Position.z = 2.0", suffix, " * gl_Position.z - gl_Position.w;");
		}

		if (options.vertex.flip_vert_y)
			statement("gl_Position.y = -gl_Position.y;");
	}
}

bool CompilerGLSL::flush_phi_required(uint32_t from, uint32_t to)
{
	auto &child = get<SPIRBlock>(to);
	for (auto &phi : child.phi_variables)
		if (phi.parent == from)
			return true;
	return false;
}

void CompilerGLSL::flush_phi(uint32_t from, uint32_t to)
{
	auto &child = get<SPIRBlock>(to);

	for (auto &phi : child.phi_variables)
	{
		if (phi.parent == from)
		{
			auto &var = get<SPIRVariable>(phi.function_variable);

			// A Phi variable might be a loop variable, so flush to static expression.
			if (var.loop_variable && !var.loop_variable_enable)
				var.static_expression = phi.local_variable;
			else
			{
				flush_variable_declaration(phi.function_variable);

				// This might be called in continue block, so make sure we
				// use this to emit ESSL 1.0 compliant increments/decrements.
				auto lhs = to_expression(phi.function_variable);
				auto rhs = to_expression(phi.local_variable);
				if (!optimize_read_modify_write(get<SPIRType>(var.basetype), lhs, rhs))
					statement(lhs, " = ", rhs, ";");
			}

			register_write(phi.function_variable);
		}
	}
}

void CompilerGLSL::branch_to_continue(uint32_t from, uint32_t to)
{
	auto &to_block = get<SPIRBlock>(to);
	if (from == to)
		return;

	assert(is_continue(to));
	if (to_block.complex_continue)
	{
		// Just emit the whole block chain as is.
		auto usage_counts = expression_usage_counts;
		auto invalid = invalid_expressions;

		emit_block_chain(to_block);

		// Expression usage counts and invalid expressions
		// are moot after returning from the continue block.
		// Since we emit the same block multiple times,
		// we don't want to invalidate ourselves.
		expression_usage_counts = usage_counts;
		invalid_expressions = invalid;
	}
	else
	{
		auto &from_block = get<SPIRBlock>(from);
		bool outside_control_flow = false;
		uint32_t loop_dominator = 0;

		// FIXME: Refactor this to not use the old loop_dominator tracking.
		if (from_block.merge_block)
		{
			// If we are a loop header, we don't set the loop dominator,
			// so just use "self" here.
			loop_dominator = from;
		}
		else if (from_block.loop_dominator != SPIRBlock::NoDominator)
		{
			loop_dominator = from_block.loop_dominator;
		}

		if (loop_dominator != 0)
		{
			auto &dominator = get<SPIRBlock>(loop_dominator);

			// For non-complex continue blocks, we implicitly branch to the continue block
			// by having the continue block be part of the loop header in for (; ; continue-block).
			outside_control_flow = block_is_outside_flow_control_from_block(dominator, from_block);
		}

		// Some simplification for for-loops. We always end up with a useless continue;
		// statement since we branch to a loop block.
		// Walk the CFG, if we uncoditionally execute the block calling continue assuming we're in the loop block,
		// we can avoid writing out an explicit continue statement.
		// Similar optimization to return statements if we know we're outside flow control.
		if (!outside_control_flow)
			statement("continue;");
	}
}

void CompilerGLSL::branch(uint32_t from, uint32_t to)
{
	flush_phi(from, to);
	flush_control_dependent_expressions(from);
	flush_all_active_variables();

	// This is only a continue if we branch to our loop dominator.
	if (loop_blocks.find(to) != end(loop_blocks) && get<SPIRBlock>(from).loop_dominator == to)
	{
		// This can happen if we had a complex continue block which was emitted.
		// Once the continue block tries to branch to the loop header, just emit continue;
		// and end the chain here.
		statement("continue;");
	}
	else if (is_break(to))
		statement("break;");
	else if (is_continue(to) || (from == to))
	{
		// For from == to case can happen for a do-while loop which branches into itself.
		// We don't mark these cases as continue blocks, but the only possible way to branch into
		// ourselves is through means of continue blocks.
		branch_to_continue(from, to);
	}
	else if (!is_conditional(to))
		emit_block_chain(get<SPIRBlock>(to));

	// It is important that we check for break before continue.
	// A block might serve two purposes, a break block for the inner scope, and
	// a continue block in the outer scope.
	// Inner scope always takes precedence.
}

void CompilerGLSL::branch(uint32_t from, uint32_t cond, uint32_t true_block, uint32_t false_block)
{
	// If we branch directly to a selection merge target, we don't really need a code path.
	bool true_sub = !is_conditional(true_block);
	bool false_sub = !is_conditional(false_block);

	if (true_sub)
	{
		emit_block_hints(get<SPIRBlock>(from));
		statement("if (", to_expression(cond), ")");
		begin_scope();
		branch(from, true_block);
		end_scope();

		if (false_sub || is_continue(false_block) || is_break(false_block))
		{
			statement("else");
			begin_scope();
			branch(from, false_block);
			end_scope();
		}
		else if (flush_phi_required(from, false_block))
		{
			statement("else");
			begin_scope();
			flush_phi(from, false_block);
			end_scope();
		}
	}
	else if (false_sub && !true_sub)
	{
		// Only need false path, use negative conditional.
		emit_block_hints(get<SPIRBlock>(from));
		statement("if (!", to_enclosed_expression(cond), ")");
		begin_scope();
		branch(from, false_block);
		end_scope();

		if (is_continue(true_block) || is_break(true_block))
		{
			statement("else");
			begin_scope();
			branch(from, true_block);
			end_scope();
		}
		else if (flush_phi_required(from, true_block))
		{
			statement("else");
			begin_scope();
			flush_phi(from, true_block);
			end_scope();
		}
	}
}

void CompilerGLSL::propagate_loop_dominators(const SPIRBlock &block)
{
	// Propagate down the loop dominator block, so that dominated blocks can back trace.
	if (block.merge == SPIRBlock::MergeLoop || block.loop_dominator)
	{
		uint32_t dominator = block.merge == SPIRBlock::MergeLoop ? block.self : block.loop_dominator;

		auto set_dominator = [this](uint32_t self, uint32_t new_dominator) {
			auto &dominated_block = this->get<SPIRBlock>(self);

			// If we already have a loop dominator, we're trying to break out to merge targets
			// which should not update the loop dominator.
			if (!dominated_block.loop_dominator)
				dominated_block.loop_dominator = new_dominator;
		};

		// After merging a loop, we inherit the loop dominator always.
		if (block.merge_block)
			set_dominator(block.merge_block, block.loop_dominator);

		if (block.true_block)
			set_dominator(block.true_block, dominator);
		if (block.false_block)
			set_dominator(block.false_block, dominator);
		if (block.next_block)
			set_dominator(block.next_block, dominator);

		for (auto &c : block.cases)
			set_dominator(c.block, dominator);

		// In older glslang output continue_block can be == loop header.
		if (block.continue_block && block.continue_block != block.self)
			set_dominator(block.continue_block, dominator);
	}
}

// FIXME: This currently cannot handle complex continue blocks
// as in do-while.
// This should be seen as a "trivial" continue block.
string CompilerGLSL::emit_continue_block(uint32_t continue_block)
{
	auto *block = &get<SPIRBlock>(continue_block);

	// While emitting the continue block, declare_temporary will check this
	// if we have to emit temporaries.
	current_continue_block = block;

	vector<string> statements;

	// Capture all statements into our list.
	auto *old = redirect_statement;
	redirect_statement = &statements;

	// Stamp out all blocks one after each other.
	while (loop_blocks.find(block->self) == end(loop_blocks))
	{
		propagate_loop_dominators(*block);
		// Write out all instructions we have in this block.
		emit_block_instructions(*block);

		// For plain branchless for/while continue blocks.
		if (block->next_block)
		{
			flush_phi(continue_block, block->next_block);
			block = &get<SPIRBlock>(block->next_block);
		}
		// For do while blocks. The last block will be a select block.
		else if (block->true_block)
		{
			flush_phi(continue_block, block->true_block);
			block = &get<SPIRBlock>(block->true_block);
		}
	}

	// Restore old pointer.
	redirect_statement = old;

	// Somewhat ugly, strip off the last ';' since we use ',' instead.
	// Ideally, we should select this behavior in statement().
	for (auto &s : statements)
	{
		if (!s.empty() && s.back() == ';')
			s.erase(s.size() - 1, 1);
	}

	current_continue_block = nullptr;
	return merge(statements);
}

void CompilerGLSL::emit_while_loop_initializers(const SPIRBlock &block)
{
	// While loops do not take initializers, so declare all of them outside.
	for (auto &loop_var : block.loop_variables)
	{
		auto &var = get<SPIRVariable>(loop_var);
		statement(variable_decl(var), ";");
	}
}

string CompilerGLSL::emit_for_loop_initializers(const SPIRBlock &block)
{
	if (block.loop_variables.empty())
		return "";

	bool same_types = for_loop_initializers_are_same_type(block);
	// We can only declare for loop initializers if all variables are of same type.
	// If we cannot do this, declare individual variables before the loop header.

	// We might have a loop variable candidate which was not assigned to for some reason.
	uint32_t missing_initializers = 0;
	for (auto &variable : block.loop_variables)
	{
		uint32_t expr = get<SPIRVariable>(variable).static_expression;

		// Sometimes loop variables are initialized with OpUndef, but we can just declare
		// a plain variable without initializer in this case.
		if (expr == 0 || ids[expr].get_type() == TypeUndef)
			missing_initializers++;
	}

	if (block.loop_variables.size() == 1 && missing_initializers == 0)
	{
		return variable_decl(get<SPIRVariable>(block.loop_variables.front()));
	}
	else if (!same_types || missing_initializers == uint32_t(block.loop_variables.size()))
	{
		for (auto &loop_var : block.loop_variables)
			statement(variable_decl(get<SPIRVariable>(loop_var)), ";");
		return "";
	}
	else
	{
		// We have a mix of loop variables, either ones with a clear initializer, or ones without.
		// Separate the two streams.
		string expr;

		for (auto &loop_var : block.loop_variables)
		{
			uint32_t static_expr = get<SPIRVariable>(loop_var).static_expression;
			if (static_expr == 0 || ids[static_expr].get_type() == TypeUndef)
			{
				statement(variable_decl(get<SPIRVariable>(loop_var)), ";");
			}
			else
			{
				if (expr.empty())
				{
					// For loop initializers are of the form <type id = value, id = value, id = value, etc ...
					auto &var = get<SPIRVariable>(loop_var);
					auto &type = get<SPIRType>(var.basetype);
					expr = join(to_qualifiers_glsl(var.self), type_to_glsl(type), " ");
				}
				else
					expr += ", ";

				auto &v = get<SPIRVariable>(loop_var);
				expr += join(to_name(loop_var), " = ", to_expression(v.static_expression));
			}
		}
		return expr;
	}
}

bool CompilerGLSL::for_loop_initializers_are_same_type(const SPIRBlock &block)
{
	if (block.loop_variables.size() <= 1)
		return true;

	uint32_t expected = 0;
	Bitset expected_flags;
	for (auto &var : block.loop_variables)
	{
		// Don't care about uninitialized variables as they will not be part of the initializers.
		uint32_t expr = get<SPIRVariable>(var).static_expression;
		if (expr == 0 || ids[expr].get_type() == TypeUndef)
			continue;

		if (expected == 0)
		{
			expected = get<SPIRVariable>(var).basetype;
			expected_flags = get_decoration_bitset(var);
		}
		else if (expected != get<SPIRVariable>(var).basetype)
			return false;

		// Precision flags and things like that must also match.
		if (expected_flags != get_decoration_bitset(var))
			return false;
	}

	return true;
}

bool CompilerGLSL::attempt_emit_loop_header(SPIRBlock &block, SPIRBlock::Method method)
{
	SPIRBlock::ContinueBlockType continue_type = continue_block_type(get<SPIRBlock>(block.continue_block));

	if (method == SPIRBlock::MergeToSelectForLoop || method == SPIRBlock::MergeToSelectContinueForLoop)
	{
		uint32_t current_count = statement_count;
		// If we're trying to create a true for loop,
		// we need to make sure that all opcodes before branch statement do not actually emit any code.
		// We can then take the condition expression and create a for (; cond ; ) { body; } structure instead.
		emit_block_instructions(block);

		bool condition_is_temporary = forced_temporaries.find(block.condition) == end(forced_temporaries);

		// This can work! We only did trivial things which could be forwarded in block body!
		if (current_count == statement_count && condition_is_temporary)
		{
			switch (continue_type)
			{
			case SPIRBlock::ForLoop:
			{
				// This block may be a dominating block, so make sure we flush undeclared variables before building the for loop header.
				flush_undeclared_variables(block);

				// Important that we do this in this order because
				// emitting the continue block can invalidate the condition expression.
				auto initializer = emit_for_loop_initializers(block);
				auto condition = to_expression(block.condition);
				emit_block_hints(block);
				if (method != SPIRBlock::MergeToSelectContinueForLoop)
				{
					auto continue_block = emit_continue_block(block.continue_block);
					statement("for (", initializer, "; ", condition, "; ", continue_block, ")");
				}
				else
					statement("for (", initializer, "; ", condition, "; )");
				break;
			}

			case SPIRBlock::WhileLoop:
				// This block may be a dominating block, so make sure we flush undeclared variables before building the while loop header.
				flush_undeclared_variables(block);
				emit_while_loop_initializers(block);
				emit_block_hints(block);
				statement("while (", to_expression(block.condition), ")");
				break;

			default:
				SPIRV_CROSS_THROW("For/while loop detected, but need while/for loop semantics.");
			}

			begin_scope();
			return true;
		}
		else
		{
			block.disable_block_optimization = true;
			force_recompile = true;
			begin_scope(); // We'll see an end_scope() later.
			return false;
		}
	}
	else if (method == SPIRBlock::MergeToDirectForLoop)
	{
		auto &child = get<SPIRBlock>(block.next_block);

		// This block may be a dominating block, so make sure we flush undeclared variables before building the for loop header.
		flush_undeclared_variables(child);

		uint32_t current_count = statement_count;

		// If we're trying to create a true for loop,
		// we need to make sure that all opcodes before branch statement do not actually emit any code.
		// We can then take the condition expression and create a for (; cond ; ) { body; } structure instead.
		emit_block_instructions(child);

		bool condition_is_temporary = forced_temporaries.find(child.condition) == end(forced_temporaries);

		if (current_count == statement_count && condition_is_temporary)
		{
			propagate_loop_dominators(child);

			switch (continue_type)
			{
			case SPIRBlock::ForLoop:
			{
				// Important that we do this in this order because
				// emitting the continue block can invalidate the condition expression.
				auto initializer = emit_for_loop_initializers(block);
				auto condition = to_expression(child.condition);
				auto continue_block = emit_continue_block(block.continue_block);
				emit_block_hints(block);
				statement("for (", initializer, "; ", condition, "; ", continue_block, ")");
				break;
			}

			case SPIRBlock::WhileLoop:
				emit_while_loop_initializers(block);
				emit_block_hints(block);
				statement("while (", to_expression(child.condition), ")");
				break;

			default:
				SPIRV_CROSS_THROW("For/while loop detected, but need while/for loop semantics.");
			}

			begin_scope();
			branch(child.self, child.true_block);
			return true;
		}
		else
		{
			block.disable_block_optimization = true;
			force_recompile = true;
			begin_scope(); // We'll see an end_scope() later.
			return false;
		}
	}
	else
		return false;
}

void CompilerGLSL::flush_undeclared_variables(SPIRBlock &block)
{
	// Enforce declaration order for regression testing purposes.
	sort(begin(block.dominated_variables), end(block.dominated_variables));

	for (auto &v : block.dominated_variables)
	{
		auto &var = get<SPIRVariable>(v);
		if (var.deferred_declaration)
			statement(variable_decl(var), ";");
		var.deferred_declaration = false;
	}
}

void CompilerGLSL::emit_hoisted_temporaries(vector<pair<uint32_t, uint32_t>> &temporaries)
{
	// If we need to force temporaries for certain IDs due to continue blocks, do it before starting loop header.
	// Need to sort these to ensure that reference output is stable.
	sort(begin(temporaries), end(temporaries),
	     [](const pair<uint32_t, uint32_t> &a, const pair<uint32_t, uint32_t> &b) { return a.second < b.second; });

	for (auto &tmp : temporaries)
	{
		add_local_variable_name(tmp.second);
		auto flags = meta[tmp.second].decoration.decoration_flags;
		auto &type = get<SPIRType>(tmp.first);
		statement(flags_to_precision_qualifiers_glsl(type, flags), variable_decl(type, to_name(tmp.second)), ";");

		hoisted_temporaries.insert(tmp.second);
		forced_temporaries.insert(tmp.second);

		// The temporary might be read from before it's assigned, set up the expression now.
		set<SPIRExpression>(tmp.second, to_name(tmp.second), tmp.first, true);
	}
}

void CompilerGLSL::emit_block_chain(SPIRBlock &block)
{
	propagate_loop_dominators(block);

	bool select_branch_to_true_block = false;
	bool skip_direct_branch = false;
	bool emitted_for_loop_header = false;
	bool force_complex_continue_block = false;

	emit_hoisted_temporaries(block.declare_temporary);

	SPIRBlock::ContinueBlockType continue_type = SPIRBlock::ContinueNone;
	if (block.continue_block)
		continue_type = continue_block_type(get<SPIRBlock>(block.continue_block));

	// If we have loop variables, stop masking out access to the variable now.
	for (auto var : block.loop_variables)
		get<SPIRVariable>(var).loop_variable_enable = true;

	// This is the method often used by spirv-opt to implement loops.
	// The loop header goes straight into the continue block.
	// However, don't attempt this on ESSL 1.0, because if a loop variable is used in a continue block,
	// it *MUST* be used in the continue block. This loop method will not work.
	if (!is_legacy_es() && block_is_loop_candidate(block, SPIRBlock::MergeToSelectContinueForLoop))
	{
		flush_undeclared_variables(block);
		if (attempt_emit_loop_header(block, SPIRBlock::MergeToSelectContinueForLoop))
		{
			select_branch_to_true_block = true;
			emitted_for_loop_header = true;
			force_complex_continue_block = true;
		}
	}
	// This is the older loop behavior in glslang which branches to loop body directly from the loop header.
	else if (block_is_loop_candidate(block, SPIRBlock::MergeToSelectForLoop))
	{
		flush_undeclared_variables(block);
		if (attempt_emit_loop_header(block, SPIRBlock::MergeToSelectForLoop))
		{
			// The body of while, is actually just the true block, so always branch there unconditionally.
			select_branch_to_true_block = true;
			emitted_for_loop_header = true;
		}
	}
	// This is the newer loop behavior in glslang which branches from Loop header directly to
	// a new block, which in turn has a OpBranchSelection without a selection merge.
	else if (block_is_loop_candidate(block, SPIRBlock::MergeToDirectForLoop))
	{
		flush_undeclared_variables(block);
		if (attempt_emit_loop_header(block, SPIRBlock::MergeToDirectForLoop))
		{
			skip_direct_branch = true;
			emitted_for_loop_header = true;
		}
	}
	else if (continue_type == SPIRBlock::DoWhileLoop)
	{
		flush_undeclared_variables(block);
		emit_while_loop_initializers(block);
		// We have some temporaries where the loop header is the dominator.
		// We risk a case where we have code like:
		// for (;;) { create-temporary; break; } consume-temporary;
		// so force-declare temporaries here.
		emit_hoisted_temporaries(block.potential_declare_temporary);
		statement("do");
		begin_scope();

		emit_block_instructions(block);
	}
	else if (block.merge == SPIRBlock::MergeLoop)
	{
		flush_undeclared_variables(block);
		emit_while_loop_initializers(block);

		// We have a generic loop without any distinguishable pattern like for, while or do while.
		get<SPIRBlock>(block.continue_block).complex_continue = true;
		continue_type = SPIRBlock::ComplexLoop;

		// We have some temporaries where the loop header is the dominator.
		// We risk a case where we have code like:
		// for (;;) { create-temporary; break; } consume-temporary;
		// so force-declare temporaries here.
		emit_hoisted_temporaries(block.potential_declare_temporary);
		statement("for (;;)");
		begin_scope();

		emit_block_instructions(block);
	}
	else
	{
		emit_block_instructions(block);
	}

	// If we didn't successfully emit a loop header and we had loop variable candidates, we have a problem
	// as writes to said loop variables might have been masked out, we need a recompile.
	if (!emitted_for_loop_header && !block.loop_variables.empty())
	{
		force_recompile = true;
		for (auto var : block.loop_variables)
			get<SPIRVariable>(var).loop_variable = false;
		block.loop_variables.clear();
	}

	flush_undeclared_variables(block);
	bool emit_next_block = true;

	// Handle end of block.
	switch (block.terminator)
	{
	case SPIRBlock::Direct:
		// True when emitting complex continue block.
		if (block.loop_dominator == block.next_block)
		{
			branch(block.self, block.next_block);
			emit_next_block = false;
		}
		// True if MergeToDirectForLoop succeeded.
		else if (skip_direct_branch)
			emit_next_block = false;
		else if (is_continue(block.next_block) || is_break(block.next_block) || is_conditional(block.next_block))
		{
			branch(block.self, block.next_block);
			emit_next_block = false;
		}
		break;

	case SPIRBlock::Select:
		// True if MergeToSelectForLoop or MergeToSelectContinueForLoop succeeded.
		if (select_branch_to_true_block)
		{
			if (force_complex_continue_block)
			{
				assert(block.true_block == block.continue_block);

				// We're going to emit a continue block directly here, so make sure it's marked as complex.
				auto &complex_continue = get<SPIRBlock>(block.continue_block).complex_continue;
				bool old_complex = complex_continue;
				complex_continue = true;
				branch(block.self, block.true_block);
				complex_continue = old_complex;
			}
			else
				branch(block.self, block.true_block);
		}
		else
			branch(block.self, block.condition, block.true_block, block.false_block);
		break;

	case SPIRBlock::MultiSelect:
	{
		auto &type = expression_type(block.condition);
		bool uint32_t_case = type.basetype == SPIRType::UInt;

		emit_block_hints(block);
		statement("switch (", to_expression(block.condition), ")");
		begin_scope();

		// Multiple case labels can branch to same block, so find all unique blocks.
		bool emitted_default = false;
		unordered_set<uint32_t> emitted_blocks;

		for (auto &c : block.cases)
		{
			if (emitted_blocks.count(c.block) != 0)
				continue;

			// Emit all case labels which branch to our target.
			// FIXME: O(n^2), revisit if we hit shaders with 100++ case labels ...
			for (auto &other_case : block.cases)
			{
				if (other_case.block == c.block)
				{
					auto case_value = uint32_t_case ? convert_to_string(uint32_t(other_case.value)) :
					                                  convert_to_string(int32_t(other_case.value));
					statement("case ", case_value, ":");
				}
			}

			// Maybe we share with default block?
			if (block.default_block == c.block)
			{
				statement("default:");
				emitted_default = true;
			}

			// Complete the target.
			emitted_blocks.insert(c.block);

			begin_scope();
			branch(block.self, c.block);
			end_scope();
		}

		if (!emitted_default)
		{
			if (block.default_block != block.next_block)
			{
				statement("default:");
				begin_scope();
				if (is_break(block.default_block))
					SPIRV_CROSS_THROW("Cannot break; out of a switch statement and out of a loop at the same time ...");
				branch(block.self, block.default_block);
				end_scope();
			}
			else if (flush_phi_required(block.self, block.next_block))
			{
				statement("default:");
				begin_scope();
				flush_phi(block.self, block.next_block);
				statement("break;");
				end_scope();
			}
		}

		end_scope();
		break;
	}

	case SPIRBlock::Return:
		for (auto &line : current_function->fixup_statements_out)
			statement(line);

		if (processing_entry_point)
			emit_fixup();

		if (block.return_value)
		{
			auto &type = expression_type(block.return_value);
			if (!type.array.empty() && !backend.can_return_array)
			{
				// If we cannot return arrays, we will have a special out argument we can write to instead.
				// The backend is responsible for setting this up, and redirection the return values as appropriate.
				if (ids.at(block.return_value).get_type() != TypeUndef)
					emit_array_copy("SPIRV_Cross_return_value", block.return_value);

				if (!block_is_outside_flow_control_from_block(get<SPIRBlock>(current_function->entry_block), block) ||
				    block.loop_dominator != SPIRBlock::NoDominator)
				{
					statement("return;");
				}
			}
			else
			{
				// OpReturnValue can return Undef, so don't emit anything for this case.
				if (ids.at(block.return_value).get_type() != TypeUndef)
					statement("return ", to_expression(block.return_value), ";");
			}
		}
		// If this block is the very final block and not called from control flow,
		// we do not need an explicit return which looks out of place. Just end the function here.
		// In the very weird case of for(;;) { return; } executing return is unconditional,
		// but we actually need a return here ...
		else if (!block_is_outside_flow_control_from_block(get<SPIRBlock>(current_function->entry_block), block) ||
		         block.loop_dominator != SPIRBlock::NoDominator)
		{
			statement("return;");
		}
		break;

	case SPIRBlock::Kill:
		statement(backend.discard_literal, ";");
		break;

	case SPIRBlock::Unreachable:
		emit_next_block = false;
		break;

	default:
		SPIRV_CROSS_THROW("Unimplemented block terminator.");
	}

	if (block.next_block && emit_next_block)
	{
		// If we hit this case, we're dealing with an unconditional branch, which means we will output
		// that block after this. If we had selection merge, we already flushed phi variables.
		if (block.merge != SPIRBlock::MergeSelection)
			flush_phi(block.self, block.next_block);

		// For merge selects we might have ignored the fact that a merge target
		// could have been a break; or continue;
		// We will need to deal with it here.
		if (is_loop_break(block.next_block))
		{
			// Cannot check for just break, because switch statements will also use break.
			assert(block.merge == SPIRBlock::MergeSelection);
			statement("break;");
		}
		else if (is_continue(block.next_block))
		{
			assert(block.merge == SPIRBlock::MergeSelection);
			branch_to_continue(block.self, block.next_block);
		}
		else
			emit_block_chain(get<SPIRBlock>(block.next_block));
	}

	if (block.merge == SPIRBlock::MergeLoop)
	{
		if (continue_type == SPIRBlock::DoWhileLoop)
		{
			// Make sure that we run the continue block to get the expressions set, but this
			// should become an empty string.
			// We have no fallbacks if we cannot forward everything to temporaries ...
			auto statements = emit_continue_block(block.continue_block);
			if (!statements.empty())
			{
				// The DoWhile block has side effects, force ComplexLoop pattern next pass.
				get<SPIRBlock>(block.continue_block).complex_continue = true;
				force_recompile = true;
			}

			end_scope_decl(join("while (", to_expression(get<SPIRBlock>(block.continue_block).condition), ")"));
		}
		else
			end_scope();

		// We cannot break out of two loops at once, so don't check for break; here.
		// Using block.self as the "from" block isn't quite right, but it has the same scope
		// and dominance structure, so it's fine.
		if (is_continue(block.merge_block))
			branch_to_continue(block.self, block.merge_block);
		else
			emit_block_chain(get<SPIRBlock>(block.merge_block));
	}

	// Forget about control dependent expressions now.
	block.invalidate_expressions.clear();
}

void CompilerGLSL::begin_scope()
{
	statement("{");
	indent++;
}

void CompilerGLSL::end_scope()
{
	if (!indent)
		SPIRV_CROSS_THROW("Popping empty indent stack.");
	indent--;
	statement("}");
}

void CompilerGLSL::end_scope_decl()
{
	if (!indent)
		SPIRV_CROSS_THROW("Popping empty indent stack.");
	indent--;
	statement("};");
}

void CompilerGLSL::end_scope_decl(const string &decl)
{
	if (!indent)
		SPIRV_CROSS_THROW("Popping empty indent stack.");
	indent--;
	statement("} ", decl, ";");
}

void CompilerGLSL::check_function_call_constraints(const uint32_t *args, uint32_t length)
{
	// If our variable is remapped, and we rely on type-remapping information as
	// well, then we cannot pass the variable as a function parameter.
	// Fixing this is non-trivial without stamping out variants of the same function,
	// so for now warn about this and suggest workarounds instead.
	for (uint32_t i = 0; i < length; i++)
	{
		auto *var = maybe_get<SPIRVariable>(args[i]);
		if (!var || !var->remapped_variable)
			continue;

		auto &type = get<SPIRType>(var->basetype);
		if (type.basetype == SPIRType::Image && type.image.dim == DimSubpassData)
		{
			SPIRV_CROSS_THROW("Tried passing a remapped subpassInput variable to a function. "
			                  "This will not work correctly because type-remapping information is lost. "
			                  "To workaround, please consider not passing the subpass input as a function parameter, "
			                  "or use in/out variables instead which do not need type remapping information.");
		}
	}
}

const Instruction *CompilerGLSL::get_next_instruction_in_block(const Instruction &instr)
{
	// FIXME: This is kind of hacky. There should be a cleaner way.
	auto offset = uint32_t(&instr - current_emitting_block->ops.data());
	if ((offset + 1) < current_emitting_block->ops.size())
		return &current_emitting_block->ops[offset + 1];
	else
		return nullptr;
}

uint32_t CompilerGLSL::mask_relevant_memory_semantics(uint32_t semantics)
{
	return semantics & (MemorySemanticsAtomicCounterMemoryMask | MemorySemanticsImageMemoryMask |
	                    MemorySemanticsWorkgroupMemoryMask | MemorySemanticsUniformMemoryMask |
	                    MemorySemanticsCrossWorkgroupMemoryMask | MemorySemanticsSubgroupMemoryMask);
}

void CompilerGLSL::emit_array_copy(const string &lhs, uint32_t rhs_id)
{
	statement(lhs, " = ", to_expression(rhs_id), ";");
}

void CompilerGLSL::bitcast_from_builtin_load(uint32_t source_id, std::string &expr,
                                             const spirv_cross::SPIRType &expr_type)
{
	// Only interested in standalone builtin variables.
	if (!has_decoration(source_id, DecorationBuiltIn))
		return;

	auto builtin = static_cast<BuiltIn>(get_decoration(source_id, DecorationBuiltIn));
	auto expected_type = expr_type.basetype;

	// TODO: Fill in for more builtins.
	switch (builtin)
	{
	case BuiltInLayer:
	case BuiltInPrimitiveId:
	case BuiltInViewportIndex:
	case BuiltInInstanceId:
	case BuiltInInstanceIndex:
	case BuiltInVertexId:
	case BuiltInVertexIndex:
	case BuiltInSampleId:
		expected_type = SPIRType::Int;
		break;

	default:
		break;
	}

	if (expected_type != expr_type.basetype)
		expr = bitcast_expression(expr_type, expected_type, expr);
}

void CompilerGLSL::bitcast_to_builtin_store(uint32_t target_id, std::string &expr,
                                            const spirv_cross::SPIRType &expr_type)
{
	// Only interested in standalone builtin variables.
	if (!has_decoration(target_id, DecorationBuiltIn))
		return;

	auto builtin = static_cast<BuiltIn>(get_decoration(target_id, DecorationBuiltIn));
	auto expected_type = expr_type.basetype;

	// TODO: Fill in for more builtins.
	switch (builtin)
	{
	case BuiltInLayer:
	case BuiltInPrimitiveId:
	case BuiltInViewportIndex:
		expected_type = SPIRType::Int;
		break;

	default:
		break;
	}

	if (expected_type != expr_type.basetype)
	{
		auto type = expr_type;
		type.basetype = expected_type;
		expr = bitcast_expression(type, expr_type.basetype, expr);
	}
}

void CompilerGLSL::emit_block_hints(const SPIRBlock &)
{
}
