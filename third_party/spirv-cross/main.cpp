/*
 * Copyright 2015-2021 Arm Limited
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

/*
 * At your option, you may choose to accept this material under either:
 *  1. The Apache License, Version 2.0, found at <http://www.apache.org/licenses/LICENSE-2.0>, or
 *  2. The MIT License, found at <http://opensource.org/licenses/MIT>.
 * SPDX-License-Identifier: Apache-2.0 OR MIT.
 */

#include "spirv_cpp.hpp"
#include "spirv_cross_util.hpp"
#include "spirv_glsl.hpp"
#include "spirv_hlsl.hpp"
#include "spirv_msl.hpp"
#include "spirv_parser.hpp"
#include "spirv_reflect.hpp"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

#ifdef HAVE_SPIRV_CROSS_GIT_VERSION
#include "gitversion.h"
#endif

using namespace spv;
using namespace SPIRV_CROSS_NAMESPACE;
using namespace std;

#ifdef SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS
static inline void THROW(const char *str)
{
	fprintf(stderr, "SPIRV-Cross will abort: %s\n", str);
	fflush(stderr);
	abort();
}
#else
#define THROW(x) throw runtime_error(x)
#endif

struct CLIParser;
struct CLICallbacks
{
	void add(const char *cli, const function<void(CLIParser &)> &func)
	{
		callbacks[cli] = func;
	}
	unordered_map<string, function<void(CLIParser &)>> callbacks;
	function<void()> error_handler;
	function<void(const char *)> default_handler;
};

struct CLIParser
{
	CLIParser(CLICallbacks cbs_, int argc_, char *argv_[])
	    : cbs(move(cbs_))
	    , argc(argc_)
	    , argv(argv_)
	{
	}

	bool parse()
	{
#ifndef SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS
		try
#endif
		{
			while (argc && !ended_state)
			{
				const char *next = *argv++;
				argc--;

				if (*next != '-' && cbs.default_handler)
				{
					cbs.default_handler(next);
				}
				else
				{
					auto itr = cbs.callbacks.find(next);
					if (itr == ::end(cbs.callbacks))
					{
						THROW("Invalid argument");
					}

					itr->second(*this);
				}
			}

			return true;
		}
#ifndef SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS
		catch (...)
		{
			if (cbs.error_handler)
			{
				cbs.error_handler();
			}
			return false;
		}
#endif
	}

	void end()
	{
		ended_state = true;
	}

	uint32_t next_uint()
	{
		if (!argc)
		{
			THROW("Tried to parse uint, but nothing left in arguments");
		}

		uint64_t val = stoul(*argv);
		if (val > numeric_limits<uint32_t>::max())
		{
			THROW("next_uint() out of range");
		}

		argc--;
		argv++;

		return uint32_t(val);
	}

	uint32_t next_hex_uint()
	{
		if (!argc)
		{
			THROW("Tried to parse uint, but nothing left in arguments");
		}

		uint64_t val = stoul(*argv, nullptr, 16);
		if (val > numeric_limits<uint32_t>::max())
		{
			THROW("next_uint() out of range");
		}

		argc--;
		argv++;

		return uint32_t(val);
	}

	double next_double()
	{
		if (!argc)
		{
			THROW("Tried to parse double, but nothing left in arguments");
		}

		double val = stod(*argv);

		argc--;
		argv++;

		return val;
	}

	// Return a string only if it's not prefixed with `--`, otherwise return the default value
	const char *next_value_string(const char *default_value)
	{
		if (!argc)
		{
			return default_value;
		}

		if (0 == strncmp("--", *argv, 2))
		{
			return default_value;
		}

		return next_string();
	}

	const char *next_string()
	{
		if (!argc)
		{
			THROW("Tried to parse string, but nothing left in arguments");
		}

		const char *ret = *argv;
		argc--;
		argv++;
		return ret;
	}

	CLICallbacks cbs;
	int argc;
	char **argv;
	bool ended_state = false;
};

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

static vector<uint32_t> read_spirv_file_stdin()
{
#ifdef _WIN32
	setmode(fileno(stdin), O_BINARY);
#endif

	vector<uint32_t> buffer;
	uint32_t tmp[256];
	size_t ret;

	while ((ret = fread(tmp, sizeof(uint32_t), 256, stdin)))
		buffer.insert(buffer.end(), tmp, tmp + ret);

	return buffer;
}

static vector<uint32_t> read_spirv_file(const char *path)
{
	if (path[0] == '-' && path[1] == '\0')
		return read_spirv_file_stdin();

	FILE *file = fopen(path, "rb");
	if (!file)
	{
		fprintf(stderr, "Failed to open SPIR-V file: %s\n", path);
		return {};
	}

	fseek(file, 0, SEEK_END);
	long len = ftell(file) / sizeof(uint32_t);
	rewind(file);

	vector<uint32_t> spirv(len);
	if (fread(spirv.data(), sizeof(uint32_t), len, file) != size_t(len))
		spirv.clear();

	fclose(file);
	return spirv;
}

static bool write_string_to_file(const char *path, const char *string)
{
	FILE *file = fopen(path, "w");
	if (!file)
	{
		fprintf(stderr, "Failed to write file: %s\n", path);
		return false;
	}

	fprintf(file, "%s", string);
	fclose(file);
	return true;
}

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

static void print_resources(const Compiler &compiler, spv::StorageClass storage,
                            const SmallVector<BuiltInResource> &resources)
{
	fprintf(stderr, "%s\n", storage == StorageClassInput ? "builtin inputs" : "builtin outputs");
	fprintf(stderr, "=============\n\n");
	for (auto &res : resources)
	{
		bool active = compiler.has_active_builtin(res.builtin, storage);
		const char *basetype = "?";
		auto &type = compiler.get_type(res.value_type_id);
		switch (type.basetype)
		{
		case SPIRType::Float: basetype = "float"; break;
		case SPIRType::Int: basetype = "int"; break;
		case SPIRType::UInt: basetype = "uint"; break;
		default: break;
		}

		uint32_t array_size = 0;
		bool array_size_literal = false;
		if (!type.array.empty())
		{
			array_size = type.array.front();
			array_size_literal = type.array_size_literal.front();
		}

		string type_str = basetype;
		if (type.vecsize > 1)
			type_str += std::to_string(type.vecsize);

		if (array_size)
		{
			if (array_size_literal)
				type_str += join("[", array_size, "]");
			else
				type_str += join("[", array_size, " (spec constant ID)]");
		}

		string builtin_str;
		switch (res.builtin)
		{
		case spv::BuiltInPosition: builtin_str = "Position"; break;
		case spv::BuiltInPointSize: builtin_str = "PointSize"; break;
		case spv::BuiltInCullDistance: builtin_str = "CullDistance"; break;
		case spv::BuiltInClipDistance: builtin_str = "ClipDistance"; break;
		case spv::BuiltInTessLevelInner: builtin_str = "TessLevelInner"; break;
		case spv::BuiltInTessLevelOuter: builtin_str = "TessLevelOuter"; break;
		default: builtin_str = string("builtin #") + to_string(res.builtin);
		}

		fprintf(stderr, "Builtin %s (%s) (active: %s).\n", builtin_str.c_str(), type_str.c_str(), active ? "yes" : "no");
	}
	fprintf(stderr, "=============\n\n");
}

static void print_resources(const Compiler &compiler, const char *tag, const SmallVector<Resource> &resources)
{
	fprintf(stderr, "%s\n", tag);
	fprintf(stderr, "=============\n\n");
	bool print_ssbo = !strcmp(tag, "ssbos");

	for (auto &res : resources)
	{
		auto &type = compiler.get_type(res.type_id);

		if (print_ssbo && compiler.buffer_is_hlsl_counter_buffer(res.id))
			continue;

		// If we don't have a name, use the fallback for the type instead of the variable
		// for SSBOs and UBOs since those are the only meaningful names to use externally.
		// Push constant blocks are still accessed by name and not block name, even though they are technically Blocks.
		bool is_push_constant = compiler.get_storage_class(res.id) == StorageClassPushConstant;
		bool is_block = compiler.get_decoration_bitset(type.self).get(DecorationBlock) ||
		                compiler.get_decoration_bitset(type.self).get(DecorationBufferBlock);
		bool is_sized_block = is_block && (compiler.get_storage_class(res.id) == StorageClassUniform ||
		                                   compiler.get_storage_class(res.id) == StorageClassUniformConstant);
		ID fallback_id = !is_push_constant && is_block ? ID(res.base_type_id) : ID(res.id);

		uint32_t block_size = 0;
		uint32_t runtime_array_stride = 0;
		if (is_sized_block)
		{
			auto &base_type = compiler.get_type(res.base_type_id);
			block_size = uint32_t(compiler.get_declared_struct_size(base_type));
			runtime_array_stride = uint32_t(compiler.get_declared_struct_size_runtime_array(base_type, 1) -
			                                compiler.get_declared_struct_size_runtime_array(base_type, 0));
		}

		Bitset mask;
		if (print_ssbo)
			mask = compiler.get_buffer_block_flags(res.id);
		else
			mask = compiler.get_decoration_bitset(res.id);

		string array;
		for (auto arr : type.array)
			array = join("[", arr ? convert_to_string(arr) : "", "]") + array;

		fprintf(stderr, " ID %03u : %s%s", uint32_t(res.id),
		        !res.name.empty() ? res.name.c_str() : compiler.get_fallback_name(fallback_id).c_str(), array.c_str());

		if (mask.get(DecorationLocation))
			fprintf(stderr, " (Location : %u)", compiler.get_decoration(res.id, DecorationLocation));
		if (mask.get(DecorationDescriptorSet))
			fprintf(stderr, " (Set : %u)", compiler.get_decoration(res.id, DecorationDescriptorSet));
		if (mask.get(DecorationBinding))
			fprintf(stderr, " (Binding : %u)", compiler.get_decoration(res.id, DecorationBinding));
		if (static_cast<const CompilerGLSL &>(compiler).variable_is_depth_or_compare(res.id))
			fprintf(stderr, " (comparison)");
		if (mask.get(DecorationInputAttachmentIndex))
			fprintf(stderr, " (Attachment : %u)", compiler.get_decoration(res.id, DecorationInputAttachmentIndex));
		if (mask.get(DecorationNonReadable))
			fprintf(stderr, " writeonly");
		if (mask.get(DecorationNonWritable))
			fprintf(stderr, " readonly");
		if (is_sized_block)
		{
			fprintf(stderr, " (BlockSize : %u bytes)", block_size);
			if (runtime_array_stride)
				fprintf(stderr, " (Unsized array stride: %u bytes)", runtime_array_stride);
		}

		uint32_t counter_id = 0;
		if (print_ssbo && compiler.buffer_get_hlsl_counter_buffer(res.id, counter_id))
			fprintf(stderr, " (HLSL counter buffer ID: %u)", counter_id);
		fprintf(stderr, "\n");
	}
	fprintf(stderr, "=============\n\n");
}

