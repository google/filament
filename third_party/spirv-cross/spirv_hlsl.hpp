/*
 * Copyright 2016-2020 Robert Konrad
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

#ifndef SPIRV_HLSL_HPP
#define SPIRV_HLSL_HPP

#include "spirv_glsl.hpp"
#include <utility>

namespace SPIRV_CROSS_NAMESPACE
{
// Interface which remaps vertex inputs to a fixed semantic name to make linking easier.
struct HLSLVertexAttributeRemap
{
	uint32_t location;
	std::string semantic;
};
// Specifying a root constant (d3d12) or push constant range (vulkan).
//
// `start` and `end` denotes the range of the root constant in bytes.
// Both values need to be multiple of 4.
struct RootConstants
{
	uint32_t start;
	uint32_t end;

	uint32_t binding;
	uint32_t space;
};

// For finer control, decorations may be removed from specific resources instead with unset_decoration().
enum HLSLBindingFlagBits
{
	HLSL_BINDING_AUTO_NONE_BIT = 0,

	// Push constant (root constant) resources will be declared as CBVs (b-space) without a register() declaration.
	// A register will be automatically assigned by the D3D compiler, but must therefore be reflected in D3D-land.
	// Push constants do not normally have a DecorationBinding set, but if they do, this can be used to ignore it.
	HLSL_BINDING_AUTO_PUSH_CONSTANT_BIT = 1 << 0,

	// cbuffer resources will be declared as CBVs (b-space) without a register() declaration.
	// A register will be automatically assigned, but must be reflected in D3D-land.
	HLSL_BINDING_AUTO_CBV_BIT = 1 << 1,

	// All SRVs (t-space) will be declared without a register() declaration.
	HLSL_BINDING_AUTO_SRV_BIT = 1 << 2,

	// All UAVs (u-space) will be declared without a register() declaration.
	HLSL_BINDING_AUTO_UAV_BIT = 1 << 3,

	// All samplers (s-space) will be declared without a register() declaration.
	HLSL_BINDING_AUTO_SAMPLER_BIT = 1 << 4,

	// No resources will be declared with register().
	HLSL_BINDING_AUTO_ALL = 0x7fffffff
};
using HLSLBindingFlags = uint32_t;

// By matching stage, desc_set and binding for a SPIR-V resource,
// register bindings are set based on whether the HLSL resource is a
// CBV, UAV, SRV or Sampler. A single binding in SPIR-V might contain multiple
// resource types, e.g. COMBINED_IMAGE_SAMPLER, and SRV/Sampler bindings will be used respectively.
// On SM 5.0 and lower, register_space is ignored.
//
// To remap a push constant block which does not have any desc_set/binding associated with it,
// use ResourceBindingPushConstant{DescriptorSet,Binding} as values for desc_set/binding.
// For deeper control of push constants, set_root_constant_layouts() can be used instead.
struct HLSLResourceBinding
{
	spv::ExecutionModel stage = spv::ExecutionModelMax;
	uint32_t desc_set = 0;
	uint32_t binding = 0;

	struct Binding
	{
		uint32_t register_space = 0;
		uint32_t register_binding = 0;
	} cbv, uav, srv, sampler;
};

class CompilerHLSL : public CompilerGLSL
{
public:
	struct Options
	{
		uint32_t shader_model = 30; // TODO: map ps_4_0_level_9_0,... somehow

		// Allows the PointSize builtin, and ignores it, as PointSize is not supported in HLSL.
		bool point_size_compat = false;

		// Allows the PointCoord builtin, returns float2(0.5, 0.5), as PointCoord is not supported in HLSL.
		bool point_coord_compat = false;

		// If true, the backend will assume that VertexIndex and InstanceIndex will need to apply
		// a base offset, and you will need to fill in a cbuffer with offsets.
		// Set to false if you know you will never use base instance or base vertex
		// functionality as it might remove an internal cbuffer.
		bool support_nonzero_base_vertex_base_instance = false;

		// Forces a storage buffer to always be declared as UAV, even if the readonly decoration is used.
		// By default, a readonly storage buffer will be declared as ByteAddressBuffer (SRV) instead.
		bool force_storage_buffer_as_uav = false;
	};

	explicit CompilerHLSL(std::vector<uint32_t> spirv_)
	    : CompilerGLSL(std::move(spirv_))
	{
	}

	CompilerHLSL(const uint32_t *ir_, size_t size)
	    : CompilerGLSL(ir_, size)
	{
	}

	explicit CompilerHLSL(const ParsedIR &ir_)
	    : CompilerGLSL(ir_)
	{
	}

	explicit CompilerHLSL(ParsedIR &&ir_)
	    : CompilerGLSL(std::move(ir_))
	{
	}

	const Options &get_hlsl_options() const
	{
		return hlsl_options;
	}

	void set_hlsl_options(const Options &opts)
	{
		hlsl_options = opts;
	}

	// Optionally specify a custom root constant layout.
	//
	// Push constants ranges will be split up according to the
	// layout specified.
	void set_root_constant_layouts(std::vector<RootConstants> layout);

	// Compiles and remaps vertex attributes at specific locations to a fixed semantic.
	// The default is TEXCOORD# where # denotes location.
	// Matrices are unrolled to vectors with notation ${SEMANTIC}_#, where # denotes row.
	// $SEMANTIC is either TEXCOORD# or a semantic name specified here.
	void add_vertex_attribute_remap(const HLSLVertexAttributeRemap &vertex_attributes);
	std::string compile() override;

	// This is a special HLSL workaround for the NumWorkGroups builtin.
	// This does not exist in HLSL, so the calling application must create a dummy cbuffer in
	// which the application will store this builtin.
	// The cbuffer layout will be:
	// cbuffer SPIRV_Cross_NumWorkgroups : register(b#, space#) { uint3 SPIRV_Cross_NumWorkgroups_count; };
	// This must be called before compile().
	// The function returns 0 if NumWorkGroups builtin is not statically used in the shader from the current entry point.
	// If non-zero, this returns the variable ID of a cbuffer which corresponds to
	// the cbuffer declared above. By default, no binding or descriptor set decoration is set,
	// so the calling application should declare explicit bindings on this ID before calling compile().
	VariableID remap_num_workgroups_builtin();

	// Controls how resource bindings are declared in the output HLSL.
	void set_resource_binding_flags(HLSLBindingFlags flags);

	// resource is a resource binding to indicate the HLSL CBV, SRV, UAV or sampler binding
	// to use for a particular SPIR-V description set
	// and binding. If resource bindings are provided,
	// is_hlsl_resource_binding_used() will return true after calling ::compile() if
	// the set/binding combination was used by the HLSL code.
	void add_hlsl_resource_binding(const HLSLResourceBinding &resource);
	bool is_hlsl_resource_binding_used(spv::ExecutionModel model, uint32_t set, uint32_t binding) const;

private:
	std::string type_to_glsl(const SPIRType &type, uint32_t id = 0) override;
	std::string image_type_hlsl(const SPIRType &type, uint32_t id);
	std::string image_type_hlsl_modern(const SPIRType &type, uint32_t id);
	std::string image_type_hlsl_legacy(const SPIRType &type, uint32_t id);
	void emit_function_prototype(SPIRFunction &func, const Bitset &return_flags) override;
	void emit_hlsl_entry_point();
	void emit_header() override;
	void emit_resources();
	void declare_undefined_values() override;
	void emit_interface_block_globally(const SPIRVariable &type);
	void emit_interface_block_in_struct(const SPIRVariable &type, std::unordered_set<uint32_t> &active_locations);
	void emit_builtin_inputs_in_struct();
	void emit_builtin_outputs_in_struct();
	void emit_texture_op(const Instruction &i) override;
	void emit_instruction(const Instruction &instruction) override;
	void emit_glsl_op(uint32_t result_type, uint32_t result_id, uint32_t op, const uint32_t *args,
	                  uint32_t count) override;
	void emit_buffer_block(const SPIRVariable &type) override;
	void emit_push_constant_block(const SPIRVariable &var) override;
	void emit_uniform(const SPIRVariable &var) override;
	void emit_modern_uniform(const SPIRVariable &var);
	void emit_legacy_uniform(const SPIRVariable &var);
	void emit_specialization_constants_and_structs();
	void emit_composite_constants();
	void emit_fixup() override;
	std::string builtin_to_glsl(spv::BuiltIn builtin, spv::StorageClass storage) override;
	std::string layout_for_member(const SPIRType &type, uint32_t index) override;
	std::string to_interpolation_qualifiers(const Bitset &flags) override;
	std::string bitcast_glsl_op(const SPIRType &result_type, const SPIRType &argument_type) override;
	std::string to_func_call_arg(const SPIRFunction::Parameter &arg, uint32_t id) override;
	std::string to_sampler_expression(uint32_t id);
	std::string to_resource_binding(const SPIRVariable &var);
	std::string to_resource_binding_sampler(const SPIRVariable &var);
	std::string to_resource_register(HLSLBindingFlagBits flag, char space, uint32_t binding, uint32_t set);
	void emit_sampled_image_op(uint32_t result_type, uint32_t result_id, uint32_t image_id, uint32_t samp_id) override;
	void emit_access_chain(const Instruction &instruction);
	void emit_load(const Instruction &instruction);
	void read_access_chain(std::string *expr, const std::string &lhs, const SPIRAccessChain &chain);
	void read_access_chain_struct(const std::string &lhs, const SPIRAccessChain &chain);
	void read_access_chain_array(const std::string &lhs, const SPIRAccessChain &chain);
	void write_access_chain(const SPIRAccessChain &chain, uint32_t value, const SmallVector<uint32_t> &composite_chain);
	void write_access_chain_struct(const SPIRAccessChain &chain, uint32_t value,
	                               const SmallVector<uint32_t> &composite_chain);
	void write_access_chain_array(const SPIRAccessChain &chain, uint32_t value,
	                              const SmallVector<uint32_t> &composite_chain);
	std::string write_access_chain_value(uint32_t value, const SmallVector<uint32_t> &composite_chain, bool enclose);
	void emit_store(const Instruction &instruction);
	void emit_atomic(const uint32_t *ops, uint32_t length, spv::Op op);
	void emit_subgroup_op(const Instruction &i) override;
	void emit_block_hints(const SPIRBlock &block) override;

	void emit_struct_member(const SPIRType &type, uint32_t member_type_id, uint32_t index, const std::string &qualifier,
	                        uint32_t base_offset = 0) override;

	const char *to_storage_qualifiers_glsl(const SPIRVariable &var) override;
	void replace_illegal_names() override;

	Options hlsl_options;

	// TODO: Refactor this to be more similar to MSL, maybe have some common system in place?
	bool requires_op_fmod = false;
	bool requires_fp16_packing = false;
	bool requires_explicit_fp16_packing = false;
	bool requires_unorm8_packing = false;
	bool requires_snorm8_packing = false;
	bool requires_unorm16_packing = false;
	bool requires_snorm16_packing = false;
	bool requires_bitfield_insert = false;
	bool requires_bitfield_extract = false;
	bool requires_inverse_2x2 = false;
	bool requires_inverse_3x3 = false;
	bool requires_inverse_4x4 = false;
	bool requires_scalar_reflect = false;
	bool requires_scalar_refract = false;
	bool requires_scalar_faceforward = false;
	uint64_t required_textureSizeVariants = 0;
	void require_texture_query_variant(const SPIRType &type);

	enum TextureQueryVariantDim
	{
		Query1D = 0,
		Query1DArray,
		Query2D,
		Query2DArray,
		Query3D,
		QueryBuffer,
		QueryCube,
		QueryCubeArray,
		Query2DMS,
		Query2DMSArray,
		QueryDimCount
	};

	enum TextureQueryVariantType
	{
		QueryTypeFloat = 0,
		QueryTypeInt = 16,
		QueryTypeUInt = 32,
		QueryTypeCount = 3
	};

	void emit_builtin_variables();
	bool require_output = false;
	bool require_input = false;
	SmallVector<HLSLVertexAttributeRemap> remap_vertex_attributes;

	uint32_t type_to_consumed_locations(const SPIRType &type) const;

	void emit_io_block(const SPIRVariable &var);
	std::string to_semantic(uint32_t location, spv::ExecutionModel em, spv::StorageClass sc);

	uint32_t num_workgroups_builtin = 0;
	HLSLBindingFlags resource_binding_flags = 0;

	// Custom root constant layout, which should be emitted
	// when translating push constant ranges.
	std::vector<RootConstants> root_constants_layout;

	void validate_shader_model();

	std::string get_unique_identifier();
	uint32_t unique_identifier_count = 0;

	std::unordered_map<StageSetBinding, std::pair<HLSLResourceBinding, bool>, InternalHasher> resource_bindings;
	void remap_hlsl_resource_binding(HLSLBindingFlagBits type, uint32_t &desc_set, uint32_t &binding);
};
} // namespace SPIRV_CROSS_NAMESPACE

#endif
