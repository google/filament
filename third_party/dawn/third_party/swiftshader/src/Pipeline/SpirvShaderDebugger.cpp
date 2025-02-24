// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "SpirvShader.hpp"

#include "System/Types.hpp"

// If enabled, each instruction will be printed before defining.
#define PRINT_EACH_DEFINED_DBG_INSTRUCTION 0
// If enabled, each instruction will be printed before emitting.
#define PRINT_EACH_EMITTED_INSTRUCTION 0
// If enabled, each instruction will be printed before executing.
#define PRINT_EACH_EXECUTED_INSTRUCTION 0
// If enabled, debugger variables will contain debug information (addresses,
// byte offset, etc).
#define DEBUG_ANNOTATE_VARIABLE_KEYS 0

#ifdef ENABLE_VK_DEBUGGER

#	include "Vulkan/Debug/Context.hpp"
#	include "Vulkan/Debug/File.hpp"
#	include "Vulkan/Debug/Thread.hpp"
#	include "Vulkan/Debug/Variable.hpp"
#	include "Vulkan/Debug/EventListener.hpp"

#	include "spirv/unified1/OpenCLDebugInfo100.h"
#	include "spirv-tools/libspirv.h"

#	include <algorithm>
#	include <queue>

////////////////////////////////////////////////////////////////////////////////
// namespace sw::SIMD
// Adds sw::SIMD::PerLane<> and typedefs for C++ versions of the Reactor SIMD
// types (sw::SIMD::Int, etc)
////////////////////////////////////////////////////////////////////////////////
namespace sw {
namespace SIMD {

// PerLane is a SIMD vector that holds N vectors of width SIMD::Width.
// PerLane operator[] returns the elements of a single lane (a transpose of the
// storage arrays).
template<typename T, int N = 1>
struct PerLane
{
	sw::vec<T, N> operator[](int lane) const
	{
		sw::vec<T, N> out;
		for(int i = 0; i < N; i++)
		{
			out[i] = elements[i][lane];
		}
		return out;
	}
	std::array<sw::vec<T, Width>, N> elements;
};

template<typename T>
struct PerLane<T, 1>
{
	const T &operator[](int lane) const { return data[lane]; }
	std::array<T, Width> data;
};

using uint_t = PerLane<unsigned int>;
using uint2 = PerLane<unsigned int, 2>;
using uint3 = PerLane<unsigned int, 3>;
using uint4 = PerLane<unsigned int, 4>;

using int_t = PerLane<int>;
using int2 = PerLane<int, 2>;
using int3 = PerLane<int, 3>;
using int4 = PerLane<int, 4>;

using float_t = PerLane<float>;
using vec2 = PerLane<float, 2>;
using vec3 = PerLane<float, 3>;
using vec4 = PerLane<float, 4>;

}  // namespace SIMD
}  // namespace sw

////////////////////////////////////////////////////////////////////////////////
// namespace ::(anonymous)
// Utility functions
////////////////////////////////////////////////////////////////////////////////
namespace {

// vecElementName() returns the element name for the i'th vector element of
// size n.
// Vectors of size 4 or less use a [x,y,z,w] element naming scheme.
// Larger vectors use a number index naming scheme.
std::string vecElementName(int i, int n)
{
	return (n > 4) ? std::to_string(i) : &"x\0y\0z\0w\0"[i * 2];
}

// laneName() returns a string describing values for the lane i.
std::string laneName(int i)
{
	return "Lane " + std::to_string(i);
}

// isEntryBreakpointForShaderType() returns true if name is equal to the
// special entry breakpoint name for the given shader type.
// This allows the IDE to request all shaders of the given type to break on
// entry.
bool isEntryBreakpointForShaderType(spv::ExecutionModel type, const std::string &name)
{
	switch(type)
	{
	case spv::ExecutionModelGLCompute: return name == "ComputeShader";
	case spv::ExecutionModelFragment: return name == "FragmentShader";
	case spv::ExecutionModelVertex: return name == "VertexShader";
	default: return false;
	}
}

// makeDbgValue() returns a vk::dbg::Value that contains a copy of val.
template<typename T>
std::shared_ptr<vk::dbg::Value> makeDbgValue(const T &val)
{
	return vk::dbg::make_constant(val);
}

// makeDbgValue() returns a vk::dbg::Value that contains a copy of vec.
template<typename T, int N>
std::shared_ptr<vk::dbg::Value> makeDbgValue(const sw::vec<T, N> &vec)
{
	return vk::dbg::Struct::create("vec" + std::to_string(N), [&](auto &vc) {
		for(int i = 0; i < N; i++)
		{
			vc->put(vecElementName(i, N), makeDbgValue<T>(vec[i]));
		}
	});
}

// NullptrValue is an implementation of vk::dbg::Value that simply displays
// "<null>" for the given type.
class NullptrValue : public vk::dbg::Value
{
public:
	NullptrValue(const std::string &ty)
	    : ty(ty)
	{}
	std::string type() override { return ty; }
	std::string get(const vk::dbg::FormatFlags &) { return "<null>"; }

private:
	std::string ty;
};

// store() emits a store instruction to copy val into ptr.
template<typename T>
void store(const rr::RValue<rr::Pointer<rr::Byte>> &ptr, const rr::RValue<T> &val)
{
	*rr::Pointer<T>(ptr) = val;
}

// store() emits a store instruction to copy val into ptr.
template<typename T>
void store(const rr::RValue<rr::Pointer<rr::Byte>> &ptr, const T &val)
{
	*rr::Pointer<T>(ptr) = val;
}

// clang-format off
template<typename T> struct ReactorTypeSize {};
template<> struct ReactorTypeSize<rr::Int>    { static constexpr const int value = 4; };
template<> struct ReactorTypeSize<rr::Float>  { static constexpr const int value = 4; };
template<> struct ReactorTypeSize<rr::Int4>   { static constexpr const int value = 16; };
template<> struct ReactorTypeSize<rr::Float4> { static constexpr const int value = 16; };
// clang-format on

// store() emits a store instruction to copy val into ptr.
template<typename T, std::size_t N>
void store(const rr::RValue<rr::Pointer<rr::Byte>> &ptr, const std::array<T, N> &val)
{
	for(std::size_t i = 0; i < N; i++)
	{
		store<T>(ptr + i * ReactorTypeSize<T>::value, val[i]);
	}
}

// ArgTy<F>::type resolves to the single argument type of the function F.
template<typename F>
struct ArgTy
{
	using type = typename ArgTy<decltype(&F::operator())>::type;
};

// ArgTy<F>::type resolves to the single argument type of the template method.
template<typename R, typename C, typename Arg>
struct ArgTy<R (C::*)(Arg) const>
{
	using type = typename std::decay<Arg>::type;
};

// ArgTyT resolves to the single argument type of the template function or
// method F.
template<typename F>
using ArgTyT = typename ArgTy<F>::type;

// getOrCreate() searchs the map for the given key. If the map contains an entry
// with the given key, then the value is returned. Otherwise, create() is called
// and the returned value is placed into the map with the given key, and this
// value is returned.
// create is a function with the signature:
//   V()
template<typename K, typename V, typename CREATE, typename HASH>
V getOrCreate(std::unordered_map<K, V, HASH> &map, const K &key, CREATE &&create)
{
	auto it = map.find(key);
	if(it != map.end())
	{
		return it->second;
	}
	auto val = create();
	map.emplace(key, val);
	return val;
}

// HoversFromLocals is an implementation of vk::dbg::Variables that is used to
// provide a scope's 'hover' variables - those that appear when you place the
// cursor over a variable in an IDE.
// Unlike the top-level SIMD lane grouping of variables in Frame::locals,
// Frame::hovers displays each variable as a value per SIMD lane.
// Instead maintaining another collection of variables per scope,
// HoversFromLocals dynamically builds the hover information from the locals.
class HoversFromLocals : public vk::dbg::Variables
{
public:
	HoversFromLocals(const std::shared_ptr<vk::dbg::Variables> &locals)
	    : locals(locals)
	{}

	void foreach(size_t startIndex, size_t count, const ForeachCallback &cb) override
	{
		// No op - hovers are only searched, never iterated.
	}

	std::shared_ptr<vk::dbg::Value> get(const std::string &name) override
	{
		// Is the hover variable a SIMD-common variable? If so, just return
		// that.
		if(auto val = locals->get(name))
		{
			return val;
		}

		// Search each of the lanes for the named variable.
		// Collect them all up, and return that in a new Struct value.
		bool found = false;
		auto str = vk::dbg::Struct::create("", [&](auto &vc) {
			for(int lane = 0; lane < sw::SIMD::Width; lane++)
			{
				auto laneN = laneName(lane);
				if(auto laneV = locals->get(laneN))
				{
					if(auto children = laneV->children())
					{
						if(auto val = children->get(name))
						{
							vc->put(laneN, val);
							found = true;
						}
					}
				}
			}
		});

		if(found)
		{
			// The value returned will be returned to the debug client by
			// identifier. As the value is a Struct, the server will include
			// a handle to the Variables, which needs to be kept alive so the
			// client can send a request for its members.
			// lastFind keeps any nested Variables alive long enough for them to
			// be requested.
			lastFind = str;
			return str;
		}

		return nullptr;
	}

private:
	std::shared_ptr<vk::dbg::Variables> locals;
	std::shared_ptr<vk::dbg::Struct> lastFind;
};

}  // anonymous namespace

namespace spvtools {

// Function implemented in third_party/SPIRV-Tools/source/disassemble.cpp
// but with no public header.
// This is a C++ function, so the name is mangled, and signature changes will
// result in a linker error instead of runtime signature mismatches.
extern std::string spvInstructionBinaryToText(const spv_target_env env,
                                              const uint32_t *inst_binary,
                                              const size_t inst_word_count,
                                              const uint32_t *binary,
                                              const size_t word_count,
                                              const uint32_t options);

}  // namespace spvtools

////////////////////////////////////////////////////////////////////////////////
// namespace ::(anonymous)::debug
// OpenCL.Debug.100 data structures
////////////////////////////////////////////////////////////////////////////////
namespace {
namespace debug {

struct Declare;
struct LocalVariable;
struct Member;
struct Value;

// Object is the common base class for all the OpenCL.Debug.100 data structures.
struct Object
{
	enum class Kind
	{
		Object,
		Declare,
		Expression,
		Function,
		InlinedAt,
		GlobalVariable,
		LocalVariable,
		Member,
		Operation,
		Source,
		SourceScope,
		Value,
		TemplateParameter,

		// Scopes
		CompilationUnit,
		LexicalBlock,

		// Types
		BasicType,
		ArrayType,
		VectorType,
		FunctionType,
		CompositeType,
		TemplateType,
	};

	using ID = sw::SpirvID<Object>;
	static constexpr auto KIND = Kind::Object;
	inline Object(Kind kind)
	    : kind(kind)
	{
		(void)KIND;  // Used in debug builds. Avoid unused variable warnings in NDEBUG builds.
	}
	const Kind kind;

	// kindof() returns true iff kind is of this type, or any type deriving from
	// this type.
	static constexpr bool kindof(Object::Kind kind) { return true; }

	virtual ~Object() = default;
};

// cstr() returns the c-string name of the given Object::Kind.
constexpr const char *cstr(Object::Kind k)
{
	switch(k)
	{
	case Object::Kind::Object: return "Object";
	case Object::Kind::Declare: return "Declare";
	case Object::Kind::Expression: return "Expression";
	case Object::Kind::Function: return "Function";
	case Object::Kind::InlinedAt: return "InlinedAt";
	case Object::Kind::GlobalVariable: return "GlobalVariable";
	case Object::Kind::LocalVariable: return "LocalVariable";
	case Object::Kind::Member: return "Member";
	case Object::Kind::Operation: return "Operation";
	case Object::Kind::Source: return "Source";
	case Object::Kind::SourceScope: return "SourceScope";
	case Object::Kind::Value: return "Value";
	case Object::Kind::TemplateParameter: return "TemplateParameter";
	case Object::Kind::CompilationUnit: return "CompilationUnit";
	case Object::Kind::LexicalBlock: return "LexicalBlock";
	case Object::Kind::BasicType: return "BasicType";
	case Object::Kind::ArrayType: return "ArrayType";
	case Object::Kind::VectorType: return "VectorType";
	case Object::Kind::FunctionType: return "FunctionType";
	case Object::Kind::CompositeType: return "CompositeType";
	case Object::Kind::TemplateType: return "TemplateType";
	}
	return "<unknown>";
}

// ObjectImpl is a helper template struct which simplifies deriving from Object.
// ObjectImpl passes down the KIND to the Object constructor, and implements
// kindof().
template<typename TYPE, typename BASE, Object::Kind KIND>
struct ObjectImpl : public BASE
{
	using ID = sw::SpirvID<TYPE>;
	static constexpr auto Kind = KIND;