static const char *execution_model_to_str(spv::ExecutionModel model)
{
	switch (model)
	{
	case spv::ExecutionModelVertex:
		return "vertex";
	case spv::ExecutionModelTessellationControl:
		return "tessellation control";
	case ExecutionModelTessellationEvaluation:
		return "tessellation evaluation";
	case ExecutionModelGeometry:
		return "geometry";
	case ExecutionModelFragment:
		return "fragment";
	case ExecutionModelGLCompute:
		return "compute";
	case ExecutionModelRayGenerationNV:
		return "raygenNV";
	case ExecutionModelIntersectionNV:
		return "intersectionNV";
	case ExecutionModelCallableNV:
		return "callableNV";
	case ExecutionModelAnyHitNV:
		return "anyhitNV";
	case ExecutionModelClosestHitNV:
		return "closesthitNV";
	case ExecutionModelMissNV:
		return "missNV";
	default:
		return "???";
	}
}

static void print_resources(const Compiler &compiler, const ShaderResources &res)
{
	auto &modes = compiler.get_execution_mode_bitset();

	fprintf(stderr, "Entry points:\n");
	auto entry_points = compiler.get_entry_points_and_stages();
	for (auto &e : entry_points)
		fprintf(stderr, "  %s (%s)\n", e.name.c_str(), execution_model_to_str(e.execution_model));
	fprintf(stderr, "\n");

	fprintf(stderr, "Execution modes:\n");
	modes.for_each_bit([&](uint32_t i) {
		auto mode = static_cast<ExecutionMode>(i);
		uint32_t arg0 = compiler.get_execution_mode_argument(mode, 0);
		uint32_t arg1 = compiler.get_execution_mode_argument(mode, 1);
		uint32_t arg2 = compiler.get_execution_mode_argument(mode, 2);

		switch (static_cast<ExecutionMode>(i))
		{
		case ExecutionModeInvocations:
			fprintf(stderr, "  Invocations: %u\n", arg0);
			break;

		case ExecutionModeLocalSize:
			fprintf(stderr, "  LocalSize: (%u, %u, %u)\n", arg0, arg1, arg2);
			break;

		case ExecutionModeOutputVertices:
			fprintf(stderr, "  OutputVertices: %u\n", arg0);
			break;

#define CHECK_MODE(m)                  \
	case ExecutionMode##m:             \
		fprintf(stderr, "  %s\n", #m); \
		break
			CHECK_MODE(SpacingEqual);
			CHECK_MODE(SpacingFractionalEven);
			CHECK_MODE(SpacingFractionalOdd);
			CHECK_MODE(VertexOrderCw);
			CHECK_MODE(VertexOrderCcw);
			CHECK_MODE(PixelCenterInteger);
			CHECK_MODE(OriginUpperLeft);
			CHECK_MODE(OriginLowerLeft);
			CHECK_MODE(EarlyFragmentTests);
			CHECK_MODE(PointMode);
			CHECK_MODE(Xfb);
			CHECK_MODE(DepthReplacing);
			CHECK_MODE(DepthGreater);
			CHECK_MODE(DepthLess);
			CHECK_MODE(DepthUnchanged);
			CHECK_MODE(LocalSizeHint);
			CHECK_MODE(InputPoints);
			CHECK_MODE(InputLines);
			CHECK_MODE(InputLinesAdjacency);
			CHECK_MODE(Triangles);
			CHECK_MODE(InputTrianglesAdjacency);
			CHECK_MODE(Quads);
			CHECK_MODE(Isolines);
			CHECK_MODE(OutputPoints);
			CHECK_MODE(OutputLineStrip);
			CHECK_MODE(OutputTriangleStrip);
			CHECK_MODE(VecTypeHint);
			CHECK_MODE(ContractionOff);

		default:
			break;
		}
	});
	fprintf(stderr, "\n");

	print_resources(compiler, "subpass inputs", res.subpass_inputs);
	print_resources(compiler, "inputs", res.stage_inputs);
	print_resources(compiler, "outputs", res.stage_outputs);
	print_resources(compiler, "textures", res.sampled_images);
	print_resources(compiler, "separate images", res.separate_images);
	print_resources(compiler, "separate samplers", res.separate_samplers);
	print_resources(compiler, "images", res.storage_images);
	print_resources(compiler, "ssbos", res.storage_buffers);
	print_resources(compiler, "ubos", res.uniform_buffers);
	print_resources(compiler, "push", res.push_constant_buffers);
	print_resources(compiler, "counters", res.atomic_counters);
	print_resources(compiler, "acceleration structures", res.acceleration_structures);
	print_resources(compiler, spv::StorageClassInput, res.builtin_inputs);
	print_resources(compiler, spv::StorageClassOutput, res.builtin_outputs);
}

static void print_push_constant_resources(const Compiler &compiler, const SmallVector<Resource> &res)
{
	for (auto &block : res)
	{
		auto ranges = compiler.get_active_buffer_ranges(block.id);
		fprintf(stderr, "Active members in buffer: %s\n",
		        !block.name.empty() ? block.name.c_str() : compiler.get_fallback_name(block.id).c_str());

		fprintf(stderr, "==================\n\n");
		for (auto &range : ranges)
		{
			const auto &name = compiler.get_member_name(block.base_type_id, range.index);

			fprintf(stderr, "Member #%3u (%s): Offset: %4u, Range: %4u\n", range.index,
			        !name.empty() ? name.c_str() : compiler.get_fallback_member_name(range.index).c_str(),
			        unsigned(range.offset), unsigned(range.range));
		}
		fprintf(stderr, "==================\n\n");
	}
}

static void print_spec_constants(const Compiler &compiler)
{
	auto spec_constants = compiler.get_specialization_constants();
	fprintf(stderr, "Specialization constants\n");
	fprintf(stderr, "==================\n\n");
	for (auto &c : spec_constants)
		fprintf(stderr, "ID: %u, Spec ID: %u\n", uint32_t(c.id), c.constant_id);
	fprintf(stderr, "==================\n\n");
}

static void print_capabilities_and_extensions(const Compiler &compiler)
{
	fprintf(stderr, "Capabilities\n");
	fprintf(stderr, "============\n");
	for (auto &capability : compiler.get_declared_capabilities())
		fprintf(stderr, "Capability: %u\n", static_cast<unsigned>(capability));
	fprintf(stderr, "============\n\n");

	fprintf(stderr, "Extensions\n");
	fprintf(stderr, "============\n");
	for (auto &ext : compiler.get_declared_extensions())
		fprintf(stderr, "Extension: %s\n", ext.c_str());
	fprintf(stderr, "============\n\n");
}

struct PLSArg
{
	PlsFormat format;
	string name;
};

struct Remap
{
	string src_name;
	string dst_name;
	unsigned components;
};

struct VariableTypeRemap
{
	string variable_name;
	string new_variable_type;
};

struct InterfaceVariableRename
{
	StorageClass storageClass;
	uint32_t location;
	string variable_name;
};

struct CLIArguments
{
	const char *input = nullptr;
	const char *output = nullptr;
	const char *cpp_interface_name = nullptr;
	uint32_t version = 0;
	uint32_t shader_model = 0;
	uint32_t msl_version = 0;
	bool es = false;
	bool set_version = false;
	bool set_shader_model = false;
	bool set_msl_version = false;
	bool set_es = false;
	bool dump_resources = false;
	bool force_temporary = false;
	bool flatten_ubo = false;
	bool fixup = false;
	bool yflip = false;
	bool sso = false;
	bool support_nonzero_baseinstance = true;
	bool msl_capture_output_to_buffer = false;
	bool msl_swizzle_texture_samples = false;
	bool msl_ios = false;
	bool msl_pad_fragment_output = false;
	bool msl_domain_lower_left = false;
	bool msl_argument_buffers = false;
	bool msl_texture_buffer_native = false;
	bool msl_framebuffer_fetch = false;
	bool msl_invariant_float_math = false;
	bool msl_emulate_cube_array = false;
	bool msl_multiview = false;
	bool msl_multiview_layered_rendering = true;
	bool msl_view_index_from_device_index = false;
	bool msl_dispatch_base = false;
	bool msl_decoration_binding = false;
	bool msl_force_active_argument_buffer_resources = false;
	bool msl_force_native_arrays = false;
	bool msl_enable_frag_depth_builtin = true;
	bool msl_enable_frag_stencil_ref_builtin = true;
	uint32_t msl_enable_frag_output_mask = 0xffffffff;
	bool msl_enable_clip_distance_user_varying = true;
	bool msl_multi_patch_workgroup = false;
	bool msl_vertex_for_tessellation = false;
	uint32_t msl_additional_fixed_sample_mask = 0xffffffff;
	bool msl_arrayed_subpass_input = false;
	uint32_t msl_r32ui_linear_texture_alignment = 4;
	uint32_t msl_r32ui_alignment_constant_id = 65535;
	bool msl_texture_1d_as_2d = false;
	bool msl_ios_use_simdgroup_functions = false;
	bool msl_emulate_subgroups = false;
	uint32_t msl_fixed_subgroup_size = 0;
	bool msl_force_sample_rate_shading = false;
	const char *msl_combined_sampler_suffix = nullptr;
	bool glsl_emit_push_constant_as_ubo = false;
	bool glsl_emit_ubo_as_plain_uniforms = false;
	bool glsl_force_flattened_io_blocks = false;
	SmallVector<pair<uint32_t, uint32_t>> glsl_ext_framebuffer_fetch;
	bool glsl_ext_framebuffer_fetch_noncoherent = false;
	bool vulkan_glsl_disable_ext_samplerless_texture_functions = false;
	bool emit_line_directives = false;
	bool enable_storage_image_qualifier_deduction = true;
	bool force_zero_initialized_variables = false;
	SmallVector<uint32_t> msl_discrete_descriptor_sets;
	SmallVector<uint32_t> msl_device_argument_buffers;
	SmallVector<pair<uint32_t, uint32_t>> msl_dynamic_buffers;
	SmallVector<pair<uint32_t, uint32_t>> msl_inline_uniform_blocks;
	SmallVector<MSLShaderInput> msl_shader_inputs;
	SmallVector<PLSArg> pls_in;
	SmallVector<PLSArg> pls_out;
	SmallVector<Remap> remaps;
	SmallVector<string> extensions;
	SmallVector<VariableTypeRemap> variable_type_remaps;
	SmallVector<InterfaceVariableRename> interface_variable_renames;
	SmallVector<HLSLVertexAttributeRemap> hlsl_attr_remap;
	SmallVector<std::pair<uint32_t, uint32_t>> masked_stage_outputs;
	SmallVector<BuiltIn> masked_stage_builtins;
	string entry;
	string entry_stage;

	struct Rename
	{
		string old_name;
		string new_name;
		ExecutionModel execution_model;
	};
	SmallVector<Rename> entry_point_rename;

	uint32_t iterations = 1;
	bool cpp = false;
	string reflect;
	bool msl = false;
	bool hlsl = false;
	bool hlsl_compat = false;
	bool hlsl_support_nonzero_base = false;
	bool hlsl_force_storage_buffer_as_uav = false;
	bool hlsl_nonwritable_uav_texture_as_srv = false;
	bool hlsl_enable_16bit_types = false;
	bool hlsl_flatten_matrix_vertex_input_semantics = false;
	HLSLBindingFlags hlsl_binding_flags = 0;
	bool vulkan_semantics = false;
	bool flatten_multidimensional_arrays = false;
	bool use_420pack_extension = true;
	bool remove_unused = false;
	bool combined_samplers_inherit_bindings = false;
};

static void print_version()
{
#ifdef HAVE_SPIRV_CROSS_GIT_VERSION
	fprintf(stderr, "%s\n", SPIRV_CROSS_GIT_REVISION);
#else
	fprintf(stderr, "Git revision unknown. Build with CMake to create timestamp and revision info.\n");
#endif
}

static void print_help_backend()
{
	// clang-format off
	fprintf(stderr, "\nSelect backend:\n"
	        "\tBy default, OpenGL-style GLSL is the target, with #version and GLSL/ESSL information inherited from the SPIR-V module if present.\n"
	        "\t[--vulkan-semantics] or [-V]:\n\t\tEmit Vulkan GLSL instead of plain GLSL. Makes use of Vulkan-only features to match SPIR-V.\n"
	        "\t[--msl]:\n\t\tEmit Metal Shading Language (MSL).\n"
	        "\t[--hlsl]:\n\t\tEmit HLSL.\n"
	        "\t[--reflect]:\n\t\tEmit JSON reflection.\n"
	        "\t[--cpp]:\n\t\tDEPRECATED. Emits C++ code.\n"
	);
	// clang-format on
}

static void print_help_glsl()
{
	// clang-format off
	fprintf(stderr, "\nGLSL options:\n"
	                "\t[--es]:\n\t\tForce ESSL.\n"
	                "\t[--no-es]:\n\t\tForce desktop GLSL.\n"
	                "\t[--version <GLSL version>]:\n\t\tE.g. --version 450 will emit '#version 450' in shader.\n"
	                "\t\tCode generation will depend on the version used.\n"
	                "\t[--flatten-ubo]:\n\t\tEmit UBOs as plain uniform arrays which are suitable for use with glUniform4*v().\n"
	                "\t\tThis can be an optimization on GL implementations where this is faster or works around buggy driver implementations.\n"
	                "\t\tE.g.: uniform MyUBO { vec4 a; float b, c, d, e; }; will be emitted as uniform vec4 MyUBO[2];\n"
	                "\t\tCaveat: You cannot mix and match floating-point and integer in the same UBO with this option.\n"
	                "\t\tLegacy GLSL/ESSL (where this flattening makes sense) does not support bit-casting, which would have been the obvious workaround.\n"
	                "\t[--extension ext]:\n\t\tAdd #extension string of your choosing to GLSL output.\n"
	                "\t\tUseful if you use variable name remapping to something that requires an extension unknown to SPIRV-Cross.\n"
	                "\t[--remove-unused-variables]:\n\t\tDo not emit interface variables which are not statically accessed by the shader.\n"
	                "\t[--separate-shader-objects]:\n\t\tRedeclare gl_PerVertex blocks to be suitable for desktop GL separate shader objects.\n"
	                "\t[--glsl-emit-push-constant-as-ubo]:\n\t\tInstead of a plain uniform of struct for push constants, emit a UBO block instead.\n"
	                "\t[--glsl-emit-ubo-as-plain-uniforms]:\n\t\tInstead of emitting UBOs, emit them as plain uniform structs.\n"
	                "\t[--glsl-remap-ext-framebuffer-fetch input-attachment color-location]:\n\t\tRemaps an input attachment to use GL_EXT_shader_framebuffer_fetch.\n"
	                "\t\tgl_LastFragData[location] is read from. The attachment to read from must be declared as an output in the shader.\n"
	                "\t[--glsl-ext-framebuffer-fetch-noncoherent]:\n\t\tUses noncoherent qualifier for framebuffer fetch.\n"
	                "\t[--vulkan-glsl-disable-ext-samplerless-texture-functions]:\n\t\tDo not allow use of GL_EXT_samperless_texture_functions, even in Vulkan GLSL.\n"
	                "\t\tUse of texelFetch and similar might have to create dummy samplers to work around it.\n"
	                "\t[--combined-samplers-inherit-bindings]:\n\t\tInherit binding information from the textures when building combined image samplers from separate textures and samplers.\n"
	                "\t[--no-support-nonzero-baseinstance]:\n\t\tWhen using gl_InstanceIndex with desktop GL,\n"
	                "\t\tassume that base instance is always 0, and do not attempt to fix up gl_InstanceID to match Vulkan semantics.\n"
	                "\t[--pls-in format input-name]:\n\t\tRemaps a subpass input with name into a GL_EXT_pixel_local_storage input.\n"
	                "\t\tEntry in PLS block is ordered where first --pls-in marks the first entry. Can be called multiple times.\n"
	                "\t\tFormats allowed: r11f_g11f_b10f, r32f, rg16f, rg16, rgb10_a2, rgba8, rgba8i, rgba8ui, rg16i, rgb10_a2ui, rg16ui, r32ui.\n"
	                "\t\tRequires ESSL.\n"
	                "\t[--pls-out format output-name]:\n\t\tRemaps a color output with name into a GL_EXT_pixel_local_storage output.\n"
	                "\t\tEntry in PLS block is ordered where first --pls-output marks the first entry. Can be called multiple times.\n"
	                "\t\tFormats allowed: r11f_g11f_b10f, r32f, rg16f, rg16, rgb10_a2, rgba8, rgba8i, rgba8ui, rg16i, rgb10_a2ui, rg16ui, r32ui.\n"
	                "\t\tRequires ESSL.\n"
	                "\t[--remap source_name target_name components]:\n\t\tRemaps a variable to a different name with N components.\n"
	                "\t\tMain use case is to remap a subpass input to gl_LastFragDepthARM.\n"
	                "\t\tE.g.:\n"
	                "\t\tuniform subpassInput uDepth;\n"
	                "\t\t--remap uDepth gl_LastFragDepthARM 1 --extension GL_ARM_shader_framebuffer_fetch_depth_stencil\n"
	                "\t[--no-420pack-extension]:\n\t\tDo not make use of GL_ARB_shading_language_420pack in older GL targets to support layout(binding).\n"
	                "\t[--remap-variable-type <variable_name> <new_variable_type>]:\n\t\tRemaps a variable type based on name.\n"
	                "\t\tPrimary use case is supporting external samplers in ESSL for video rendering on Android where you could remap a texture to a YUV one.\n"
	                "\t[--glsl-force-flattened-io-blocks]:\n\t\tAlways flatten I/O blocks and structs.\n"
	);
	// clang-format on
}

static void print_help_hlsl()
{
	// clang-format off
	fprintf(stderr, "\nHLSL options:\n"
	                "\t[--shader-model]:\n\t\tEnables a specific shader model, e.g. --shader-model 50 for SM 5.0.\n"
	                "\t[--hlsl-enable-compat]:\n\t\tAllow point size and point coord to be used, even if they won't work as expected.\n"
	                "\t\tPointSize is ignored, and PointCoord returns (0.5, 0.5).\n"
	                "\t[--hlsl-support-nonzero-basevertex-baseinstance]:\n\t\tSupport base vertex and base instance by emitting a special cbuffer declared as:\n"
	                "\t\tcbuffer SPIRV_Cross_VertexInfo { int SPIRV_Cross_BaseVertex; int SPIRV_Cross_BaseInstance; };\n"
	                "\t[--hlsl-auto-binding (push, cbv, srv, uav, sampler, all)]\n"
	                "\t\tDo not emit any : register(#) bindings for specific resource types, and rely on HLSL compiler to assign something.\n"
	                "\t[--hlsl-force-storage-buffer-as-uav]:\n\t\tAlways emit SSBOs as UAVs, even when marked as read-only.\n"
	                "\t\tNormally, SSBOs marked with NonWritable will be emitted as SRVs.\n"
	                "\t[--hlsl-nonwritable-uav-texture-as-srv]:\n\t\tEmit NonWritable storage images as SRV textures instead of UAV.\n"
	                "\t\tUsing this option messes with the type system. SPIRV-Cross cannot guarantee that this will work.\n"
	                "\t\tOne major problem area with this feature is function arguments, where we won't know if we're seeing a UAV or SRV.\n"
	                "\t\tShader must ensure that read/write state is consistent at all call sites.\n"
	                "\t[--set-hlsl-vertex-input-semantic <location> <semantic>]:\n\t\tEmits a specific vertex input semantic for a given location.\n"
	                "\t\tOtherwise, TEXCOORD# is used as semantics, where # is location.\n"
	                "\t[--hlsl-enable-16bit-types]:\n\t\tEnables native use of half/int16_t/uint16_t and ByteAddressBuffer interaction with these types. Requires SM 6.2.\n"
	                "\t[--hlsl-flatten-matrix-vertex-input-semantics]:\n\t\tEmits matrix vertex inputs with input semantics as if they were independent vectors, e.g. TEXCOORD{2,3,4} rather than matrix form TEXCOORD2_{0,1,2}.\n"
	);
	// clang-format on
}

static void print_help_msl()
{
	// clang-format off
	fprintf(stderr, "\nMSL options:\n"
	                "\t[--msl-version <MMmmpp>]:\n\t\tUses a specific MSL version, e.g. --msl-version 20100 for MSL 2.1.\n"
	                "\t[--msl-capture-output]:\n\t\tWrites geometry varyings to a buffer instead of as stage-outputs.\n"
	                "\t[--msl-swizzle-texture-samples]:\n\t\tWorks around lack of support for VkImageView component swizzles.\n"
	                "\t\tThis has a massive impact on performance and bloat. Do not use this unless you are absolutely forced to.\n"
	                "\t\tTo use this feature, the API side must pass down swizzle buffers.\n"
	                "\t\tShould only be used by translation layers as a last resort.\n"
	                "\t\tRecent Metal versions do not require this workaround.\n"
	                "\t[--msl-ios]:\n\t\tTarget iOS Metal instead of macOS Metal.\n"
	                "\t[--msl-pad-fragment-output]:\n\t\tAlways emit color outputs as 4-component variables.\n"
	                "\t\tIn Metal, the fragment shader must emit at least as many components as the render target format.\n"
	                "\t[--msl-domain-lower-left]:\n\t\tUse a lower-left tessellation domain.\n"
	                "\t[--msl-argument-buffers]:\n\t\tEmit Indirect Argument buffers instead of plain bindings.\n"
	                "\t\tRequires MSL 2.0 to be enabled.\n"
	                "\t[--msl-texture-buffer-native]:\n\t\tEnable native support for texel buffers. Otherwise, it is emulated as a normal texture.\n"
	                "\t[--msl-framebuffer-fetch]:\n\t\tImplement subpass inputs with frame buffer fetch.\n"
	                "\t\tEmits [[color(N)]] inputs in fragment stage.\n"
	                "\t\tRequires an Apple GPU.\n"
	                "\t[--msl-emulate-cube-array]:\n\t\tEmulate cube arrays with 2D array and manual math.\n"
	                "\t[--msl-discrete-descriptor-set <index>]:\n\t\tWhen using argument buffers, forces a specific descriptor set to be implemented without argument buffers.\n"
	                "\t\tUseful for implementing push descriptors in emulation layers.\n"
	                "\t\tCan be used multiple times for each descriptor set in question.\n"
	                "\t[--msl-device-argument-buffer <descriptor set index>]:\n\t\tUse device address space to hold indirect argument buffers instead of constant.\n"
	                "\t\tComes up when trying to support argument buffers which are larger than 64 KiB.\n"
	                "\t[--msl-multiview]:\n\t\tEnable SPV_KHR_multiview emulation.\n"
	                "\t[--msl-multiview-no-layered-rendering]:\n\t\tDon't set [[render_target_array_index]] in multiview shaders.\n"
	                "\t\tUseful for devices which don't support layered rendering. Only effective when --msl-multiview is enabled.\n"
	                "\t[--msl-view-index-from-device-index]:\n\t\tTreat the view index as the device index instead.\n"
	                "\t\tFor multi-GPU rendering.\n"
	                "\t[--msl-dispatch-base]:\n\t\tAdd support for vkCmdDispatchBase() or similar APIs.\n"
	                "\t\tOffsets the workgroup ID based on a buffer.\n"
	                "\t[--msl-dynamic-buffer <set index> <binding>]:\n\t\tMarks a buffer as having dynamic offset.\n"
	                "\t\tThe offset is applied in the shader with pointer arithmetic.\n"
	                "\t\tUseful for argument buffers where it is non-trivial to apply dynamic offset otherwise.\n"
	                "\t[--msl-inline-uniform-block <set index> <binding>]:\n\t\tIn argument buffers, mark an UBO as being an inline uniform block which is embedded into the argument buffer itself.\n"
	                "\t[--msl-decoration-binding]:\n\t\tUse SPIR-V bindings directly as MSL bindings.\n"
	                "\t\tThis does not work in the general case as there is no descriptor set support, and combined image samplers are split up.\n"
	                "\t\tHowever, if the shader author knows of binding limitations, this option will avoid the need for reflection on Metal side.\n"
	                "\t[--msl-force-active-argument-buffer-resources]:\n\t\tAlways emit resources which are part of argument buffers.\n"
	                "\t\tThis makes sure that similar shaders with same resource declarations can share the argument buffer as declaring an argument buffer implies an ABI.\n"
	                "\t[--msl-force-native-arrays]:\n\t\tRather than implementing array types as a templated value type ala std::array<T>, use plain, native arrays.\n"
	                "\t\tThis will lead to worse code-gen, but can work around driver bugs on certain driver revisions of certain Intel-based Macbooks where template arrays break.\n"
	                "\t[--msl-disable-frag-depth-builtin]:\n\t\tDisables FragDepth output. Useful if pipeline does not enable depth, as pipeline creation might otherwise fail.\n"
	                "\t[--msl-disable-frag-stencil-ref-builtin]:\n\t\tDisable FragStencilRef output. Useful if pipeline does not enable stencil output, as pipeline creation might otherwise fail.\n"
	                "\t[--msl-enable-frag-output-mask <mask>]:\n\t\tOnly selectively enable fragment outputs. Useful if pipeline does not enable fragment output for certain locations, as pipeline creation might otherwise fail.\n"
	                "\t[--msl-no-clip-distance-user-varying]:\n\t\tDo not emit user varyings to emulate gl_ClipDistance in fragment shaders.\n"
	                "\t[--msl-shader-input <index> <format> <size>]:\n\t\tSpecify the format of the shader input at <index>.\n"
	                "\t\t<format> can be 'any32', 'any16', 'u16', 'u8', or 'other', to indicate a 32-bit opaque value, 16-bit opaque value, 16-bit unsigned integer, 8-bit unsigned integer, "
	                "or other-typed variable. <size> is the vector length of the variable, which must be greater than or equal to that declared in the shader.\n"
	                "\t\tUseful if shader stage interfaces don't match up, as pipeline creation might otherwise fail.\n"
	                "\t[--msl-multi-patch-workgroup]:\n\t\tUse the new style of tessellation control processing, where multiple patches are processed per workgroup.\n"
					"\t\tThis should increase throughput by ensuring all the GPU's SIMD lanes are occupied, but it is not compatible with the old style.\n"
					"\t\tIn addition, this style also passes input variables in buffers directly instead of using vertex attribute processing.\n"
					"\t\tIn a future version of SPIRV-Cross, this will become the default.\n"
	                "\t[--msl-vertex-for-tessellation]:\n\t\tWhen handling a vertex shader, marks it as one that will be used with a new-style tessellation control shader.\n"
					"\t\tThe vertex shader is output to MSL as a compute kernel which outputs vertices to the buffer in the order they are received, rather than in index order as with --msl-capture-output normally.\n"
	                "\t[--msl-additional-fixed-sample-mask <mask>]:\n"
	                "\t\tSet an additional fixed sample mask. If the shader outputs a sample mask, then the final sample mask will be a bitwise AND of the two.\n"
	                "\t[--msl-arrayed-subpass-input]:\n\t\tAssume that images of dimension SubpassData have multiple layers. Layered input attachments are accessed relative to BuiltInLayer.\n"
	                "\t\tThis option has no effect if multiview is also enabled.\n"
	                "\t[--msl-r32ui-linear-texture-align <alignment>]:\n\t\tThe required alignment of linear textures of format MTLPixelFormatR32Uint.\n"
	                "\t\tThis is used to align the row stride for atomic accesses to such images.\n"
	                "\t[--msl-r32ui-linear-texture-align-constant-id <id>]:\n\t\tThe function constant ID to use for the linear texture alignment.\n"
	                "\t\tOn MSL 1.2 or later, you can override the alignment by setting this function constant.\n"
	                "\t[--msl-texture-1d-as-2d]:\n\t\tEmit Image variables of dimension Dim1D as texture2d.\n"
	                "\t\tIn Metal, 1D textures do not support all features that 2D textures do. Use this option if your code relies on these features.\n"
	                "\t[--msl-ios-use-simdgroup-functions]:\n\t\tUse simd_*() functions for subgroup ops instead of quad_*().\n"
	                "\t\tRecent Apple GPUs support SIMD-groups larger than a quad. Use this option to take advantage of this support.\n"
	                "\t[--msl-emulate-subgroups]:\n\t\tAssume subgroups of size 1.\n"
	                "\t\tIntended for Vulkan Portability implementations where Metal support for SIMD-groups is insufficient for true subgroups.\n"
	                "\t[--msl-fixed-subgroup-size <size>]:\n\t\tAssign a constant <size> to the SubgroupSize builtin.\n"
	                "\t\tIntended for Vulkan Portability implementations where VK_EXT_subgroup_size_control is not supported or disabled.\n"
	                "\t\tIf 0, assume variable subgroup size as actually exposed by Metal.\n"
	                "\t[--msl-force-sample-rate-shading]:\n\t\tForce fragment shaders to run per sample.\n"
	                "\t\tThis adds a [[sample_id]] parameter if none is already present.\n"
	                "\t[--msl-combined-sampler-suffix <suffix>]:\n\t\tUses a custom suffix for combined samplers.\n");
	// clang-format on
}

static void print_help_common()
{
	// clang-format off
	fprintf(stderr, "\nCommon options:\n"
	                "\t[--entry name]:\n\t\tUse a specific entry point. By default, the first entry point in the module is used.\n"
	                "\t[--stage <stage (vert, frag, geom, tesc, tese comp)>]:\n\t\tForces use of a certain shader stage.\n"
	                "\t\tCan disambiguate the entry point if more than one entry point exists with same name, but different stage.\n"
	                "\t[--emit-line-directives]:\n\t\tIf SPIR-V has OpLine directives, aim to emit those accurately in output code as well.\n"
	                "\t[--rename-entry-point <old> <new> <stage>]:\n\t\tRenames an entry point from what is declared in SPIR-V to code output.\n"
	                "\t\tMostly relevant for HLSL or MSL.\n"
	                "\t[--rename-interface-variable <in|out> <location> <new_variable_name>]:\n\t\tRename an interface variable based on location decoration.\n"
	                "\t[--force-zero-initialized-variables]:\n\t\tForces temporary variables to be initialized to zero.\n"
	                "\t\tCan be useful in environments where compilers do not allow potentially uninitialized variables.\n"
	                "\t\tThis usually comes up with Phi temporaries.\n"
	                "\t[--fixup-clipspace]:\n\t\tFixup Z clip-space at the end of a vertex shader. The behavior is backend-dependent.\n"
	                "\t\tGLSL: Rewrites [0, w] Z range (D3D/Metal/Vulkan) to GL-style [-w, w].\n"
	                "\t\tHLSL/MSL: Rewrites [-w, w] Z range (GL) to D3D/Metal/Vulkan-style [0, w].\n"
	                "\t[--flip-vert-y]:\n\t\tInverts gl_Position.y (or equivalent) at the end of a vertex shader. This is equivalent to using negative viewport height.\n"
	                "\t[--mask-stage-output-location <location> <component>]:\n"
	                "\t\tIf a stage output variable with matching location and component is active, optimize away the variable if applicable.\n"
	                "\t[--mask-stage-output-builtin <Position|PointSize|ClipDistance|CullDistance>]:\n"
	                "\t\tIf a stage output variable with matching builtin is active, "
	                "optimize away the variable if it can affect cross-stage linking correctness.\n"
	);
	// clang-format on
}

static void print_help_obscure()
{
	// clang-format off
	fprintf(stderr, "\nObscure options:\n"
	                "\tThese options are not meant to be used on a regular basis. They have some occasional uses in the test suite.\n"

	                "\t[--force-temporary]:\n\t\tAggressively emit temporary expressions instead of forwarding expressions. Very rarely used and under-tested.\n"
	                "\t[--revision]:\n\t\tPrints build timestamp and Git commit information (updated when cmake is configured).\n"
	                "\t[--iterations iter]:\n\t\tRecompiles the same shader over and over, benchmarking related.\n"
	                "\t[--disable-storage-image-qualifier-deduction]:\n\t\tIf storage images are received without any nonwritable or nonreadable information,\n"""
	                "\t\tdo not attempt to analyze usage, and always emit read/write state.\n"
	                "\t[--flatten-multidimensional-arrays]:\n\t\tDo not support multi-dimensional arrays and flatten them to one dimension.\n"
	                "\t[--cpp-interface-name <name>]:\n\t\tEmit a specific class name in C++ codegen.\n"
	);
	// clang-format on
}

static void print_help()
{
	print_version();

	// clang-format off
	fprintf(stderr, "Usage: spirv-cross <...>\n"
	                "\nBasic:\n"
	                "\t[SPIR-V file] (- is stdin)\n"
	                "\t[--output <output path>]: If not provided, prints output to stdout.\n"
	                "\t[--dump-resources]:\n\t\tPrints a basic reflection of the SPIR-V module along with other output.\n"
	                "\t[--help]:\n\t\tPrints this help message.\n"
	);
	// clang-format on

	print_help_backend();
	print_help_common();
	print_help_glsl();
	print_help_msl();
	print_help_hlsl();
	print_help_obscure();
}

static bool remap_generic(Compiler &compiler, const SmallVector<Resource> &resources, const Remap &remap)
{
	auto itr =
	    find_if(begin(resources), end(resources), [&remap](const Resource &res) { return res.name == remap.src_name; });

	if (itr != end(resources))
	{
		compiler.set_remapped_variable_state(itr->id, true);
		compiler.set_name(itr->id, remap.dst_name);
		compiler.set_subpass_input_remapped_components(itr->id, remap.components);
		return true;
	}
	else
		return false;
}

static vector<PlsRemap> remap_pls(const SmallVector<PLSArg> &pls_variables, const SmallVector<Resource> &resources,
                                  const SmallVector<Resource> *secondary_resources)
{
	vector<PlsRemap> ret;

	for (auto &pls : pls_variables)
	{
		bool found = false;
		for (auto &res : resources)
		{
			if (res.name == pls.name)
			{
				ret.push_back({ res.id, pls.format });
				found = true;
				break;
			}
		}

		if (!found && secondary_resources)
		{
			for (auto &res : *secondary_resources)
			{
				if (res.name == pls.name)
				{
					ret.push_back({ res.id, pls.format });
					found = true;
					break;
				}
			}
		}

		if (!found)
			fprintf(stderr, "Did not find stage input/output/target with name \"%s\".\n", pls.name.c_str());
	}

	return ret;
}

static PlsFormat pls_format(const char *str)
{
	if (!strcmp(str, "r11f_g11f_b10f"))
		return PlsR11FG11FB10F;
	else if (!strcmp(str, "r32f"))
		return PlsR32F;
	else if (!strcmp(str, "rg16f"))
		return PlsRG16F;
	else if (!strcmp(str, "rg16"))
		return PlsRG16;
	else if (!strcmp(str, "rgb10_a2"))
		return PlsRGB10A2;
	else if (!strcmp(str, "rgba8"))
		return PlsRGBA8;
	else if (!strcmp(str, "rgba8i"))
		return PlsRGBA8I;
	else if (!strcmp(str, "rgba8ui"))
		return PlsRGBA8UI;
	else if (!strcmp(str, "rg16i"))
		return PlsRG16I;
	else if (!strcmp(str, "rgb10_a2ui"))
		return PlsRGB10A2UI;
	else if (!strcmp(str, "rg16ui"))
		return PlsRG16UI;
	else if (!strcmp(str, "r32ui"))
		return PlsR32UI;
	else
		return PlsNone;
}

static ExecutionModel stage_to_execution_model(const std::string &stage)
{
	if (stage == "vert")
		return ExecutionModelVertex;
	else if (stage == "frag")
		return ExecutionModelFragment;
	else if (stage == "comp")
		return ExecutionModelGLCompute;
	else if (stage == "tesc")
		return ExecutionModelTessellationControl;
	else if (stage == "tese")
		return ExecutionModelTessellationEvaluation;
	else if (stage == "geom")
		return ExecutionModelGeometry;
	else
		SPIRV_CROSS_THROW("Invalid stage.");
}

static HLSLBindingFlags hlsl_resource_type_to_flag(const std::string &arg)
{
	if (arg == "push")
		return HLSL_BINDING_AUTO_PUSH_CONSTANT_BIT;
	else if (arg == "cbv")
		return HLSL_BINDING_AUTO_CBV_BIT;
	else if (arg == "srv")
		return HLSL_BINDING_AUTO_SRV_BIT;
	else if (arg == "uav")
		return HLSL_BINDING_AUTO_UAV_BIT;
	else if (arg == "sampler")
		return HLSL_BINDING_AUTO_SAMPLER_BIT;
	else if (arg == "all")
		return HLSL_BINDING_AUTO_ALL;
	else
	{
		fprintf(stderr, "Invalid resource type for --hlsl-auto-binding: %s\n", arg.c_str());
		return 0;
	}
}

static string compile_iteration(const CLIArguments &args, std::vector<uint32_t> spirv_file)
{
	Parser spirv_parser(move(spirv_file));
	spirv_parser.parse();

	unique_ptr<CompilerGLSL> compiler;
	bool combined_image_samplers = false;
	bool build_dummy_sampler = false;

	if (args.cpp)
	{
		compiler.reset(new CompilerCPP(move(spirv_parser.get_parsed_ir())));
		if (args.cpp_interface_name)
			static_cast<CompilerCPP *>(compiler.get())->set_interface_name(args.cpp_interface_name);
	}
	else if (args.msl)
	{
		compiler.reset(new CompilerMSL(move(spirv_parser.get_parsed_ir())));

		auto *msl_comp = static_cast<CompilerMSL *>(compiler.get());
		auto msl_opts = msl_comp->get_msl_options();
		if (args.set_msl_version)
			msl_opts.msl_version = args.msl_version;
		msl_opts.capture_output_to_buffer = args.msl_capture_output_to_buffer;
		msl_opts.swizzle_texture_samples = args.msl_swizzle_texture_samples;
		msl_opts.invariant_float_math = args.msl_invariant_float_math;
		if (args.msl_ios)
		{
			msl_opts.platform = CompilerMSL::Options::iOS;
			msl_opts.emulate_cube_array = args.msl_emulate_cube_array;
		}
		msl_opts.use_framebuffer_fetch_subpasses = args.msl_framebuffer_fetch;
		msl_opts.pad_fragment_output_components = args.msl_pad_fragment_output;
		msl_opts.tess_domain_origin_lower_left = args.msl_domain_lower_left;
		msl_opts.argument_buffers = args.msl_argument_buffers;
		msl_opts.texture_buffer_native = args.msl_texture_buffer_native;
		msl_opts.multiview = args.msl_multiview;
		msl_opts.multiview_layered_rendering = args.msl_multiview_layered_rendering;
		msl_opts.view_index_from_device_index = args.msl_view_index_from_device_index;
		msl_opts.dispatch_base = args.msl_dispatch_base;
		msl_opts.enable_decoration_binding = args.msl_decoration_binding;
		msl_opts.force_active_argument_buffer_resources = args.msl_force_active_argument_buffer_resources;
		msl_opts.force_native_arrays = args.msl_force_native_arrays;
		msl_opts.enable_frag_depth_builtin = args.msl_enable_frag_depth_builtin;
		msl_opts.enable_frag_stencil_ref_builtin = args.msl_enable_frag_stencil_ref_builtin;
		msl_opts.enable_frag_output_mask = args.msl_enable_frag_output_mask;
		msl_opts.enable_clip_distance_user_varying = args.msl_enable_clip_distance_user_varying;
		msl_opts.multi_patch_workgroup = args.msl_multi_patch_workgroup;
		msl_opts.vertex_for_tessellation = args.msl_vertex_for_tessellation;
		msl_opts.additional_fixed_sample_mask = args.msl_additional_fixed_sample_mask;
		msl_opts.arrayed_subpass_input = args.msl_arrayed_subpass_input;
		msl_opts.r32ui_linear_texture_alignment = args.msl_r32ui_linear_texture_alignment;
		msl_opts.r32ui_alignment_constant_id = args.msl_r32ui_alignment_constant_id;
		msl_opts.texture_1D_as_2D = args.msl_texture_1d_as_2d;
		msl_opts.ios_use_simdgroup_functions = args.msl_ios_use_simdgroup_functions;
		msl_opts.emulate_subgroups = args.msl_emulate_subgroups;
		msl_opts.fixed_subgroup_size = args.msl_fixed_subgroup_size;
		msl_opts.force_sample_rate_shading = args.msl_force_sample_rate_shading;
		msl_comp->set_msl_options(msl_opts);
		for (auto &v : args.msl_discrete_descriptor_sets)
			msl_comp->add_discrete_descriptor_set(v);
		for (auto &v : args.msl_device_argument_buffers)
			msl_comp->set_argument_buffer_device_address_space(v, true);
		uint32_t i = 0;
		for (auto &v : args.msl_dynamic_buffers)
			msl_comp->add_dynamic_buffer(v.first, v.second, i++);
		for (auto &v : args.msl_inline_uniform_blocks)
			msl_comp->add_inline_uniform_block(v.first, v.second);
		for (auto &v : args.msl_shader_inputs)
			msl_comp->add_msl_shader_input(v);
		if (args.msl_combined_sampler_suffix)
			msl_comp->set_combined_sampler_suffix(args.msl_combined_sampler_suffix);
	}
	else if (args.hlsl)
		compiler.reset(new CompilerHLSL(move(spirv_parser.get_parsed_ir())));
	else
	{
		combined_image_samplers = !args.vulkan_semantics;
		if (!args.vulkan_semantics || args.vulkan_glsl_disable_ext_samplerless_texture_functions)
			build_dummy_sampler = true;
		compiler.reset(new CompilerGLSL(move(spirv_parser.get_parsed_ir())));
	}

	if (!args.variable_type_remaps.empty())
	{
		auto remap_cb = [&](const SPIRType &, const string &name, string &out) -> void {
			for (const VariableTypeRemap &remap : args.variable_type_remaps)
				if (name == remap.variable_name)
					out = remap.new_variable_type;
		};

		compiler->set_variable_type_remap_callback(move(remap_cb));
	}

	for (auto &masked : args.masked_stage_outputs)
		compiler->mask_stage_output_by_location(masked.first, masked.second);
	for (auto &masked : args.masked_stage_builtins)
		compiler->mask_stage_output_by_builtin(masked);

	for (auto &rename : args.entry_point_rename)
		compiler->rename_entry_point(rename.old_name, rename.new_name, rename.execution_model);

	auto entry_points = compiler->get_entry_points_and_stages();
	auto entry_point = args.entry;
	ExecutionModel model = ExecutionModelMax;

	if (!args.entry_stage.empty())
	{
		model = stage_to_execution_model(args.entry_stage);
		if (entry_point.empty())
		{
			// Just use the first entry point with this stage.
			for (auto &e : entry_points)
			{
				if (e.execution_model == model)
				{
					entry_point = e.name;
					break;
				}
			}

			if (entry_point.empty())
			{
				fprintf(stderr, "Could not find an entry point with stage: %s\n", args.entry_stage.c_str());
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			// Make sure both stage and name exists.
			bool exists = false;
			for (auto &e : entry_points)
			{
				if (e.execution_model == model && e.name == entry_point)
				{
					exists = true;
					break;
				}
			}

			if (!exists)
			{
				fprintf(stderr, "Could not find an entry point %s with stage: %s\n", entry_point.c_str(),
				        args.entry_stage.c_str());
				exit(EXIT_FAILURE);
			}
		}
	}
	else if (!entry_point.empty())
	{
		// Make sure there is just one entry point with this name, or the stage
		// is ambiguous.
		uint32_t stage_count = 0;
		for (auto &e : entry_points)
		{
			if (e.name == entry_point)
			{
				stage_count++;
				model = e.execution_model;
			}
		}

		if (stage_count == 0)
		{
			fprintf(stderr, "There is no entry point with name: %s\n", entry_point.c_str());
			exit(EXIT_FAILURE);
		}
		else if (stage_count > 1)
		{
			fprintf(stderr, "There is more than one entry point with name: %s. Use --stage.\n", entry_point.c_str());
			exit(EXIT_FAILURE);
		}
	}

	if (!entry_point.empty())
		compiler->set_entry_point(entry_point, model);

	if (!args.set_version && !compiler->get_common_options().version)
	{
		fprintf(stderr, "Didn't specify GLSL version and SPIR-V did not specify language.\n");
		print_help();
		exit(EXIT_FAILURE);
	}

	CompilerGLSL::Options opts = compiler->get_common_options();
	if (args.set_version)
		opts.version = args.version;
	if (args.set_es)
		opts.es = args.es;
	opts.force_temporary = args.force_temporary;
	opts.separate_shader_objects = args.sso;
	opts.flatten_multidimensional_arrays = args.flatten_multidimensional_arrays;
	opts.enable_420pack_extension = args.use_420pack_extension;
	opts.vulkan_semantics = args.vulkan_semantics;
	opts.vertex.fixup_clipspace = args.fixup;
	opts.vertex.flip_vert_y = args.yflip;
	opts.vertex.support_nonzero_base_instance = args.support_nonzero_baseinstance;
	opts.emit_push_constant_as_uniform_buffer = args.glsl_emit_push_constant_as_ubo;
	opts.emit_uniform_buffer_as_plain_uniforms = args.glsl_emit_ubo_as_plain_uniforms;
	opts.force_flattened_io_blocks = args.glsl_force_flattened_io_blocks;
	opts.emit_line_directives = args.emit_line_directives;
	opts.enable_storage_image_qualifier_deduction = args.enable_storage_image_qualifier_deduction;
	opts.force_zero_initialized_variables = args.force_zero_initialized_variables;
	compiler->set_common_options(opts);

	for (auto &fetch : args.glsl_ext_framebuffer_fetch)
		compiler->remap_ext_framebuffer_fetch(fetch.first, fetch.second, !args.glsl_ext_framebuffer_fetch_noncoherent);

	// Set HLSL specific options.
	if (args.hlsl)
	{
		auto *hlsl = static_cast<CompilerHLSL *>(compiler.get());
		auto hlsl_opts = hlsl->get_hlsl_options();
		if (args.set_shader_model)
		{
			if (args.shader_model < 30)
			{
				fprintf(stderr, "Shader model earlier than 30 (3.0) not supported.\n");
				exit(EXIT_FAILURE);
			}

			hlsl_opts.shader_model = args.shader_model;
		}

		if (args.hlsl_compat)
		{
			// Enable all compat options.
			hlsl_opts.point_size_compat = true;
			hlsl_opts.point_coord_compat = true;
		}

		if (hlsl_opts.shader_model <= 30)
		{
			combined_image_samplers = true;
			build_dummy_sampler = true;
		}

		hlsl_opts.support_nonzero_base_vertex_base_instance = args.hlsl_support_nonzero_base;
		hlsl_opts.force_storage_buffer_as_uav = args.hlsl_force_storage_buffer_as_uav;
		hlsl_opts.nonwritable_uav_texture_as_srv = args.hlsl_nonwritable_uav_texture_as_srv;
		hlsl_opts.enable_16bit_types = args.hlsl_enable_16bit_types;
		hlsl_opts.flatten_matrix_vertex_input_semantics = args.hlsl_flatten_matrix_vertex_input_semantics;
		hlsl->set_hlsl_options(hlsl_opts);
		hlsl->set_resource_binding_flags(args.hlsl_binding_flags);
	}

	if (build_dummy_sampler)
	{
		uint32_t sampler = compiler->build_dummy_sampler_for_combined_images();
		if (sampler != 0)
		{
			// Set some defaults to make validation happy.
			compiler->set_decoration(sampler, DecorationDescriptorSet, 0);
			compiler->set_decoration(sampler, DecorationBinding, 0);
		}
	}

	ShaderResources res;
	if (args.remove_unused)
	{
		auto active = compiler->get_active_interface_variables();
		res = compiler->get_shader_resources(active);
		compiler->set_enabled_interface_variables(move(active));
	}
	else
		res = compiler->get_shader_resources();

	if (args.flatten_ubo)
	{
		for (auto &ubo : res.uniform_buffers)
			compiler->flatten_buffer_block(ubo.id);
		for (auto &ubo : res.push_constant_buffers)
			compiler->flatten_buffer_block(ubo.id);
	}

	auto pls_inputs = remap_pls(args.pls_in, res.stage_inputs, &res.subpass_inputs);
	auto pls_outputs = remap_pls(args.pls_out, res.stage_outputs, nullptr);
	compiler->remap_pixel_local_storage(move(pls_inputs), move(pls_outputs));

	for (auto &ext : args.extensions)
		compiler->require_extension(ext);

	for (auto &remap : args.remaps)
	{
		if (remap_generic(*compiler, res.stage_inputs, remap))
			continue;
		if (remap_generic(*compiler, res.stage_outputs, remap))
			continue;
		if (remap_generic(*compiler, res.subpass_inputs, remap))
			continue;
	}

	for (auto &rename : args.interface_variable_renames)
	{
		if (rename.storageClass == StorageClassInput)
			spirv_cross_util::rename_interface_variable(*compiler, res.stage_inputs, rename.location,
			                                            rename.variable_name);
		else if (rename.storageClass == StorageClassOutput)
			spirv_cross_util::rename_interface_variable(*compiler, res.stage_outputs, rename.location,
			                                            rename.variable_name);
		else
		{
			fprintf(stderr, "error at --rename-interface-variable <in|out> ...\n");
			exit(EXIT_FAILURE);
		}
	}

	if (combined_image_samplers)
	{
		compiler->build_combined_image_samplers();
		if (args.combined_samplers_inherit_bindings)
			spirv_cross_util::inherit_combined_sampler_bindings(*compiler);

		// Give the remapped combined samplers new names.
		for (auto &remap : compiler->get_combined_image_samplers())
		{
			compiler->set_name(remap.combined_id, join("SPIRV_Cross_Combined", compiler->get_name(remap.image_id),
			                                           compiler->get_name(remap.sampler_id)));
		}
	}

	if (args.hlsl)
	{
		auto *hlsl_compiler = static_cast<CompilerHLSL *>(compiler.get());
		uint32_t new_builtin = hlsl_compiler->remap_num_workgroups_builtin();
		if (new_builtin)
		{
			hlsl_compiler->set_decoration(new_builtin, DecorationDescriptorSet, 0);
			hlsl_compiler->set_decoration(new_builtin, DecorationBinding, 0);
		}
	}

	if (args.hlsl)
	{
		for (auto &remap : args.hlsl_attr_remap)
			static_cast<CompilerHLSL *>(compiler.get())->add_vertex_attribute_remap(remap);
	}

	auto ret = compiler->compile();

	if (args.dump_resources)
	{
		compiler->update_active_builtins();
		print_resources(*compiler, res);
		print_push_constant_resources(*compiler, res.push_constant_buffers);
		print_spec_constants(*compiler);
		print_capabilities_and_extensions(*compiler);
	}

	return ret;
}

static int main_inner(int argc, char *argv[])
{
	CLIArguments args;
	CLICallbacks cbs;

	cbs.add("--help", [](CLIParser &parser) {
		print_help();
		parser.end();
	});
	cbs.add("--revision", [](CLIParser &parser) {
		print_version();
		parser.end();
	});
	cbs.add("--output", [&args](CLIParser &parser) { args.output = parser.next_string(); });
	cbs.add("--es", [&args](CLIParser &) {
		args.es = true;
		args.set_es = true;
	});
	cbs.add("--no-es", [&args](CLIParser &) {
		args.es = false;
		args.set_es = true;
	});
	cbs.add("--version", [&args](CLIParser &parser) {
		args.version = parser.next_uint();
		args.set_version = true;
	});
	cbs.add("--dump-resources", [&args](CLIParser &) { args.dump_resources = true; });
	cbs.add("--force-temporary", [&args](CLIParser &) { args.force_temporary = true; });
	cbs.add("--flatten-ubo", [&args](CLIParser &) { args.flatten_ubo = true; });
	cbs.add("--fixup-clipspace", [&args](CLIParser &) { args.fixup = true; });
	cbs.add("--flip-vert-y", [&args](CLIParser &) { args.yflip = true; });
	cbs.add("--iterations", [&args](CLIParser &parser) { args.iterations = parser.next_uint(); });
	cbs.add("--cpp", [&args](CLIParser &) { args.cpp = true; });
	cbs.add("--reflect", [&args](CLIParser &parser) { args.reflect = parser.next_value_string("json"); });
	cbs.add("--cpp-interface-name", [&args](CLIParser &parser) { args.cpp_interface_name = parser.next_string(); });
	cbs.add("--metal", [&args](CLIParser &) { args.msl = true; }); // Legacy compatibility
	cbs.add("--glsl-emit-push-constant-as-ubo", [&args](CLIParser &) { args.glsl_emit_push_constant_as_ubo = true; });
	cbs.add("--glsl-emit-ubo-as-plain-uniforms", [&args](CLIParser &) { args.glsl_emit_ubo_as_plain_uniforms = true; });
	cbs.add("--glsl-force-flattened-io-blocks", [&args](CLIParser &) { args.glsl_force_flattened_io_blocks = true; });
	cbs.add("--glsl-remap-ext-framebuffer-fetch", [&args](CLIParser &parser) {
		uint32_t input_index = parser.next_uint();
		uint32_t color_attachment = parser.next_uint();
		args.glsl_ext_framebuffer_fetch.push_back({ input_index, color_attachment });
	});
	cbs.add("--glsl-ext-framebuffer-fetch-noncoherent", [&args](CLIParser &) {
		args.glsl_ext_framebuffer_fetch_noncoherent = true;
	});
	cbs.add("--vulkan-glsl-disable-ext-samplerless-texture-functions",
	        [&args](CLIParser &) { args.vulkan_glsl_disable_ext_samplerless_texture_functions = true; });
	cbs.add("--disable-storage-image-qualifier-deduction",
	        [&args](CLIParser &) { args.enable_storage_image_qualifier_deduction = false; });
	cbs.add("--force-zero-initialized-variables",
	        [&args](CLIParser &) { args.force_zero_initialized_variables = true; });
	cbs.add("--msl", [&args](CLIParser &) { args.msl = true; });
	cbs.add("--hlsl", [&args](CLIParser &) { args.hlsl = true; });
	cbs.add("--hlsl-enable-compat", [&args](CLIParser &) { args.hlsl_compat = true; });
	cbs.add("--hlsl-support-nonzero-basevertex-baseinstance",
	        [&args](CLIParser &) { args.hlsl_support_nonzero_base = true; });
	cbs.add("--hlsl-auto-binding", [&args](CLIParser &parser) {
		args.hlsl_binding_flags |= hlsl_resource_type_to_flag(parser.next_string());
	});
	cbs.add("--hlsl-force-storage-buffer-as-uav",
	        [&args](CLIParser &) { args.hlsl_force_storage_buffer_as_uav = true; });
	cbs.add("--hlsl-nonwritable-uav-texture-as-srv",
	        [&args](CLIParser &) { args.hlsl_nonwritable_uav_texture_as_srv = true; });
	cbs.add("--hlsl-enable-16bit-types", [&args](CLIParser &) { args.hlsl_enable_16bit_types = true; });
	cbs.add("--hlsl-flatten-matrix-vertex-input-semantics",
	        [&args](CLIParser &) { args.hlsl_flatten_matrix_vertex_input_semantics = true; });
	cbs.add("--vulkan-semantics", [&args](CLIParser &) { args.vulkan_semantics = true; });
	cbs.add("-V", [&args](CLIParser &) { args.vulkan_semantics = true; });
	cbs.add("--flatten-multidimensional-arrays", [&args](CLIParser &) { args.flatten_multidimensional_arrays = true; });
	cbs.add("--no-420pack-extension", [&args](CLIParser &) { args.use_420pack_extension = false; });
	cbs.add("--msl-capture-output", [&args](CLIParser &) { args.msl_capture_output_to_buffer = true; });
	cbs.add("--msl-swizzle-texture-samples", [&args](CLIParser &) { args.msl_swizzle_texture_samples = true; });
	cbs.add("--msl-ios", [&args](CLIParser &) { args.msl_ios = true; });
	cbs.add("--msl-pad-fragment-output", [&args](CLIParser &) { args.msl_pad_fragment_output = true; });
	cbs.add("--msl-domain-lower-left", [&args](CLIParser &) { args.msl_domain_lower_left = true; });
	cbs.add("--msl-argument-buffers", [&args](CLIParser &) { args.msl_argument_buffers = true; });
	cbs.add("--msl-discrete-descriptor-set",
	        [&args](CLIParser &parser) { args.msl_discrete_descriptor_sets.push_back(parser.next_uint()); });
	cbs.add("--msl-device-argument-buffer",
	        [&args](CLIParser &parser) { args.msl_device_argument_buffers.push_back(parser.next_uint()); });
	cbs.add("--msl-texture-buffer-native", [&args](CLIParser &) { args.msl_texture_buffer_native = true; });
	cbs.add("--msl-framebuffer-fetch", [&args](CLIParser &) { args.msl_framebuffer_fetch = true; });
	cbs.add("--msl-invariant-float-math", [&args](CLIParser &) { args.msl_invariant_float_math = true; });
	cbs.add("--msl-emulate-cube-array", [&args](CLIParser &) { args.msl_emulate_cube_array = true; });
	cbs.add("--msl-multiview", [&args](CLIParser &) { args.msl_multiview = true; });
	cbs.add("--msl-multiview-no-layered-rendering",
	        [&args](CLIParser &) { args.msl_multiview_layered_rendering = false; });
	cbs.add("--msl-view-index-from-device-index",
	        [&args](CLIParser &) { args.msl_view_index_from_device_index = true; });
	cbs.add("--msl-dispatch-base", [&args](CLIParser &) { args.msl_dispatch_base = true; });
	cbs.add("--msl-dynamic-buffer", [&args](CLIParser &parser) {
		args.msl_argument_buffers = true;
		// Make sure next_uint() is called in-order.
		uint32_t desc_set = parser.next_uint();
		uint32_t binding = parser.next_uint();
		args.msl_dynamic_buffers.push_back(make_pair(desc_set, binding));
	});
	cbs.add("--msl-decoration-binding", [&args](CLIParser &) { args.msl_decoration_binding = true; });
	cbs.add("--msl-force-active-argument-buffer-resources",
	        [&args](CLIParser &) { args.msl_force_active_argument_buffer_resources = true; });
	cbs.add("--msl-inline-uniform-block", [&args](CLIParser &parser) {
		args.msl_argument_buffers = true;
		// Make sure next_uint() is called in-order.
		uint32_t desc_set = parser.next_uint();
		uint32_t binding = parser.next_uint();
		args.msl_inline_uniform_blocks.push_back(make_pair(desc_set, binding));
	});
	cbs.add("--msl-force-native-arrays", [&args](CLIParser &) { args.msl_force_native_arrays = true; });
	cbs.add("--msl-disable-frag-depth-builtin", [&args](CLIParser &) { args.msl_enable_frag_depth_builtin = false; });
	cbs.add("--msl-disable-frag-stencil-ref-builtin",
	        [&args](CLIParser &) { args.msl_enable_frag_stencil_ref_builtin = false; });
	cbs.add("--msl-enable-frag-output-mask",
	        [&args](CLIParser &parser) { args.msl_enable_frag_output_mask = parser.next_hex_uint(); });
	cbs.add("--msl-no-clip-distance-user-varying",
	        [&args](CLIParser &) { args.msl_enable_clip_distance_user_varying = false; });
	cbs.add("--msl-shader-input", [&args](CLIParser &parser) {
		MSLShaderInput input;
		// Make sure next_uint() is called in-order.
		input.location = parser.next_uint();
		const char *format = parser.next_value_string("other");
		if (strcmp(format, "any32") == 0)
			input.format = MSL_SHADER_INPUT_FORMAT_ANY32;
		else if (strcmp(format, "any16") == 0)
			input.format = MSL_SHADER_INPUT_FORMAT_ANY16;
		else if (strcmp(format, "u16") == 0)
			input.format = MSL_SHADER_INPUT_FORMAT_UINT16;
		else if (strcmp(format, "u8") == 0)
			input.format = MSL_SHADER_INPUT_FORMAT_UINT8;
		else
			input.format = MSL_SHADER_INPUT_FORMAT_OTHER;
		input.vecsize = parser.next_uint();
		args.msl_shader_inputs.push_back(input);
	});
	cbs.add("--msl-multi-patch-workgroup", [&args](CLIParser &) { args.msl_multi_patch_workgroup = true; });
	cbs.add("--msl-vertex-for-tessellation", [&args](CLIParser &) { args.msl_vertex_for_tessellation = true; });
	cbs.add("--msl-additional-fixed-sample-mask",
	        [&args](CLIParser &parser) { args.msl_additional_fixed_sample_mask = parser.next_hex_uint(); });
	cbs.add("--msl-arrayed-subpass-input", [&args](CLIParser &) { args.msl_arrayed_subpass_input = true; });
	cbs.add("--msl-r32ui-linear-texture-align",
	        [&args](CLIParser &parser) { args.msl_r32ui_linear_texture_alignment = parser.next_uint(); });
	cbs.add("--msl-r32ui-linear-texture-align-constant-id",
	        [&args](CLIParser &parser) { args.msl_r32ui_alignment_constant_id = parser.next_uint(); });
	cbs.add("--msl-texture-1d-as-2d", [&args](CLIParser &) { args.msl_texture_1d_as_2d = true; });
	cbs.add("--msl-ios-use-simdgroup-functions", [&args](CLIParser &) { args.msl_ios_use_simdgroup_functions = true; });
	cbs.add("--msl-emulate-subgroups", [&args](CLIParser &) { args.msl_emulate_subgroups = true; });
	cbs.add("--msl-fixed-subgroup-size",
	        [&args](CLIParser &parser) { args.msl_fixed_subgroup_size = parser.next_uint(); });
	cbs.add("--msl-force-sample-rate-shading", [&args](CLIParser &) { args.msl_force_sample_rate_shading = true; });
	cbs.add("--msl-combined-sampler-suffix", [&args](CLIParser &parser) {
		args.msl_combined_sampler_suffix = parser.next_string();
	});
	cbs.add("--extension", [&args](CLIParser &parser) { args.extensions.push_back(parser.next_string()); });
	cbs.add("--rename-entry-point", [&args](CLIParser &parser) {
		auto old_name = parser.next_string();
		auto new_name = parser.next_string();
		auto model = stage_to_execution_model(parser.next_string());
		args.entry_point_rename.push_back({ old_name, new_name, move(model) });
	});
	cbs.add("--entry", [&args](CLIParser &parser) { args.entry = parser.next_string(); });
	cbs.add("--stage", [&args](CLIParser &parser) { args.entry_stage = parser.next_string(); });
	cbs.add("--separate-shader-objects", [&args](CLIParser &) { args.sso = true; });
	cbs.add("--set-hlsl-vertex-input-semantic", [&args](CLIParser &parser) {
		HLSLVertexAttributeRemap remap;
		remap.location = parser.next_uint();
		remap.semantic = parser.next_string();
		args.hlsl_attr_remap.push_back(move(remap));
	});

	cbs.add("--remap", [&args](CLIParser &parser) {
		string src = parser.next_string();
		string dst = parser.next_string();
		uint32_t components = parser.next_uint();
		args.remaps.push_back({ move(src), move(dst), components });
	});

	cbs.add("--remap-variable-type", [&args](CLIParser &parser) {
		string var_name = parser.next_string();
		string new_type = parser.next_string();
		args.variable_type_remaps.push_back({ move(var_name), move(new_type) });
	});

	cbs.add("--rename-interface-variable", [&args](CLIParser &parser) {
		StorageClass cls = StorageClassMax;
		string clsStr = parser.next_string();
		if (clsStr == "in")
			cls = StorageClassInput;
		else if (clsStr == "out")
			cls = StorageClassOutput;

		uint32_t loc = parser.next_uint();
		string var_name = parser.next_string();
		args.interface_variable_renames.push_back({ cls, loc, move(var_name) });
	});

	cbs.add("--pls-in", [&args](CLIParser &parser) {
		auto fmt = pls_format(parser.next_string());
		auto name = parser.next_string();
		args.pls_in.push_back({ move(fmt), move(name) });
	});
	cbs.add("--pls-out", [&args](CLIParser &parser) {
		auto fmt = pls_format(parser.next_string());
		auto name = parser.next_string();
		args.pls_out.push_back({ move(fmt), move(name) });
	});
	cbs.add("--shader-model", [&args](CLIParser &parser) {
		args.shader_model = parser.next_uint();
		args.set_shader_model = true;
	});
	cbs.add("--msl-version", [&args](CLIParser &parser) {
		args.msl_version = parser.next_uint();
		args.set_msl_version = true;
	});

	cbs.add("--remove-unused-variables", [&args](CLIParser &) { args.remove_unused = true; });
	cbs.add("--combined-samplers-inherit-bindings",
	        [&args](CLIParser &) { args.combined_samplers_inherit_bindings = true; });

	cbs.add("--no-support-nonzero-baseinstance", [&](CLIParser &) { args.support_nonzero_baseinstance = false; });
	cbs.add("--emit-line-directives", [&args](CLIParser &) { args.emit_line_directives = true; });

	cbs.add("--mask-stage-output-location", [&](CLIParser &parser) {
		uint32_t location = parser.next_uint();
		uint32_t component = parser.next_uint();
		args.masked_stage_outputs.push_back({ location, component });
	});

	cbs.add("--mask-stage-output-builtin", [&](CLIParser &parser) {
		BuiltIn masked_builtin = BuiltInMax;
		std::string builtin = parser.next_string();
		if (builtin == "Position")
			masked_builtin = BuiltInPosition;
		else if (builtin == "PointSize")
			masked_builtin = BuiltInPointSize;
		else if (builtin == "CullDistance")
			masked_builtin = BuiltInCullDistance;
		else if (builtin == "ClipDistance")
			masked_builtin = BuiltInClipDistance;
		else
		{
			print_help();
			exit(EXIT_FAILURE);
		}
		args.masked_stage_builtins.push_back(masked_builtin);
	});

	cbs.default_handler = [&args](const char *value) { args.input = value; };
	cbs.add("-", [&args](CLIParser &) { args.input = "-"; });
	cbs.error_handler = [] { print_help(); };

	CLIParser parser{ move(cbs), argc - 1, argv + 1 };
	if (!parser.parse())
		return EXIT_FAILURE;
	else if (parser.ended_state)
		return EXIT_SUCCESS;

	if (!args.input)
	{
		fprintf(stderr, "Didn't specify input file.\n");
		print_help();
		return EXIT_FAILURE;
	}

	auto spirv_file = read_spirv_file(args.input);
	if (spirv_file.empty())
		return EXIT_FAILURE;

	// Special case reflection because it has little to do with the path followed by code-outputting compilers
	if (!args.reflect.empty())
	{
		Parser spirv_parser(move(spirv_file));
		spirv_parser.parse();

		CompilerReflection compiler(move(spirv_parser.get_parsed_ir()));
		compiler.set_format(args.reflect);
		auto json = compiler.compile();
		if (args.output)
			write_string_to_file(args.output, json.c_str());
		else
			printf("%s", json.c_str());
		return EXIT_SUCCESS;
	}

	string compiled_output;

	if (args.iterations == 1)
		compiled_output = compile_iteration(args, move(spirv_file));
	else
	{
		for (unsigned i = 0; i < args.iterations; i++)
			compiled_output = compile_iteration(args, spirv_file);
	}

	if (args.output)
		write_string_to_file(args.output, compiled_output.c_str());
	else
		printf("%s", compiled_output.c_str());

	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
#ifdef SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS
	return main_inner(argc, argv);
#else
	// Make sure we catch the exception or it just disappears into the aether on Windows.
	try
	{
		return main_inner(argc, argv);
	}
	catch (const std::exception &e)
	{
		fprintf(stderr, "SPIRV-Cross threw an exception: %s\n", e.what());
		return EXIT_FAILURE;
	}
#endif
}
