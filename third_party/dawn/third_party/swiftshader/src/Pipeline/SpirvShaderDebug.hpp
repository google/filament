// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
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

#ifndef sw_SpirvShaderDebug_hpp
#define sw_SpirvShaderDebug_hpp

#include "SpirvShader.hpp"

// Enable this to print verbose debug messages as each SPIR-V instruction is
// executed. Very handy for performing text diffs when the thread count is
// reduced to 1 and execution is deterministic.
#define SPIRV_SHADER_ENABLE_DBG 0

// Enable this to write a GraphViz dot file containing a graph of the shader's
// control flow to the given file path. Helpful for diagnosing control-flow
// related issues.
#if 0
#	define SPIRV_SHADER_CFG_GRAPHVIZ_DOT_FILEPATH "swiftshader_%d.dot"
#endif

#if SPIRV_SHADER_ENABLE_DBG
#	define SPIRV_SHADER_DBG(fmt, ...) rr::Print(fmt "\n", ##__VA_ARGS__)
#	include "spirv-tools/libspirv.h"
namespace spvtools {
// Function implemented in third_party/SPIRV-Tools/source/disassemble.cpp
// but with no public header.
std::string spvInstructionBinaryToText(const spv_target_env env,
                                       const uint32_t *inst_binary,
                                       const size_t inst_word_count,
                                       const uint32_t *binary,
                                       const size_t word_count,
                                       const uint32_t options);

}  // namespace spvtools
#else
#	define SPIRV_SHADER_DBG(...)
#endif  // SPIRV_SHADER_ENABLE_DBG

#ifdef ENABLE_RR_PRINT
namespace rr {
template<>
struct PrintValue::Ty<sw::Spirv::Object::ID>
{
	static inline std::string fmt(sw::Spirv::Object::ID v) { return "Object<" + std::to_string(v.value()) + ">"; }
	static inline std::vector<Value *> val(sw::Spirv::Object::ID v) { return {}; }
};
template<>
struct PrintValue::Ty<sw::Spirv::Type::ID>
{
	static inline std::string fmt(sw::Spirv::Type::ID v) { return "Type<" + std::to_string(v.value()) + ">"; }
	static inline std::vector<Value *> val(sw::Spirv::Type::ID v) { return {}; }
};
template<>
struct PrintValue::Ty<sw::Spirv::Block::ID>
{
	static inline std::string fmt(sw::Spirv::Block::ID v) { return "Block<" + std::to_string(v.value()) + ">"; }
	static inline std::vector<Value *> val(sw::Spirv::Block::ID v) { return {}; }
};

template<>
struct PrintValue::Ty<sw::Intermediate>
{
	static inline std::string fmt(const sw::Intermediate &v, uint32_t i)
	{
		switch(v.typeHint)
		{
		case sw::Intermediate::TypeHint::Float:
			return PrintValue::Ty<sw::SIMD::Float>::fmt(v.Float(i));
		case sw::Intermediate::TypeHint::Int:
			return PrintValue::Ty<sw::SIMD::Int>::fmt(v.Int(i));
		case sw::Intermediate::TypeHint::UInt:
			return PrintValue::Ty<sw::SIMD::UInt>::fmt(v.UInt(i));
		}
		return "";
	}

	static inline std::vector<Value *> val(const sw::Intermediate &v, uint32_t i)
	{
		switch(v.typeHint)
		{
		case sw::Intermediate::TypeHint::Float:
			return PrintValue::Ty<sw::SIMD::Float>::val(v.Float(i));
		case sw::Intermediate::TypeHint::Int:
			return PrintValue::Ty<sw::SIMD::Int>::val(v.Int(i));
		case sw::Intermediate::TypeHint::UInt:
			return PrintValue::Ty<sw::SIMD::UInt>::val(v.UInt(i));
		}
		return {};
	}

	static inline std::string fmt(const sw::Intermediate &v)
	{
		if(v.componentCount == 1)
		{
			return fmt(v, 0);
		}

		std::string out = "[";
		for(uint32_t i = 0; i < v.componentCount; i++)
		{
			if(i > 0) { out += ", "; }
			out += std::to_string(i) + ": ";
			out += fmt(v, i);
		}
		return out + "]";
	}

	static inline std::vector<Value *> val(const sw::Intermediate &v)
	{
		std::vector<Value *> out;
		for(uint32_t i = 0; i < v.componentCount; i++)
		{
			auto vals = val(v, i);
			out.insert(out.end(), vals.begin(), vals.end());
		}
		return out;
	}
};

template<>
struct PrintValue::Ty<sw::SpirvEmitter::Operand>
{
	static inline std::string fmt(const sw::SpirvEmitter::Operand &v)
	{
		return (v.intermediate != nullptr) ? PrintValue::Ty<sw::Intermediate>::fmt(*v.intermediate) : PrintValue::Ty<sw::SIMD::UInt>::fmt(v.UInt(0));
	}

	static inline std::vector<Value *> val(const sw::SpirvEmitter::Operand &v)
	{
		return (v.intermediate != nullptr) ? PrintValue::Ty<sw::Intermediate>::val(*v.intermediate) : PrintValue::Ty<sw::SIMD::UInt>::val(v.UInt(0));
	}
};
}  // namespace rr
#endif  // ENABLE_RR_PRINT

#endif  // sw_SpirvShaderDebug_hpp