	ObjectImpl()
	    : BASE(Kind)
	{}
	static_assert(BASE::kindof(KIND), "BASE::kindof() returned false");

	// kindof() returns true iff kind is of this type, or any type deriving from
	// this type.
	static constexpr bool kindof(Object::Kind kind) { return kind == Kind; }
};

// cast() casts the debug type pointer obj to TO.
// If obj is null or not of the type TO, then nullptr is returned.
template<typename TO, typename FROM>
TO *cast(FROM *obj)
{
	if(obj == nullptr) { return nullptr; }  // None
	return (TO::kindof(obj->kind)) ? static_cast<TO *>(obj) : nullptr;
}

// cast() casts the debug type pointer obj to TO.
// If obj is null or not of the type TO, then nullptr is returned.
template<typename TO, typename FROM>
const TO *cast(const FROM *obj)
{
	if(obj == nullptr) { return nullptr; }  // None
	return (TO::kindof(obj->kind)) ? static_cast<const TO *>(obj) : nullptr;
}

// Scope is the base class for all OpenCL.DebugInfo.100 scope objects.
struct Scope : public Object
{
	using ID = sw::SpirvID<Scope>;
	inline Scope(Kind kind)
	    : Object(kind)
	{}

	// kindof() returns true iff kind is of this type, or any type deriving from
	// this type.
	static constexpr bool kindof(Kind kind)
	{
		return kind == Kind::CompilationUnit ||
		       kind == Kind::Function ||
		       kind == Kind::LexicalBlock;
	}

	struct Source *source = nullptr;
	Scope *parent = nullptr;
};

// Type is the base class for all OpenCL.DebugInfo.100 type objects.
struct Type : public Object
{
	using ID = sw::SpirvID<Type>;

	struct Member
	{
		Type *type;
		std::string name;
	};

	inline Type(Kind kind)
	    : Object(kind)
	{}

	// kindof() returns true iff kind is of this type, or any type deriving from
	// this type.
	static constexpr bool kindof(Kind kind)
	{
		return kind == Kind::BasicType ||
		       kind == Kind::ArrayType ||
		       kind == Kind::VectorType ||
		       kind == Kind::FunctionType ||
		       kind == Kind::CompositeType ||
		       kind == Kind::TemplateType;
	}

	// name() returns the type name.
	virtual std::string name() const = 0;

	// sizeInBytes() returns the number of bytes of the given debug type.
	virtual uint32_t sizeInBytes() const = 0;

	// value() returns a shared pointer to a vk::dbg::Value that views the data
	// at ptr of this type.
	virtual std::shared_ptr<vk::dbg::Value> value(void *ptr, bool interleaved) const = 0;

	// numMembers() returns the number of members for the given type.
	virtual size_t numMembers() const = 0;

	// getMember() returns the member by index.
	virtual Member getMember(size_t) const = 0;

	// undefined() returns a shared pointer to a vk::dbg::Value that represents
	// an undefined value of this type.
	std::shared_ptr<vk::dbg::Value> undefined() const
	{
		struct Undef : public vk::dbg::Value
		{
			Undef(const std::string &ty)
			    : ty(ty)
			{}
			const std::string ty;
			std::string type() override { return ty; }
			std::string get(const vk::dbg::FormatFlags &) override { return "<undefined>"; }
		};
		return std::make_shared<Undef>(name());
	}
};

// CompilationUnit represents the OpenCL.DebugInfo.100 DebugCompilationUnit
// instruction.
// https://www.khronos.org/registry/spir-v/specs/unified1/OpenCL.DebugInfo.100.html#DebugCompilationUnit
struct CompilationUnit : ObjectImpl<CompilationUnit, Scope, Object::Kind::CompilationUnit>
{
};

// Source represents the OpenCL.DebugInfo.100 DebugSource instruction.
// https://www.khronos.org/registry/spir-v/specs/unified1/OpenCL.DebugInfo.100.html#DebugSource
struct Source : ObjectImpl<Source, Object, Object::Kind::Source>
{
	spv::SourceLanguage language;
	uint32_t version = 0;
	std::string file;
	std::string source;

	std::shared_ptr<vk::dbg::File> dbgFile;
};

// BasicType represents the OpenCL.DebugInfo.100 DebugBasicType instruction.
// https://www.khronos.org/registry/spir-v/specs/unified1/OpenCL.DebugInfo.100.html#DebugBasicType
struct BasicType : ObjectImpl<BasicType, Type, Object::Kind::BasicType>
{
	std::string name_;
	uint32_t size = 0;  // in bits.
	OpenCLDebugInfo100DebugBaseTypeAttributeEncoding encoding = OpenCLDebugInfo100Unspecified;

	std::string name() const override { return name_; }
	uint32_t sizeInBytes() const override { return size / 8; }
	size_t numMembers() const override { return 0; }
	Member getMember(size_t) const override { return {}; }

	std::shared_ptr<vk::dbg::Value> value(void *ptr, bool interleaved) const override
	{
		if(ptr == nullptr) { return std::make_shared<NullptrValue>(name()); }

		switch(encoding)
		{
		case OpenCLDebugInfo100Address:
			// return vk::dbg::make_reference(*static_cast<void **>(ptr));
			UNIMPLEMENTED("b/148401179 OpenCLDebugInfo100 OpenCLDebugInfo100Address BasicType");
			return nullptr;
		case OpenCLDebugInfo100Boolean:
			return vk::dbg::make_reference(*static_cast<bool *>(ptr));
		case OpenCLDebugInfo100Float:
			return vk::dbg::make_reference(*static_cast<float *>(ptr));
		case OpenCLDebugInfo100Signed:
			return vk::dbg::make_reference(*static_cast<int32_t *>(ptr));
		case OpenCLDebugInfo100SignedChar:
			return vk::dbg::make_reference(*static_cast<int8_t *>(ptr));
		case OpenCLDebugInfo100Unsigned:
			return vk::dbg::make_reference(*static_cast<uint32_t *>(ptr));
		case OpenCLDebugInfo100UnsignedChar:
			return vk::dbg::make_reference(*static_cast<uint8_t *>(ptr));
		default:
			UNIMPLEMENTED("b/148401179 OpenCLDebugInfo100 encoding %d", int(encoding));
			return nullptr;
		}
	}
};

// ArrayType represents the OpenCL.DebugInfo.100 DebugTypeArray instruction.
// https://www.khronos.org/registry/spir-v/specs/unified1/OpenCL.DebugInfo.100.html#DebugTypeArray
//
// Unlike OpenCL.DebugInfo.100's DebugTypeArray, ArrayType is always
// single-dimensional. Multi-dimensional arrays are transformed into multiple
// nested ArrayTypes. This is done to simplify logic.
struct ArrayType : ObjectImpl<ArrayType, Type, Object::Kind::ArrayType>
{
	Type *base = nullptr;
	bool ownsBase = false;  // If true, base is owned by this ArrayType.
	uint32_t size;          // In elements

	~ArrayType()
	{
		if(ownsBase) { delete base; }
	}

	std::string name() const override { return base->name() + "[]"; }
	uint32_t sizeInBytes() const override { return base->sizeInBytes() * size; }
	size_t numMembers() const override { return size; }
	Member getMember(size_t i) const override { return { base, std::to_string(i) }; }

	std::shared_ptr<vk::dbg::Value> value(void *ptr, bool interleaved) const override
	{
		if(ptr == nullptr) { return std::make_shared<NullptrValue>(name()); }

		auto members = std::make_shared<vk::dbg::VariableContainer>();

		auto addr = static_cast<uint8_t *>(ptr);
		for(size_t i = 0; i < size; i++)
		{
			auto member = getMember(i);

#	if DEBUG_ANNOTATE_VARIABLE_KEYS
			key += " (" + std::to_string(addr) + " +" + std::to_string(offset) + ", i: " + std::to_string(i) + ")" + (interleaved ? "I" : "F");
#	endif
			members->put(member.name, base->value(addr, interleaved));

			addr += base->sizeInBytes() * (interleaved ? sw::SIMD::Width : 1);
		}
		return std::make_shared<vk::dbg::Struct>(name(), members);
	}
};

// VectorType represents the OpenCL.DebugInfo.100 DebugTypeVector instruction.
// https://www.khronos.org/registry/spir-v/specs/unified1/OpenCL.DebugInfo.100.html#DebugTypeVector
struct VectorType : ObjectImpl<VectorType, Type, Object::Kind::VectorType>
{
	Type *base = nullptr;
	uint32_t components = 0;

	std::string name() const override { return "vec" + std::to_string(components) + "<" + base->name() + ">"; }
	uint32_t sizeInBytes() const override { return base->sizeInBytes() * components; }
	size_t numMembers() const override { return components; }
	Member getMember(size_t i) const override { return { base, vecElementName(i, components) }; }

	std::shared_ptr<vk::dbg::Value> value(void *ptr, bool interleaved) const override
	{
		if(ptr == nullptr) { return std::make_shared<NullptrValue>(name()); }

		const auto elSize = base->sizeInBytes();
		auto members = std::make_shared<vk::dbg::VariableContainer>();
		for(uint32_t i = 0; i < components; i++)
		{
			auto offset = elSize * i * (interleaved ? sw::SIMD::Width : 1);
			auto elPtr = static_cast<uint8_t *>(ptr) + offset;
#	if DEBUG_ANNOTATE_VARIABLE_KEYS
			elKey += " (" + std::to_string(elPtr) + " +" + std::to_string(offset) + ")" + (interleaved ? "I" : "F");
#	endif
			members->put(getMember(i).name, base->value(elPtr, interleaved));
		}
		return std::make_shared<vk::dbg::Struct>(name(), members);
	}
};

// FunctionType represents the OpenCL.DebugInfo.100 DebugTypeFunction
// instruction.
// https://www.khronos.org/registry/spir-v/specs/unified1/OpenCL.DebugInfo.100.html#DebugTypeFunction
struct FunctionType : ObjectImpl<FunctionType, Type, Object::Kind::FunctionType>
{
	uint32_t flags = 0;  // OR'd from OpenCLDebugInfo100DebugInfoFlags
	Type *returnTy = nullptr;
	std::vector<Type *> paramTys;

	std::string name() const override { return "function"; }
	uint32_t sizeInBytes() const override { return 0; }
	size_t numMembers() const override { return 0; }
	Member getMember(size_t i) const override { return {}; }
	std::shared_ptr<vk::dbg::Value> value(void *ptr, bool interleaved) const override { return nullptr; }
};

// Member represents the OpenCL.DebugInfo.100 DebugTypeMember instruction.
// Despite the instruction name, this is not a type - rather a member of a type.
// https://www.khronos.org/registry/spir-v/specs/unified1/OpenCL.DebugInfo.100.html#DebugTypeMember
struct Member : ObjectImpl<Member, Object, Object::Kind::Member>
{
	std::string name;
	Type *type = nullptr;
	Source *source = nullptr;
	uint32_t line = 0;
	uint32_t column = 0;
	struct CompositeType *parent = nullptr;
	uint32_t offset = 0;  // in bits
	uint32_t size = 0;    // in bits
	uint32_t flags = 0;   // OR'd from OpenCLDebugInfo100DebugInfoFlags
};

// CompositeType represents the OpenCL.DebugInfo.100 DebugTypeComposite
// instruction.
// https://www.khronos.org/registry/spir-v/specs/unified1/OpenCL.DebugInfo.100.html#DebugTypeComposite
struct CompositeType : ObjectImpl<CompositeType, Type, Object::Kind::CompositeType>
{
	std::string name_;
	OpenCLDebugInfo100DebugCompositeType tag = OpenCLDebugInfo100Class;
	Source *source = nullptr;
	uint32_t line = 0;
	uint32_t column = 0;
	Object *parent = nullptr;
	std::string linkage;
	uint32_t size = 0;   // in bits.
	uint32_t flags = 0;  // OR'd from OpenCLDebugInfo100DebugInfoFlags
	std::vector<debug::Member *> members_;

	std::string name() const override { return name_; }
	uint32_t sizeInBytes() const override { return size / 8; }
	size_t numMembers() const override { return members_.size(); }
	Member getMember(size_t i) const override { return { members_[i]->type, members_[i]->name }; }

