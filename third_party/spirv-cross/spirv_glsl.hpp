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

#ifndef SPIRV_CROSS_GLSL_HPP
#define SPIRV_CROSS_GLSL_HPP

#include "spirv_cross.hpp"
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace spirv_cross
{
enum PlsFormat
{
	PlsNone = 0,

	PlsR11FG11FB10F,
	PlsR32F,
	PlsRG16F,
	PlsRGB10A2,
	PlsRGBA8,
	PlsRG16,

	PlsRGBA8I,
	PlsRG16I,

	PlsRGB10A2UI,
	PlsRGBA8UI,
	PlsRG16UI,
	PlsR32UI
};

struct PlsRemap
{
	uint32_t id;
	PlsFormat format;
};

class CompilerGLSL : public Compiler
{
public:
	struct Options
	{
		// The shading language version. Corresponds to #version $VALUE.
		uint32_t version = 450;

		// Emit the OpenGL ES shading language instead of desktop OpenGL.
		bool es = false;

		// Debug option to always emit temporary variables for all expressions.
		bool force_temporary = false;

		// If true, Vulkan GLSL features are used instead of GL-compatible features.
		// Mostly useful for debugging SPIR-V files.
		bool vulkan_semantics = false;

		// If true, gl_PerVertex is explicitly redeclared in vertex, geometry and tessellation shaders.
		// The members of gl_PerVertex is determined by which built-ins are declared by the shader.
		// This option is ignored in ES versions, as redeclaration in ES is not required, and it depends on a different extension
		// (EXT_shader_io_blocks) which makes things a bit more fuzzy.
		bool separate_shader_objects = false;

		// Flattens multidimensional arrays, e.g. float foo[a][b][c] into single-dimensional arrays,
		// e.g. float foo[a * b * c].
		// This function does not change the actual SPIRType of any object.
		// Only the generated code, including declarations of interface variables are changed to be single array dimension.
		bool flatten_multidimensional_arrays = false;

		// For older desktop GLSL targets than version 420, the
		// GL_ARB_shading_language_420pack extensions is used to be able to support
		// layout(binding) on UBOs and samplers.
		// If disabled on older targets, binding decorations will be stripped.
		bool enable_420pack_extension = true;

		enum Precision
		{
			DontCare,
			Lowp,
			Mediump,
			Highp
		};

		struct
		{
			// GLSL: In vertex shaders, rewrite [0, w] depth (Vulkan/D3D style) to [-w, w] depth (GL style).
			// MSL: In vertex shaders, rewrite [-w, w] depth (GL style) to [0, w] depth.
			// HLSL: In vertex shaders, rewrite [-w, w] depth (GL style) to [0, w] depth.
			bool fixup_clipspace = false;

			// Inverts gl_Position.y or equivalent.
			bool flip_vert_y = false;

			// If true, the backend will assume that InstanceIndex will need to apply
			// a base instance offset. Set to false if you know you will never use base instance
			// functionality as it might remove some internal uniforms.
			bool support_nonzero_base_instance = true;
		} vertex;

		struct
		{
			// Add precision mediump float in ES targets when emitting GLES source.
			// Add precision highp int in ES targets when emitting GLES source.
			Precision default_float_precision = Mediump;
			Precision default_int_precision = Highp;
		} fragment;
	};

	void remap_pixel_local_storage(std::vector<PlsRemap> inputs, std::vector<PlsRemap> outputs)
	{
		pls_inputs = std::move(inputs);
		pls_outputs = std::move(outputs);
		remap_pls_variables();
	}

	CompilerGLSL(std::vector<uint32_t> spirv_)
	    : Compiler(move(spirv_))
	{
		init();
	}

	CompilerGLSL(const uint32_t *ir, size_t word_count)
	    : Compiler(ir, word_count)
	{
		init();
	}

	// Deprecate this interface because it doesn't overload properly with subclasses.
	// Requires awkward static casting, which was a mistake.
	SPIRV_CROSS_DEPRECATED("get_options() is obsolete, use get_common_options() instead.")
	const Options &get_options() const
	{
		return options;
	}

	const Options &get_common_options() const
	{
		return options;
	}

	// Deprecate this interface because it doesn't overload properly with subclasses.
	// Requires awkward static casting, which was a mistake.
	SPIRV_CROSS_DEPRECATED("set_options() is obsolete, use set_common_options() instead.")
	void set_options(Options &opts)
	{
		options = opts;
	}

	void set_common_options(const Options &opts)
	{
		options = opts;
	}

	std::string compile() override;

	// Returns the current string held in the conversion buffer. Useful for
	// capturing what has been converted so far when compile() throws an error.
	std::string get_partial_source();

	// Adds a line to be added right after #version in GLSL backend.
	// This is useful for enabling custom extensions which are outside the scope of SPIRV-Cross.
	// This can be combined with variable remapping.
	// A new-line will be added.
	//
	// While add_header_line() is a more generic way of adding arbitrary text to the header
	// of a GLSL file, require_extension() should be used when adding extensions since it will
	// avoid creating collisions with SPIRV-Cross generated extensions.
	//
	// Code added via add_header_line() is typically backend-specific.
	void add_header_line(const std::string &str);

	// Adds an extension which is required to run this shader, e.g.
	// require_extension("GL_KHR_my_extension");
	void require_extension(const std::string &ext);

	// Legacy GLSL compatibility method.
	// Takes a uniform or push constant variable and flattens it into a (i|u)vec4 array[N]; array instead.
	// For this to work, all types in the block must be the same basic type, e.g. mixing vec2 and vec4 is fine, but
	// mixing int and float is not.
	// The name of the uniform array will be the same as the interface block name.
	void flatten_buffer_block(uint32_t id);

protected:
	void reset();
	void emit_function(SPIRFunction &func, const Bitset &return_flags);

	bool has_extension(const std::string &ext) const;
	void require_extension_internal(const std::string &ext);

	// Virtualize methods which need to be overridden by subclass targets like C++ and such.
	virtual void emit_function_prototype(SPIRFunction &func, const Bitset &return_flags);

	SPIRBlock *current_emitting_block = nullptr;

	virtual void emit_instruction(const Instruction &instr);
	void emit_block_instructions(SPIRBlock &block);
	virtual void emit_glsl_op(uint32_t result_type, uint32_t result_id, uint32_t op, const uint32_t *args,
	                          uint32_t count);
	virtual void emit_spv_amd_shader_ballot_op(uint32_t result_type, uint32_t result_id, uint32_t op,
	                                           const uint32_t *args, uint32_t count);
	virtual void emit_spv_amd_shader_explicit_vertex_parameter_op(uint32_t result_type, uint32_t result_id, uint32_t op,
	                                                              const uint32_t *args, uint32_t count);
	virtual void emit_spv_amd_shader_trinary_minmax_op(uint32_t result_type, uint32_t result_id, uint32_t op,
	                                                   const uint32_t *args, uint32_t count);
	virtual void emit_spv_amd_gcn_shader_op(uint32_t result_type, uint32_t result_id, uint32_t op, const uint32_t *args,
	                                        uint32_t count);
	virtual void emit_header();
	virtual void emit_sampled_image_op(uint32_t result_type, uint32_t result_id, uint32_t image_id, uint32_t samp_id);
	virtual void emit_texture_op(const Instruction &i);
	virtual void emit_subgroup_op(const Instruction &i);
	virtual std::string type_to_glsl(const SPIRType &type, uint32_t id = 0);
	virtual std::string builtin_to_glsl(spv::BuiltIn builtin, spv::StorageClass storage);
	virtual void emit_struct_member(const SPIRType &type, uint32_t member_type_id, uint32_t index,
	                                const std::string &qualifier = "", uint32_t base_offset = 0);
	virtual std::string image_type_glsl(const SPIRType &type, uint32_t id = 0);
	virtual std::string constant_expression(const SPIRConstant &c);
	std::string constant_op_expression(const SPIRConstantOp &cop);
	virtual std::string constant_expression_vector(const SPIRConstant &c, uint32_t vector);
	virtual void emit_fixup();
	virtual std::string variable_decl(const SPIRType &type, const std::string &name, uint32_t id = 0);
	virtual std::string to_func_call_arg(uint32_t id);
	virtual std::string to_function_name(uint32_t img, const SPIRType &imgtype, bool is_fetch, bool is_gather,
	                                     bool is_proj, bool has_array_offsets, bool has_offset, bool has_grad,
	                                     bool has_dref, uint32_t lod);
	virtual std::string to_function_args(uint32_t img, const SPIRType &imgtype, bool is_fetch, bool is_gather,
	                                     bool is_proj, uint32_t coord, uint32_t coord_components, uint32_t dref,
	                                     uint32_t grad_x, uint32_t grad_y, uint32_t lod, uint32_t coffset,
	                                     uint32_t offset, uint32_t bias, uint32_t comp, uint32_t sample,
	                                     bool *p_forward);
	virtual void emit_buffer_block(const SPIRVariable &type);
	virtual void emit_push_constant_block(const SPIRVariable &var);
	virtual void emit_uniform(const SPIRVariable &var);
	virtual std::string unpack_expression_type(std::string expr_str, const SPIRType &type);

	std::unique_ptr<std::ostringstream> buffer;

	template <typename T>
	inline void statement_inner(T &&t)
	{
		(*buffer) << std::forward<T>(t);
		statement_count++;
	}

	template <typename T, typename... Ts>
	inline void statement_inner(T &&t, Ts &&... ts)
	{
		(*buffer) << std::forward<T>(t);
		statement_count++;
		statement_inner(std::forward<Ts>(ts)...);
	}

	template <typename... Ts>
	inline void statement(Ts &&... ts)
	{
		if (force_recompile)
		{
			// Do not bother emitting code while force_recompile is active.
			// We will compile again.
			statement_count++;
			return;
		}

		if (redirect_statement)
			redirect_statement->push_back(join(std::forward<Ts>(ts)...));
		else
		{
			for (uint32_t i = 0; i < indent; i++)
				(*buffer) << "    ";
			statement_inner(std::forward<Ts>(ts)...);
			(*buffer) << '\n';
		}
	}

	template <typename... Ts>
	inline void statement_no_indent(Ts &&... ts)
	{
		auto old_indent = indent;
		indent = 0;
		statement(std::forward<Ts>(ts)...);
		indent = old_indent;
	}

	// Used for implementing continue blocks where
	// we want to obtain a list of statements we can merge
	// on a single line separated by comma.
	std::vector<std::string> *redirect_statement = nullptr;
	const SPIRBlock *current_continue_block = nullptr;

	void begin_scope();
	void end_scope();
	void end_scope_decl();
	void end_scope_decl(const std::string &decl);

	Options options;

	std::string type_to_array_glsl(const SPIRType &type);
	std::string to_array_size(const SPIRType &type, uint32_t index);
	uint32_t to_array_size_literal(const SPIRType &type, uint32_t index) const;
	std::string variable_decl(const SPIRVariable &variable);
	std::string variable_decl_function_local(SPIRVariable &variable);

	void add_local_variable_name(uint32_t id);
	void add_resource_name(uint32_t id);
	void add_member_name(SPIRType &type, uint32_t name);
	void add_function_overload(const SPIRFunction &func);

	virtual bool is_non_native_row_major_matrix(uint32_t id);
	virtual bool member_is_non_native_row_major_matrix(const SPIRType &type, uint32_t index);
	bool member_is_packed_type(const SPIRType &type, uint32_t index) const;
	virtual std::string convert_row_major_matrix(std::string exp_str, const SPIRType &exp_type, bool is_packed);

	std::unordered_set<std::string> local_variable_names;
	std::unordered_set<std::string> resource_names;
	std::unordered_map<std::string, std::unordered_set<uint64_t>> function_overloads;

	bool processing_entry_point = false;

	// Can be overriden by subclass backends for trivial things which
	// shouldn't need polymorphism.
	struct BackendVariations
	{
		std::string discard_literal = "discard";
		bool float_literal_suffix = false;
		bool double_literal_suffix = true;
		bool uint32_t_literal_suffix = true;
		bool long_long_literal_suffix = false;
		const char *basic_int_type = "int";
		const char *basic_uint_type = "uint";
		const char *half_literal_suffix = "hf";
		bool swizzle_is_function = false;
		bool shared_is_implied = false;
		bool flexible_member_array_supported = true;
		bool explicit_struct_type = false;
		bool use_initializer_list = false;
		bool use_typed_initializer_list = false;
		bool can_declare_struct_inline = true;
		bool can_declare_arrays_inline = true;
		bool native_row_major_matrix = true;
		bool use_constructor_splatting = true;
		bool boolean_mix_support = true;
		bool allow_precision_qualifiers = false;
		bool can_swizzle_scalar = false;
		bool force_gl_in_out_block = false;
		bool can_return_array = true;
		bool allow_truncated_access_chain = false;
		bool supports_extensions = false;
		bool supports_empty_struct = false;
	} backend;

	void emit_struct(SPIRType &type);
	void emit_resources();
	void emit_buffer_block_native(const SPIRVariable &var);
	void emit_buffer_block_legacy(const SPIRVariable &var);
	void emit_buffer_block_flattened(const SPIRVariable &type);
	void emit_declared_builtin_block(spv::StorageClass storage, spv::ExecutionModel model);
	void emit_push_constant_block_vulkan(const SPIRVariable &var);
	void emit_push_constant_block_glsl(const SPIRVariable &var);
	void emit_interface_block(const SPIRVariable &type);
	void emit_flattened_io_block(const SPIRVariable &var, const char *qual);
	void emit_block_chain(SPIRBlock &block);
	void emit_hoisted_temporaries(std::vector<std::pair<uint32_t, uint32_t>> &temporaries);
	void emit_constant(const SPIRConstant &constant);
	void emit_specialization_constant_op(const SPIRConstantOp &constant);
	std::string emit_continue_block(uint32_t continue_block);
	bool attempt_emit_loop_header(SPIRBlock &block, SPIRBlock::Method method);
	void propagate_loop_dominators(const SPIRBlock &block);

	void branch(uint32_t from, uint32_t to);
	void branch_to_continue(uint32_t from, uint32_t to);
	void branch(uint32_t from, uint32_t cond, uint32_t true_block, uint32_t false_block);
	void flush_phi(uint32_t from, uint32_t to);
	bool flush_phi_required(uint32_t from, uint32_t to);
	void flush_variable_declaration(uint32_t id);
	void flush_undeclared_variables(SPIRBlock &block);

	bool should_forward(uint32_t id);
	void emit_mix_op(uint32_t result_type, uint32_t id, uint32_t left, uint32_t right, uint32_t lerp);
	bool to_trivial_mix_op(const SPIRType &type, std::string &op, uint32_t left, uint32_t right, uint32_t lerp);
	void emit_quaternary_func_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, uint32_t op2,
	                             uint32_t op3, const char *op);
	void emit_trinary_func_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, uint32_t op2,
	                          const char *op);
	void emit_binary_func_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, const char *op);
	void emit_binary_func_op_cast(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, const char *op,
	                              SPIRType::BaseType input_type, bool skip_cast_if_equal_type);
	void emit_unary_func_op(uint32_t result_type, uint32_t result_id, uint32_t op0, const char *op);
	void emit_unrolled_unary_op(uint32_t result_type, uint32_t result_id, uint32_t operand, const char *op);
	void emit_binary_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, const char *op);
	void emit_unrolled_binary_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, const char *op);
	void emit_binary_op_cast(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, const char *op,
	                         SPIRType::BaseType input_type, bool skip_cast_if_equal_type);

	SPIRType binary_op_bitcast_helper(std::string &cast_op0, std::string &cast_op1, SPIRType::BaseType &input_type,
	                                  uint32_t op0, uint32_t op1, bool skip_cast_if_equal_type);

	std::string to_ternary_expression(const SPIRType &result_type, uint32_t select, uint32_t true_value,
	                                  uint32_t false_value);

	void emit_unary_op(uint32_t result_type, uint32_t result_id, uint32_t op0, const char *op);
	bool expression_is_forwarded(uint32_t id);
	SPIRExpression &emit_op(uint32_t result_type, uint32_t result_id, const std::string &rhs, bool forward_rhs,
	                        bool suppress_usage_tracking = false);
	std::string access_chain_internal(uint32_t base, const uint32_t *indices, uint32_t count, bool index_is_literal,
	                                  bool chain_only = false, bool *need_transpose = nullptr,
	                                  bool *result_is_packed = nullptr);
	std::string access_chain(uint32_t base, const uint32_t *indices, uint32_t count, const SPIRType &target_type,
	                         bool *need_transpose = nullptr, bool *result_is_packed = nullptr);

	std::string flattened_access_chain(uint32_t base, const uint32_t *indices, uint32_t count,
	                                   const SPIRType &target_type, uint32_t offset, uint32_t matrix_stride,
	                                   bool need_transpose);
	std::string flattened_access_chain_struct(uint32_t base, const uint32_t *indices, uint32_t count,
	                                          const SPIRType &target_type, uint32_t offset);
	std::string flattened_access_chain_matrix(uint32_t base, const uint32_t *indices, uint32_t count,
	                                          const SPIRType &target_type, uint32_t offset, uint32_t matrix_stride,
	                                          bool need_transpose);
	std::string flattened_access_chain_vector(uint32_t base, const uint32_t *indices, uint32_t count,
	                                          const SPIRType &target_type, uint32_t offset, uint32_t matrix_stride,
	                                          bool need_transpose);
	std::pair<std::string, uint32_t> flattened_access_chain_offset(const SPIRType &basetype, const uint32_t *indices,
	                                                               uint32_t count, uint32_t offset,
	                                                               uint32_t word_stride, bool *need_transpose = nullptr,
	                                                               uint32_t *matrix_stride = nullptr);

	const char *index_to_swizzle(uint32_t index);
	std::string remap_swizzle(const SPIRType &result_type, uint32_t input_components, const std::string &expr);
	std::string declare_temporary(uint32_t type, uint32_t id);
	void append_global_func_args(const SPIRFunction &func, uint32_t index, std::vector<std::string> &arglist);
	std::string to_expression(uint32_t id);
	std::string to_enclosed_expression(uint32_t id);
	std::string to_unpacked_expression(uint32_t id);
	std::string to_enclosed_unpacked_expression(uint32_t id);
	std::string to_extract_component_expression(uint32_t id, uint32_t index);
	std::string enclose_expression(const std::string &expr);
	void strip_enclosed_expression(std::string &expr);
	std::string to_member_name(const SPIRType &type, uint32_t index);
	std::string type_to_glsl_constructor(const SPIRType &type);
	std::string argument_decl(const SPIRFunction::Parameter &arg);
	virtual std::string to_qualifiers_glsl(uint32_t id);
	const char *to_precision_qualifiers_glsl(uint32_t id);
	virtual const char *to_storage_qualifiers_glsl(const SPIRVariable &var);
	const char *flags_to_precision_qualifiers_glsl(const SPIRType &type, const Bitset &flags);
	const char *format_to_glsl(spv::ImageFormat format);
	virtual std::string layout_for_member(const SPIRType &type, uint32_t index);
	virtual std::string to_interpolation_qualifiers(const Bitset &flags);
	std::string layout_for_variable(const SPIRVariable &variable);
	std::string to_combined_image_sampler(uint32_t image_id, uint32_t samp_id);
	virtual bool skip_argument(uint32_t id) const;
	virtual void emit_array_copy(const std::string &lhs, uint32_t rhs_id);
	virtual void emit_block_hints(const SPIRBlock &block);
	virtual std::string to_initializer_expression(const SPIRVariable &var);

	bool buffer_is_packing_standard(const SPIRType &type, BufferPackingStandard packing, uint32_t start_offset = 0,
	                                uint32_t end_offset = ~(0u));
	uint32_t type_to_packed_base_size(const SPIRType &type, BufferPackingStandard packing);
	uint32_t type_to_packed_alignment(const SPIRType &type, const Bitset &flags, BufferPackingStandard packing);
	uint32_t type_to_packed_array_stride(const SPIRType &type, const Bitset &flags, BufferPackingStandard packing);
	uint32_t type_to_packed_size(const SPIRType &type, const Bitset &flags, BufferPackingStandard packing);

	std::string bitcast_glsl(const SPIRType &result_type, uint32_t arg);
	virtual std::string bitcast_glsl_op(const SPIRType &result_type, const SPIRType &argument_type);

	std::string bitcast_expression(SPIRType::BaseType target_type, uint32_t arg);
	std::string bitcast_expression(const SPIRType &target_type, SPIRType::BaseType expr_type, const std::string &expr);

	std::string build_composite_combiner(uint32_t result_type, const uint32_t *elems, uint32_t length);
	bool remove_duplicate_swizzle(std::string &op);
	bool remove_unity_swizzle(uint32_t base, std::string &op);

	// Can modify flags to remote readonly/writeonly if image type
	// and force recompile.
	bool check_atomic_image(uint32_t id);

	virtual void replace_illegal_names();
	virtual void emit_entry_point_declarations();

	void replace_fragment_output(SPIRVariable &var);
	void replace_fragment_outputs();
	bool check_explicit_lod_allowed(uint32_t lod);
	std::string legacy_tex_op(const std::string &op, const SPIRType &imgtype, uint32_t lod, uint32_t id);

	uint32_t indent = 0;

	std::unordered_set<uint32_t> emitted_functions;

	std::unordered_set<uint32_t> flattened_buffer_blocks;
	std::unordered_set<uint32_t> flattened_structs;

	std::string load_flattened_struct(SPIRVariable &var);
	std::string to_flattened_struct_member(const SPIRVariable &var, uint32_t index);
	void store_flattened_struct(SPIRVariable &var, uint32_t value);

	// Usage tracking. If a temporary is used more than once, use the temporary instead to
	// avoid AST explosion when SPIRV is generated with pure SSA and doesn't write stuff to variables.
	std::unordered_map<uint32_t, uint32_t> expression_usage_counts;
	void track_expression_read(uint32_t id);

	std::vector<std::string> forced_extensions;
	std::vector<std::string> header_lines;

	uint32_t statement_count;

	inline bool is_legacy() const
	{
		return (options.es && options.version < 300) || (!options.es && options.version < 130);
	}

	inline bool is_legacy_es() const
	{
		return options.es && options.version < 300;
	}

	inline bool is_legacy_desktop() const
	{
		return !options.es && options.version < 130;
	}

	bool args_will_forward(uint32_t id, const uint32_t *args, uint32_t num_args, bool pure);
	void register_call_out_argument(uint32_t id);
	void register_impure_function_call();
	void register_control_dependent_expression(uint32_t expr);

	// GL_EXT_shader_pixel_local_storage support.
	std::vector<PlsRemap> pls_inputs;
	std::vector<PlsRemap> pls_outputs;
	std::string pls_decl(const PlsRemap &variable);
	const char *to_pls_qualifiers_glsl(const SPIRVariable &variable);
	void emit_pls();
	void remap_pls_variables();

	void add_variable(std::unordered_set<std::string> &variables, uint32_t id);
	void add_variable(std::unordered_set<std::string> &variables, std::string &name);
	void check_function_call_constraints(const uint32_t *args, uint32_t length);
	void handle_invalid_expression(uint32_t id);
	void find_static_extensions();

	std::string emit_for_loop_initializers(const SPIRBlock &block);
	void emit_while_loop_initializers(const SPIRBlock &block);
	bool for_loop_initializers_are_same_type(const SPIRBlock &block);
	bool optimize_read_modify_write(const SPIRType &type, const std::string &lhs, const std::string &rhs);
	void fixup_image_load_store_access();

	bool type_is_empty(const SPIRType &type);

	virtual void declare_undefined_values();

	static std::string sanitize_underscores(const std::string &str);

	bool can_use_io_location(spv::StorageClass storage, bool block);
	const Instruction *get_next_instruction_in_block(const Instruction &instr);
	static uint32_t mask_relevant_memory_semantics(uint32_t semantics);

	std::string convert_half_to_string(const SPIRConstant &value, uint32_t col, uint32_t row);
	std::string convert_float_to_string(const SPIRConstant &value, uint32_t col, uint32_t row);
	std::string convert_double_to_string(const SPIRConstant &value, uint32_t col, uint32_t row);

	std::string convert_separate_image_to_combined(uint32_t id);

	// Builtins in GLSL are always specific signedness, but the SPIR-V can declare them
	// as either unsigned or signed.
	// Sometimes we will need to automatically perform bitcasts on load and store to make this work.
	virtual void bitcast_to_builtin_store(uint32_t target_id, std::string &expr, const SPIRType &expr_type);
	virtual void bitcast_from_builtin_load(uint32_t source_id, std::string &expr, const SPIRType &expr_type);

private:
	void init()
	{
		if (source.known)
		{
			options.es = source.es;
			options.version = source.version;
		}
	}
};
} // namespace spirv_cross

#endif
