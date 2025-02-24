// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef rr_LLVMReactor_hpp
#define rr_LLVMReactor_hpp

#include "Nucleus.hpp"

#include "Debug.hpp"
#include "LLVMReactorDebugInfo.hpp"
#include "Print.hpp"

#ifdef _MSC_VER
__pragma(warning(push))
    __pragma(warning(disable : 4146))  // unary minus operator applied to unsigned type, result still unsigned
#endif

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"

#ifdef _MSC_VER
    __pragma(warning(pop))
#endif

#include <memory>

        namespace llvm
{

	class Type;
	class Value;

}  // namespace llvm

namespace rr {

class Type;
class Value;

llvm::Type *T(Type *t);

inline Type *T(llvm::Type *t)
{
	return reinterpret_cast<Type *>(t);
}

inline llvm::Value *V(Value *t)
{
	return reinterpret_cast<llvm::Value *>(t);
}

inline Value *V(llvm::Value *t)
{
	return reinterpret_cast<Value *>(t);
}

inline std::vector<llvm::Value *> V(const std::vector<Value *> &values)
{
	std::vector<llvm::Value *> result;
	result.reserve(values.size());
	for(auto &v : values)
	{
		result.push_back(V(v));
	}
	return result;
}

// Emits a no-op instruction that will not be optimized away.
// Useful for emitting something that can have a source location without
// effect.
void Nop();

class Routine;

// JITBuilder holds all the LLVM state for building routines.
class JITBuilder
{
public:
	JITBuilder();

	void runPasses();

	std::shared_ptr<rr::Routine> acquireRoutine(const char *name, llvm::Function **funcs, size_t count);

	std::unique_ptr<llvm::LLVMContext> context;
	std::unique_ptr<llvm::Module> module;
	std::unique_ptr<llvm::IRBuilder<>> builder;
	llvm::Function *function = nullptr;

	struct CoroutineState
	{
		llvm::Function *await = nullptr;
		llvm::Function *destroy = nullptr;
		llvm::Value *handle = nullptr;
		llvm::Value *id = nullptr;
		llvm::Value *promise = nullptr;
		llvm::Type *yieldType = nullptr;
		llvm::BasicBlock *entryBlock = nullptr;
		llvm::BasicBlock *suspendBlock = nullptr;
		llvm::BasicBlock *endBlock = nullptr;
		llvm::BasicBlock *destroyBlock = nullptr;
	};
	CoroutineState coroutine;

#ifdef ENABLE_RR_DEBUG_INFO
	std::unique_ptr<rr::DebugInfo> debugInfo;
#endif

	bool msanInstrumentation = false;
};

inline std::memory_order atomicOrdering(llvm::AtomicOrdering memoryOrder)
{
	switch(memoryOrder)
	{
	case llvm::AtomicOrdering::Monotonic: return std::memory_order_relaxed;  // https://llvm.org/docs/Atomics.html#monotonic
	case llvm::AtomicOrdering::Acquire: return std::memory_order_acquire;
	case llvm::AtomicOrdering::Release: return std::memory_order_release;
	case llvm::AtomicOrdering::AcquireRelease: return std::memory_order_acq_rel;
	case llvm::AtomicOrdering::SequentiallyConsistent: return std::memory_order_seq_cst;
	default:
		UNREACHABLE("memoryOrder: %d", int(memoryOrder));
		return std::memory_order_acq_rel;
	}
}

inline llvm::AtomicOrdering atomicOrdering(bool atomic, std::memory_order memoryOrder)
{
	if(!atomic)
	{
		return llvm::AtomicOrdering::NotAtomic;
	}

	switch(memoryOrder)
	{
	case std::memory_order_relaxed: return llvm::AtomicOrdering::Monotonic;  // https://llvm.org/docs/Atomics.html#monotonic
	case std::memory_order_consume: return llvm::AtomicOrdering::Acquire;    // https://llvm.org/docs/Atomics.html#acquire: "It should also be used for C++11/C11 memory_order_consume."
	case std::memory_order_acquire: return llvm::AtomicOrdering::Acquire;
	case std::memory_order_release: return llvm::AtomicOrdering::Release;
	case std::memory_order_acq_rel: return llvm::AtomicOrdering::AcquireRelease;
	case std::memory_order_seq_cst: return llvm::AtomicOrdering::SequentiallyConsistent;
	default:
		UNREACHABLE("memoryOrder: %d", int(memoryOrder));
		return llvm::AtomicOrdering::AcquireRelease;
	}
}

}  // namespace rr

#endif  // rr_LLVMReactor_hpp