	std::shared_ptr<vk::dbg::Value> value(void *ptr, bool interleaved) const override
	{
		auto fields = std::make_shared<vk::dbg::VariableContainer>();
		for(auto &member : members_)
		{
			auto offset = (member->offset / 8) * (interleaved ? sw::SIMD::Width : 1);
			auto elPtr = static_cast<uint8_t *>(ptr) + offset;
			auto elKey = member->name;
#	if DEBUG_ANNOTATE_VARIABLE_KEYS
			// elKey += " (" + std::to_string(elPtr) + " +" + std::to_string(offset) + ")" + (interleaved ? "I" : "F");
#	endif
			fields->put(elKey, member->type->value(elPtr, interleaved));
		}
		return std::make_shared<vk::dbg::Struct>(name_, fields);
	}
};

// TemplateParameter represents the OpenCL.DebugInfo.100
// DebugTypeTemplateParameter instruction.
// https://www.khronos.org/registry/spir-v/specs/unified1/OpenCL.DebugInfo.100.html#DebugTypeTemplateParameter
struct TemplateParameter : ObjectImpl<TemplateParameter, Object, Object::Kind::TemplateParameter>
{
	std::string name;
	Type *type = nullptr;
	uint32_t value = 0;
	Source *source = nullptr;
	uint32_t line = 0;
	uint32_t column = 0;
};

// TemplateType represents the OpenCL.DebugInfo.100 DebugTypeTemplate
// instruction.
// https://www.khronos.org/registry/spir-v/specs/unified1/OpenCL.DebugInfo.100.html#DebugTypeTemplate
struct TemplateType : ObjectImpl<TemplateType, Type, Object::Kind::TemplateType>
{
	Type *target = nullptr;  // Class, struct or function.
	std::vector<TemplateParameter *> parameters;

	std::string name() const override { return "template<>"; }
	uint32_t sizeInBytes() const override { return target->sizeInBytes(); }
	size_t numMembers() const override { return 0; }
	Member getMember(size_t i) const override { return {}; }
	std::shared_ptr<vk::dbg::Value> value(void *ptr, bool interleaved) const override
	{
		return target->value(ptr, interleaved);
	}
};

// LexicalBlock represents the OpenCL.DebugInfo.100 DebugLexicalBlock instruction.
// https://www.khronos.org/registry/spir-v/specs/unified1/OpenCL.DebugInfo.100.html#DebugLexicalBlock
struct LexicalBlock : Scope
{
	using ID = sw::SpirvID<LexicalBlock>;
	static constexpr auto Kind = Object::Kind::LexicalBlock;

	inline LexicalBlock(Object::Kind kind = Kind)
	    : Scope(kind)
	{}

	uint32_t line = 0;
	uint32_t column = 0;
	std::string name;

	std::vector<LocalVariable *> variables;

	static constexpr bool kindof(Object::Kind kind) { return kind == Kind || kind == Object::Kind::Function; }
};

// Function represents the OpenCL.DebugInfo.100 DebugFunction instruction.
// https://www.khronos.org/registry/spir-v/specs/unified1/OpenCL.DebugInfo.100.html#DebugFunction
struct Function : ObjectImpl<Function, LexicalBlock, Object::Kind::Function>
{
	std::string name;
	FunctionType *type = nullptr;
	uint32_t declLine = 0;
	uint32_t declColumn = 0;
	std::string linkage;
	uint32_t flags = 0;  // OR'd from OpenCLDebugInfo100DebugInfoFlags
	sw::SpirvShader::Function::ID function;
};

// InlinedAt represents the OpenCL.DebugInfo.100 DebugInlinedAt instruction.
// https://www.khronos.org/registry/spir-v/specs/unified1/OpenCL.DebugInfo.100.html#DebugInlinedAt
struct InlinedAt : ObjectImpl<InlinedAt, Object, Object::Kind::InlinedAt>
{
	uint32_t line = 0;
	Scope *scope = nullptr;
	InlinedAt *inlined = nullptr;
};

// SourceScope represents the OpenCL.DebugInfo.100 DebugScope instruction.
// https://www.khronos.org/registry/spir-v/specs/unified1/OpenCL.DebugInfo.100.html#DebugScope
struct SourceScope : ObjectImpl<SourceScope, Object, Object::Kind::SourceScope>
{
	Scope *scope = nullptr;
	InlinedAt *inlinedAt = nullptr;
};

// GlobalVariable represents the OpenCL.DebugInfo.100 DebugGlobalVariable instruction.
// https://www.khronos.org/registry/spir-v/specs/unified1/OpenCL.DebugInfo.100.html#DebugGlobalVariable
struct GlobalVariable : ObjectImpl<GlobalVariable, Object, Object::Kind::GlobalVariable>
{
	std::string name;
	Type *type = nullptr;
	Source *source = nullptr;
	uint32_t line = 0;
	uint32_t column = 0;
	Scope *parent = nullptr;
	std::string linkage;
	sw::SpirvShader::Object::ID variable;
	uint32_t flags = 0;  // OR'd from OpenCLDebugInfo100DebugInfoFlags
};

// LocalVariable represents the OpenCL.DebugInfo.100 DebugLocalVariable
// instruction.
// Local variables are essentially just a scoped variable name.
// Their value comes from either a DebugDeclare (which has an immutable pointer
// to the actual data), or from a number of DebugValues (which can change
// any nested members of the variable over time).
// https://www.khronos.org/registry/spir-v/specs/unified1/OpenCL.DebugInfo.100.html#DebugLocalVariable
struct LocalVariable : ObjectImpl<LocalVariable, Object, Object::Kind::LocalVariable>
{
	static constexpr uint32_t NoArg = ~uint32_t(0);

	enum class Definition
	{
		Undefined,    // Variable has no defined value
		Declaration,  // Variable value comes from definition
		Values        // Variable value comes from values
	};

	std::string name;
	Type *type = nullptr;
	Source *source = nullptr;
	uint32_t line = 0;
	uint32_t column = 0;
	Scope *parent = nullptr;
	uint32_t arg = NoArg;

	Definition definition = Definition::Undefined;
	Declare *declaration = nullptr;  // Used if definition == Definition::Declaration

	// ValueNode is a tree node of debug::Value definitions.
	// Each node in the tree represents an element in the type tree.
	struct ValueNode
	{
		// NoDebugValueIndex indicates that this node is never assigned a value.
		static constexpr const uint32_t NoDebugValueIndex = ~0u;

		uint32_t debugValueIndex = NoDebugValueIndex;  // Index into State::lastReachedDebugValues
		std::unordered_map<uint32_t, std::unique_ptr<ValueNode>> children;
	};
	ValueNode values;  // Used if definition == Definition::Values
};

// Operation represents the OpenCL.DebugInfo.100 DebugOperation instruction.
// https://www.khronos.org/registry/spir-v/specs/unified1/OpenCL.DebugInfo.100.html#DebugOperation
struct Operation : ObjectImpl<Operation, Object, Object::Kind::Operation>
{
	uint32_t opcode = 0;
	std::vector<uint32_t> operands;
};

// Expression represents the OpenCL.DebugInfo.100 DebugExpression instruction.
// https://www.khronos.org/registry/spir-v/specs/unified1/OpenCL.DebugInfo.100.html#DebugExpression
struct Expression : ObjectImpl<Expression, Object, Object::Kind::Expression>
{
	std::vector<Operation *> operations;
};

// Declare represents the OpenCL.DebugInfo.100 DebugDeclare instruction.
// https://www.khronos.org/registry/spir-v/specs/unified1/OpenCL.DebugInfo.100.html#DebugDeclare
struct Declare : ObjectImpl<Declare, Object, Object::Kind::Declare>
{
	LocalVariable *local = nullptr;
	sw::SpirvShader::Object::ID variable;
	Expression *expression = nullptr;
};

// Value represents the OpenCL.DebugInfo.100 DebugValue instruction.
// https://www.khronos.org/registry/spir-v/specs/unified1/OpenCL.DebugInfo.100.html#DebugValue
struct Value : ObjectImpl<Value, Object, Object::Kind::Value>
{
	LocalVariable *local = nullptr;
	sw::SpirvShader::Object::ID value;
	Expression *expression = nullptr;
	std::vector<uint32_t> indexes;
};

// find<T>() searches the nested scopes, returning for the first scope that is
// castable to type T. If no scope can be found of type T, then nullptr is
// returned.
template<typename T>
T *find(Scope *scope)
{
	if(auto out = cast<T>(scope)) { return out; }
	return scope->parent ? find<T>(scope->parent) : nullptr;
}

inline const char *tostring(LocalVariable::Definition def)
{
	switch(def)
	{
	case LocalVariable::Definition::Undefined: return "Undefined";
	case LocalVariable::Definition::Declaration: return "Declaration";
	case LocalVariable::Definition::Values: return "Values";
	default: return "<unknown>";
	}
}

}  // namespace debug
}  // anonymous namespace

////////////////////////////////////////////////////////////////////////////////
// namespace ::sw
//
// Implementations for:
//   sw::SpirvShader::Impl::Debugger
//   sw::SpirvShader::Impl::Debugger::LocalVariableValue
//   sw::SpirvShader::Impl::Debugger::State
//   sw::SpirvShader::Impl::Debugger::State::Data
////////////////////////////////////////////////////////////////////////////////
namespace sw {

////////////////////////////////////////////////////////////////////////////////
// sw::SpirvShader::Impl::Debugger
//
// SpirvShader-private struct holding compile-time-mutable and
// execution-time-immutable debugger information.
//
// There is an instance of this class per shader program.
////////////////////////////////////////////////////////////////////////////////
struct SpirvShader::Impl::Debugger : public vk::dbg::ClientEventListener
{
	class State;
	class LocalVariableValue;

	Debugger(const SpirvShader *shader, const std::shared_ptr<vk::dbg::Context> &ctx);
	~Debugger();

	enum class Pass
	{
		Define,  // Pre-pass (called from SpirvShader constructor)
		Emit     // Code generation pass (called from SpirvShader::emit()).
	};

	// process() is called for each debugger instruction in two compiler passes.
	// For the Define pass, process() constructs ::debug objects and
	// registers them in the objects map.
	// For the Emit pass, process() populates the fields of ::debug objects and
	// potentially emits instructions for the shader program.
	void process(const InsnIterator &insn, EmitState *state, Pass pass);

	// finalize() must be called after all shader instruction have been emitted.
	// finalize() allocates the trap memory and registers the Debugger for
	// client debugger events so that it can monitor for changes in breakpoints.
	void finalize();

	// setNextSetLocationIsSteppable() indicates that the next call to
	// setLocation() must be a debugger steppable line.
	void setNextSetLocationIsSteppable();

	// setScope() sets the current debug source scope. Used by setLocation()
	// when the next location is debugger steppable.
	void setScope(debug::SourceScope *);

	// setLocation() sets the current codegen source location to the given file
	// and line.
	void setLocation(EmitState *state, const std::shared_ptr<vk::dbg::File> &, int line);
	void setLocation(EmitState *state, const char *file, int line);

	using SpirvInstruction = const void *;

	const SpirvShader *const shader;                              // The shader program being debugged
	std::shared_ptr<vk::dbg::Context> const ctx;                  // The debugger context
	bool shaderHasDebugInfo;                                      // True if the shader has high-level debug info (OpenCL.Debug100 instructions)
	std::shared_ptr<vk::dbg::File> spirvFile;                     // Virtual file containing SPIR-V disassembly instructions
	std::unordered_map<SpirvInstruction, int> spirvLineMappings;  // Instruction pointer to line
	std::unordered_map<SpirvInstruction, Object::ID> results;     // Instruction pointer to result ID

	// LocationAndScope holds a source location and scope pair.
	struct LocationAndScope
	{
		vk::dbg::Location location;
		debug::SourceScope *scope;

		inline bool operator==(const LocationAndScope &other) const
		{
			return location == other.location && scope == other.scope;
		}
		struct Hash
		{
			uint64_t operator()(const LocationAndScope &l) const
			{
				return std::hash<decltype(l.location)>()(l.location) ^ std::hash<decltype(l.scope)>()(l.scope);
			}
		};
	};

	// Traps holds information about debugger traps - points in the shader
	// program where execution may pause for the debugger, either due to hitting
	// a breakpoint or following a single line step.
	// The Traps::memory is continually read during execution of a shader,
	// triggering a trap when the byte is non-zero. Traps can also be enabled
	// via the State::alwaysTrap field.
	struct Traps
	{
		// Source location + scope -> line trap index
		std::unordered_map<LocationAndScope, size_t, LocationAndScope::Hash> byLocationAndScope;

		// Function name -> entry trap index
		std::unordered_map<std::string, size_t> byFunctionName;

		// Trap index -> source location + scope
		std::vector<LocationAndScope> byIndex;

		// Trap memory - shared for all running instances of the shader.
		// Each byte represents a single trap enabled (1) / disabled (0) state.
		std::unique_ptr<uint8_t[]> memory;
	} traps;

