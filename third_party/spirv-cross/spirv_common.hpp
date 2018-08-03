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

#ifndef SPIRV_CROSS_COMMON_HPP
#define SPIRV_CROSS_COMMON_HPP

#include "spirv.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <functional>
#include <locale>
#include <memory>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace spirv_cross
{

#ifdef SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS
#ifndef _MSC_VER
[[noreturn]]
#endif
    inline void
    report_and_abort(const std::string &msg)
{
#ifdef NDEBUG
	(void)msg;
#else
	fprintf(stderr, "There was a compiler error: %s\n", msg.c_str());
#endif
	fflush(stderr);
	abort();
}

#define SPIRV_CROSS_THROW(x) report_and_abort(x)
#else
class CompilerError : public std::runtime_error
{
public:
	CompilerError(const std::string &str)
	    : std::runtime_error(str)
	{
	}
};

#define SPIRV_CROSS_THROW(x) throw CompilerError(x)
#endif

#if __cplusplus >= 201402l
#define SPIRV_CROSS_DEPRECATED(reason) [[deprecated(reason)]]
#elif defined(__GNUC__)
#define SPIRV_CROSS_DEPRECATED(reason) __attribute__((deprecated))
#elif defined(_MSC_VER)
#define SPIRV_CROSS_DEPRECATED(reason) __declspec(deprecated(reason))
#else
#define SPIRV_CROSS_DEPRECATED(reason)
#endif

namespace inner
{
template <typename T>
void join_helper(std::ostringstream &stream, T &&t)
{
	stream << std::forward<T>(t);
}

template <typename T, typename... Ts>
void join_helper(std::ostringstream &stream, T &&t, Ts &&... ts)
{
	stream << std::forward<T>(t);
	join_helper(stream, std::forward<Ts>(ts)...);
}
} // namespace inner

class Bitset
{
public:
	Bitset() = default;
	explicit inline Bitset(uint64_t lower_)
	    : lower(lower_)
	{
	}

	inline bool get(uint32_t bit) const
	{
		if (bit < 64)
			return (lower & (1ull << bit)) != 0;
		else
			return higher.count(bit) != 0;
	}

	inline void set(uint32_t bit)
	{
		if (bit < 64)
			lower |= 1ull << bit;
		else
			higher.insert(bit);
	}

	inline void clear(uint32_t bit)
	{
		if (bit < 64)
			lower &= ~(1ull << bit);
		else
			higher.erase(bit);
	}

	inline uint64_t get_lower() const
	{
		return lower;
	}

	inline void reset()
	{
		lower = 0;
		higher.clear();
	}

	inline void merge_and(const Bitset &other)
	{
		lower &= other.lower;
		std::unordered_set<uint32_t> tmp_set;
		for (auto &v : higher)
			if (other.higher.count(v) != 0)
				tmp_set.insert(v);
		higher = std::move(tmp_set);
	}

	inline void merge_or(const Bitset &other)
	{
		lower |= other.lower;
		for (auto &v : other.higher)
			higher.insert(v);
	}

	inline bool operator==(const Bitset &other) const
	{
		if (lower != other.lower)
			return false;

		if (higher.size() != other.higher.size())
			return false;

		for (auto &v : higher)
			if (other.higher.count(v) == 0)
				return false;

		return true;
	}

	inline bool operator!=(const Bitset &other) const
	{
		return !(*this == other);
	}

	template <typename Op>
	void for_each_bit(const Op &op) const
	{
		// TODO: Add ctz-based iteration.
		for (uint32_t i = 0; i < 64; i++)
		{
			if (lower & (1ull << i))
				op(i);
		}

		if (higher.empty())
			return;

		// Need to enforce an order here for reproducible results,
		// but hitting this path should happen extremely rarely, so having this slow path is fine.
		std::vector<uint32_t> bits;
		bits.reserve(higher.size());
		for (auto &v : higher)
			bits.push_back(v);
		std::sort(std::begin(bits), std::end(bits));

		for (auto &v : bits)
			op(v);
	}

	inline bool empty() const
	{
		return lower == 0 && higher.empty();
	}

private:
	// The most common bits to set are all lower than 64,
	// so optimize for this case. Bits spilling outside 64 go into a slower data structure.
	// In almost all cases, higher data structure will not be used.
	uint64_t lower = 0;
	std::unordered_set<uint32_t> higher;
};

// Helper template to avoid lots of nasty string temporary munging.
template <typename... Ts>
std::string join(Ts &&... ts)
{
	std::ostringstream stream;
	inner::join_helper(stream, std::forward<Ts>(ts)...);
	return stream.str();
}

inline std::string merge(const std::vector<std::string> &list)
{
	std::string s;
	for (auto &elem : list)
	{
		s += elem;
		if (&elem != &list.back())
			s += ", ";
	}
	return s;
}

template <typename T>
inline std::string convert_to_string(T &&t)
{
	return std::to_string(std::forward<T>(t));
}

// Allow implementations to set a convenient standard precision
#ifndef SPIRV_CROSS_FLT_FMT
#define SPIRV_CROSS_FLT_FMT "%.32g"
#endif

#ifdef _MSC_VER
// sprintf warning.
// We cannot rely on snprintf existing because, ..., MSVC.
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

inline std::string convert_to_string(float t)
{
	// std::to_string for floating point values is broken.
	// Fallback to something more sane.
	char buf[64];
	sprintf(buf, SPIRV_CROSS_FLT_FMT, t);
	// Ensure that the literal is float.
	if (!strchr(buf, '.') && !strchr(buf, 'e'))
		strcat(buf, ".0");
	return buf;
}

inline std::string convert_to_string(double t)
{
	// std::to_string for floating point values is broken.
	// Fallback to something more sane.
	char buf[64];
	sprintf(buf, SPIRV_CROSS_FLT_FMT, t);
	// Ensure that the literal is float.
	if (!strchr(buf, '.') && !strchr(buf, 'e'))
		strcat(buf, ".0");
	return buf;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

struct Instruction
{
	Instruction(const std::vector<uint32_t> &spirv, uint32_t &index);

	uint16_t op;
	uint16_t count;
	uint32_t offset;
	uint32_t length;
};

// Helper for Variant interface.
struct IVariant
{
	virtual ~IVariant() = default;
	uint32_t self = 0;
};

enum Types
{
	TypeNone,
	TypeType,
	TypeVariable,
	TypeConstant,
	TypeFunction,
	TypeFunctionPrototype,
	TypePointer,
	TypeBlock,
	TypeExtension,
	TypeExpression,
	TypeConstantOp,
	TypeCombinedImageSampler,
	TypeAccessChain,
	TypeUndef
};

struct SPIRUndef : IVariant
{
	enum
	{
		type = TypeUndef
	};
	SPIRUndef(uint32_t basetype_)
	    : basetype(basetype_)
	{
	}
	uint32_t basetype;
};

// This type is only used by backends which need to access the combined image and sampler IDs separately after
// the OpSampledImage opcode.
struct SPIRCombinedImageSampler : IVariant
{
	enum
	{
		type = TypeCombinedImageSampler
	};
	SPIRCombinedImageSampler(uint32_t type_, uint32_t image_, uint32_t sampler_)
	    : combined_type(type_)
	    , image(image_)
	    , sampler(sampler_)
	{
	}
	uint32_t combined_type;
	uint32_t image;
	uint32_t sampler;
};

struct SPIRConstantOp : IVariant
{
	enum
	{
		type = TypeConstantOp
	};

	SPIRConstantOp(uint32_t result_type, spv::Op op, const uint32_t *args, uint32_t length)
	    : opcode(op)
	    , arguments(args, args + length)
	    , basetype(result_type)
	{
	}

	spv::Op opcode;
	std::vector<uint32_t> arguments;
	uint32_t basetype;
};

struct SPIRType : IVariant
{
	enum
	{
		type = TypeType
	};

	enum BaseType
	{
		Unknown,
		Void,
		Boolean,
		Char,
		Int,
		UInt,
		Int64,
		UInt64,
		AtomicCounter,
		Half,
		Float,
		Double,
		Struct,
		Image,
		SampledImage,
		Sampler
	};

	// Scalar/vector/matrix support.
	BaseType basetype = Unknown;
	uint32_t width = 0;
	uint32_t vecsize = 1;
	uint32_t columns = 1;

	// Arrays, support array of arrays by having a vector of array sizes.
	std::vector<uint32_t> array;

	// Array elements can be either specialization constants or specialization ops.
	// This array determines how to interpret the array size.
	// If an element is true, the element is a literal,
	// otherwise, it's an expression, which must be resolved on demand.
	// The actual size is not really known until runtime.
	std::vector<bool> array_size_literal;

	// Pointers
	bool pointer = false;
	spv::StorageClass storage = spv::StorageClassGeneric;

	std::vector<uint32_t> member_types;

	struct ImageType
	{
		uint32_t type;
		spv::Dim dim;
		bool depth;
		bool arrayed;
		bool ms;
		uint32_t sampled;
		spv::ImageFormat format;
		spv::AccessQualifier access;
	} image;

	// Structs can be declared multiple times if they are used as part of interface blocks.
	// We want to detect this so that we only emit the struct definition once.
	// Since we cannot rely on OpName to be equal, we need to figure out aliases.
	uint32_t type_alias = 0;

	// Denotes the type which this type is based on.
	// Allows the backend to traverse how a complex type is built up during access chains.
	uint32_t parent_type = 0;

	// Used in backends to avoid emitting members with conflicting names.
	std::unordered_set<std::string> member_name_cache;
};

struct SPIRExtension : IVariant
{
	enum
	{
		type = TypeExtension
	};

	enum Extension
	{
		Unsupported,
		GLSL,
		SPV_AMD_shader_ballot,
		SPV_AMD_shader_explicit_vertex_parameter,
		SPV_AMD_shader_trinary_minmax,
		SPV_AMD_gcn_shader
	};

	SPIRExtension(Extension ext_)
	    : ext(ext_)
	{
	}

	Extension ext;
};

// SPIREntryPoint is not a variant since its IDs are used to decorate OpFunction,
// so in order to avoid conflicts, we can't stick them in the ids array.
struct SPIREntryPoint
{
	SPIREntryPoint(uint32_t self_, spv::ExecutionModel execution_model, const std::string &entry_name)
	    : self(self_)
	    , name(entry_name)
	    , orig_name(entry_name)
	    , model(execution_model)
	{
	}
	SPIREntryPoint() = default;

	uint32_t self = 0;
	std::string name;
	std::string orig_name;
	std::vector<uint32_t> interface_variables;

	Bitset flags;
	struct
	{
		uint32_t x = 0, y = 0, z = 0;
		uint32_t constant = 0; // Workgroup size can be expressed as a constant/spec-constant instead.
	} workgroup_size;
	uint32_t invocations = 0;
	uint32_t output_vertices = 0;
	spv::ExecutionModel model;
};

struct SPIRExpression : IVariant
{
	enum
	{
		type = TypeExpression
	};

	// Only created by the backend target to avoid creating tons of temporaries.
	SPIRExpression(std::string expr, uint32_t expression_type_, bool immutable_)
	    : expression(move(expr))
	    , expression_type(expression_type_)
	    , immutable(immutable_)
	{
	}

	// If non-zero, prepend expression with to_expression(base_expression).
	// Used in amortizing multiple calls to to_expression()
	// where in certain cases that would quickly force a temporary when not needed.
	uint32_t base_expression = 0;

	std::string expression;
	uint32_t expression_type = 0;

	// If this expression is a forwarded load,
	// allow us to reference the original variable.
	uint32_t loaded_from = 0;

	// If this expression will never change, we can avoid lots of temporaries
	// in high level source.
	// An expression being immutable can be speculative,
	// it is assumed that this is true almost always.
	bool immutable = false;

	// Before use, this expression must be transposed.
	// This is needed for targets which don't support row_major layouts.
	bool need_transpose = false;

	// A list of expressions which this expression depends on.
	std::vector<uint32_t> expression_dependencies;
};

struct SPIRFunctionPrototype : IVariant
{
	enum
	{
		type = TypeFunctionPrototype
	};

	SPIRFunctionPrototype(uint32_t return_type_)
	    : return_type(return_type_)
	{
	}

	uint32_t return_type;
	std::vector<uint32_t> parameter_types;
};

struct SPIRBlock : IVariant
{
	enum
	{
		type = TypeBlock
	};

	enum Terminator
	{
		Unknown,
		Direct, // Emit next block directly without a particular condition.

		Select, // Block ends with an if/else block.
		MultiSelect, // Block ends with switch statement.

		Return, // Block ends with return.
		Unreachable, // Noop
		Kill // Discard
	};

	enum Merge
	{
		MergeNone,
		MergeLoop,
		MergeSelection
	};

	enum Hints
	{
		HintNone,
		HintUnroll,
		HintDontUnroll,
		HintFlatten,
		HintDontFlatten
	};

	enum Method
	{
		MergeToSelectForLoop,
		MergeToDirectForLoop,
		MergeToSelectContinueForLoop
	};

	enum ContinueBlockType
	{
		ContinueNone,

		// Continue block is branchless and has at least one instruction.
		ForLoop,

		// Noop continue block.
		WhileLoop,

		// Continue block is conditional.
		DoWhileLoop,

		// Highly unlikely that anything will use this,
		// since it is really awkward/impossible to express in GLSL.
		ComplexLoop
	};

	enum
	{
		NoDominator = 0xffffffffu
	};

	Terminator terminator = Unknown;
	Merge merge = MergeNone;
	Hints hint = HintNone;
	uint32_t next_block = 0;
	uint32_t merge_block = 0;
	uint32_t continue_block = 0;

	uint32_t return_value = 0; // If 0, return nothing (void).
	uint32_t condition = 0;
	uint32_t true_block = 0;
	uint32_t false_block = 0;
	uint32_t default_block = 0;

	std::vector<Instruction> ops;

	struct Phi
	{
		uint32_t local_variable; // flush local variable ...
		uint32_t parent; // If we're in from_block and want to branch into this block ...
		uint32_t function_variable; // to this function-global "phi" variable first.
	};

	// Before entering this block flush out local variables to magical "phi" variables.
	std::vector<Phi> phi_variables;

	// Declare these temporaries before beginning the block.
	// Used for handling complex continue blocks which have side effects.
	std::vector<std::pair<uint32_t, uint32_t>> declare_temporary;

	// Declare these temporaries, but only conditionally if this block turns out to be
	// a complex loop header.
	std::vector<std::pair<uint32_t, uint32_t>> potential_declare_temporary;

	struct Case
	{
		uint32_t value;
		uint32_t block;
	};
	std::vector<Case> cases;

	// If we have tried to optimize code for this block but failed,
	// keep track of this.
	bool disable_block_optimization = false;

	// If the continue block is complex, fallback to "dumb" for loops.
	bool complex_continue = false;

	// The dominating block which this block might be within.
	// Used in continue; blocks to determine if we really need to write continue.
	uint32_t loop_dominator = 0;

	// All access to these variables are dominated by this block,
	// so before branching anywhere we need to make sure that we declare these variables.
	std::vector<uint32_t> dominated_variables;

	// These are variables which should be declared in a for loop header, if we
	// fail to use a classic for-loop,
	// we remove these variables, and fall back to regular variables outside the loop.
	std::vector<uint32_t> loop_variables;

	// Some expressions are control-flow dependent, i.e. any instruction which relies on derivatives or
	// sub-group-like operations.
	// Make sure that we only use these expressions in the original block.
	std::vector<uint32_t> invalidate_expressions;
};

struct SPIRFunction : IVariant
{
	enum
	{
		type = TypeFunction
	};

	SPIRFunction(uint32_t return_type_, uint32_t function_type_)
	    : return_type(return_type_)
	    , function_type(function_type_)
	{
	}

	struct Parameter
	{
		uint32_t type;
		uint32_t id;
		uint32_t read_count;
		uint32_t write_count;

		// Set to true if this parameter aliases a global variable,
		// used mostly in Metal where global variables
		// have to be passed down to functions as regular arguments.
		// However, for this kind of variable, we should not care about
		// read and write counts as access to the function arguments
		// is not local to the function in question.
		bool alias_global_variable;
	};

	// When calling a function, and we're remapping separate image samplers,
	// resolve these arguments into combined image samplers and pass them
	// as additional arguments in this order.
	// It gets more complicated as functions can pull in their own globals
	// and combine them with parameters,
	// so we need to distinguish if something is local parameter index
	// or a global ID.
	struct CombinedImageSamplerParameter
	{
		uint32_t id;
		uint32_t image_id;
		uint32_t sampler_id;
		bool global_image;
		bool global_sampler;
		bool depth;
	};

	uint32_t return_type;
	uint32_t function_type;
	std::vector<Parameter> arguments;

	// Can be used by backends to add magic arguments.
	// Currently used by combined image/sampler implementation.

	std::vector<Parameter> shadow_arguments;
	std::vector<uint32_t> local_variables;
	uint32_t entry_block = 0;
	std::vector<uint32_t> blocks;
	std::vector<CombinedImageSamplerParameter> combined_parameters;

	void add_local_variable(uint32_t id)
	{
		local_variables.push_back(id);
	}

	void add_parameter(uint32_t parameter_type, uint32_t id, bool alias_global_variable = false)
	{
		// Arguments are read-only until proven otherwise.
		arguments.push_back({ parameter_type, id, 0u, 0u, alias_global_variable });
	}

	// Statements to be emitted when the function returns.
	// Mostly used for lowering internal data structures onto flattened structures.
	std::vector<std::string> fixup_statements_out;

	// Statements to be emitted when the function begins.
	// Mostly used for populating internal data structures from flattened structures.
	std::vector<std::string> fixup_statements_in;

	bool active = false;
	bool flush_undeclared = true;
	bool do_combined_parameters = true;
};

struct SPIRAccessChain : IVariant
{
	enum
	{
		type = TypeAccessChain
	};

	SPIRAccessChain(uint32_t basetype_, spv::StorageClass storage_, std::string base_, std::string dynamic_index_,
	                int32_t static_index_)
	    : basetype(basetype_)
	    , storage(storage_)
	    , base(base_)
	    , dynamic_index(std::move(dynamic_index_))
	    , static_index(static_index_)
	{
	}

	// The access chain represents an offset into a buffer.
	// Some backends need more complicated handling of access chains to be able to use buffers, like HLSL
	// which has no usable buffer type ala GLSL SSBOs.
	// StructuredBuffer is too limited, so our only option is to deal with ByteAddressBuffer which works with raw addresses.

	uint32_t basetype;
	spv::StorageClass storage;
	std::string base;
	std::string dynamic_index;
	int32_t static_index;

	uint32_t loaded_from = 0;
	uint32_t matrix_stride = 0;
	bool row_major_matrix = false;
	bool immutable = false;
};

struct SPIRVariable : IVariant
{
	enum
	{
		type = TypeVariable
	};

	SPIRVariable() = default;
	SPIRVariable(uint32_t basetype_, spv::StorageClass storage_, uint32_t initializer_ = 0, uint32_t basevariable_ = 0)
	    : basetype(basetype_)
	    , storage(storage_)
	    , initializer(initializer_)
	    , basevariable(basevariable_)
	{
	}

	uint32_t basetype = 0;
	spv::StorageClass storage = spv::StorageClassGeneric;
	uint32_t decoration = 0;
	uint32_t initializer = 0;
	uint32_t basevariable = 0;

	std::vector<uint32_t> dereference_chain;
	bool compat_builtin = false;

	// If a variable is shadowed, we only statically assign to it
	// and never actually emit a statement for it.
	// When we read the variable as an expression, just forward
	// shadowed_id as the expression.
	bool statically_assigned = false;
	uint32_t static_expression = 0;

	// Temporaries which can remain forwarded as long as this variable is not modified.
	std::vector<uint32_t> dependees;
	bool forwardable = true;

	bool deferred_declaration = false;
	bool phi_variable = false;
	bool remapped_variable = false;
	uint32_t remapped_components = 0;

	// The block which dominates all access to this variable.
	uint32_t dominator = 0;
	// If true, this variable is a loop variable, when accessing the variable
	// outside a loop,
	// we should statically forward it.
	bool loop_variable = false;
	// Set to true while we're inside the for loop.
	bool loop_variable_enable = false;

	SPIRFunction::Parameter *parameter = nullptr;
};

struct SPIRConstant : IVariant
{
	enum
	{
		type = TypeConstant
	};

	union Constant {
		uint32_t u32;
		int32_t i32;
		float f32;

		uint64_t u64;
		int64_t i64;
		double f64;
	};

	struct ConstantVector
	{
		Constant r[4];
		// If != 0, this element is a specialization constant, and we should keep track of it as such.
		uint32_t id[4];
		uint32_t vecsize = 1;

		// Workaround for MSVC 2013, initializing an array breaks.
		ConstantVector()
		{
			memset(r, 0, sizeof(r));
			for (unsigned i = 0; i < 4; i++)
				id[i] = 0;
		}
	};

	struct ConstantMatrix
	{
		ConstantVector c[4];
		// If != 0, this column is a specialization constant, and we should keep track of it as such.
		uint32_t id[4];
		uint32_t columns = 1;

		// Workaround for MSVC 2013, initializing an array breaks.
		ConstantMatrix()
		{
			for (unsigned i = 0; i < 4; i++)
				id[i] = 0;
		}
	};

	static inline float f16_to_f32(uint16_t u16_value)
	{
		// Based on the GLM implementation.
		int s = (u16_value >> 15) & 0x1;
		int e = (u16_value >> 10) & 0x1f;
		int m = (u16_value >> 0) & 0x3ff;

		union {
			float f32;
			uint32_t u32;
		} u;

		if (e == 0)
		{
			if (m == 0)
			{
				u.u32 = uint32_t(s) << 31;
				return u.f32;
			}
			else
			{
				while ((m & 0x400) == 0)
				{
					m <<= 1;
					e--;
				}

				e++;
				m &= ~0x400;
			}
		}
		else if (e == 31)
		{
			if (m == 0)
			{
				u.u32 = (uint32_t(s) << 31) | 0x7f800000u;
				return u.f32;
			}
			else
			{
				u.u32 = (uint32_t(s) << 31) | 0x7f800000u | (m << 13);
				return u.f32;
			}
		}

		e += 127 - 15;
		m <<= 13;
		u.u32 = (uint32_t(s) << 31) | (e << 23) | m;
		return u.f32;
	}

	inline uint32_t specialization_constant_id(uint32_t col, uint32_t row) const
	{
		return m.c[col].id[row];
	}

	inline uint32_t specialization_constant_id(uint32_t col) const
	{
		return m.id[col];
	}

	inline uint32_t scalar(uint32_t col = 0, uint32_t row = 0) const
	{
		return m.c[col].r[row].u32;
	}

	inline uint16_t scalar_u16(uint32_t col = 0, uint32_t row = 0) const
	{
		return uint16_t(m.c[col].r[row].u32 & 0xffffu);
	}

	inline float scalar_f16(uint32_t col = 0, uint32_t row = 0) const
	{
		return f16_to_f32(scalar_u16(col, row));
	}

	inline float scalar_f32(uint32_t col = 0, uint32_t row = 0) const
	{
		return m.c[col].r[row].f32;
	}

	inline int32_t scalar_i32(uint32_t col = 0, uint32_t row = 0) const
	{
		return m.c[col].r[row].i32;
	}

	inline double scalar_f64(uint32_t col = 0, uint32_t row = 0) const
	{
		return m.c[col].r[row].f64;
	}

	inline int64_t scalar_i64(uint32_t col = 0, uint32_t row = 0) const
	{
		return m.c[col].r[row].i64;
	}

	inline uint64_t scalar_u64(uint32_t col = 0, uint32_t row = 0) const
	{
		return m.c[col].r[row].u64;
	}

	inline const ConstantVector &vector() const
	{
		return m.c[0];
	}

	inline uint32_t vector_size() const
	{
		return m.c[0].vecsize;
	}

	inline uint32_t columns() const
	{
		return m.columns;
	}

	inline void make_null(const SPIRType &constant_type_)
	{
		m = {};
		m.columns = constant_type_.columns;
		for (auto &c : m.c)
			c.vecsize = constant_type_.vecsize;
	}

	explicit SPIRConstant(uint32_t constant_type_)
	    : constant_type(constant_type_)
	{
	}

	SPIRConstant() = default;

	SPIRConstant(uint32_t constant_type_, const uint32_t *elements, uint32_t num_elements, bool specialized)
	    : constant_type(constant_type_)
	    , specialization(specialized)
	{
		subconstants.insert(end(subconstants), elements, elements + num_elements);
		specialization = specialized;
	}

	// Construct scalar (32-bit).
	SPIRConstant(uint32_t constant_type_, uint32_t v0, bool specialized)
	    : constant_type(constant_type_)
	    , specialization(specialized)
	{
		m.c[0].r[0].u32 = v0;
		m.c[0].vecsize = 1;
		m.columns = 1;
	}

	// Construct scalar (64-bit).
	SPIRConstant(uint32_t constant_type_, uint64_t v0, bool specialized)
	    : constant_type(constant_type_)
	    , specialization(specialized)
	{
		m.c[0].r[0].u64 = v0;
		m.c[0].vecsize = 1;
		m.columns = 1;
	}

	// Construct vectors and matrices.
	SPIRConstant(uint32_t constant_type_, const SPIRConstant *const *vector_elements, uint32_t num_elements,
	             bool specialized)
	    : constant_type(constant_type_)
	    , specialization(specialized)
	{
		bool matrix = vector_elements[0]->m.c[0].vecsize > 1;

		if (matrix)
		{
			m.columns = num_elements;

			for (uint32_t i = 0; i < num_elements; i++)
			{
				m.c[i] = vector_elements[i]->m.c[0];
				if (vector_elements[i]->specialization)
					m.id[i] = vector_elements[i]->self;
			}
		}
		else
		{
			m.c[0].vecsize = num_elements;
			m.columns = 1;

			for (uint32_t i = 0; i < num_elements; i++)
			{
				m.c[0].r[i] = vector_elements[i]->m.c[0].r[0];
				if (vector_elements[i]->specialization)
					m.c[0].id[i] = vector_elements[i]->self;
			}
		}
	}

	uint32_t constant_type;
	ConstantMatrix m;

	// If this constant is a specialization constant (i.e. created with OpSpecConstant*).
	bool specialization = false;
	// If this constant is used as an array length which creates specialization restrictions on some backends.
	bool is_used_as_array_length = false;

	// If true, this is a LUT, and should always be declared in the outer scope.
	bool is_used_as_lut = false;

	// For composites which are constant arrays, etc.
	std::vector<uint32_t> subconstants;
};

class Variant
{
public:
	// MSVC 2013 workaround, we shouldn't need these constructors.
	Variant() = default;
	Variant(Variant &&other)
	{
		*this = std::move(other);
	}
	Variant &operator=(Variant &&other)
	{
		if (this != &other)
		{
			holder = move(other.holder);
			type = other.type;
			other.type = TypeNone;
		}
		return *this;
	}

	void set(std::unique_ptr<IVariant> val, uint32_t new_type)
	{
		holder = std::move(val);
		if (!allow_type_rewrite && type != TypeNone && type != new_type)
			SPIRV_CROSS_THROW("Overwriting a variant with new type.");
		type = new_type;
		allow_type_rewrite = false;
	}

	template <typename T>
	T &get()
	{
		if (!holder)
			SPIRV_CROSS_THROW("nullptr");
		if (T::type != type)
			SPIRV_CROSS_THROW("Bad cast");
		return *static_cast<T *>(holder.get());
	}

	template <typename T>
	const T &get() const
	{
		if (!holder)
			SPIRV_CROSS_THROW("nullptr");
		if (T::type != type)
			SPIRV_CROSS_THROW("Bad cast");
		return *static_cast<const T *>(holder.get());
	}

	uint32_t get_type() const
	{
		return type;
	}
	uint32_t get_id() const
	{
		return holder ? holder->self : 0;
	}
	bool empty() const
	{
		return !holder;
	}
	void reset()
	{
		holder.reset();
		type = TypeNone;
	}

	void set_allow_type_rewrite()
	{
		allow_type_rewrite = true;
	}

private:
	std::unique_ptr<IVariant> holder;
	uint32_t type = TypeNone;
	bool allow_type_rewrite = false;
};

template <typename T>
T &variant_get(Variant &var)
{
	return var.get<T>();
}

template <typename T>
const T &variant_get(const Variant &var)
{
	return var.get<T>();
}

template <typename T, typename... P>
T &variant_set(Variant &var, P &&... args)
{
	auto uptr = std::unique_ptr<T>(new T(std::forward<P>(args)...));
	auto ptr = uptr.get();
	var.set(std::move(uptr), T::type);
	return *ptr;
}

struct Meta
{
	struct Decoration
	{
		std::string alias;
		std::string qualified_alias;
		std::string hlsl_semantic;
		Bitset decoration_flags;
		spv::BuiltIn builtin_type;
		uint32_t location = 0;
		uint32_t set = 0;
		uint32_t binding = 0;
		uint32_t offset = 0;
		uint32_t array_stride = 0;
		uint32_t matrix_stride = 0;
		uint32_t input_attachment = 0;
		uint32_t spec_id = 0;
		uint32_t index = 0;
		bool builtin = false;
	};

	Decoration decoration;
	std::vector<Decoration> members;
	uint32_t sampler = 0;

	std::unordered_map<uint32_t, uint32_t> decoration_word_offset;

	// Used when the parser has detected a candidate identifier which matches
	// known "magic" counter buffers as emitted by HLSL frontends.
	// We will need to match the identifiers by name later when reflecting resources.
	// We could use the regular alias later, but the alias will be mangled when parsing SPIR-V because the identifier
	// is not a valid identifier in any high-level language.
	std::string hlsl_magic_counter_buffer_name;
	bool hlsl_magic_counter_buffer_candidate = false;

	// For SPV_GOOGLE_hlsl_functionality1, this avoids the workaround.
	bool hlsl_is_magic_counter_buffer = false;
	// ID for the sibling counter buffer.
	uint32_t hlsl_magic_counter_buffer = 0;
};

// A user callback that remaps the type of any variable.
// var_name is the declared name of the variable.
// name_of_type is the textual name of the type which will be used in the code unless written to by the callback.
using VariableTypeRemapCallback =
    std::function<void(const SPIRType &type, const std::string &var_name, std::string &name_of_type)>;

class ClassicLocale
{
public:
	ClassicLocale()
	{
		old = std::locale::global(std::locale::classic());
	}
	~ClassicLocale()
	{
		std::locale::global(old);
	}

private:
	std::locale old;
};

class Hasher
{
public:
	inline void u32(uint32_t value)
	{
		h = (h * 0x100000001b3ull) ^ value;
	}

	inline uint64_t get() const
	{
		return h;
	}

private:
	uint64_t h = 0xcbf29ce484222325ull;
};

static inline bool type_is_floating_point(const SPIRType &type)
{
	return type.basetype == SPIRType::Half || type.basetype == SPIRType::Float || type.basetype == SPIRType::Double;
}
} // namespace spirv_cross

#endif