	// Shadow memory is used to construct a contiguous memory block
	// (State::shadow) that contains an up-to-date copy of each
	// SpirvShader::Object's value(s) in the currently executing shader.
	// Shadow memory either contains SIMD-interleaved values for all components
	// in the object, or a SIMD-pointer (Shadow::Pointer).
	struct Shadow
	{
		// Entry describes the byte offset and kind of the shadow memory for
		// a single SpirvShader::Object.
		struct Entry
		{
			enum class Kind
			{
				Value,
				Pointer,
			};
			Kind kind;
			uint32_t offset;
		};

		// Pointer is the structure stored in shadow memory for pointer types.
		// The address for a given SIMD lane is the base + offsets[lane].
		struct Pointer
		{
			uint8_t *base;                      // Common base address for all SIMD lanes.
			uint32_t offsets[sw::SIMD::Width];  // Per lane offsets.
		};

		// Memory is returned by get().
		// Memory holds a pointer (addr) to the entry in the shadow memory, and
		// provides the dref() method for dereferencing a pointer for the given
		// SIMD lane.
		struct Memory
		{
			inline operator void *();
			inline Memory dref(int lane) const;
			uint8_t *addr;
		};

		// create() adds a new entry for the object with the given id.
		void create(const SpirvShader *, const EmitState *, Object::ID);

		// get() returns a Memory pointing to the shadow memory for the object
		// with the given id.
		Memory get(const State *, Object::ID) const;

		std::unordered_map<Object::ID, Entry> entries;
		uint32_t size = 0;  // Total size of the shadow memory in bytes.
	} shadow;

	// vk::dbg::ClientEventListener
	void onSetBreakpoint(const vk::dbg::Location &location, bool &handled) override;
	void onSetBreakpoint(const std::string &func, bool &handled) override;
	void onBreakpointsChanged() override;

private:
	// add() registers the debug object with the given id.
	template<typename ID>
	void add(ID id, std::unique_ptr<debug::Object> &&);

	// addNone() registers given id as a None value or type.
	void addNone(debug::Object::ID id);

	// isNone() returns true if the given id was registered as none with
	// addNone().
	bool isNone(debug::Object::ID id) const;

	// get() returns the debug object with the given id.
	// The object must exist and be of type (or derive from type) T.
	// A returned nullptr represents a None value or type.
	template<typename T>
	T *get(SpirvID<T> id) const;

	// getOrNull() returns the debug object with the given id if
	// the object exists and is of type (or derive from type) T.
	// Otherwise, returns nullptr.
	template<typename T>
	T *getOrNull(SpirvID<T> id) const;

	// use get() and add() to access this
	std::unordered_map<debug::Object::ID, std::unique_ptr<debug::Object>> objects;

	// defineOrEmit() when called in Pass::Define, creates and stores a
	// zero-initialized object into the Debugger::objects map using the
	// object identifier held by second instruction operand.
	// When called in Pass::Emit, defineOrEmit() calls the function F with the
	// previously-built object.
	//
	// F must be a function with the signature:
	//   void(OBJECT_TYPE *)
	//
	// The object type is automatically inferred from the function signature.
	template<typename F, typename T = typename std::remove_pointer<ArgTyT<F>>::type>
	void defineOrEmit(InsnIterator insn, Pass pass, F &&emit);

	std::unordered_map<std::string, std::shared_ptr<vk::dbg::File>> files;
	uint32_t numDebugValueSlots = 0;  // Number of independent debug::Values which need to be tracked
	bool nextSetLocationIsSteppable = true;
	debug::SourceScope *lastSetScope = nullptr;
	vk::dbg::Location lastSetLocation;
};

////////////////////////////////////////////////////////////////////////////////
// sw::SpirvShader::Impl::Debugger::LocalVariableValue
//
// Implementation of vk::dbg::Value that displays a debug::LocalVariable that
// has its value(s) defined by debug::Value(s).
//
// TODO(b/145351270) Note: The OpenCL.DebugInfo.100 spec does not state how
// DebugValues should be applied to the DebugLocalVariable.
//
// This implementation keeps track of the order of DebugValues as they are
// 'executed', and uses the most recent values for each specific index.
// OpenCL.DebugInfo.100 is significantly derived from the LLVM debug
// instructions, and so it can be assumed that DebugValue is intended to behave
// like llvm.dbg.value.
//
// https://llvm.org/docs/SourceLevelDebugging.html#object-lifetime-in-optimized-code
// describes the expected behavior of llvm.dbg.value, which instead of runtime
// tracking, uses static analysis of the LLVM IR to determine which debug
// values should be used.
//
// If DebugValue is to behave the same way as llvm.dbg.value, then this
// implementation should be changed to examine the order of DebugValue
// instructions in the SPIR-V. This can only be done once the SPIR-V generating
// compiler and SPIR-V optimization passes generate and preserve the DebugValue
// ordering as described in the LLVM SourceLevelDebugging document.
////////////////////////////////////////////////////////////////////////////////
class sw::SpirvShader::Impl::Debugger::LocalVariableValue : public vk::dbg::Value
{
public:
	// Data shared across all nodes in the LocalVariableValue.
	struct Shared
	{
		Shared(const debug::LocalVariable *const variable, const State *const state, int const lane)
		    : variable(variable)
		    , state(state)
		    , lane(lane)
		{
			ASSERT(variable->definition == debug::LocalVariable::Definition::Values);
		}

		const debug::LocalVariable *const variable;
		const State *const state;
		int const lane;
	};

	LocalVariableValue(debug::LocalVariable *variable, const State *const state, int lane);

	LocalVariableValue(
	    const std::shared_ptr<const Shared> &shared,
	    const debug::Type *ty,
	    const debug::LocalVariable::ValueNode *node);

private:
	// vk::dbg::Value
	std::string type() override;
	std::string get(const vk::dbg::FormatFlags &) override;
	std::shared_ptr<vk::dbg::Variables> children() override;

	void updateValue();
	std::shared_ptr<const Shared> const shared;
	const debug::Type *const ty;
	const debug::LocalVariable::ValueNode *const node;
	debug::Value *activeValue = nullptr;
	std::shared_ptr<vk::dbg::Value> value;
};

////////////////////////////////////////////////////////////////////////////////
// sw::SpirvShader::Impl::Debugger::State
//
// State holds the runtime data structures for the shader debug session.
//
// When debugging is enabled, the shader program will construct a State with a
// call to create(), and during execution write shader information into fields
// of this class, including:
//  * Shadow memory for keeping track of register-held values.
//  * Global variables.
//  * Last reached ::debug::Values (see LocalVariableValue)
//
// Bulky data that is only needed once the shader has hit a trap is held by
// State::Data. This is lazily constructed by the first call to trap().
//
// There is an instance of this class per shader invocation.
////////////////////////////////////////////////////////////////////////////////
class SpirvShader::Impl::Debugger::State
{
public:
	// Globals holds a copy of the shader's builtin global variables.
	struct Globals
	{
		struct Compute
		{
			sw::uint3 numWorkgroups;
			sw::uint3 workgroupID;
			sw::uint3 workgroupSize;
			uint32_t numSubgroups;
			uint32_t subgroupIndex;
			sw::SIMD::uint3 globalInvocationId;
			sw::SIMD::uint3 localInvocationId;
			sw::SIMD::uint3 localInvocationIndex;
		};
		struct Fragment
		{
			uint32_t viewIndex;
			sw::SIMD::vec4 fragCoord;
			sw::SIMD::vec4 pointCoord;
			sw::SIMD::int2 windowSpacePosition;
			sw::SIMD::uint_t helperInvocation;
		};
		struct Vertex
		{
			uint32_t viewIndex;
			uint32_t instanceIndex;
			sw::SIMD::uint_t vertexIndex;
		};

		// Common for all shader types
		uint32_t subgroupSize;
		sw::SIMD::uint_t activeLaneMask;

		// Shader type specific globals
		union
		{
			Compute compute;
			Fragment fragment;
			Vertex vertex;
		};
	};

	// create() allocates, constructs and returns a State.
	// Called at the start of the debugger-enabled shader program.
	static State *create(const Debugger *debugger);

	// destroy() destructs and frees a state.
	// Called at the end of the debugger-enabled shader program.
	static void destroy(State *);

	// trap() is called by the debugger-enabled shader program to suspend
	// execution of the shader. This will appear in the attached debugger as if
	// a breakpoint has been hit.
	// trap() will be called if the Debugger::Traps::memory[index] is non-zero,
	// or if alwaysTrap is non-zero.
	// index is the index of the trap (see Debugger::Traps).
	void trap(int index);

	const Debugger *const debugger;

	// traps is a simple copy of Debugger::Traps::memory.
	// Copied here to reduce pointer chasing during shader execution.
	uint8_t *traps = nullptr;

	// alwaysTrap (if non-zero) forces a call trap() even if
	// Debugger::Traps::memory[index] is zero. Used to perform single line
	// stepping (pause at next line / instruction).
	uint8_t alwaysTrap = 0;

	// Global variable values. Written to at shader start.
	Globals globals;

	// Shadow memory for all SpirvShader::Objects in the executing shader
	// program.
	// See Debugger::Shadow for more information.
	std::unique_ptr<uint8_t[]> const shadow;

	// Array of last reached debug::Value.
	// Indexed by ::debug::LocalVariable::ValueNode::debugValueIndex.
	std::unique_ptr<debug::Value *[]> const lastReachedDebugValues;

private:
	// Data holds the debugger-interface state (vk::dbg::*).
	// This is only constructed on the first call to Debugger::State::trap() as
	// it contains data that is only needed when the debugger is actively
	// inspecting execution of the shader program.
	struct Data
	{
		Data(State *state);

		// terminate() is called at the end of execution of the shader program.
		// terminate() ensures that the debugger thread stack is at the same
		// level as when the program entered.
		void terminate(State *state);

		// trap() updates the debugger thread with the stack frames and
		// variables at the trap's scoped location.
		// trap() will notify the debugger that the thread has paused, and will
		// block until instructed to resume (either continue or step) by the
		// user.
		void trap(int index, State *state);

	private:
		using PerLaneVariables = std::array<std::shared_ptr<vk::dbg::VariableContainer>, sw::SIMD::Width>;

		struct StackEntry
		{
			debug::LexicalBlock *block;
			uint32_t line;

			bool operator!=(const StackEntry &other) const { return block != other.block || line != other.line; }
		};

		struct GlobalVariables
		{
			std::shared_ptr<vk::dbg::VariableContainer> common;
			PerLaneVariables lanes;
		};

		// updateFrameLocals() updates the local variables in the frame with
		// those in the lexical block.
		void updateFrameLocals(State *state, vk::dbg::Frame &frame, debug::LexicalBlock *block);

		// getOrCreateLocals() creates and returns the per-lane local variables
		// from those in the lexical block.
		PerLaneVariables getOrCreateLocals(State *state, const debug::LexicalBlock *block);

		// buildGlobal() creates and adds to globals global variable with the
		// given name and value. The value is copied instead of holding a
		// pointer to val.
		template<typename T>
		void buildGlobal(const char *name, const T &val);
		template<typename T, int N>
		void buildGlobal(const char *name, const sw::SIMD::PerLane<T, N> &vec);

		// buildGlobals() builds all the global variable values, populating
		// globals.
		void buildGlobals(State *state);

		// buildSpirvVariables() builds a Struct holding all the SPIR-V named
		// values for the given lane.
		std::shared_ptr<vk::dbg::Struct> buildSpirvVariables(State *state, int lane) const;

		// buildSpirvValue() returns a debugger value for the SPIR-V shadow
		// value at memory of the given type and for the given lane.
		std::shared_ptr<vk::dbg::Value> buildSpirvValue(State *state, Shadow::Memory memory, const SpirvShader::Type &type, int lane) const;

		GlobalVariables globals;
		std::shared_ptr<vk::dbg::Thread> thread;
		std::vector<StackEntry> stack;
		std::unordered_map<const debug::LexicalBlock *, PerLaneVariables> locals;
	};

	State(const Debugger *debugger);
	~State();
	std::unique_ptr<Data> data;
};

////////////////////////////////////////////////////////////////////////////////
// sw::SpirvShader::Impl::Debugger methods
////////////////////////////////////////////////////////////////////////////////
SpirvShader::Impl::Debugger::Debugger(const SpirvShader *shader, const std::shared_ptr<vk::dbg::Context> &ctx)
    : shader(shader)
    , ctx(ctx)
{
}

SpirvShader::Impl::Debugger::~Debugger()
{
	ctx->removeListener(this);
}

void SpirvShader::Impl::Debugger::finalize()
{
	ASSERT(traps.byIndex.size() == traps.byLocationAndScope.size());
	traps.memory = std::make_unique<uint8_t[]>(traps.byIndex.size());

	ctx->addListener(this);

	// Register existing breakpoints.
	onBreakpointsChanged();
}

void sw::SpirvShader::Impl::Debugger::setNextSetLocationIsSteppable()
{
	nextSetLocationIsSteppable = true;
}

void SpirvShader::Impl::Debugger::setScope(debug::SourceScope *scope)
{
	lastSetScope = scope;
}

void SpirvShader::Impl::Debugger::setLocation(EmitState *state, const std::shared_ptr<vk::dbg::File> &file, int line)
{
	vk::dbg::Location location{ file, line };

	if(location != lastSetLocation)
	{
		// If the location has changed, then this is always a step.
		nextSetLocationIsSteppable = true;
		lastSetLocation = location;
	}

	if(nextSetLocationIsSteppable)
	{
		// Get or create the trap for the given location and scope.
		LocationAndScope locationAndScope{ location, lastSetScope };
		int index = getOrCreate(traps.byLocationAndScope, locationAndScope, [&] {
			traps.byIndex.emplace_back(locationAndScope);
			return traps.byIndex.size() - 1;
		});

		// Also create a map index for the given scope's function so we can
		// break on function entry.
		if(lastSetScope)
		{
			if(auto func = debug::find<debug::Function>(lastSetScope->scope))
			{
				getOrCreate(traps.byFunctionName, func->name, [&] { return index; });
			}
		}

		// Emit the shader logic to test the trap value (either through via
		// Debugger::State::traps[] or Debugger::State::alwaysTrap), and call
		// Debugger::State::trap() if either are true.
		auto dbgState = state->routine->dbgState;
		auto alwaysTrap = *Pointer<Byte>(dbgState + OFFSET(Impl::Debugger::State, alwaysTrap));
		auto traps = *Pointer<Pointer<Byte>>(dbgState + OFFSET(Impl::Debugger::State, traps));
		auto trap = Pointer<Byte>(traps)[index];
		If(alwaysTrap != Byte(0) || trap != Byte(0))
		{
			rr::Call(&State::trap, state->routine->dbgState, index);
		}
		nextSetLocationIsSteppable = false;
	}
}

void SpirvShader::Impl::Debugger::setLocation(EmitState *state, const char *path, int line)
{
	auto lock = ctx->lock();
	auto file = lock.findFile(path);
	if(!file)
	{
		file = lock.createPhysicalFile(path);
	}
	setLocation(state, file, line);
}

void SpirvShader::Impl::Debugger::onSetBreakpoint(const vk::dbg::Location &location, bool &handled)
{
	// Notify the debugger if the breakpoint location is handled.
	// We don't actually set the trap here as this is performed by
	// onBreakpointsChanged(), which is only called once, even for multiple
	// breakpoint changes.
	for(auto it : traps.byLocationAndScope)
	{
		if(location == it.first.location)
		{
			handled = true;
			return;
		}
	}
}

void SpirvShader::Impl::Debugger::onSetBreakpoint(const std::string &func, bool &handled)
{
	// Notify the debugger if the function-entry breakpoint is handled.
	// We don't actually set the trap here as this is performed by
	// onBreakpointsChanged(), which is only called once, even for multiple
	// breakpoint changes.
	auto it = traps.byFunctionName.find(func);
	if(it != traps.byFunctionName.end())
	{
		handled = true;
	}

	if(isEntryBreakpointForShaderType(shader->executionModel, func))
	{
		handled = true;
	}
}

void SpirvShader::Impl::Debugger::onBreakpointsChanged()
{
	// TODO(b/145351270): TSAN will probably moan that traps.memory is being
	// modified while being read on othe threads. We can solve this by adding
	// a shared mutex (RWMutex) for the traps, read-locking for execution, and
	// write locking here. This will prevent setting breakpoints while a shader
	// is executing (maybe problematic if you want to debug a slow or
	// never-completing shader).
	// For now, just be racy. It's unlikely that this will cause any noticable
	// problems.

	// Start by disabling all traps.
	memset(traps.memory.get(), 0, traps.byIndex.size() * sizeof(traps.memory[0]));

	// Add traps for all breakpoints by location.
	for(auto it : files)
	{
		auto &file = it.second;
		for(auto line : file->getBreakpoints())
		{
			for(auto it : traps.byLocationAndScope)
			{
				if(it.first.location == vk::dbg::Location{ file, line })
				{
					traps.memory[it.second] = 1;
				}
			}
		}
	}

	// Add traps for all breakpoints by function name.
	auto lock = ctx->lock();
	for(auto it : traps.byFunctionName)
	{
		if(lock.isFunctionBreakpoint(it.first))
		{
			traps.memory[it.second] = 1;
		}
	}

	// Add traps for breakpoints by shader type.
	for(auto bp : lock.getFunctionBreakpoints())
	{
		if(isEntryBreakpointForShaderType(shader->executionModel, bp))
		{
			traps.memory[0] = 1;
		}
	}
}

template<typename F, typename T>
void SpirvShader::Impl::Debugger::defineOrEmit(InsnIterator insn, Pass pass, F &&emit)
{
	auto id = SpirvID<T>(insn.word(2));
	switch(pass)
	{
	case Pass::Define:
		add(id, std::unique_ptr<debug::Object>(new T()));
		break;
	case Pass::Emit:
		emit(get<T>(id));
		break;
	}
}

void SpirvShader::Impl::Debugger::process(const InsnIterator &insn, EmitState *state, Pass pass)
{
	auto extInstIndex = insn.word(4);
	switch(extInstIndex)
	{
	case OpenCLDebugInfo100DebugInfoNone:
		if(pass == Pass::Define)
		{
			addNone(debug::Object::ID(insn.word(2)));
		}
		break;
	case OpenCLDebugInfo100DebugCompilationUnit:
		defineOrEmit(insn, pass, [&](debug::CompilationUnit *cu) {
			cu->source = get(debug::Source::ID(insn.word(7)));
		});
		break;
	case OpenCLDebugInfo100DebugTypeBasic:
		defineOrEmit(insn, pass, [&](debug::BasicType *type) {
			type->name_ = shader->getString(insn.word(5));
			type->size = shader->GetConstScalarInt(insn.word(6));
			type->encoding = static_cast<OpenCLDebugInfo100DebugBaseTypeAttributeEncoding>(insn.word(7));
		});
		break;
	case OpenCLDebugInfo100DebugTypeArray:
		defineOrEmit(insn, pass, [&](debug::ArrayType *type) {
			type->base = get(debug::Type::ID(insn.word(5)));
			type->size = shader->GetConstScalarInt(insn.word(6));
			for(uint32_t i = 7; i < insn.wordCount(); i++)
			{
				// Decompose multi-dimentional into nested single
				// dimensional arrays. Greatly simplifies logic.
				auto inner = new debug::ArrayType();
				inner->base = type->base;
				type->size = shader->GetConstScalarInt(insn.word(i));
				type->base = inner;
				type->ownsBase = true;
				type = inner;
			}
		});
		break;
	case OpenCLDebugInfo100DebugTypeVector:
		defineOrEmit(insn, pass, [&](debug::VectorType *type) {
			type->base = get(debug::Type::ID(insn.word(5)));
			type->components = insn.word(6);
		});
		break;
	case OpenCLDebugInfo100DebugTypeFunction:
		defineOrEmit(insn, pass, [&](debug::FunctionType *type) {
			type->flags = insn.word(5);
			type->returnTy = getOrNull(debug::Type::ID(insn.word(6)));

			// 'Return Type' operand must be a debug type or OpTypeVoid. See
			// https://www.khronos.org/registry/spir-v/specs/unified1/OpenCL.DebugInfo.100.html#DebugTypeFunction
			ASSERT_MSG(type->returnTy != nullptr || shader->getType(insn.word(6)).opcode() == spv::Op::OpTypeVoid, "Invalid return type of DebugTypeFunction: %d", insn.word(6));

			for(uint32_t i = 7; i < insn.wordCount(); i++)
			{
				type->paramTys.push_back(get(debug::Type::ID(insn.word(i))));
			}
		});
		break;
	case OpenCLDebugInfo100DebugTypeComposite:
		defineOrEmit(insn, pass, [&](debug::CompositeType *type) {
			type->name_ = shader->getString(insn.word(5));
			type->tag = static_cast<OpenCLDebugInfo100DebugCompositeType>(insn.word(6));
			type->source = get(debug::Source::ID(insn.word(7)));
			type->line = insn.word(8);
			type->column = insn.word(9);
			type->parent = get(debug::Object::ID(insn.word(10)));
			type->linkage = shader->getString(insn.word(11));
			type->size = isNone(insn.word(12)) ? 0 : shader->GetConstScalarInt(insn.word(12));
			type->flags = insn.word(13);
			for(uint32_t i = 14; i < insn.wordCount(); i++)
			{
				auto obj = get(debug::Object::ID(insn.word(i)));
				if(auto member = debug::cast<debug::Member>(obj))  // Can also be Function or TypeInheritance, which we don't care about.
				{
					type->members_.push_back(member);
				}
			}
		});
		break;
	case OpenCLDebugInfo100DebugTypeMember:
		defineOrEmit(insn, pass, [&](debug::Member *member) {
			member->name = shader->getString(insn.word(5));
			member->type = get(debug::Type::ID(insn.word(6)));
			member->source = get(debug::Source::ID(insn.word(7)));
			member->line = insn.word(8);
			member->column = insn.word(9);
			member->parent = get(debug::CompositeType::ID(insn.word(10)));
			member->offset = shader->GetConstScalarInt(insn.word(11));
			member->size = shader->GetConstScalarInt(insn.word(12));
			member->flags = insn.word(13);
		});
		break;
	case OpenCLDebugInfo100DebugTypeTemplate:
		defineOrEmit(insn, pass, [&](debug::TemplateType *tpl) {
			tpl->target = get(debug::Type::ID(insn.word(5)));
			for(size_t i = 6, c = insn.wordCount(); i < c; i++)
			{
				tpl->parameters.emplace_back(get(debug::TemplateParameter::ID(insn.word(i))));
			}
		});
		break;
	case OpenCLDebugInfo100DebugTypeTemplateParameter:
		defineOrEmit(insn, pass, [&](debug::TemplateParameter *param) {
			param->name = shader->getString(insn.word(5));
			param->type = get(debug::Type::ID(insn.word(6)));
			param->value = 0;  // TODO: Get value from OpConstant if "a template value parameter".
			param->source = get(debug::Source::ID(insn.word(8)));
			param->line = insn.word(9);
			param->column = insn.word(10);
		});
		break;
	case OpenCLDebugInfo100DebugGlobalVariable:
		defineOrEmit(insn, pass, [&](debug::GlobalVariable *var) {
			var->name = shader->getString(insn.word(5));
			var->type = get(debug::Type::ID(insn.word(6)));
			var->source = get(debug::Source::ID(insn.word(7)));
			var->line = insn.word(8);
			var->column = insn.word(9);
			var->parent = get(debug::Scope::ID(insn.word(10)));
			var->linkage = shader->getString(insn.word(11));
			var->variable = isNone(insn.word(12)) ? 0 : insn.word(12);
			var->flags = insn.word(13);
			// static member declaration: word(14)
		});
		break;
	case OpenCLDebugInfo100DebugFunction:
		defineOrEmit(insn, pass, [&](debug::Function *func) {
			func->name = shader->getString(insn.word(5));
			func->type = get(debug::FunctionType::ID(insn.word(6)));
			func->source = get(debug::Source::ID(insn.word(7)));
			func->declLine = insn.word(8);
			func->declColumn = insn.word(9);
			func->parent = get(debug::Scope::ID(insn.word(10)));
			func->linkage = shader->getString(insn.word(11));
			func->flags = insn.word(12);
			func->line = insn.word(13);
			func->function = Function::ID(insn.word(14));
			// declaration: word(13)
		});
		break;
	case OpenCLDebugInfo100DebugLexicalBlock:
		defineOrEmit(insn, pass, [&](debug::LexicalBlock *scope) {
			scope->source = get(debug::Source::ID(insn.word(5)));
			scope->line = insn.word(6);
			scope->column = insn.word(7);
			scope->parent = get(debug::Scope::ID(insn.word(8)));
			if(insn.wordCount() > 9)
			{
				scope->name = shader->getString(insn.word(9));
			}
		});
		break;
	case OpenCLDebugInfo100DebugScope:
		defineOrEmit(insn, pass, [&](debug::SourceScope *ss) {
			ss->scope = get(debug::Scope::ID(insn.word(5)));
			if(insn.wordCount() > 6)
			{
				ss->inlinedAt = get(debug::InlinedAt::ID(insn.word(6)));
			}
			setScope(ss);
		});
		break;
	case OpenCLDebugInfo100DebugNoScope:
		break;
	case OpenCLDebugInfo100DebugInlinedAt:
		defineOrEmit(insn, pass, [&](debug::InlinedAt *ia) {
			ia->line = insn.word(5);
			ia->scope = get(debug::Scope::ID(insn.word(6)));
			if(insn.wordCount() > 7)
			{
				ia->inlined = get(debug::InlinedAt::ID(insn.word(7)));
			}
		});
		break;
	case OpenCLDebugInfo100DebugLocalVariable:
		defineOrEmit(insn, pass, [&](debug::LocalVariable *var) {
			var->name = shader->getString(insn.word(5));
			var->type = get(debug::Type::ID(insn.word(6)));
			var->source = get(debug::Source::ID(insn.word(7)));
			var->line = insn.word(8);
			var->column = insn.word(9);
			var->parent = get(debug::Scope::ID(insn.word(10)));
			if(insn.wordCount() > 11)
			{
				var->arg = insn.word(11);
			}
			if(auto block = debug::find<debug::LexicalBlock>(var->parent))
			{
				block->variables.emplace_back(var);
			}
		});
		break;
	case OpenCLDebugInfo100DebugDeclare:
		defineOrEmit(insn, pass, [&](debug::Declare *decl) {
			decl->local = get(debug::LocalVariable::ID(insn.word(5)));
			decl->variable = Object::ID(insn.word(6));
			decl->expression = get(debug::Expression::ID(insn.word(7)));

			decl->local->declaration = decl;

			ASSERT_MSG(decl->local->definition == debug::LocalVariable::Definition::Undefined,
			           "DebugLocalVariable '%s' declared at %s:%d was previously defined as %s, now again as %s",
			           decl->local->name.c_str(),
			           decl->local->source ? decl->local->source->file.c_str() : "<unknown>",
			           (int)decl->local->line,
			           tostring(decl->local->definition),
			           tostring(debug::LocalVariable::Definition::Declaration));
			decl->local->definition = debug::LocalVariable::Definition::Declaration;
		});
		break;
	case OpenCLDebugInfo100DebugValue:
		defineOrEmit(insn, pass, [&](debug::Value *value) {
			value->local = get(debug::LocalVariable::ID(insn.word(5)));
			value->value = insn.word(6);
			value->expression = get(debug::Expression::ID(insn.word(7)));

			if(value->local->definition == debug::LocalVariable::Definition::Undefined)
			{
				value->local->definition = debug::LocalVariable::Definition::Values;
			}
			else
			{
				ASSERT_MSG(value->local->definition == debug::LocalVariable::Definition::Values,
				           "DebugLocalVariable '%s' declared at %s:%d was previously defined as %s, now again as %s",
				           value->local->name.c_str(),
				           value->local->source ? value->local->source->file.c_str() : "<unknown>",
				           (int)value->local->line,
				           tostring(value->local->definition),
				           tostring(debug::LocalVariable::Definition::Values));
			}

			auto node = &value->local->values;
			for(uint32_t i = 8; i < insn.wordCount(); i++)
			{
				auto idx = shader->GetConstScalarInt(insn.word(i));
				value->indexes.push_back(idx);

				auto it = node->children.find(idx);
				if(it != node->children.end())
				{
					node = it->second.get();
				}
				else
				{
					auto parent = node;
					auto child = std::make_unique<debug::LocalVariable::ValueNode>();
					node = child.get();
					parent->children.emplace(idx, std::move(child));
				}
			}

			if(node->debugValueIndex == debug::LocalVariable::ValueNode::NoDebugValueIndex)
			{
				node->debugValueIndex = numDebugValueSlots++;
			}

			rr::Pointer<rr::Pointer<Byte>> lastReachedArray = *rr::Pointer<rr::Pointer<rr::Pointer<Byte>>>(
			    state->routine->dbgState + OFFSET(Impl::Debugger::State, lastReachedDebugValues));
			rr::Pointer<rr::Pointer<Byte>> lastReached = &lastReachedArray[node->debugValueIndex];
			*lastReached = rr::ConstantPointer(value);
		});
		break;
	case OpenCLDebugInfo100DebugExpression:
		defineOrEmit(insn, pass, [&](debug::Expression *expr) {
			for(uint32_t i = 5; i < insn.wordCount(); i++)
			{
				expr->operations.push_back(get(debug::Operation::ID(insn.word(i))));
			}
		});
		break;
	case OpenCLDebugInfo100DebugSource:
		defineOrEmit(insn, pass, [&](debug::Source *source) {
			source->file = shader->getString(insn.word(5));
			if(insn.wordCount() > 6)
			{
				source->source = shader->getString(insn.word(6));
				auto file = ctx->lock().createVirtualFile(source->file.c_str(), source->source.c_str());
				source->dbgFile = file;
				files.emplace(source->file.c_str(), file);
			}
			else
			{
				auto file = ctx->lock().createPhysicalFile(source->file.c_str());
				source->dbgFile = file;
				files.emplace(source->file.c_str(), file);
			}
		});
		break;
	case OpenCLDebugInfo100DebugOperation:
		defineOrEmit(insn, pass, [&](debug::Operation *operation) {
			operation->opcode = insn.word(5);
			for(uint32_t i = 6; i < insn.wordCount(); i++)
			{
				operation->operands.push_back(insn.word(i));
			}
		});
		break;

	case OpenCLDebugInfo100DebugTypePointer:
	case OpenCLDebugInfo100DebugTypeQualifier:
	case OpenCLDebugInfo100DebugTypedef:
	case OpenCLDebugInfo100DebugTypeEnum:
	case OpenCLDebugInfo100DebugTypeInheritance:
	case OpenCLDebugInfo100DebugTypePtrToMember:
	case OpenCLDebugInfo100DebugTypeTemplateTemplateParameter:
	case OpenCLDebugInfo100DebugTypeTemplateParameterPack:
	case OpenCLDebugInfo100DebugFunctionDeclaration:
	case OpenCLDebugInfo100DebugLexicalBlockDiscriminator:
	case OpenCLDebugInfo100DebugInlinedVariable:
	case OpenCLDebugInfo100DebugMacroDef:
	case OpenCLDebugInfo100DebugMacroUndef:
	case OpenCLDebugInfo100DebugImportedEntity:
		UNIMPLEMENTED("b/148401179 OpenCLDebugInfo100 instruction %d", int(extInstIndex));
		break;
	default:
		UNSUPPORTED("OpenCLDebugInfo100 instruction %d", int(extInstIndex));
	}
}

template<typename ID>
void SpirvShader::Impl::Debugger::add(ID id, std::unique_ptr<debug::Object> &&obj)
{
	ASSERT_MSG(obj != nullptr, "add() called with nullptr obj");
	bool added = objects.emplace(debug::Object::ID(id.value()), std::move(obj)).second;
	ASSERT_MSG(added, "Debug object with %d already exists", id.value());
}

void SpirvShader::Impl::Debugger::addNone(debug::Object::ID id)
{
	bool added = objects.emplace(debug::Object::ID(id.value()), nullptr).second;
	ASSERT_MSG(added, "Debug object with %d already exists", id.value());
}

bool SpirvShader::Impl::Debugger::isNone(debug::Object::ID id) const
{
	auto it = objects.find(debug::Object::ID(id.value()));
	if(it == objects.end()) { return false; }
	return it->second.get() == nullptr;
}

template<typename T>
T *SpirvShader::Impl::Debugger::get(SpirvID<T> id) const
{
	auto it = objects.find(debug::Object::ID(id.value()));
	ASSERT_MSG(it != objects.end(), "Unknown debug object %d", id.value());
	auto ptr = debug::cast<T>(it->second.get());
	ASSERT_MSG(ptr, "Debug object %d is not of the correct type. Got: %s, want: %s",
	           id.value(), cstr(it->second->kind), cstr(T::KIND));
	return ptr;
}

template<typename T>
T *SpirvShader::Impl::Debugger::getOrNull(SpirvID<T> id) const
{
	auto it = objects.find(debug::Object::ID(id.value()));
	if(it == objects.end()) { return nullptr; }  // Not found.
	auto ptr = debug::cast<T>(it->second.get());
	ASSERT_MSG(ptr, "Debug object %d is not of the correct type. Got: %s, want: %s",
	           id.value(), cstr(it->second->kind), cstr(T::KIND));
	return ptr;
}

////////////////////////////////////////////////////////////////////////////////
// SpirvShader::Impl::Debugger::Shadow methods
////////////////////////////////////////////////////////////////////////////////
void SpirvShader::Impl::Debugger::Shadow::create(const SpirvShader *shader, const EmitState *state, Object::ID objId)
{
	ASSERT_MSG(entries.find(objId) == entries.end(),
	           "Object %%%d already has shadow memory allocated?", (int)objId.value());

	Entry entry{};
	entry.offset = size;

	rr::Pointer<Byte> base = *rr::Pointer<rr::Pointer<Byte>>(state->routine->dbgState + OFFSET(Impl::Debugger::State, shadow));
	base += entry.offset;

	auto &obj = shader->getObject(objId);
	auto &objTy = shader->getType(obj.typeId());
	auto mask = state->activeLaneMask();
	switch(obj.kind)
	{
	case Object::Kind::Constant:
	case Object::Kind::Intermediate:
		{
			size += objTy.componentCount * sizeof(uint32_t) * sw::SIMD::Width;
			auto dst = GetElementPointer(SIMD::Pointer(base, 0), 0, true);
			for(uint32_t i = 0u; i < objTy.componentCount; i++)
			{
				auto val = SpirvShader::Operand(shader, state, objId).Int(i);
				dst.Store(val, sw::OutOfBoundsBehavior::UndefinedBehavior, mask);
				dst += sizeof(uint32_t) * SIMD::Width;
			}
			entry.kind = Entry::Kind::Value;
		}
		break;
	case Object::Kind::Pointer:
	case Object::Kind::InterfaceVariable:
		{
			size += sizeof(void *) + sizeof(uint32_t) * SIMD::Width;
			auto ptr = state->getPointer(objId);
			store(base, ptr.getUniformPointer());
			store(base + sizeof(void *), ptr.offsets());
			entry.kind = Entry::Kind::Pointer;
		}
		break;
	default:
		break;
	}
	entries.emplace(objId, entry);
}

SpirvShader::Impl::Debugger::Shadow::Memory
SpirvShader::Impl::Debugger::Shadow::get(const State *state, Object::ID objId) const
{
	auto entryIt = entries.find(objId);
	ASSERT_MSG(entryIt != entries.end(), "Missing shadow entry for object %%%d (%s)",
	           (int)objId.value(),
	           OpcodeName(state->debugger->shader->getObject(objId).opcode()));
	auto &entry = entryIt->second;
	auto data = &state->shadow[entry.offset];
	return Memory{ data };
}

SpirvShader::Impl::Debugger::Shadow::Memory::operator void *()
{
	return addr;
}

SpirvShader::Impl::Debugger::Shadow::Memory
SpirvShader::Impl::Debugger::Shadow::Memory::dref(int lane) const
{
	auto ptr = *reinterpret_cast<Pointer *>(addr);
	return Memory{ ptr.base + ptr.offsets[lane] };
}

////////////////////////////////////////////////////////////////////////////////
// sw::SpirvShader::Impl::Debugger::LocalVariableValue methods
////////////////////////////////////////////////////////////////////////////////
sw::SpirvShader::Impl::Debugger::LocalVariableValue::LocalVariableValue(
    debug::LocalVariable *variable,
    const State *state,
    int lane)
    : LocalVariableValue(std::make_shared<Shared>(variable, state, lane), variable->type, &variable->values)
{}

sw::SpirvShader::Impl::Debugger::LocalVariableValue::LocalVariableValue(
    const std::shared_ptr<const Shared> &shared,
    const debug::Type *ty,
    const debug::LocalVariable::ValueNode *node)
    : shared(shared)
    , ty(ty)
    , node(node)
{
}

std::string sw::SpirvShader::Impl::Debugger::LocalVariableValue::type()
{
	updateValue();
	return value->type();
}

std::string sw::SpirvShader::Impl::Debugger::LocalVariableValue::get(const vk::dbg::FormatFlags &fmt)
{
	updateValue();
	return value->get(fmt);
}

std::shared_ptr<vk::dbg::Variables> sw::SpirvShader::Impl::Debugger::LocalVariableValue::children()
{
	updateValue();
	return value->children();
}

void sw::SpirvShader::Impl::Debugger::LocalVariableValue::updateValue()
{
	// Fetch the last reached ::debug::Value for this local variable node.
	auto newActiveValue = (node->debugValueIndex != debug::LocalVariable::ValueNode::NoDebugValueIndex)
	                          ? shared->state->lastReachedDebugValues[node->debugValueIndex]
	                          : nullptr;
	auto activeValueChanged = activeValue != newActiveValue;
	activeValue = newActiveValue;

	if(activeValue && activeValueChanged)
	{  // We have a new ::debug::Value, read it.

		ASSERT(activeValue->local == shared->variable);  // If this isn't true, then something is very wonky.

		// Update the value.
		auto ptr = shared->state->debugger->shadow.get(shared->state, activeValue->value);
		for(auto op : activeValue->expression->operations)
		{
			switch(op->opcode)
			{
			case OpenCLDebugInfo100Deref:
				ptr = ptr.dref(shared->lane);
				break;
			default:
				UNIMPLEMENTED("b/148401179 OpenCLDebugInfo100DebugOperation %d", (int)op->opcode);
				break;
			}
		}
		value = ty->value(ptr, true);
	}
	else if(!value || activeValueChanged)
	{  // We have no ::debug::Value. Display <undefined>

		if(node->children.empty())
		{  // No children? Just have the node display <undefined>
			value = ty->undefined();
		}
		else
		{  // Node has children.
			// Display <undefined> for those that don't have sub-nodes, and
			// create child LocalVariableValues for those that do.
			value = vk::dbg::Struct::create(ty->name(), [&](auto &vc) {
				auto numMembers = ty->numMembers();
				for(size_t i = 0; i < numMembers; i++)
				{
					auto member = ty->getMember(i);

					auto it = node->children.find(i);
					if(it != node->children.end())
					{
						auto child = std::make_shared<LocalVariableValue>(shared, member.type, it->second.get());
						vc->put(member.name, child);
					}
					else
					{
						vc->put(member.name, member.type->undefined());
					}
				}
			});
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// sw::SpirvShader::Impl::Debugger::State methods
////////////////////////////////////////////////////////////////////////////////
SpirvShader::Impl::Debugger::State *SpirvShader::Impl::Debugger::State::create(const Debugger *debugger)
{
	return new State(debugger);
}

void SpirvShader::Impl::Debugger::State::destroy(State *state)
{
	delete state;
}

SpirvShader::Impl::Debugger::State::State(const Debugger *debugger)
    : debugger(debugger)
    , traps(debugger->traps.memory.get())
    , shadow(new uint8_t[debugger->shadow.size])
    , lastReachedDebugValues(new debug::Value *[debugger->numDebugValueSlots])
{
	memset(shadow.get(), 0, debugger->shadow.size);
	memset(lastReachedDebugValues.get(), 0, sizeof(lastReachedDebugValues[0]) * debugger->numDebugValueSlots);
}

SpirvShader::Impl::Debugger::State::~State()
{
	if(data) { data->terminate(this); }
}

void SpirvShader::Impl::Debugger::State::trap(int index)
{
	if(std::all_of(globals.activeLaneMask.data.begin(),
	               globals.activeLaneMask.data.end(),
	               [](auto v) { return v == 0; }))
	{
		// Don't trap if no lanes are active.
		// Ideally, we would be simply jumping over blocks that have no active
		// lanes, but this is complicated due to ensuring that all reactor
		// RValues dominate their usage blocks.
		return;
	}

	if(!data)
	{
		data = std::make_unique<Data>(this);
	}
	data->trap(index, this);
}

SpirvShader::Impl::Debugger::State::Data::Data(State *state)
{
	buildGlobals(state);

	thread = state->debugger->ctx->lock().currentThread();

	if(!state->debugger->shaderHasDebugInfo)
	{
		// Enter the stack frame entry for the SPIR-V.
		thread->enter(state->debugger->spirvFile, "SPIR-V", [&](vk::dbg::Frame &frame) {
			for(size_t lane = 0; lane < sw::SIMD::Width; lane++)
			{
				auto laneLocals = std::make_shared<vk::dbg::Struct>("Lane", globals.lanes[lane]);
				frame.locals->variables->put(laneName(lane), laneLocals);
				frame.hovers->variables->extend(std::make_shared<HoversFromLocals>(frame.locals->variables));
			}
		});
	}
}

void SpirvShader::Impl::Debugger::State::Data::terminate(State *state)
{
	if(state->debugger->shaderHasDebugInfo)
	{
		for(size_t i = 0; i < stack.size(); i++)
		{
			thread->exit();
		}
	}
	else
	{
		thread->exit();
	}
}

void SpirvShader::Impl::Debugger::State::Data::trap(int index, State *state)
{
	auto debugger = state->debugger;

	// Update the thread frames from the stack of scopes
	const auto &locationAndScope = debugger->traps.byIndex[index];

	if(locationAndScope.scope)
	{
		// Gather the new stack as LexicalBlocks.
		std::vector<StackEntry> newStack;
		if(auto block = debug::find<debug::LexicalBlock>(locationAndScope.scope->scope))
		{
			newStack.emplace_back(StackEntry{ block, block->line });
		}
		for(auto inlined = locationAndScope.scope->inlinedAt; inlined != nullptr; inlined = inlined->inlined)
		{
			if(auto block = debug::find<debug::LexicalBlock>(inlined->scope))
			{
				newStack.emplace_back(StackEntry{ block, inlined->line });
			}
		}
		std::reverse(newStack.begin(), newStack.end());

		// shrink pop stack frames until stack length is at most maxLen.
		auto shrink = [&](size_t maxLen) {
			while(stack.size() > maxLen)
			{
				thread->exit(true);
				stack.pop_back();
			}
		};

		// Pop stack frames until stack length is at most newStack length.
		shrink(newStack.size());

		// Find first deviation in stack frames, and shrink to that point.
		// Special care is taken for deviation in just the top most frame so we
		// don't end up reconstructing the top most stack frame every scope
		// change.
		for(size_t i = 0; i < stack.size(); i++)
		{
			if(stack[i] != newStack[i])
			{
				bool wasTopMostFrame = i == (stack.size() - 1);
				auto oldFunction = debug::find<debug::Function>(stack[i].block);
				auto newFunction = debug::find<debug::Function>(newStack[i].block);
				if(wasTopMostFrame && oldFunction == newFunction)
				{
					// Deviation is just a movement in the top most frame's
					// function.
					// Don't exit() and enter() for the same function - it'll
					// be treated as a step out and step in, breaking stepping
					// commands. Instead, just update the frame variables for
					// the new scope.
					stack[i] = newStack[i];
					thread->update(true, [&](vk::dbg::Frame &frame) {
						// Update the frame location if we're entering a
						// function. This allows the debugger to pause at the
						// line (which may not have any instructions or OpLines)
						// of a inlined function call. This is less jarring
						// than magically appearing in another function before
						// you've reached the line of the call site.
						// See b/170650010 for more context.
						if(stack.size() < newStack.size())
						{
							auto function = debug::find<debug::Function>(stack[i].block);
							frame.location = vk::dbg::Location{ function->source->dbgFile, (int)stack[i].line };
						}
						updateFrameLocals(state, frame, stack[i].block);
					});
				}
				else
				{
					shrink(i);
				}
				break;
			}
		}

		// Now rebuild the parts of stack frames that are new.
		//
		// This is done in two stages:
		// (1) thread->enter() is called to construct the new stack frame with
		//     the opening scope line. The frames locals and hovers are built
		//     and assigned.
		// (2) thread->update() is called to adjust the frame's location to
		//     entry.line. This may be different to the function entry in the
		//     case of multiple nested inline functions. If its the same, then
		//     this is a no-op.
		//
		// This two-stage approach allows the debugger to step through chains of
		// inlined function calls without having a jarring jump from the outer
		// function to the first statement within the function.
		// See b/170650010 for more context.
		for(size_t i = stack.size(); i < newStack.size(); i++)
		{
			auto entry = newStack[i];
			stack.emplace_back(entry);
			auto function = debug::find<debug::Function>(entry.block);
			thread->enter(entry.block->source->dbgFile, function->name, [&](vk::dbg::Frame &frame) {
				frame.location = vk::dbg::Location{ function->source->dbgFile, (int)function->line };
				frame.hovers->variables->extend(std::make_shared<HoversFromLocals>(frame.locals->variables));
				updateFrameLocals(state, frame, entry.block);
			});
			thread->update(true, [&](vk::dbg::Frame &frame) {
				frame.location.line = (int)entry.line;
			});
		}
	}

	// If the debugger thread is running, notify that we're pausing due to the
	// trap.
	if(thread->state() == vk::dbg::Thread::State::Running)
	{
		// pause() changes the thread state Paused, and will cause the next
		// frame location changing call update() to block until the debugger
		// instructs the thread to resume or step.
		thread->pause();
		debugger->ctx->serverEventBroadcast()->onLineBreakpointHit(thread->id);
	}

	// Update the frame location. This will likely block until the debugger
	// instructs the thread to resume or step.
	thread->update(true, [&](vk::dbg::Frame &frame) {
		frame.location = locationAndScope.location;
	});

	// Clear the alwaysTrap state if the debugger instructed the thread to
	// resume, or set it if we're single line stepping (so we can keep track of
	// location).
	state->alwaysTrap = thread->state() != vk::dbg::Thread::State::Running;
}

void SpirvShader::Impl::Debugger::State::Data::updateFrameLocals(State *state, vk::dbg::Frame &frame, debug::LexicalBlock *block)
{
	auto locals = getOrCreateLocals(state, block);
	for(size_t lane = 0; lane < sw::SIMD::Width; lane++)
	{
		auto laneLocals = std::make_shared<vk::dbg::Struct>("Lane", locals[lane]);
		frame.locals->variables->put(laneName(lane), laneLocals);
	}
}

SpirvShader::Impl::Debugger::State::Data::PerLaneVariables
SpirvShader::Impl::Debugger::State::Data::getOrCreateLocals(State *state, const debug::LexicalBlock *block)
{
	return getOrCreate(locals, block, [&] {
		PerLaneVariables locals;
		for(int lane = 0; lane < sw::SIMD::Width; lane++)
		{
			auto vc = std::make_shared<vk::dbg::VariableContainer>();

			for(auto var : block->variables)
			{
				auto name = var->name;

				switch(var->definition)
				{
				case debug::LocalVariable::Definition::Undefined:
					{
						vc->put(name, var->type->undefined());
					}
					break;
				case debug::LocalVariable::Definition::Declaration:
					{
						auto data = state->debugger->shadow.get(state, var->declaration->variable);
						vc->put(name, var->type->value(data.dref(lane), true));
					}
					break;
				case debug::LocalVariable::Definition::Values:
					{
						vc->put(name, std::make_shared<LocalVariableValue>(var, state, lane));
						break;
					}
				}
			}

			locals[lane] = std::move(vc);
		}
		if(auto parent = debug::find<debug::LexicalBlock>(block->parent))
		{
			auto extend = getOrCreateLocals(state, parent);
			for(int lane = 0; lane < sw::SIMD::Width; lane++)
			{
				locals[lane]->extend(extend[lane]);
			}
		}
		else
		{
			for(int lane = 0; lane < sw::SIMD::Width; lane++)
			{
				locals[lane]->extend(globals.lanes[lane]);
			}
		}
		return locals;
	});
}

template<typename T>
void SpirvShader::Impl::Debugger::State::Data::buildGlobal(const char *name, const T &val)
{
	globals.common->put(name, makeDbgValue(val));
}

template<typename T, int N>
void SpirvShader::Impl::Debugger::State::Data::buildGlobal(const char *name, const sw::SIMD::PerLane<T, N> &simd)
{
	for(int lane = 0; lane < sw::SIMD::Width; lane++)
	{
		globals.lanes[lane]->put(name, makeDbgValue(simd[lane]));
	}
}

void SpirvShader::Impl::Debugger::State::Data::buildGlobals(State *state)
{
	globals.common = std::make_shared<vk::dbg::VariableContainer>();
	globals.common->put("subgroupSize", vk::dbg::make_reference(state->globals.subgroupSize));

	for(int lane = 0; lane < sw::SIMD::Width; lane++)
	{
		auto vc = std::make_shared<vk::dbg::VariableContainer>();

		vc->put("enabled", vk::dbg::make_reference(reinterpret_cast<const bool &>(state->globals.activeLaneMask[lane])));

		for(auto &it : state->debugger->objects)
		{
			if(auto var = debug::cast<debug::GlobalVariable>(it.second.get()))
			{
				if(var->variable != 0)
				{
					auto data = state->debugger->shadow.get(state, var->variable);
					vc->put(var->name, var->type->value(data.dref(lane), true));
				}
			}
		}

		auto spirv = buildSpirvVariables(state, lane);
		if(state->debugger->shaderHasDebugInfo)
		{
			vc->put("SPIR-V", spirv);
		}
		else
		{
			vc->extend(spirv->children());
		}

		vc->extend(globals.common);
		globals.lanes[lane] = vc;
	}

	switch(state->debugger->shader->executionModel)
	{
	case spv::ExecutionModelGLCompute:
		{
			buildGlobal("numWorkgroups", state->globals.compute.numWorkgroups);
			buildGlobal("workgroupID", state->globals.compute.workgroupID);
			buildGlobal("workgroupSize", state->globals.compute.workgroupSize);
			buildGlobal("numSubgroups", state->globals.compute.numSubgroups);
			buildGlobal("subgroupIndex", state->globals.compute.subgroupIndex);
			buildGlobal("globalInvocationId", state->globals.compute.globalInvocationId);
			buildGlobal("localInvocationIndex", state->globals.compute.localInvocationIndex);
		}
		break;
	case spv::ExecutionModelFragment:
		{
			buildGlobal("viewIndex", state->globals.fragment.viewIndex);
			buildGlobal("fragCoord", state->globals.fragment.fragCoord);
			buildGlobal("pointCoord", state->globals.fragment.pointCoord);
			buildGlobal("windowSpacePosition", state->globals.fragment.windowSpacePosition);
			buildGlobal("helperInvocation", state->globals.fragment.helperInvocation);
		}
		break;
	case spv::ExecutionModelVertex:
		{
			buildGlobal("viewIndex", state->globals.vertex.viewIndex);
			buildGlobal("instanceIndex", state->globals.vertex.instanceIndex);
			buildGlobal("vertexIndex", state->globals.vertex.vertexIndex);
		}
		break;
	default:
		break;
	}
}

std::shared_ptr<vk::dbg::Struct>
SpirvShader::Impl::Debugger::State::Data::buildSpirvVariables(State *state, int lane) const
{
	return vk::dbg::Struct::create("SPIR-V", [&](auto &vc) {
		auto debugger = state->debugger;
		auto &entries = debugger->shadow.entries;
		std::vector<Object::ID> ids;
		ids.reserve(entries.size());
		for(auto it : entries)
		{
			ids.emplace_back(it.first);
		}
		std::sort(ids.begin(), ids.end());
		for(auto id : ids)
		{
			auto &obj = debugger->shader->getObject(id);
			auto &objTy = debugger->shader->getType(obj.typeId());
			auto name = "%" + std::to_string(id.value());
			auto memory = debugger->shadow.get(state, id);
			switch(obj.kind)
			{
			case Object::Kind::Intermediate:
			case Object::Kind::Constant:
				if(auto val = buildSpirvValue(state, memory, objTy, lane))
				{
					vc->put(name, val);
				}
				break;
			default:
				break;  // Not handled yet.
			}
		}
	});
}

std::shared_ptr<vk::dbg::Value>
SpirvShader::Impl::Debugger::State::Data::buildSpirvValue(State *state, Shadow::Memory memory, const SpirvShader::Type &type, int lane) const
{
	auto debugger = state->debugger;
	auto shader = debugger->shader;

	switch(type.definition.opcode())
	{
	case spv::OpTypeInt:
		return vk::dbg::make_reference(reinterpret_cast<uint32_t *>(memory.addr)[lane]);
	case spv::OpTypeFloat:
		return vk::dbg::make_reference(reinterpret_cast<float *>(memory.addr)[lane]);
	case spv::OpTypeVector:
		{
			auto elTy = shader->getType(type.element);
			return vk::dbg::Struct::create("vector", [&](auto &fields) {
				for(uint32_t i = 0; i < type.componentCount; i++)
				{
					if(auto val = buildSpirvValue(state, memory, elTy, lane))
					{
						fields->put(vecElementName(i, type.componentCount), val);
						memory.addr += sizeof(uint32_t) * sw::SIMD::Width;
					}
				}
			});
		}
	default:
		return nullptr;  // Not handled yet
	}
}

////////////////////////////////////////////////////////////////////////////////
// sw::SpirvShader methods
////////////////////////////////////////////////////////////////////////////////
void SpirvShader::dbgInit(const std::shared_ptr<vk::dbg::Context> &ctx)
{
	impl.debugger = new Impl::Debugger(this, ctx);
}

void SpirvShader::dbgTerm()
{
	if(impl.debugger)
	{
		delete impl.debugger;
	}
}

void SpirvShader::dbgCreateFile()
{
	auto dbg = impl.debugger;
	if(!dbg) { return; }

	int currentLine = 1;
	std::string source;
	for(auto insn : *this)
	{
		auto instruction = spvtools::spvInstructionBinaryToText(
		                       vk::SPIRV_VERSION,
		                       insn.data(),
		                       insn.wordCount(),
		                       insns.data(),
		                       insns.size(),
		                       SPV_BINARY_TO_TEXT_OPTION_NO_HEADER) +
		                   "\n";
		dbg->spirvLineMappings[insn.data()] = currentLine;
		currentLine += std::count(instruction.begin(), instruction.end(), '\n');
		source += instruction;
	}
	std::string name;
	switch(executionModel)
	{
	case spv::ExecutionModelVertex: name = "VertexShader"; break;
	case spv::ExecutionModelFragment: name = "FragmentShader"; break;
	case spv::ExecutionModelGLCompute: name = "ComputeShader"; break;
	default: name = "SPIR-V Shader"; break;
	}
	static std::atomic<int> id = { 0 };
	name += std::to_string(id++) + ".spvasm";
	dbg->spirvFile = dbg->ctx->lock().createVirtualFile(name.c_str(), source.c_str());
}

void SpirvShader::dbgBeginEmit(EmitState *state) const
{
	auto dbg = impl.debugger;
	if(!dbg) { return; }

	dbg->shaderHasDebugInfo = extensionsImported.count(Extension::OpenCLDebugInfo100) > 0;

	auto routine = state->routine;

	auto dbgState = rr::Call(&Impl::Debugger::State::create, dbg);

	routine->dbgState = dbgState;

	SetActiveLaneMask(state->activeLaneMask(), state);

	for(int i = 0; i < SIMD::Width; i++)
	{
		using Globals = Impl::Debugger::State::Globals;

		auto globals = dbgState + OFFSET(Impl::Debugger::State, globals);
		store(globals + OFFSET(Globals, subgroupSize), routine->invocationsPerSubgroup);

		switch(executionModel)
		{
		case spv::ExecutionModelGLCompute:
			{
				auto compute = globals + OFFSET(Globals, compute);
				store(compute + OFFSET(Globals::Compute, numWorkgroups), routine->numWorkgroups);
				store(compute + OFFSET(Globals::Compute, workgroupID), routine->workgroupID);
				store(compute + OFFSET(Globals::Compute, workgroupSize), routine->workgroupSize);
				store(compute + OFFSET(Globals::Compute, numSubgroups), routine->subgroupsPerWorkgroup);
				store(compute + OFFSET(Globals::Compute, subgroupIndex), routine->subgroupIndex);
				store(compute + OFFSET(Globals::Compute, globalInvocationId), routine->globalInvocationID);
				store(compute + OFFSET(Globals::Compute, localInvocationIndex), routine->localInvocationIndex);
			}
			break;
		case spv::ExecutionModelFragment:
			{
				auto fragment = globals + OFFSET(Globals, fragment);
				store(fragment + OFFSET(Globals::Fragment, viewIndex), routine->layer);
				store(fragment + OFFSET(Globals::Fragment, fragCoord), routine->fragCoord);
				store(fragment + OFFSET(Globals::Fragment, pointCoord), routine->pointCoord);
				store(fragment + OFFSET(Globals::Fragment, windowSpacePosition), routine->windowSpacePosition);
				store(fragment + OFFSET(Globals::Fragment, helperInvocation), routine->helperInvocation);
			}
			break;
		case spv::ExecutionModelVertex:
			{
				auto vertex = globals + OFFSET(Globals, vertex);
				store(vertex + OFFSET(Globals::Vertex, viewIndex), routine->layer);
				store(vertex + OFFSET(Globals::Vertex, instanceIndex), routine->instanceID);
				store(vertex + OFFSET(Globals::Vertex, vertexIndex), routine->vertexIndex);
			}
			break;
		default:
			break;
		}
	}
}

void SpirvShader::dbgEndEmit(EmitState *state) const
{
	auto dbg = impl.debugger;
	if(!dbg) { return; }

	dbg->finalize();

	rr::Call(&Impl::Debugger::State::destroy, state->routine->dbgState);
}

void SpirvShader::dbgBeginEmitInstruction(InsnIterator insn, EmitState *state) const
{
#	if PRINT_EACH_EMITTED_INSTRUCTION
	{
		auto instruction = spvtools::spvInstructionBinaryToText(
		    vk::SPIRV_VERSION,
		    insn.data(),
		    insn.wordCount(),
		    insns.data(),
		    insns.size(),
		    SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
		printf("%s\n", instruction.c_str());
	}
#	endif  // PRINT_EACH_EMITTED_INSTRUCTION

#	if PRINT_EACH_EXECUTED_INSTRUCTION
	{
		auto instruction = spvtools::spvInstructionBinaryToText(
		    vk::SPIRV_VERSION,
		    insn.data(),
		    insn.wordCount(),
		    insns.data(),
		    insns.size(),
		    SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
		rr::Print("{0}\n", instruction);
	}
#	endif  // PRINT_EACH_EXECUTED_INSTRUCTION

	// Only single line step over statement instructions.

	if(auto dbg = impl.debugger)
	{
		if(insn.opcode() == spv::OpLabel)
		{
			// Whenever we hit a label, force the next OpLine to be steppable.
			// This handles the case where we have control flow on the same line
			// For example:
			//   while(true) { foo(); }
			// foo() should be repeatedly steppable.
			dbg->setNextSetLocationIsSteppable();
		}

		if(!dbg->shaderHasDebugInfo)
		{
			// We're emitting debugger logic for SPIR-V.
			if(IsStatement(insn.opcode()))
			{
				auto line = dbg->spirvLineMappings.at(insn.data());
				dbg->setLocation(state, dbg->spirvFile, line);
			}
		}
	}
}

void SpirvShader::dbgEndEmitInstruction(InsnIterator insn, EmitState *state) const
{
	auto dbg = impl.debugger;
	if(!dbg) { return; }

	switch(insn.opcode())
	{
	case spv::OpVariable:
	case spv::OpConstant:  // TODO: Move constants out of shadow memory.
	case spv::OpConstantNull:
	case spv::OpConstantTrue:
	case spv::OpConstantFalse:
	case spv::OpConstantComposite:
		dbg->shadow.create(this, state, insn.resultId());
		break;
	default:
		{
			auto resIt = dbg->results.find(insn.data());
			if(resIt != dbg->results.end())
			{
				dbg->shadow.create(this, state, resIt->second);
			}
		}
	}
}

void SpirvShader::dbgUpdateActiveLaneMask(RValue<SIMD::Int> mask, EmitState *state) const
{
	auto dbg = impl.debugger;
	if(!dbg) { return; }

	auto dbgState = state->routine->dbgState;
	auto globals = dbgState + OFFSET(Impl::Debugger::State, globals);
	store(globals + OFFSET(Impl::Debugger::State::Globals, activeLaneMask), mask);
}

void SpirvShader::dbgDeclareResult(const InsnIterator &insn, Object::ID resultId) const
{
	auto dbg = impl.debugger;
	if(!dbg) { return; }

	dbg->results.emplace(insn.data(), resultId);
}

SpirvShader::EmitResult SpirvShader::EmitLine(InsnIterator insn, EmitState *state) const
{
	if(auto dbg = impl.debugger)
	{
		auto path = getString(insn.word(1));
		auto line = insn.word(2);
		dbg->setLocation(state, path.c_str(), line);
	}
	return EmitResult::Continue;
}

void SpirvShader::DefineOpenCLDebugInfo100(const InsnIterator &insn)
{
#	if PRINT_EACH_DEFINED_DBG_INSTRUCTION
	{
		auto instruction = spvtools::spvInstructionBinaryToText(
		    vk::SPIRV_VERSION,
		    insn.data(),
		    insn.wordCount(),
		    insns.data(),
		    insns.size(),
		    SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
		printf("%s\n", instruction.c_str());
	}
#	endif  // PRINT_EACH_DEFINED_DBG_INSTRUCTION

	auto dbg = impl.debugger;
	if(!dbg) { return; }

	dbg->process(insn, nullptr, Impl::Debugger::Pass::Define);
}

SpirvShader::EmitResult SpirvShader::EmitOpenCLDebugInfo100(InsnIterator insn, EmitState *state) const
{
	if(auto dbg = impl.debugger)
	{
		dbg->process(insn, state, Impl::Debugger::Pass::Emit);
	}
	return EmitResult::Continue;
}

}  // namespace sw

#endif  // ENABLE_VK_DEBUGGER
