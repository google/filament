// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
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

#include "LLVMReactor.hpp"

#include "CPUID.hpp"
#include "Debug.hpp"
#include "LLVMReactorDebugInfo.hpp"
#include "PragmaInternals.hpp"
#include "Print.hpp"
#include "Reactor.hpp"
#include "SIMD.hpp"
#include "x86.hpp"

#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicsX86.h"
#include "llvm/Support/Alignment.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/ManagedStatic.h"

#include <fstream>
#include <iostream>
#include <mutex>
#include <numeric>
#include <thread>
#include <unordered_map>

#if defined(__i386__) || defined(__x86_64__)
#	include <xmmintrin.h>
#endif

#include <math.h>

#if defined(__x86_64__) && defined(_WIN32)
extern "C" void X86CompilationCallback()
{
	UNIMPLEMENTED_NO_BUG("X86CompilationCallback");
}
#endif

#if !LLVM_ENABLE_THREADS
#	error "LLVM_ENABLE_THREADS needs to be enabled"
#endif

#if LLVM_VERSION_MAJOR < 11
namespace llvm {
using FixedVectorType = VectorType;
}  // namespace llvm
#endif

namespace {

// Used to automatically invoke llvm_shutdown() when driver is unloaded
llvm::llvm_shutdown_obj llvmShutdownObj;

// This has to be a raw pointer because glibc 2.17 doesn't support __cxa_thread_atexit_impl
// for destructing objects at exit. See crbug.com/1074222
thread_local rr::JITBuilder *jit = nullptr;

auto getNumElements(llvm::FixedVectorType *vec)
{
#if LLVM_VERSION_MAJOR >= 11
	return vec->getElementCount();
#else
	return vec->getNumElements();
#endif
}

llvm::Value *lowerPAVG(llvm::Value *x, llvm::Value *y)
{
	llvm::VectorType *ty = llvm::cast<llvm::VectorType>(x->getType());

	llvm::VectorType *extTy =
	    llvm::VectorType::getExtendedElementVectorType(ty);
	x = jit->builder->CreateZExt(x, extTy);
	y = jit->builder->CreateZExt(y, extTy);

	// (x + y + 1) >> 1
	llvm::Constant *one = llvm::ConstantInt::get(extTy, 1);
	llvm::Value *res = jit->builder->CreateAdd(x, y);
	res = jit->builder->CreateAdd(res, one);
	res = jit->builder->CreateLShr(res, one);
	return jit->builder->CreateTrunc(res, ty);
}

llvm::Value *lowerPMINMAX(llvm::Value *x, llvm::Value *y,
                          llvm::ICmpInst::Predicate pred)
{
	return jit->builder->CreateSelect(jit->builder->CreateICmp(pred, x, y), x, y);
}

llvm::Value *lowerPCMP(llvm::ICmpInst::Predicate pred, llvm::Value *x,
                       llvm::Value *y, llvm::Type *dstTy)
{
	return jit->builder->CreateSExt(jit->builder->CreateICmp(pred, x, y), dstTy, "");
}

[[maybe_unused]] llvm::Value *lowerPFMINMAX(llvm::Value *x, llvm::Value *y,
                                            llvm::FCmpInst::Predicate pred)
{
	return jit->builder->CreateSelect(jit->builder->CreateFCmp(pred, x, y), x, y);
}

[[maybe_unused]] llvm::Value *lowerRound(llvm::Value *x)
{
	llvm::Function *nearbyint = llvm::Intrinsic::getDeclaration(
	    jit->module.get(), llvm::Intrinsic::nearbyint, { x->getType() });
	return jit->builder->CreateCall(nearbyint, { x });
}

[[maybe_unused]] llvm::Value *lowerRoundInt(llvm::Value *x, llvm::Type *ty)
{
	return jit->builder->CreateFPToSI(lowerRound(x), ty);
}

[[maybe_unused]] llvm::Value *lowerFloor(llvm::Value *x)
{
	llvm::Function *floor = llvm::Intrinsic::getDeclaration(
	    jit->module.get(), llvm::Intrinsic::floor, { x->getType() });
	return jit->builder->CreateCall(floor, { x });
}

[[maybe_unused]] llvm::Value *lowerTrunc(llvm::Value *x)
{
	llvm::Function *trunc = llvm::Intrinsic::getDeclaration(
	    jit->module.get(), llvm::Intrinsic::trunc, { x->getType() });
	return jit->builder->CreateCall(trunc, { x });
}

[[maybe_unused]] llvm::Value *lowerSQRT(llvm::Value *x)
{
	llvm::Function *sqrt = llvm::Intrinsic::getDeclaration(
	    jit->module.get(), llvm::Intrinsic::sqrt, { x->getType() });
	return jit->builder->CreateCall(sqrt, { x });
}

[[maybe_unused]] llvm::Value *lowerRCP(llvm::Value *x)
{
	llvm::Type *ty = x->getType();
	llvm::Constant *one;
	if(llvm::FixedVectorType *vectorTy = llvm::dyn_cast<llvm::FixedVectorType>(ty))
	{
		one = llvm::ConstantVector::getSplat(getNumElements(vectorTy),
		                                     llvm::ConstantFP::get(vectorTy->getElementType(), 1));
	}
	else
	{
		one = llvm::ConstantFP::get(ty, 1);
	}
	return jit->builder->CreateFDiv(one, x);
}

[[maybe_unused]] llvm::Value *lowerRSQRT(llvm::Value *x)
{
	return lowerRCP(lowerSQRT(x));
}

[[maybe_unused]] llvm::Value *lowerVectorShl(llvm::Value *x, uint64_t scalarY)
{
	llvm::FixedVectorType *ty = llvm::cast<llvm::FixedVectorType>(x->getType());
	llvm::Value *y = llvm::ConstantVector::getSplat(getNumElements(ty),
	                                                llvm::ConstantInt::get(ty->getElementType(), scalarY));
	return jit->builder->CreateShl(x, y);
}

[[maybe_unused]] llvm::Value *lowerVectorAShr(llvm::Value *x, uint64_t scalarY)
{
	llvm::FixedVectorType *ty = llvm::cast<llvm::FixedVectorType>(x->getType());
	llvm::Value *y = llvm::ConstantVector::getSplat(getNumElements(ty),
	                                                llvm::ConstantInt::get(ty->getElementType(), scalarY));
	return jit->builder->CreateAShr(x, y);
}

[[maybe_unused]] llvm::Value *lowerVectorLShr(llvm::Value *x, uint64_t scalarY)
{
	llvm::FixedVectorType *ty = llvm::cast<llvm::FixedVectorType>(x->getType());
	llvm::Value *y = llvm::ConstantVector::getSplat(getNumElements(ty),
	                                                llvm::ConstantInt::get(ty->getElementType(), scalarY));
	return jit->builder->CreateLShr(x, y);
}

llvm::Value *lowerShuffleVector(llvm::Value *v1, llvm::Value *v2, llvm::ArrayRef<int> select)
{
	int size = select.size();
	const int maxSize = 16;
	llvm::Constant *swizzle[maxSize];
	ASSERT(size <= maxSize);

	for(int i = 0; i < size; i++)
	{
		swizzle[i] = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*jit->context), select[i]);
	}

	llvm::Value *shuffle = llvm::ConstantVector::get(llvm::ArrayRef<llvm::Constant *>(swizzle, size));

	return jit->builder->CreateShuffleVector(v1, v2, shuffle);
}

[[maybe_unused]] llvm::Value *lowerMulAdd(llvm::Value *x, llvm::Value *y)
{
	llvm::FixedVectorType *ty = llvm::cast<llvm::FixedVectorType>(x->getType());
	llvm::VectorType *extTy = llvm::VectorType::getExtendedElementVectorType(ty);

	llvm::Value *extX = jit->builder->CreateSExt(x, extTy);
	llvm::Value *extY = jit->builder->CreateSExt(y, extTy);
	llvm::Value *mult = jit->builder->CreateMul(extX, extY);

	llvm::Value *undef = llvm::UndefValue::get(extTy);

	llvm::SmallVector<int, 16> evenIdx;
	llvm::SmallVector<int, 16> oddIdx;
	for(uint64_t i = 0, n = ty->getNumElements(); i < n; i += 2)
	{
		evenIdx.push_back(i);
		oddIdx.push_back(i + 1);
	}

	llvm::Value *lhs = lowerShuffleVector(mult, undef, evenIdx);
	llvm::Value *rhs = lowerShuffleVector(mult, undef, oddIdx);
	return jit->builder->CreateAdd(lhs, rhs);
}

[[maybe_unused]] llvm::Value *lowerPack(llvm::Value *x, llvm::Value *y, bool isSigned)
{
	llvm::FixedVectorType *srcTy = llvm::cast<llvm::FixedVectorType>(x->getType());
	llvm::VectorType *dstTy = llvm::VectorType::getTruncatedElementVectorType(srcTy);

	llvm::IntegerType *dstElemTy =
	    llvm::cast<llvm::IntegerType>(dstTy->getElementType());

	uint64_t truncNumBits = dstElemTy->getIntegerBitWidth();
	ASSERT_MSG(truncNumBits < 64, "shift 64 must be handled separately. truncNumBits: %d", int(truncNumBits));
	llvm::Constant *max, *min;
	if(isSigned)
	{
		max = llvm::ConstantInt::get(srcTy, (1LL << (truncNumBits - 1)) - 1, true);
		min = llvm::ConstantInt::get(srcTy, (-1LL << (truncNumBits - 1)), true);
	}
	else
	{
		max = llvm::ConstantInt::get(srcTy, (1ULL << truncNumBits) - 1, false);
		min = llvm::ConstantInt::get(srcTy, 0, false);
	}

	x = lowerPMINMAX(x, min, llvm::ICmpInst::ICMP_SGT);
	x = lowerPMINMAX(x, max, llvm::ICmpInst::ICMP_SLT);
	y = lowerPMINMAX(y, min, llvm::ICmpInst::ICMP_SGT);
	y = lowerPMINMAX(y, max, llvm::ICmpInst::ICMP_SLT);

	x = jit->builder->CreateTrunc(x, dstTy);
	y = jit->builder->CreateTrunc(y, dstTy);

	llvm::SmallVector<int, 16> index(srcTy->getNumElements() * 2);
	std::iota(index.begin(), index.end(), 0);

	return lowerShuffleVector(x, y, index);
}

[[maybe_unused]] llvm::Value *lowerSignMask(llvm::Value *x, llvm::Type *retTy)
{
	llvm::FixedVectorType *ty = llvm::cast<llvm::FixedVectorType>(x->getType());
	llvm::Constant *zero = llvm::ConstantInt::get(ty, 0);
	llvm::Value *cmp = jit->builder->CreateICmpSLT(x, zero);

	llvm::Value *ret = jit->builder->CreateZExt(
	    jit->builder->CreateExtractElement(cmp, static_cast<uint64_t>(0)), retTy);
	for(uint64_t i = 1, n = ty->getNumElements(); i < n; ++i)
	{
		llvm::Value *elem = jit->builder->CreateZExt(
		    jit->builder->CreateExtractElement(cmp, i), retTy);
		ret = jit->builder->CreateOr(ret, jit->builder->CreateShl(elem, i));
	}
	return ret;
}

[[maybe_unused]] llvm::Value *lowerFPSignMask(llvm::Value *x, llvm::Type *retTy)
{
	llvm::FixedVectorType *ty = llvm::cast<llvm::FixedVectorType>(x->getType());
	llvm::Constant *zero = llvm::ConstantFP::get(ty, 0);
	llvm::Value *cmp = jit->builder->CreateFCmpULT(x, zero);

	llvm::Value *ret = jit->builder->CreateZExt(
	    jit->builder->CreateExtractElement(cmp, static_cast<uint64_t>(0)), retTy);
	for(uint64_t i = 1, n = ty->getNumElements(); i < n; ++i)
	{
		llvm::Value *elem = jit->builder->CreateZExt(
		    jit->builder->CreateExtractElement(cmp, i), retTy);
		ret = jit->builder->CreateOr(ret, jit->builder->CreateShl(elem, i));
	}
	return ret;
}

llvm::Value *lowerPUADDSAT(llvm::Value *x, llvm::Value *y)
{
	return jit->builder->CreateBinaryIntrinsic(llvm::Intrinsic::uadd_sat, x, y);
}

llvm::Value *lowerPSADDSAT(llvm::Value *x, llvm::Value *y)
{
	return jit->builder->CreateBinaryIntrinsic(llvm::Intrinsic::sadd_sat, x, y);
}

llvm::Value *lowerPUSUBSAT(llvm::Value *x, llvm::Value *y)
{
	return jit->builder->CreateBinaryIntrinsic(llvm::Intrinsic::usub_sat, x, y);
}

llvm::Value *lowerPSSUBSAT(llvm::Value *x, llvm::Value *y)
{
	return jit->builder->CreateBinaryIntrinsic(llvm::Intrinsic::ssub_sat, x, y);
}

llvm::Value *lowerMulHigh(llvm::Value *x, llvm::Value *y, bool sext)
{
	llvm::VectorType *ty = llvm::cast<llvm::VectorType>(x->getType());
	llvm::VectorType *extTy = llvm::VectorType::getExtendedElementVectorType(ty);

	llvm::Value *extX, *extY;
	if(sext)
	{
		extX = jit->builder->CreateSExt(x, extTy);
		extY = jit->builder->CreateSExt(y, extTy);
	}
	else
	{
		extX = jit->builder->CreateZExt(x, extTy);
		extY = jit->builder->CreateZExt(y, extTy);
	}

	llvm::Value *mult = jit->builder->CreateMul(extX, extY);

	llvm::IntegerType *intTy = llvm::cast<llvm::IntegerType>(ty->getElementType());
	llvm::Value *mulh = jit->builder->CreateAShr(mult, intTy->getBitWidth());
	return jit->builder->CreateTrunc(mulh, ty);
}

// TODO(crbug.com/swiftshader/185): A temporary workaround for failing chromium tests.
llvm::Value *clampForShift(llvm::Value *rhs)
{
	llvm::Value *max;
	if(auto *vec = llvm::dyn_cast<llvm::FixedVectorType>(rhs->getType()))
	{
		auto N = vec->getElementType()->getIntegerBitWidth() - 1;
		max = llvm::ConstantVector::getSplat(getNumElements(vec), llvm::ConstantInt::get(vec->getElementType(), N));
	}
	else
	{
		auto N = rhs->getType()->getIntegerBitWidth() - 1;
		max = llvm::ConstantInt::get(rhs->getType(), N);
	}
	return jit->builder->CreateSelect(jit->builder->CreateICmpULE(rhs, max), rhs, max);
}

}  // namespace

namespace rr {

const int SIMD::Width = 4;

std::string Caps::backendName()
{
	return std::string("LLVM ") + LLVM_VERSION_STRING;
}

bool Caps::coroutinesSupported()
{
	return true;
}

bool Caps::fmaIsFast()
{
	static bool AVX2 = CPUID::supportsAVX2();  // Also checks for FMA support

	// If x86 FMA instructions are supported, assume LLVM will emit them instead of making calls to std::fma().
	return AVX2;
}

// The abstract Type* types are implemented as LLVM types, except that
// 64-bit vectors are emulated using 128-bit ones to avoid use of MMX in x86
// and VFP in ARM, and eliminate the overhead of converting them to explicit
// 128-bit ones. LLVM types are pointers, so we can represent emulated types
// as abstract pointers with small enum values.
enum InternalType : uintptr_t
{
	// Emulated types:
	Type_v2i32,
	Type_v4i16,
	Type_v2i16,
	Type_v8i8,
	Type_v4i8,
	Type_v2f32,
	EmulatedTypeCount,
	// Returned by asInternalType() to indicate that the abstract Type*
	// should be interpreted as LLVM type pointer:
	Type_LLVM
};

inline InternalType asInternalType(Type *type)
{
	InternalType t = static_cast<InternalType>(reinterpret_cast<uintptr_t>(type));
	return (t < EmulatedTypeCount) ? t : Type_LLVM;
}

llvm::Type *T(Type *t)
{
	// Use 128-bit vectors to implement logically shorter ones.
	switch(asInternalType(t))
	{
	case Type_v2i32: return T(Int4::type());
	case Type_v4i16: return T(Short8::type());
	case Type_v2i16: return T(Short8::type());
	case Type_v8i8: return T(Byte16::type());
	case Type_v4i8: return T(Byte16::type());
	case Type_v2f32: return T(Float4::type());
	case Type_LLVM: return reinterpret_cast<llvm::Type *>(t);
	default:
		UNREACHABLE("asInternalType(t): %d", int(asInternalType(t)));
		return nullptr;
	}
}

Type *T(InternalType t)
{
	return reinterpret_cast<Type *>(t);
}

inline const std::vector<llvm::Type *> &T(const std::vector<Type *> &t)
{
	return reinterpret_cast<const std::vector<llvm::Type *> &>(t);
}

inline llvm::BasicBlock *B(BasicBlock *t)
{
	return reinterpret_cast<llvm::BasicBlock *>(t);
}

inline BasicBlock *B(llvm::BasicBlock *t)
{
	return reinterpret_cast<BasicBlock *>(t);
}

static size_t typeSize(Type *type)
{
	switch(asInternalType(type))
	{
	case Type_v2i32: return 8;
	case Type_v4i16: return 8;
	case Type_v2i16: return 4;
	case Type_v8i8: return 8;
	case Type_v4i8: return 4;
	case Type_v2f32: return 8;
	case Type_LLVM:
		{
			llvm::Type *t = T(type);

			if(t->isPointerTy())
			{
				return sizeof(void *);
			}

			// At this point we should only have LLVM 'primitive' types.
			unsigned int bits = t->getPrimitiveSizeInBits();
			ASSERT_MSG(bits != 0, "bits: %d", int(bits));

			// TODO(capn): Booleans are 1 bit integers in LLVM's SSA type system,
			// but are typically stored as one byte. The DataLayout structure should
			// be used here and many other places if this assumption fails.
			return (bits + 7) / 8;
		}
		break;
	default:
		UNREACHABLE("asInternalType(type): %d", int(asInternalType(type)));
		return 0;
	}
}

static llvm::Function *createFunction(const char *name, llvm::Type *retTy, const std::vector<llvm::Type *> &params)
{
	llvm::FunctionType *functionType = llvm::FunctionType::get(retTy, params, false);
	auto func = llvm::Function::Create(functionType, llvm::GlobalValue::InternalLinkage, name, jit->module.get());

	func->setLinkage(llvm::GlobalValue::ExternalLinkage);
	func->setDoesNotThrow();
	func->setCallingConv(llvm::CallingConv::C);

	if(__has_feature(address_sanitizer))
	{
		func->addFnAttr(llvm::Attribute::SanitizeAddress);
	}

	func->addFnAttr("warn-stack-size", "524288");  // Warn when a function uses more than 512 KiB of stack memory

	return func;
}

Nucleus::Nucleus()
{
#if !__has_feature(memory_sanitizer)
	// thread_local variables in shared libraries are initialized at load-time,
	// but this is not observed by MemorySanitizer if the loader itself was not
	// instrumented, leading to false-positive uninitialized variable errors.
	ASSERT(jit == nullptr);
	ASSERT(Variable::unmaterializedVariables == nullptr);
#endif

	jit = new JITBuilder();
	Variable::unmaterializedVariables = new Variable::UnmaterializedVariables();
}

Nucleus::~Nucleus()
{
	delete Variable::unmaterializedVariables;
	Variable::unmaterializedVariables = nullptr;

	delete jit;
	jit = nullptr;
}

std::shared_ptr<Routine> Nucleus::acquireRoutine(const char *name)
{
	if(jit->builder->GetInsertBlock()->empty() || !jit->builder->GetInsertBlock()->back().isTerminator())
	{
		llvm::Type *type = jit->function->getReturnType();

		if(type->isVoidTy())
		{
			createRetVoid();
		}
		else
		{
			createRet(V(llvm::UndefValue::get(type)));
		}
	}

	std::shared_ptr<Routine> routine;

	auto acquire = [&](rr::JITBuilder *jit) {
	// ::jit is thread-local, so when this is executed on a separate thread (see JIT_IN_SEPARATE_THREAD)
	// it needs to only use the jit variable passed in as an argument.

#ifdef ENABLE_RR_DEBUG_INFO
		if(jit->debugInfo != nullptr)
		{
			jit->debugInfo->Finalize();
		}
#endif  // ENABLE_RR_DEBUG_INFO

		if(false)
		{
			std::error_code error;
			llvm::raw_fd_ostream file(std::string(name) + "-llvm-dump-unopt.txt", error);
			jit->module->print(file, 0);
		}

		jit->runPasses();

		if(false)
		{
			std::error_code error;
			llvm::raw_fd_ostream file(std::string(name) + "-llvm-dump-opt.txt", error);
			jit->module->print(file, 0);
		}

		routine = jit->acquireRoutine(name, &jit->function, 1);
	};

#ifdef JIT_IN_SEPARATE_THREAD
	// Perform optimizations and codegen in a separate thread to avoid stack overflow.
	// FIXME(b/149829034): This is not a long-term solution. Reactor has no control
	// over the threading and stack sizes of its users, so this should be addressed
	// at a higher level instead.
	std::thread thread(acquire, jit);
	thread.join();
#else
	acquire(jit);
#endif

	return routine;
}

Value *Nucleus::allocateStackVariable(Type *type, int arraySize)
{
	// Need to allocate it in the entry block for mem2reg to work
	llvm::BasicBlock &entryBlock = jit->function->getEntryBlock();

	llvm::Instruction *declaration;

#if LLVM_VERSION_MAJOR >= 11
	auto align = jit->module->getDataLayout().getPrefTypeAlign(T(type));
#else
	auto align = llvm::MaybeAlign(jit->module->getDataLayout().getPrefTypeAlignment(T(type)));
#endif

	if(arraySize)
	{
		Value *size = (sizeof(size_t) == 8) ? Nucleus::createConstantLong(arraySize) : Nucleus::createConstantInt(arraySize);
		declaration = new llvm::AllocaInst(T(type), 0, V(size), align);
	}
	else
	{
		declaration = new llvm::AllocaInst(T(type), 0, (llvm::Value *)nullptr, align);
	}

#if LLVM_VERSION_MAJOR >= 16
	declaration->insertInto(&entryBlock, entryBlock.begin());
#else
	entryBlock.getInstList().push_front(declaration);
#endif

	if(getPragmaState(InitializeLocalVariables))
	{
		llvm::Type *i8PtrTy = llvm::Type::getInt8Ty(*jit->context)->getPointerTo();
		llvm::Type *i32Ty = llvm::Type::getInt32Ty(*jit->context);
		llvm::Function *memset = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::memset, { i8PtrTy, i32Ty });

		jit->builder->CreateCall(memset, { jit->builder->CreatePointerCast(declaration, i8PtrTy),
		                                   V(Nucleus::createConstantByte((unsigned char)0)),
		                                   V(Nucleus::createConstantInt((int)typeSize(type) * (arraySize ? arraySize : 1))),
		                                   V(Nucleus::createConstantBool(false)) });
	}

	return V(declaration);
}

BasicBlock *Nucleus::createBasicBlock()
{
	return B(llvm::BasicBlock::Create(*jit->context, "", jit->function));
}

BasicBlock *Nucleus::getInsertBlock()
{
	return B(jit->builder->GetInsertBlock());
}

void Nucleus::setInsertBlock(BasicBlock *basicBlock)
{
	// assert(jit->builder->GetInsertBlock()->back().isTerminator());

	jit->builder->SetInsertPoint(B(basicBlock));
}

void Nucleus::createFunction(Type *ReturnType, const std::vector<Type *> &Params)
{
	jit->function = rr::createFunction("", T(ReturnType), T(Params));

#ifdef ENABLE_RR_DEBUG_INFO
	jit->debugInfo = std::make_unique<DebugInfo>(jit->builder.get(), jit->context.get(), jit->module.get(), jit->function);
#endif  // ENABLE_RR_DEBUG_INFO

	jit->builder->SetInsertPoint(llvm::BasicBlock::Create(*jit->context, "", jit->function));
}

Value *Nucleus::getArgument(unsigned int index)
{
	llvm::Function::arg_iterator args = jit->function->arg_begin();

	while(index)
	{
		args++;
		index--;
	}

	return V(&*args);
}

void Nucleus::createRetVoid()
{
	RR_DEBUG_INFO_UPDATE_LOC();

	ASSERT_MSG(jit->function->getReturnType() == T(Void::type()), "Return type mismatch");

	// Code generated after this point is unreachable, so any variables
	// being read can safely return an undefined value. We have to avoid
	// materializing variables after the terminator ret instruction.
	Variable::killUnmaterialized();

	jit->builder->CreateRetVoid();
}

void Nucleus::createRet(Value *v)
{
	RR_DEBUG_INFO_UPDATE_LOC();

	ASSERT_MSG(jit->function->getReturnType() == V(v)->getType(), "Return type mismatch");

	// Code generated after this point is unreachable, so any variables
	// being read can safely return an undefined value. We have to avoid
	// materializing variables after the terminator ret instruction.
	Variable::killUnmaterialized();

	jit->builder->CreateRet(V(v));
}

void Nucleus::createBr(BasicBlock *dest)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Variable::materializeAll();

	jit->builder->CreateBr(B(dest));
}

void Nucleus::createCondBr(Value *cond, BasicBlock *ifTrue, BasicBlock *ifFalse)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Variable::materializeAll();
	jit->builder->CreateCondBr(V(cond), B(ifTrue), B(ifFalse));
}

Value *Nucleus::createAdd(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateAdd(V(lhs), V(rhs)));
}

Value *Nucleus::createSub(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateSub(V(lhs), V(rhs)));
}

Value *Nucleus::createMul(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateMul(V(lhs), V(rhs)));
}

Value *Nucleus::createUDiv(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateUDiv(V(lhs), V(rhs)));
}

Value *Nucleus::createSDiv(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateSDiv(V(lhs), V(rhs)));
}

Value *Nucleus::createFAdd(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateFAdd(V(lhs), V(rhs)));
}

Value *Nucleus::createFSub(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateFSub(V(lhs), V(rhs)));
}

Value *Nucleus::createFMul(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateFMul(V(lhs), V(rhs)));
}

Value *Nucleus::createFDiv(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateFDiv(V(lhs), V(rhs)));
}

Value *Nucleus::createURem(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateURem(V(lhs), V(rhs)));
}

Value *Nucleus::createSRem(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateSRem(V(lhs), V(rhs)));
}

Value *Nucleus::createFRem(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateFRem(V(lhs), V(rhs)));
}

RValue<Float4> operator%(RValue<Float4> lhs, RValue<Float4> rhs)
{
	return RValue<Float4>(Nucleus::createFRem(lhs.value(), rhs.value()));
}

Value *Nucleus::createShl(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	auto *clamped_rhs = clampForShift(V(rhs));
	return V(jit->builder->CreateShl(V(lhs), clamped_rhs));
}

Value *Nucleus::createLShr(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	auto *clamped_rhs = clampForShift(V(rhs));
	return V(jit->builder->CreateLShr(V(lhs), clamped_rhs));
}

Value *Nucleus::createAShr(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateAShr(V(lhs), V(rhs)));
}

Value *Nucleus::createAnd(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateAnd(V(lhs), V(rhs)));
}

Value *Nucleus::createOr(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateOr(V(lhs), V(rhs)));
}

Value *Nucleus::createXor(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateXor(V(lhs), V(rhs)));
}

Value *Nucleus::createNeg(Value *v)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateNeg(V(v)));
}

Value *Nucleus::createFNeg(Value *v)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateFNeg(V(v)));
}

Value *Nucleus::createNot(Value *v)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateNot(V(v)));
}

Value *Nucleus::createLoad(Value *ptr, Type *type, bool isVolatile, unsigned int alignment, bool atomic, std::memory_order memoryOrder)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	switch(asInternalType(type))
	{
	case Type_v2i32:
	case Type_v4i16:
	case Type_v8i8:
	case Type_v2f32:
		return createBitCast(
		    createInsertElement(
		        V(llvm::UndefValue::get(llvm::VectorType::get(T(Long::type()), 2, false))),
		        createLoad(createBitCast(ptr, Pointer<Long>::type()), Long::type(), isVolatile, alignment, atomic, memoryOrder),
		        0),
		    type);
	case Type_v2i16:
	case Type_v4i8:
		if(alignment != 0)  // Not a local variable (all vectors are 128-bit).
		{
			Value *u = V(llvm::UndefValue::get(llvm::VectorType::get(T(Long::type()), 2, false)));
			Value *i = createLoad(createBitCast(ptr, Pointer<Int>::type()), Int::type(), isVolatile, alignment, atomic, memoryOrder);
			i = createZExt(i, Long::type());
			Value *v = createInsertElement(u, i, 0);
			return createBitCast(v, type);
		}
		// Fallthrough to non-emulated case.
	case Type_LLVM:
		{
			auto elTy = T(type);

			if(!atomic)
			{
				return V(jit->builder->CreateAlignedLoad(elTy, V(ptr), llvm::MaybeAlign(alignment), isVolatile));
			}
			else if(elTy->isIntegerTy() || elTy->isPointerTy())
			{
				// Integers and pointers can be atomically loaded by setting
				// the ordering constraint on the load instruction.
				auto load = jit->builder->CreateAlignedLoad(elTy, V(ptr), llvm::MaybeAlign(alignment), isVolatile);
				load->setAtomic(atomicOrdering(atomic, memoryOrder));
				return V(load);
			}
			else if(elTy->isFloatTy() || elTy->isDoubleTy())
			{
				// LLVM claims to support atomic loads of float types as
				// above, but certain backends cannot deal with this.
				// Load as an integer and bitcast. See b/136037244.
				auto size = jit->module->getDataLayout().getTypeStoreSize(elTy);
				auto elAsIntTy = llvm::IntegerType::get(*jit->context, size * 8);
				auto ptrCast = jit->builder->CreatePointerCast(V(ptr), elAsIntTy->getPointerTo());
				auto load = jit->builder->CreateAlignedLoad(elAsIntTy, ptrCast, llvm::MaybeAlign(alignment), isVolatile);
				load->setAtomic(atomicOrdering(atomic, memoryOrder));
				auto loadCast = jit->builder->CreateBitCast(load, elTy);
				return V(loadCast);
			}
			else
			{
				// More exotic types require falling back to the extern:
				// void __atomic_load(size_t size, void *ptr, void *ret, int ordering)
				auto sizetTy = llvm::IntegerType::get(*jit->context, sizeof(size_t) * 8);
				auto intTy = llvm::IntegerType::get(*jit->context, sizeof(int) * 8);
				auto i8Ty = llvm::Type::getInt8Ty(*jit->context);
				auto i8PtrTy = i8Ty->getPointerTo();
				auto voidTy = llvm::Type::getVoidTy(*jit->context);
				auto funcTy = llvm::FunctionType::get(voidTy, { sizetTy, i8PtrTy, i8PtrTy, intTy }, false);
				auto func = jit->module->getOrInsertFunction("__atomic_load", funcTy);
				auto size = jit->module->getDataLayout().getTypeStoreSize(elTy);
				auto out = allocateStackVariable(type);
				jit->builder->CreateCall(func, {
				                                   llvm::ConstantInt::get(sizetTy, size),
				                                   jit->builder->CreatePointerCast(V(ptr), i8PtrTy),
				                                   jit->builder->CreatePointerCast(V(out), i8PtrTy),
				                                   llvm::ConstantInt::get(intTy, uint64_t(atomicOrdering(true, memoryOrder))),
				                               });
				return V(jit->builder->CreateLoad(T(type), V(out)));
			}
		}
	default:
		UNREACHABLE("asInternalType(type): %d", int(asInternalType(type)));
		return nullptr;
	}
}

Value *Nucleus::createStore(Value *value, Value *ptr, Type *type, bool isVolatile, unsigned int alignment, bool atomic, std::memory_order memoryOrder)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	switch(asInternalType(type))
	{
	case Type_v2i32:
	case Type_v4i16:
	case Type_v8i8:
	case Type_v2f32:
		createStore(
		    createExtractElement(
		        createBitCast(value, T(llvm::VectorType::get(T(Long::type()), 2, false))), Long::type(), 0),
		    createBitCast(ptr, Pointer<Long>::type()),
		    Long::type(), isVolatile, alignment, atomic, memoryOrder);
		return value;
	case Type_v2i16:
	case Type_v4i8:
		if(alignment != 0)  // Not a local variable (all vectors are 128-bit).
		{
			createStore(
			    createExtractElement(createBitCast(value, Int4::type()), Int::type(), 0),
			    createBitCast(ptr, Pointer<Int>::type()),
			    Int::type(), isVolatile, alignment, atomic, memoryOrder);
			return value;
		}
		// Fallthrough to non-emulated case.
	case Type_LLVM:
		{
			auto elTy = T(type);

			if(__has_feature(memory_sanitizer) && !jit->msanInstrumentation)
			{
				// Mark all memory writes as initialized by calling __msan_unpoison
				// void __msan_unpoison(const volatile void *a, size_t size)
				auto voidTy = llvm::Type::getVoidTy(*jit->context);
				auto i8Ty = llvm::Type::getInt8Ty(*jit->context);
				auto voidPtrTy = i8Ty->getPointerTo();
				auto sizetTy = llvm::IntegerType::get(*jit->context, sizeof(size_t) * 8);
				auto funcTy = llvm::FunctionType::get(voidTy, { voidPtrTy, sizetTy }, false);
				auto func = jit->module->getOrInsertFunction("__msan_unpoison", funcTy);
				auto size = jit->module->getDataLayout().getTypeStoreSize(elTy);

				jit->builder->CreateCall(func, { jit->builder->CreatePointerCast(V(ptr), voidPtrTy),
				                                 llvm::ConstantInt::get(sizetTy, size) });
			}

			if(!atomic)
			{
				jit->builder->CreateAlignedStore(V(value), V(ptr), llvm::MaybeAlign(alignment), isVolatile);
			}
			else if(elTy->isIntegerTy() || elTy->isPointerTy())
			{
				// Integers and pointers can be atomically stored by setting
				// the ordering constraint on the store instruction.
				auto store = jit->builder->CreateAlignedStore(V(value), V(ptr), llvm::MaybeAlign(alignment), isVolatile);
				store->setAtomic(atomicOrdering(atomic, memoryOrder));
			}
			else if(elTy->isFloatTy() || elTy->isDoubleTy())
			{
				// LLVM claims to support atomic stores of float types as
				// above, but certain backends cannot deal with this.
				// Store as an bitcast integer. See b/136037244.
				auto size = jit->module->getDataLayout().getTypeStoreSize(elTy);
				auto elAsIntTy = llvm::IntegerType::get(*jit->context, size * 8);
				auto valCast = jit->builder->CreateBitCast(V(value), elAsIntTy);
				auto ptrCast = jit->builder->CreatePointerCast(V(ptr), elAsIntTy->getPointerTo());
				auto store = jit->builder->CreateAlignedStore(valCast, ptrCast, llvm::MaybeAlign(alignment), isVolatile);
				store->setAtomic(atomicOrdering(atomic, memoryOrder));
			}
			else
			{
				// More exotic types require falling back to the extern:
				// void __atomic_store(size_t size, void *ptr, void *val, int ordering)
				auto sizetTy = llvm::IntegerType::get(*jit->context, sizeof(size_t) * 8);
				auto intTy = llvm::IntegerType::get(*jit->context, sizeof(int) * 8);
				auto i8Ty = llvm::Type::getInt8Ty(*jit->context);
				auto i8PtrTy = i8Ty->getPointerTo();
				auto voidTy = llvm::Type::getVoidTy(*jit->context);
				auto funcTy = llvm::FunctionType::get(voidTy, { sizetTy, i8PtrTy, i8PtrTy, intTy }, false);
				auto func = jit->module->getOrInsertFunction("__atomic_store", funcTy);
				auto size = jit->module->getDataLayout().getTypeStoreSize(elTy);
				auto copy = allocateStackVariable(type);
				jit->builder->CreateStore(V(value), V(copy));
				jit->builder->CreateCall(func, {
				                                   llvm::ConstantInt::get(sizetTy, size),
				                                   jit->builder->CreatePointerCast(V(ptr), i8PtrTy),
				                                   jit->builder->CreatePointerCast(V(copy), i8PtrTy),
				                                   llvm::ConstantInt::get(intTy, uint64_t(atomicOrdering(true, memoryOrder))),
				                               });
			}

			return value;
		}
	default:
		UNREACHABLE("asInternalType(type): %d", int(asInternalType(type)));
		return nullptr;
	}
}

Value *Nucleus::createMaskedLoad(Value *ptr, Type *elTy, Value *mask, unsigned int alignment, bool zeroMaskedLanes)
{
	RR_DEBUG_INFO_UPDATE_LOC();

	ASSERT(V(ptr)->getType()->isPointerTy());
	ASSERT(V(mask)->getType()->isVectorTy());

	auto numEls = llvm::cast<llvm::FixedVectorType>(V(mask)->getType())->getNumElements();
	auto i1Ty = llvm::Type::getInt1Ty(*jit->context);
	auto i32Ty = llvm::Type::getInt32Ty(*jit->context);
	auto elVecTy = llvm::VectorType::get(T(elTy), numEls, false);
	auto elVecPtrTy = elVecTy->getPointerTo();
	auto i8Mask = jit->builder->CreateIntCast(V(mask), llvm::VectorType::get(i1Ty, numEls, false), false);  // vec<int, int, ...> -> vec<bool, bool, ...>
	auto passthrough = zeroMaskedLanes ? llvm::Constant::getNullValue(elVecTy) : llvm::UndefValue::get(elVecTy);
	auto align = llvm::ConstantInt::get(i32Ty, alignment);
	auto func = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::masked_load, { elVecTy, elVecPtrTy });
	return V(jit->builder->CreateCall(func, { V(ptr), align, i8Mask, passthrough }));
}

void Nucleus::createMaskedStore(Value *ptr, Value *val, Value *mask, unsigned int alignment)
{
	RR_DEBUG_INFO_UPDATE_LOC();

	ASSERT(V(ptr)->getType()->isPointerTy());
	ASSERT(V(val)->getType()->isVectorTy());
	ASSERT(V(mask)->getType()->isVectorTy());

	auto numEls = llvm::cast<llvm::FixedVectorType>(V(mask)->getType())->getNumElements();
	auto i1Ty = llvm::Type::getInt1Ty(*jit->context);
	auto i32Ty = llvm::Type::getInt32Ty(*jit->context);
	auto elVecTy = V(val)->getType();
	auto elVecPtrTy = elVecTy->getPointerTo();
	auto i1Mask = jit->builder->CreateIntCast(V(mask), llvm::VectorType::get(i1Ty, numEls, false), false);  // vec<int, int, ...> -> vec<bool, bool, ...>
	auto align = llvm::ConstantInt::get(i32Ty, alignment);
	auto func = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::masked_store, { elVecTy, elVecPtrTy });
	jit->builder->CreateCall(func, { V(val), V(ptr), align, i1Mask });

	if(__has_feature(memory_sanitizer) && !jit->msanInstrumentation)
	{
		// Mark memory writes as initialized by calling __msan_unpoison
		// void __msan_unpoison(const volatile void *a, size_t size)
		auto voidTy = llvm::Type::getVoidTy(*jit->context);
		auto voidPtrTy = voidTy->getPointerTo();
		auto sizetTy = llvm::IntegerType::get(*jit->context, sizeof(size_t) * 8);
		auto funcTy = llvm::FunctionType::get(voidTy, { voidPtrTy, sizetTy }, false);
		auto func = jit->module->getOrInsertFunction("__msan_unpoison", funcTy);
		auto size = jit->module->getDataLayout().getTypeStoreSize(llvm::cast<llvm::VectorType>(elVecTy)->getElementType());

		for(unsigned i = 0; i < numEls; i++)
		{
			// Check mask for this element
			auto idx = llvm::ConstantInt::get(i32Ty, i);
			auto thenBlock = llvm::BasicBlock::Create(*jit->context, "", jit->function);
			auto mergeBlock = llvm::BasicBlock::Create(*jit->context, "", jit->function);
			jit->builder->CreateCondBr(jit->builder->CreateExtractElement(i1Mask, idx), thenBlock, mergeBlock);
			jit->builder->SetInsertPoint(thenBlock);

			// Insert __msan_unpoison call in conditional block
			auto elPtr = jit->builder->CreateGEP(elVecTy, V(ptr), idx);
			jit->builder->CreateCall(func, { jit->builder->CreatePointerCast(elPtr, voidPtrTy),
			                                 llvm::ConstantInt::get(sizetTy, size) });

			jit->builder->CreateBr(mergeBlock);
			jit->builder->SetInsertPoint(mergeBlock);
		}
	}
}

static llvm::Value *createGather(llvm::Value *base, llvm::Type *elTy, llvm::Value *offsets, llvm::Value *mask, unsigned int alignment, bool zeroMaskedLanes)
{
	ASSERT(base->getType()->isPointerTy());
	ASSERT(offsets->getType()->isVectorTy());
	ASSERT(mask->getType()->isVectorTy());

	auto numEls = llvm::cast<llvm::FixedVectorType>(mask->getType())->getNumElements();
	auto i1Ty = llvm::Type::getInt1Ty(*jit->context);
	auto i32Ty = llvm::Type::getInt32Ty(*jit->context);
	auto i8Ty = llvm::Type::getInt8Ty(*jit->context);
	auto i8PtrTy = i8Ty->getPointerTo();
	auto elPtrTy = elTy->getPointerTo();
	auto elVecTy = llvm::VectorType::get(elTy, numEls, false);
	auto elPtrVecTy = llvm::VectorType::get(elPtrTy, numEls, false);
	auto i8Base = jit->builder->CreatePointerCast(base, i8PtrTy);
	auto i8Ptrs = jit->builder->CreateGEP(i8Ty, i8Base, offsets);
	auto elPtrs = jit->builder->CreatePointerCast(i8Ptrs, elPtrVecTy);
	auto i1Mask = jit->builder->CreateIntCast(mask, llvm::VectorType::get(i1Ty, numEls, false), false);  // vec<int, int, ...> -> vec<bool, bool, ...>
	auto passthrough = zeroMaskedLanes ? llvm::Constant::getNullValue(elVecTy) : llvm::UndefValue::get(elVecTy);

	if(!__has_feature(memory_sanitizer))
	{
		auto align = llvm::ConstantInt::get(i32Ty, alignment);
		auto func = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::masked_gather, { elVecTy, elPtrVecTy });
		return jit->builder->CreateCall(func, { elPtrs, align, i1Mask, passthrough });
	}
	else  // __has_feature(memory_sanitizer)
	{
		// MemorySanitizer currently does not support instrumenting llvm::Intrinsic::masked_gather
		// Work around it by emulating gather with element-wise loads.
		// TODO(b/172238865): Remove when supported by MemorySanitizer.

		Value *result = Nucleus::allocateStackVariable(T(elVecTy));
		Nucleus::createStore(V(passthrough), result, T(elVecTy));

		for(unsigned i = 0; i < numEls; i++)
		{
			// Check mask for this element
			Value *elementMask = Nucleus::createExtractElement(V(i1Mask), T(i1Ty), i);

			If(RValue<Bool>(elementMask))
			{
				Value *elPtr = Nucleus::createExtractElement(V(elPtrs), T(elPtrTy), i);
				Value *el = Nucleus::createLoad(elPtr, T(elTy), /*isVolatile */ false, alignment, /* atomic */ false, std::memory_order_relaxed);

				Value *v = Nucleus::createLoad(result, T(elVecTy));
				v = Nucleus::createInsertElement(v, el, i);
				Nucleus::createStore(v, result, T(elVecTy));
			}
		}

		return V(Nucleus::createLoad(result, T(elVecTy)));
	}
}

RValue<SIMD::Float> Gather(RValue<Pointer<Float>> base, RValue<SIMD::Int> offsets, RValue<SIMD::Int> mask, unsigned int alignment, bool zeroMaskedLanes /* = false */)
{
	return As<SIMD::Float>(V(createGather(V(base.value()), T(Float::type()), V(offsets.value()), V(mask.value()), alignment, zeroMaskedLanes)));
}

RValue<SIMD::Int> Gather(RValue<Pointer<Int>> base, RValue<SIMD::Int> offsets, RValue<SIMD::Int> mask, unsigned int alignment, bool zeroMaskedLanes /* = false */)
{
	return As<SIMD::Int>(V(createGather(V(base.value()), T(Int::type()), V(offsets.value()), V(mask.value()), alignment, zeroMaskedLanes)));
}

static void createScatter(llvm::Value *base, llvm::Value *val, llvm::Value *offsets, llvm::Value *mask, unsigned int alignment)
{
	ASSERT(base->getType()->isPointerTy());
	ASSERT(val->getType()->isVectorTy());
	ASSERT(offsets->getType()->isVectorTy());
	ASSERT(mask->getType()->isVectorTy());

	auto numEls = llvm::cast<llvm::FixedVectorType>(mask->getType())->getNumElements();
	auto i1Ty = llvm::Type::getInt1Ty(*jit->context);
	auto i32Ty = llvm::Type::getInt32Ty(*jit->context);
	auto i8Ty = llvm::Type::getInt8Ty(*jit->context);
	auto i8PtrTy = i8Ty->getPointerTo();
	auto elVecTy = val->getType();
	auto elTy = llvm::cast<llvm::VectorType>(elVecTy)->getElementType();
	auto elPtrTy = elTy->getPointerTo();
	auto elPtrVecTy = llvm::VectorType::get(elPtrTy, numEls, false);

	auto i8Base = jit->builder->CreatePointerCast(base, i8PtrTy);
	auto i8Ptrs = jit->builder->CreateGEP(i8Ty, i8Base, offsets);
	auto elPtrs = jit->builder->CreatePointerCast(i8Ptrs, elPtrVecTy);
	auto i1Mask = jit->builder->CreateIntCast(mask, llvm::VectorType::get(i1Ty, numEls, false), false);  // vec<int, int, ...> -> vec<bool, bool, ...>

	if(!__has_feature(memory_sanitizer))
	{
		auto align = llvm::ConstantInt::get(i32Ty, alignment);
		auto func = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::masked_scatter, { elVecTy, elPtrVecTy });
		jit->builder->CreateCall(func, { val, elPtrs, align, i1Mask });
	}
	else  // __has_feature(memory_sanitizer)
	{
		// MemorySanitizer currently does not support instrumenting llvm::Intrinsic::masked_scatter
		// Work around it by emulating scatter with element-wise stores.
		// TODO(b/172238865): Remove when supported by MemorySanitizer.

		for(unsigned i = 0; i < numEls; i++)
		{
			// Check mask for this element
			auto idx = llvm::ConstantInt::get(i32Ty, i);
			auto thenBlock = llvm::BasicBlock::Create(*jit->context, "", jit->function);
			auto mergeBlock = llvm::BasicBlock::Create(*jit->context, "", jit->function);
			jit->builder->CreateCondBr(jit->builder->CreateExtractElement(i1Mask, idx), thenBlock, mergeBlock);
			jit->builder->SetInsertPoint(thenBlock);

			auto el = jit->builder->CreateExtractElement(val, idx);
			auto elPtr = jit->builder->CreateExtractElement(elPtrs, idx);
			Nucleus::createStore(V(el), V(elPtr), T(elTy), /*isVolatile */ false, alignment, /* atomic */ false, std::memory_order_relaxed);

			jit->builder->CreateBr(mergeBlock);
			jit->builder->SetInsertPoint(mergeBlock);
		}
	}
}

void Scatter(RValue<Pointer<Float>> base, RValue<SIMD::Float> val, RValue<SIMD::Int> offsets, RValue<SIMD::Int> mask, unsigned int alignment)
{
	return createScatter(V(base.value()), V(val.value()), V(offsets.value()), V(mask.value()), alignment);
}

void Scatter(RValue<Pointer<Int>> base, RValue<SIMD::Int> val, RValue<SIMD::Int> offsets, RValue<SIMD::Int> mask, unsigned int alignment)
{
	return createScatter(V(base.value()), V(val.value()), V(offsets.value()), V(mask.value()), alignment);
}

void Nucleus::createFence(std::memory_order memoryOrder)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	jit->builder->CreateFence(atomicOrdering(true, memoryOrder));
}

Value *Nucleus::createGEP(Value *ptr, Type *type, Value *index, bool unsignedIndex)
{
	RR_DEBUG_INFO_UPDATE_LOC();

	if(sizeof(void *) == 8)
	{
		// LLVM manual: "When indexing into an array, pointer or vector,
		// integers of any width are allowed, and they are not required to
		// be constant. These integers are treated as signed values where
		// relevant."
		//
		// Thus if we want indexes to be treated as unsigned we have to
		// zero-extend them ourselves.
		//
		// Note that this is not because we want to address anywhere near
		// 4 GB of data. Instead this is important for performance because
		// x86 supports automatic zero-extending of 32-bit registers to
		// 64-bit. Thus when indexing into an array using a uint32 is
		// actually faster than an int32.
		index = unsignedIndex ? createZExt(index, Long::type()) : createSExt(index, Long::type());
	}

	// For non-emulated types we can rely on LLVM's GEP to calculate the
	// effective address correctly.
	if(asInternalType(type) == Type_LLVM)
	{
		return V(jit->builder->CreateGEP(T(type), V(ptr), V(index)));
	}

	// For emulated types we have to multiply the index by the intended
	// type size ourselves to obain the byte offset.
	index = (sizeof(void *) == 8) ? createMul(index, createConstantLong((int64_t)typeSize(type))) : createMul(index, createConstantInt((int)typeSize(type)));

	// Cast to a byte pointer, apply the byte offset, and cast back to the
	// original pointer type.
	return createBitCast(
	    V(jit->builder->CreateGEP(T(Byte::type()), V(createBitCast(ptr, T(llvm::PointerType::get(T(Byte::type()), 0)))), V(index))),
	    T(llvm::PointerType::get(T(type), 0)));
}

Value *Nucleus::createAtomicAdd(Value *ptr, Value *value, std::memory_order memoryOrder)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateAtomicRMW(llvm::AtomicRMWInst::Add, V(ptr), V(value),
#if LLVM_VERSION_MAJOR >= 11
	                                       llvm::MaybeAlign(),
#endif
	                                       atomicOrdering(true, memoryOrder)));
}

Value *Nucleus::createAtomicSub(Value *ptr, Value *value, std::memory_order memoryOrder)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateAtomicRMW(llvm::AtomicRMWInst::Sub, V(ptr), V(value),
#if LLVM_VERSION_MAJOR >= 11
	                                       llvm::MaybeAlign(),
#endif
	                                       atomicOrdering(true, memoryOrder)));
}

Value *Nucleus::createAtomicAnd(Value *ptr, Value *value, std::memory_order memoryOrder)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateAtomicRMW(llvm::AtomicRMWInst::And, V(ptr), V(value),
#if LLVM_VERSION_MAJOR >= 11
	                                       llvm::MaybeAlign(),
#endif
	                                       atomicOrdering(true, memoryOrder)));
}

Value *Nucleus::createAtomicOr(Value *ptr, Value *value, std::memory_order memoryOrder)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateAtomicRMW(llvm::AtomicRMWInst::Or, V(ptr), V(value),
#if LLVM_VERSION_MAJOR >= 11
	                                       llvm::MaybeAlign(),
#endif
	                                       atomicOrdering(true, memoryOrder)));
}

Value *Nucleus::createAtomicXor(Value *ptr, Value *value, std::memory_order memoryOrder)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateAtomicRMW(llvm::AtomicRMWInst::Xor, V(ptr), V(value),
#if LLVM_VERSION_MAJOR >= 11
	                                       llvm::MaybeAlign(),
#endif
	                                       atomicOrdering(true, memoryOrder)));
}

Value *Nucleus::createAtomicMin(Value *ptr, Value *value, std::memory_order memoryOrder)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateAtomicRMW(llvm::AtomicRMWInst::Min, V(ptr), V(value),
#if LLVM_VERSION_MAJOR >= 11
	                                       llvm::MaybeAlign(),
#endif
	                                       atomicOrdering(true, memoryOrder)));
}

Value *Nucleus::createAtomicMax(Value *ptr, Value *value, std::memory_order memoryOrder)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateAtomicRMW(llvm::AtomicRMWInst::Max, V(ptr), V(value),
#if LLVM_VERSION_MAJOR >= 11
	                                       llvm::MaybeAlign(),
#endif
	                                       atomicOrdering(true, memoryOrder)));
}

Value *Nucleus::createAtomicUMin(Value *ptr, Value *value, std::memory_order memoryOrder)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateAtomicRMW(llvm::AtomicRMWInst::UMin, V(ptr), V(value),
#if LLVM_VERSION_MAJOR >= 11
	                                       llvm::MaybeAlign(),
#endif
	                                       atomicOrdering(true, memoryOrder)));
}

Value *Nucleus::createAtomicUMax(Value *ptr, Value *value, std::memory_order memoryOrder)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateAtomicRMW(llvm::AtomicRMWInst::UMax, V(ptr), V(value),
#if LLVM_VERSION_MAJOR >= 11
	                                       llvm::MaybeAlign(),
#endif
	                                       atomicOrdering(true, memoryOrder)));
}

Value *Nucleus::createAtomicExchange(Value *ptr, Value *value, std::memory_order memoryOrder)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateAtomicRMW(llvm::AtomicRMWInst::Xchg, V(ptr), V(value),
#if LLVM_VERSION_MAJOR >= 11
	                                       llvm::MaybeAlign(),
#endif
	                                       atomicOrdering(true, memoryOrder)));
}

Value *Nucleus::createAtomicCompareExchange(Value *ptr, Value *value, Value *compare, std::memory_order memoryOrderEqual, std::memory_order memoryOrderUnequal)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	// Note: AtomicCmpXchgInstruction returns a 2-member struct containing {result, success-flag}, not the result directly.
	return V(jit->builder->CreateExtractValue(
	    jit->builder->CreateAtomicCmpXchg(V(ptr), V(compare), V(value),
#if LLVM_VERSION_MAJOR >= 11
	                                      llvm::MaybeAlign(),
#endif
	                                      atomicOrdering(true, memoryOrderEqual),
	                                      atomicOrdering(true, memoryOrderUnequal)),
	    llvm::ArrayRef<unsigned>(0u)));
}

Value *Nucleus::createTrunc(Value *v, Type *destType)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateTrunc(V(v), T(destType)));
}

Value *Nucleus::createZExt(Value *v, Type *destType)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateZExt(V(v), T(destType)));
}

Value *Nucleus::createSExt(Value *v, Type *destType)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateSExt(V(v), T(destType)));
}

Value *Nucleus::createFPToUI(Value *v, Type *destType)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateFPToUI(V(v), T(destType)));
}

Value *Nucleus::createFPToSI(Value *v, Type *destType)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateFPToSI(V(v), T(destType)));
}

Value *Nucleus::createSIToFP(Value *v, Type *destType)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateSIToFP(V(v), T(destType)));
}

Value *Nucleus::createFPTrunc(Value *v, Type *destType)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateFPTrunc(V(v), T(destType)));
}

Value *Nucleus::createFPExt(Value *v, Type *destType)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateFPExt(V(v), T(destType)));
}

Value *Nucleus::createBitCast(Value *v, Type *destType)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	// Bitcasts must be between types of the same logical size. But with emulated narrow vectors we need
	// support for casting between scalars and wide vectors. Emulate them by writing to the stack and
	// reading back as the destination type.
	if(!V(v)->getType()->isVectorTy() && T(destType)->isVectorTy())
	{
		Value *readAddress = allocateStackVariable(destType);
		Value *writeAddress = createBitCast(readAddress, T(llvm::PointerType::get(V(v)->getType(), 0)));
		createStore(v, writeAddress, T(V(v)->getType()));
		return createLoad(readAddress, destType);
	}
	else if(V(v)->getType()->isVectorTy() && !T(destType)->isVectorTy())
	{
		Value *writeAddress = allocateStackVariable(T(V(v)->getType()));
		createStore(v, writeAddress, T(V(v)->getType()));
		Value *readAddress = createBitCast(writeAddress, T(llvm::PointerType::get(T(destType), 0)));
		return createLoad(readAddress, destType);
	}

	return V(jit->builder->CreateBitCast(V(v), T(destType)));
}

Value *Nucleus::createICmpEQ(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateICmpEQ(V(lhs), V(rhs)));
}

Value *Nucleus::createICmpNE(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateICmpNE(V(lhs), V(rhs)));
}

Value *Nucleus::createICmpUGT(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateICmpUGT(V(lhs), V(rhs)));
}

Value *Nucleus::createICmpUGE(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateICmpUGE(V(lhs), V(rhs)));
}

Value *Nucleus::createICmpULT(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateICmpULT(V(lhs), V(rhs)));
}

Value *Nucleus::createICmpULE(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateICmpULE(V(lhs), V(rhs)));
}

Value *Nucleus::createICmpSGT(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateICmpSGT(V(lhs), V(rhs)));
}

Value *Nucleus::createICmpSGE(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateICmpSGE(V(lhs), V(rhs)));
}

Value *Nucleus::createICmpSLT(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateICmpSLT(V(lhs), V(rhs)));
}

Value *Nucleus::createICmpSLE(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateICmpSLE(V(lhs), V(rhs)));
}

Value *Nucleus::createFCmpOEQ(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateFCmpOEQ(V(lhs), V(rhs)));
}

Value *Nucleus::createFCmpOGT(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateFCmpOGT(V(lhs), V(rhs)));
}

Value *Nucleus::createFCmpOGE(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateFCmpOGE(V(lhs), V(rhs)));
}

Value *Nucleus::createFCmpOLT(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateFCmpOLT(V(lhs), V(rhs)));
}

Value *Nucleus::createFCmpOLE(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateFCmpOLE(V(lhs), V(rhs)));
}

Value *Nucleus::createFCmpONE(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateFCmpONE(V(lhs), V(rhs)));
}

Value *Nucleus::createFCmpORD(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateFCmpORD(V(lhs), V(rhs)));
}

Value *Nucleus::createFCmpUNO(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateFCmpUNO(V(lhs), V(rhs)));
}

Value *Nucleus::createFCmpUEQ(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateFCmpUEQ(V(lhs), V(rhs)));
}

Value *Nucleus::createFCmpUGT(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateFCmpUGT(V(lhs), V(rhs)));
}

Value *Nucleus::createFCmpUGE(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateFCmpUGE(V(lhs), V(rhs)));
}

Value *Nucleus::createFCmpULT(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateFCmpULT(V(lhs), V(rhs)));
}

Value *Nucleus::createFCmpULE(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateFCmpULE(V(lhs), V(rhs)));
}

Value *Nucleus::createFCmpUNE(Value *lhs, Value *rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateFCmpUNE(V(lhs), V(rhs)));
}

Value *Nucleus::createExtractElement(Value *vector, Type *type, int index)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	ASSERT(V(vector)->getType()->getContainedType(0) == T(type));
	return V(jit->builder->CreateExtractElement(V(vector), V(createConstantInt(index))));
}

Value *Nucleus::createInsertElement(Value *vector, Value *element, int index)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateInsertElement(V(vector), V(element), V(createConstantInt(index))));
}

Value *Nucleus::createShuffleVector(Value *v1, Value *v2, std::vector<int> select)
{
	RR_DEBUG_INFO_UPDATE_LOC();

	size_t size = llvm::cast<llvm::FixedVectorType>(V(v1)->getType())->getNumElements();
	ASSERT(size == llvm::cast<llvm::FixedVectorType>(V(v2)->getType())->getNumElements());

	llvm::SmallVector<int, 16> mask;
	const size_t selectSize = select.size();
	for(size_t i = 0; i < size; i++)
	{
		mask.push_back(select[i % selectSize]);
	}

	return V(lowerShuffleVector(V(v1), V(v2), mask));
}

Value *Nucleus::createSelect(Value *c, Value *ifTrue, Value *ifFalse)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(jit->builder->CreateSelect(V(c), V(ifTrue), V(ifFalse)));
}

SwitchCases *Nucleus::createSwitch(Value *control, BasicBlock *defaultBranch, unsigned numCases)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return reinterpret_cast<SwitchCases *>(jit->builder->CreateSwitch(V(control), B(defaultBranch), numCases));
}

void Nucleus::addSwitchCase(SwitchCases *switchCases, int label, BasicBlock *branch)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	llvm::SwitchInst *sw = reinterpret_cast<llvm::SwitchInst *>(switchCases);
	sw->addCase(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*jit->context), label, true), B(branch));
}

void Nucleus::createUnreachable()
{
	RR_DEBUG_INFO_UPDATE_LOC();
	jit->builder->CreateUnreachable();
}

Type *Nucleus::getType(Value *value)
{
	return T(V(value)->getType());
}

Type *Nucleus::getContainedType(Type *vectorType)
{
	return T(T(vectorType)->getContainedType(0));
}

Type *Nucleus::getPointerType(Type *ElementType)
{
	return T(llvm::PointerType::get(T(ElementType), 0));
}

static llvm::Type *getNaturalIntType()
{
	return llvm::Type::getIntNTy(*jit->context, sizeof(int) * 8);
}

Type *Nucleus::getPrintfStorageType(Type *valueType)
{
	llvm::Type *valueTy = T(valueType);
	if(valueTy->isIntegerTy())
	{
		return T(getNaturalIntType());
	}
	if(valueTy->isFloatTy())
	{
		return T(llvm::Type::getDoubleTy(*jit->context));
	}

	UNIMPLEMENTED_NO_BUG("getPrintfStorageType: add more cases as needed");
	return {};
}

Value *Nucleus::createNullValue(Type *Ty)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(llvm::Constant::getNullValue(T(Ty)));
}

Value *Nucleus::createConstantLong(int64_t i)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(llvm::ConstantInt::get(llvm::Type::getInt64Ty(*jit->context), i, true));
}

Value *Nucleus::createConstantInt(int i)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*jit->context), i, true));
}

Value *Nucleus::createConstantInt(unsigned int i)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*jit->context), i, false));
}

Value *Nucleus::createConstantBool(bool b)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(llvm::ConstantInt::get(llvm::Type::getInt1Ty(*jit->context), b));
}

Value *Nucleus::createConstantByte(signed char i)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(llvm::ConstantInt::get(llvm::Type::getInt8Ty(*jit->context), i, true));
}

Value *Nucleus::createConstantByte(unsigned char i)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(llvm::ConstantInt::get(llvm::Type::getInt8Ty(*jit->context), i, false));
}

Value *Nucleus::createConstantShort(short i)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(llvm::ConstantInt::get(llvm::Type::getInt16Ty(*jit->context), i, true));
}

Value *Nucleus::createConstantShort(unsigned short i)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(llvm::ConstantInt::get(llvm::Type::getInt16Ty(*jit->context), i, false));
}

Value *Nucleus::createConstantFloat(float x)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(llvm::ConstantFP::get(T(Float::type()), x));
}

Value *Nucleus::createNullPointer(Type *Ty)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return V(llvm::ConstantPointerNull::get(llvm::PointerType::get(T(Ty), 0)));
}

Value *Nucleus::createConstantVector(std::vector<int64_t> constants, Type *type)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	ASSERT(llvm::isa<llvm::VectorType>(T(type)));
	const size_t numConstants = constants.size();                                             // Number of provided constants for the (emulated) type.
	const size_t numElements = llvm::cast<llvm::FixedVectorType>(T(type))->getNumElements();  // Number of elements of the underlying vector type.
	llvm::SmallVector<llvm::Constant *, 16> constantVector;

	for(size_t i = 0; i < numElements; i++)
	{
		constantVector.push_back(llvm::ConstantInt::get(T(type)->getContainedType(0), constants[i % numConstants]));
	}

	return V(llvm::ConstantVector::get(llvm::ArrayRef<llvm::Constant *>(constantVector)));
}

Value *Nucleus::createConstantVector(std::vector<double> constants, Type *type)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	ASSERT(llvm::isa<llvm::VectorType>(T(type)));
	const size_t numConstants = constants.size();                                             // Number of provided constants for the (emulated) type.
	const size_t numElements = llvm::cast<llvm::FixedVectorType>(T(type))->getNumElements();  // Number of elements of the underlying vector type.
	llvm::SmallVector<llvm::Constant *, 16> constantVector;

	for(size_t i = 0; i < numElements; i++)
	{
		constantVector.push_back(llvm::ConstantFP::get(T(type)->getContainedType(0), constants[i % numConstants]));
	}

	return V(llvm::ConstantVector::get(llvm::ArrayRef<llvm::Constant *>(constantVector)));
}

Value *Nucleus::createConstantString(const char *v)
{
	// NOTE: Do not call RR_DEBUG_INFO_UPDATE_LOC() here to avoid recursion when called from rr::Printv
	auto ptr = jit->builder->CreateGlobalStringPtr(v);
	return V(ptr);
}

void Nucleus::setOptimizerCallback(OptimizerCallback *callback)
{
	// The LLVM backend does not produce optimizer reports.
	(void)callback;
}

Type *Void::type()
{
	return T(llvm::Type::getVoidTy(*jit->context));
}

Type *Bool::type()
{
	return T(llvm::Type::getInt1Ty(*jit->context));
}

Type *Byte::type()
{
	return T(llvm::Type::getInt8Ty(*jit->context));
}

Type *SByte::type()
{
	return T(llvm::Type::getInt8Ty(*jit->context));
}

Type *Short::type()
{
	return T(llvm::Type::getInt16Ty(*jit->context));
}

Type *UShort::type()
{
	return T(llvm::Type::getInt16Ty(*jit->context));
}

Type *Byte4::type()
{
	return T(Type_v4i8);
}

Type *SByte4::type()
{
	return T(Type_v4i8);
}

RValue<Byte8> AddSat(RValue<Byte8> x, RValue<Byte8> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::paddusb(x, y);
#else
	return As<Byte8>(V(lowerPUADDSAT(V(x.value()), V(y.value()))));
#endif
}

RValue<Byte8> SubSat(RValue<Byte8> x, RValue<Byte8> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::psubusb(x, y);
#else
	return As<Byte8>(V(lowerPUSUBSAT(V(x.value()), V(y.value()))));
#endif
}

RValue<Int> SignMask(RValue<Byte8> x)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::pmovmskb(x);
#else
	return As<Int>(V(lowerSignMask(V(x.value()), T(Int::type()))));
#endif
}

//	RValue<Byte8> CmpGT(RValue<Byte8> x, RValue<Byte8> y)
//	{
//#if defined(__i386__) || defined(__x86_64__)
//		return x86::pcmpgtb(x, y);   // FIXME: Signedness
//#else
//		return As<Byte8>(V(lowerPCMP(llvm::ICmpInst::ICMP_SGT, V(x.value()), V(y.value()), T(Byte8::type()))));
//#endif
//	}

RValue<Byte8> CmpEQ(RValue<Byte8> x, RValue<Byte8> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::pcmpeqb(x, y);
#else
	return As<Byte8>(V(lowerPCMP(llvm::ICmpInst::ICMP_EQ, V(x.value()), V(y.value()), T(Byte8::type()))));
#endif
}

Type *Byte8::type()
{
	return T(Type_v8i8);
}

RValue<SByte8> AddSat(RValue<SByte8> x, RValue<SByte8> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::paddsb(x, y);
#else
	return As<SByte8>(V(lowerPSADDSAT(V(x.value()), V(y.value()))));
#endif
}

RValue<SByte8> SubSat(RValue<SByte8> x, RValue<SByte8> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::psubsb(x, y);
#else
	return As<SByte8>(V(lowerPSSUBSAT(V(x.value()), V(y.value()))));
#endif
}

RValue<Int> SignMask(RValue<SByte8> x)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::pmovmskb(As<Byte8>(x));
#else
	return As<Int>(V(lowerSignMask(V(x.value()), T(Int::type()))));
#endif
}

RValue<Byte8> CmpGT(RValue<SByte8> x, RValue<SByte8> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::pcmpgtb(x, y);
#else
	return As<Byte8>(V(lowerPCMP(llvm::ICmpInst::ICMP_SGT, V(x.value()), V(y.value()), T(Byte8::type()))));
#endif
}

RValue<Byte8> CmpEQ(RValue<SByte8> x, RValue<SByte8> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::pcmpeqb(As<Byte8>(x), As<Byte8>(y));
#else
	return As<Byte8>(V(lowerPCMP(llvm::ICmpInst::ICMP_EQ, V(x.value()), V(y.value()), T(Byte8::type()))));
#endif
}

Type *SByte8::type()
{
	return T(Type_v8i8);
}

Type *Byte16::type()
{
	return T(llvm::VectorType::get(T(Byte::type()), 16, false));
}

Type *SByte16::type()
{
	return T(llvm::VectorType::get(T(SByte::type()), 16, false));
}

Type *Short2::type()
{
	return T(Type_v2i16);
}

Type *UShort2::type()
{
	return T(Type_v2i16);
}

Short4::Short4(RValue<Int4> cast)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	std::vector<int> select = { 0, 2, 4, 6, 0, 2, 4, 6 };
	Value *short8 = Nucleus::createBitCast(cast.value(), Short8::type());

	Value *packed = Nucleus::createShuffleVector(short8, short8, select);
	Value *short4 = As<Short4>(Int2(As<Int4>(packed))).value();

	storeValue(short4);
}

//	Short4::Short4(RValue<Float> cast)
//	{
//	}

Short4::Short4(RValue<Float4> cast)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Int4 v4i32 = Int4(cast);
#if defined(__i386__) || defined(__x86_64__)
	v4i32 = As<Int4>(x86::packssdw(v4i32, v4i32));
#else
	Value *v = v4i32.loadValue();
	v4i32 = As<Int4>(V(lowerPack(V(v), V(v), true)));
#endif

	storeValue(As<Short4>(Int2(v4i32)).value());
}

RValue<Short4> operator<<(RValue<Short4> lhs, unsigned char rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	//	return RValue<Short4>(Nucleus::createShl(lhs.value(), rhs.value()));

	return x86::psllw(lhs, rhs);
#else
	return As<Short4>(V(lowerVectorShl(V(lhs.value()), rhs)));
#endif
}

RValue<Short4> operator>>(RValue<Short4> lhs, unsigned char rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::psraw(lhs, rhs);
#else
	return As<Short4>(V(lowerVectorAShr(V(lhs.value()), rhs)));
#endif
}

RValue<Short4> Max(RValue<Short4> x, RValue<Short4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::pmaxsw(x, y);
#else
	return RValue<Short4>(V(lowerPMINMAX(V(x.value()), V(y.value()), llvm::ICmpInst::ICMP_SGT)));
#endif
}

RValue<Short4> Min(RValue<Short4> x, RValue<Short4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::pminsw(x, y);
#else
	return RValue<Short4>(V(lowerPMINMAX(V(x.value()), V(y.value()), llvm::ICmpInst::ICMP_SLT)));
#endif
}

RValue<Short4> AddSat(RValue<Short4> x, RValue<Short4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::paddsw(x, y);
#else
	return As<Short4>(V(lowerPSADDSAT(V(x.value()), V(y.value()))));
#endif
}

RValue<Short4> SubSat(RValue<Short4> x, RValue<Short4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::psubsw(x, y);
#else
	return As<Short4>(V(lowerPSSUBSAT(V(x.value()), V(y.value()))));
#endif
}

RValue<Short4> MulHigh(RValue<Short4> x, RValue<Short4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::pmulhw(x, y);
#else
	return As<Short4>(V(lowerMulHigh(V(x.value()), V(y.value()), true)));
#endif
}

RValue<Int2> MulAdd(RValue<Short4> x, RValue<Short4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::pmaddwd(x, y);
#else
	return As<Int2>(V(lowerMulAdd(V(x.value()), V(y.value()))));
#endif
}

RValue<SByte8> PackSigned(RValue<Short4> x, RValue<Short4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	auto result = x86::packsswb(x, y);
#else
	auto result = V(lowerPack(V(x.value()), V(y.value()), true));
#endif
	return As<SByte8>(Swizzle(As<Int4>(result), 0x0202));
}

RValue<Byte8> PackUnsigned(RValue<Short4> x, RValue<Short4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	auto result = x86::packuswb(x, y);
#else
	auto result = V(lowerPack(V(x.value()), V(y.value()), false));
#endif
	return As<Byte8>(Swizzle(As<Int4>(result), 0x0202));
}

RValue<Short4> CmpGT(RValue<Short4> x, RValue<Short4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::pcmpgtw(x, y);
#else
	return As<Short4>(V(lowerPCMP(llvm::ICmpInst::ICMP_SGT, V(x.value()), V(y.value()), T(Short4::type()))));
#endif
}

RValue<Short4> CmpEQ(RValue<Short4> x, RValue<Short4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::pcmpeqw(x, y);
#else
	return As<Short4>(V(lowerPCMP(llvm::ICmpInst::ICMP_EQ, V(x.value()), V(y.value()), T(Short4::type()))));
#endif
}

Type *Short4::type()
{
	return T(Type_v4i16);
}

UShort4::UShort4(RValue<Float4> cast, bool saturate)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	if(saturate)
	{
#if defined(__i386__) || defined(__x86_64__)
		if(CPUID::supportsSSE4_1())
		{
			Int4 int4(Min(cast, Float4(0xFFFF)));  // packusdw takes care of 0x0000 saturation
			*this = As<Short4>(PackUnsigned(int4, int4));
		}
		else
#endif
		{
			*this = Short4(Int4(Max(Min(cast, Float4(0xFFFF)), Float4(0x0000))));
		}
	}
	else
	{
		*this = Short4(Int4(cast));
	}
}

RValue<UShort4> operator<<(RValue<UShort4> lhs, unsigned char rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	//	return RValue<Short4>(Nucleus::createShl(lhs.value(), rhs.value()));

	return As<UShort4>(x86::psllw(As<Short4>(lhs), rhs));
#else
	return As<UShort4>(V(lowerVectorShl(V(lhs.value()), rhs)));
#endif
}

RValue<UShort4> operator>>(RValue<UShort4> lhs, unsigned char rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	//	return RValue<Short4>(Nucleus::createLShr(lhs.value(), rhs.value()));

	return x86::psrlw(lhs, rhs);
#else
	return As<UShort4>(V(lowerVectorLShr(V(lhs.value()), rhs)));
#endif
}

RValue<UShort4> Max(RValue<UShort4> x, RValue<UShort4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<UShort4>(Max(As<Short4>(x) - Short4(0x8000u, 0x8000u, 0x8000u, 0x8000u), As<Short4>(y) - Short4(0x8000u, 0x8000u, 0x8000u, 0x8000u)) + Short4(0x8000u, 0x8000u, 0x8000u, 0x8000u));
}

RValue<UShort4> Min(RValue<UShort4> x, RValue<UShort4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<UShort4>(Min(As<Short4>(x) - Short4(0x8000u, 0x8000u, 0x8000u, 0x8000u), As<Short4>(y) - Short4(0x8000u, 0x8000u, 0x8000u, 0x8000u)) + Short4(0x8000u, 0x8000u, 0x8000u, 0x8000u));
}

RValue<UShort4> AddSat(RValue<UShort4> x, RValue<UShort4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::paddusw(x, y);
#else
	return As<UShort4>(V(lowerPUADDSAT(V(x.value()), V(y.value()))));
#endif
}

RValue<UShort4> SubSat(RValue<UShort4> x, RValue<UShort4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::psubusw(x, y);
#else
	return As<UShort4>(V(lowerPUSUBSAT(V(x.value()), V(y.value()))));
#endif
}

RValue<UShort4> MulHigh(RValue<UShort4> x, RValue<UShort4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::pmulhuw(x, y);
#else
	return As<UShort4>(V(lowerMulHigh(V(x.value()), V(y.value()), false)));
#endif
}

RValue<UShort4> Average(RValue<UShort4> x, RValue<UShort4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::pavgw(x, y);
#else
	return As<UShort4>(V(lowerPAVG(V(x.value()), V(y.value()))));
#endif
}

Type *UShort4::type()
{
	return T(Type_v4i16);
}

RValue<Short8> operator<<(RValue<Short8> lhs, unsigned char rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::psllw(lhs, rhs);
#else
	return As<Short8>(V(lowerVectorShl(V(lhs.value()), rhs)));
#endif
}

RValue<Short8> operator>>(RValue<Short8> lhs, unsigned char rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::psraw(lhs, rhs);
#else
	return As<Short8>(V(lowerVectorAShr(V(lhs.value()), rhs)));
#endif
}

RValue<Int4> MulAdd(RValue<Short8> x, RValue<Short8> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::pmaddwd(x, y);
#else
	return As<Int4>(V(lowerMulAdd(V(x.value()), V(y.value()))));
#endif
}

RValue<Short8> MulHigh(RValue<Short8> x, RValue<Short8> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::pmulhw(x, y);
#else
	return As<Short8>(V(lowerMulHigh(V(x.value()), V(y.value()), true)));
#endif
}

Type *Short8::type()
{
	return T(llvm::VectorType::get(T(Short::type()), 8, false));
}

RValue<UShort8> operator<<(RValue<UShort8> lhs, unsigned char rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return As<UShort8>(x86::psllw(As<Short8>(lhs), rhs));
#else
	return As<UShort8>(V(lowerVectorShl(V(lhs.value()), rhs)));
#endif
}

RValue<UShort8> operator>>(RValue<UShort8> lhs, unsigned char rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::psrlw(lhs, rhs);  // FIXME: Fallback required
#else
	return As<UShort8>(V(lowerVectorLShr(V(lhs.value()), rhs)));
#endif
}

RValue<UShort8> MulHigh(RValue<UShort8> x, RValue<UShort8> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::pmulhuw(x, y);
#else
	return As<UShort8>(V(lowerMulHigh(V(x.value()), V(y.value()), false)));
#endif
}

Type *UShort8::type()
{
	return T(llvm::VectorType::get(T(UShort::type()), 8, false));
}

RValue<Int> operator++(Int &val, int)  // Post-increment
{
	RR_DEBUG_INFO_UPDATE_LOC();
	RValue<Int> res = val;

	Value *inc = Nucleus::createAdd(res.value(), Nucleus::createConstantInt(1));
	val.storeValue(inc);

	return res;
}

const Int &operator++(Int &val)  // Pre-increment
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *inc = Nucleus::createAdd(val.loadValue(), Nucleus::createConstantInt(1));
	val.storeValue(inc);

	return val;
}

RValue<Int> operator--(Int &val, int)  // Post-decrement
{
	RR_DEBUG_INFO_UPDATE_LOC();
	RValue<Int> res = val;

	Value *inc = Nucleus::createSub(res.value(), Nucleus::createConstantInt(1));
	val.storeValue(inc);

	return res;
}

const Int &operator--(Int &val)  // Pre-decrement
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *inc = Nucleus::createSub(val.loadValue(), Nucleus::createConstantInt(1));
	val.storeValue(inc);

	return val;
}

RValue<Int> RoundInt(RValue<Float> cast)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::cvtss2si(cast);
#else
	return RValue<Int>(V(lowerRoundInt(V(cast.value()), T(Int::type()))));
#endif
}

Type *Int::type()
{
	return T(llvm::Type::getInt32Ty(*jit->context));
}

Type *Long::type()
{
	return T(llvm::Type::getInt64Ty(*jit->context));
}

UInt::UInt(RValue<Float> cast)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *integer = Nucleus::createFPToUI(cast.value(), UInt::type());
	storeValue(integer);
}

RValue<UInt> operator++(UInt &val, int)  // Post-increment
{
	RR_DEBUG_INFO_UPDATE_LOC();
	RValue<UInt> res = val;

	Value *inc = Nucleus::createAdd(res.value(), Nucleus::createConstantInt(1));
	val.storeValue(inc);

	return res;
}

const UInt &operator++(UInt &val)  // Pre-increment
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *inc = Nucleus::createAdd(val.loadValue(), Nucleus::createConstantInt(1));
	val.storeValue(inc);

	return val;
}

RValue<UInt> operator--(UInt &val, int)  // Post-decrement
{
	RR_DEBUG_INFO_UPDATE_LOC();
	RValue<UInt> res = val;

	Value *inc = Nucleus::createSub(res.value(), Nucleus::createConstantInt(1));
	val.storeValue(inc);

	return res;
}

const UInt &operator--(UInt &val)  // Pre-decrement
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *inc = Nucleus::createSub(val.loadValue(), Nucleus::createConstantInt(1));
	val.storeValue(inc);

	return val;
}

//	RValue<UInt> RoundUInt(RValue<Float> cast)
//	{
//#if defined(__i386__) || defined(__x86_64__)
//		return x86::cvtss2si(val);   // FIXME: Unsigned
//#else
//		return IfThenElse(cast > 0.0f, Int(cast + 0.5f), Int(cast - 0.5f));
//#endif
//	}

Type *UInt::type()
{
	return T(llvm::Type::getInt32Ty(*jit->context));
}

//	Int2::Int2(RValue<Int> cast)
//	{
//		Value *extend = Nucleus::createZExt(cast.value(), Long::type());
//		Value *vector = Nucleus::createBitCast(extend, Int2::type());
//
//		int shuffle[2] = {0, 0};
//		Value *replicate = Nucleus::createShuffleVector(vector, vector, shuffle);
//
//		storeValue(replicate);
//	}

RValue<Int2> operator<<(RValue<Int2> lhs, unsigned char rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	//	return RValue<Int2>(Nucleus::createShl(lhs.value(), rhs.value()));

	return x86::pslld(lhs, rhs);
#else
	return As<Int2>(V(lowerVectorShl(V(lhs.value()), rhs)));
#endif
}

RValue<Int2> operator>>(RValue<Int2> lhs, unsigned char rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	//	return RValue<Int2>(Nucleus::createAShr(lhs.value(), rhs.value()));

	return x86::psrad(lhs, rhs);
#else
	return As<Int2>(V(lowerVectorAShr(V(lhs.value()), rhs)));
#endif
}

Type *Int2::type()
{
	return T(Type_v2i32);
}

RValue<UInt2> operator<<(RValue<UInt2> lhs, unsigned char rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	//	return RValue<UInt2>(Nucleus::createShl(lhs.value(), rhs.value()));

	return As<UInt2>(x86::pslld(As<Int2>(lhs), rhs));
#else
	return As<UInt2>(V(lowerVectorShl(V(lhs.value()), rhs)));
#endif
}

RValue<UInt2> operator>>(RValue<UInt2> lhs, unsigned char rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	//	return RValue<UInt2>(Nucleus::createLShr(lhs.value(), rhs.value()));

	return x86::psrld(lhs, rhs);
#else
	return As<UInt2>(V(lowerVectorLShr(V(lhs.value()), rhs)));
#endif
}

Type *UInt2::type()
{
	return T(Type_v2i32);
}

Int4::Int4(RValue<Byte4> cast)
    : XYZW(this)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	std::vector<int> swizzle = { 0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23 };
	Value *a = Nucleus::createBitCast(cast.value(), Byte16::type());
	Value *b = Nucleus::createShuffleVector(a, Nucleus::createNullValue(Byte16::type()), swizzle);

	std::vector<int> swizzle2 = { 0, 8, 1, 9, 2, 10, 3, 11 };
	Value *c = Nucleus::createBitCast(b, Short8::type());
	Value *d = Nucleus::createShuffleVector(c, Nucleus::createNullValue(Short8::type()), swizzle2);

	*this = As<Int4>(d);
}

Int4::Int4(RValue<SByte4> cast)
    : XYZW(this)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	std::vector<int> swizzle = { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7 };
	Value *a = Nucleus::createBitCast(cast.value(), Byte16::type());
	Value *b = Nucleus::createShuffleVector(a, a, swizzle);

	std::vector<int> swizzle2 = { 0, 0, 1, 1, 2, 2, 3, 3 };
	Value *c = Nucleus::createBitCast(b, Short8::type());
	Value *d = Nucleus::createShuffleVector(c, c, swizzle2);

	*this = As<Int4>(d) >> 24;
}

Int4::Int4(RValue<Short4> cast)
    : XYZW(this)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	std::vector<int> swizzle = { 0, 0, 1, 1, 2, 2, 3, 3 };
	Value *c = Nucleus::createShuffleVector(cast.value(), cast.value(), swizzle);
	*this = As<Int4>(c) >> 16;
}

Int4::Int4(RValue<UShort4> cast)
    : XYZW(this)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	std::vector<int> swizzle = { 0, 8, 1, 9, 2, 10, 3, 11 };
	Value *c = Nucleus::createShuffleVector(cast.value(), Short8(0, 0, 0, 0, 0, 0, 0, 0).loadValue(), swizzle);
	*this = As<Int4>(c);
}

Int4::Int4(RValue<Int> rhs)
    : XYZW(this)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *vector = loadValue();
	Value *insert = Nucleus::createInsertElement(vector, rhs.value(), 0);

	std::vector<int> swizzle = { 0, 0, 0, 0 };
	Value *replicate = Nucleus::createShuffleVector(insert, insert, swizzle);

	storeValue(replicate);
}

RValue<Int4> operator<<(RValue<Int4> lhs, unsigned char rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::pslld(lhs, rhs);
#else
	return As<Int4>(V(lowerVectorShl(V(lhs.value()), rhs)));
#endif
}

RValue<Int4> operator>>(RValue<Int4> lhs, unsigned char rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::psrad(lhs, rhs);
#else
	return As<Int4>(V(lowerVectorAShr(V(lhs.value()), rhs)));
#endif
}

RValue<Int4> CmpEQ(RValue<Int4> x, RValue<Int4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<Int4>(Nucleus::createSExt(Nucleus::createICmpEQ(x.value(), y.value()), Int4::type()));
}

RValue<Int4> CmpLT(RValue<Int4> x, RValue<Int4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<Int4>(Nucleus::createSExt(Nucleus::createICmpSLT(x.value(), y.value()), Int4::type()));
}

RValue<Int4> CmpLE(RValue<Int4> x, RValue<Int4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<Int4>(Nucleus::createSExt(Nucleus::createICmpSLE(x.value(), y.value()), Int4::type()));
}

RValue<Int4> CmpNEQ(RValue<Int4> x, RValue<Int4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<Int4>(Nucleus::createSExt(Nucleus::createICmpNE(x.value(), y.value()), Int4::type()));
}

RValue<Int4> CmpNLT(RValue<Int4> x, RValue<Int4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<Int4>(Nucleus::createSExt(Nucleus::createICmpSGE(x.value(), y.value()), Int4::type()));
}

RValue<Int4> CmpNLE(RValue<Int4> x, RValue<Int4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<Int4>(Nucleus::createSExt(Nucleus::createICmpSGT(x.value(), y.value()), Int4::type()));
}

RValue<Int4> Abs(RValue<Int4> x)
{
#if LLVM_VERSION_MAJOR >= 12
	auto func = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::abs, { V(x.value())->getType() });
	return RValue<Int4>(V(jit->builder->CreateCall(func, { V(x.value()), llvm::ConstantInt::getFalse(*jit->context) })));
#else
	auto negative = x >> 31;
	return (x ^ negative) - negative;
#endif
}

RValue<Int4> Max(RValue<Int4> x, RValue<Int4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	if(CPUID::supportsSSE4_1())
	{
		return x86::pmaxsd(x, y);
	}
	else
#endif
	{
		RValue<Int4> greater = CmpNLE(x, y);
		return (x & greater) | (y & ~greater);
	}
}

RValue<Int4> Min(RValue<Int4> x, RValue<Int4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	if(CPUID::supportsSSE4_1())
	{
		return x86::pminsd(x, y);
	}
	else
#endif
	{
		RValue<Int4> less = CmpLT(x, y);
		return (x & less) | (y & ~less);
	}
}

RValue<Int4> RoundInt(RValue<Float4> cast)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if(defined(__i386__) || defined(__x86_64__)) && !__has_feature(memory_sanitizer)
	return x86::cvtps2dq(cast);
#else
	return As<Int4>(V(lowerRoundInt(V(cast.value()), T(Int4::type()))));
#endif
}

RValue<Int4> RoundIntClamped(RValue<Float4> cast)
{
	RR_DEBUG_INFO_UPDATE_LOC();

// TODO(b/165000222): Check if fptosi_sat produces optimal code for x86 and ARM.
#if(defined(__i386__) || defined(__x86_64__)) && !__has_feature(memory_sanitizer)
	// cvtps2dq produces 0x80000000, a negative value, for input larger than
	// 2147483520.0, so clamp to 2147483520. Values less than -2147483520.0
	// saturate to 0x80000000.
	return x86::cvtps2dq(Min(cast, Float4(0x7FFFFF80)));
#elif defined(__arm__) || defined(__aarch64__)
	// ARM saturates to the largest positive or negative integer. Unit tests
	// verify that lowerRoundInt() behaves as desired.
	return As<Int4>(V(lowerRoundInt(V(cast.value()), T(Int4::type()))));
#elif LLVM_VERSION_MAJOR >= 14
	llvm::Value *rounded = lowerRound(V(cast.value()));
	llvm::Function *fptosi_sat = llvm::Intrinsic::getDeclaration(
	    jit->module.get(), llvm::Intrinsic::fptosi_sat, { T(Int4::type()), T(Float4::type()) });
	return RValue<Int4>(V(jit->builder->CreateCall(fptosi_sat, { rounded })));
#else
	RValue<Float4> clamped = Max(Min(cast, Float4(0x7FFFFF80)), Float4(static_cast<int>(0x80000000)));
	return As<Int4>(V(lowerRoundInt(V(clamped.value()), T(Int4::type()))));
#endif
}

RValue<Int4> MulHigh(RValue<Int4> x, RValue<Int4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	// TODO: For x86, build an intrinsics version of this which uses shuffles + pmuludq.
	return As<Int4>(V(lowerMulHigh(V(x.value()), V(y.value()), true)));
}

RValue<UInt4> MulHigh(RValue<UInt4> x, RValue<UInt4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	// TODO: For x86, build an intrinsics version of this which uses shuffles + pmuludq.
	return As<UInt4>(V(lowerMulHigh(V(x.value()), V(y.value()), false)));
}

RValue<Short8> PackSigned(RValue<Int4> x, RValue<Int4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::packssdw(x, y);
#else
	return As<Short8>(V(lowerPack(V(x.value()), V(y.value()), true)));
#endif
}

RValue<UShort8> PackUnsigned(RValue<Int4> x, RValue<Int4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::packusdw(x, y);
#else
	return As<UShort8>(V(lowerPack(V(x.value()), V(y.value()), false)));
#endif
}

RValue<Int> SignMask(RValue<Int4> x)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::movmskps(As<Float4>(x));
#else
	return As<Int>(V(lowerSignMask(V(x.value()), T(Int::type()))));
#endif
}

Type *Int4::type()
{
	return T(llvm::VectorType::get(T(Int::type()), 4, false));
}

UInt4::UInt4(RValue<Float4> cast)
    : XYZW(this)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *xyzw = Nucleus::createFPToUI(cast.value(), UInt4::type());
	storeValue(xyzw);
}

UInt4::UInt4(RValue<UInt> rhs)
    : XYZW(this)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *vector = loadValue();
	Value *insert = Nucleus::createInsertElement(vector, rhs.value(), 0);

	std::vector<int> swizzle = { 0, 0, 0, 0 };
	Value *replicate = Nucleus::createShuffleVector(insert, insert, swizzle);

	storeValue(replicate);
}

RValue<UInt4> operator<<(RValue<UInt4> lhs, unsigned char rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return As<UInt4>(x86::pslld(As<Int4>(lhs), rhs));
#else
	return As<UInt4>(V(lowerVectorShl(V(lhs.value()), rhs)));
#endif
}

RValue<UInt4> operator>>(RValue<UInt4> lhs, unsigned char rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::psrld(lhs, rhs);
#else
	return As<UInt4>(V(lowerVectorLShr(V(lhs.value()), rhs)));
#endif
}

RValue<UInt4> CmpEQ(RValue<UInt4> x, RValue<UInt4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<UInt4>(Nucleus::createSExt(Nucleus::createICmpEQ(x.value(), y.value()), Int4::type()));
}

RValue<UInt4> CmpLT(RValue<UInt4> x, RValue<UInt4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<UInt4>(Nucleus::createSExt(Nucleus::createICmpULT(x.value(), y.value()), Int4::type()));
}

RValue<UInt4> CmpLE(RValue<UInt4> x, RValue<UInt4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<UInt4>(Nucleus::createSExt(Nucleus::createICmpULE(x.value(), y.value()), Int4::type()));
}

RValue<UInt4> CmpNEQ(RValue<UInt4> x, RValue<UInt4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<UInt4>(Nucleus::createSExt(Nucleus::createICmpNE(x.value(), y.value()), Int4::type()));
}

RValue<UInt4> CmpNLT(RValue<UInt4> x, RValue<UInt4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<UInt4>(Nucleus::createSExt(Nucleus::createICmpUGE(x.value(), y.value()), Int4::type()));
}

RValue<UInt4> CmpNLE(RValue<UInt4> x, RValue<UInt4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<UInt4>(Nucleus::createSExt(Nucleus::createICmpUGT(x.value(), y.value()), Int4::type()));
}

RValue<UInt4> Max(RValue<UInt4> x, RValue<UInt4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	if(CPUID::supportsSSE4_1())
	{
		return x86::pmaxud(x, y);
	}
	else
#endif
	{
		RValue<UInt4> greater = CmpNLE(x, y);
		return (x & greater) | (y & ~greater);
	}
}

RValue<UInt4> Min(RValue<UInt4> x, RValue<UInt4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	if(CPUID::supportsSSE4_1())
	{
		return x86::pminud(x, y);
	}
	else
#endif
	{
		RValue<UInt4> less = CmpLT(x, y);
		return (x & less) | (y & ~less);
	}
}

Type *UInt4::type()
{
	return T(llvm::VectorType::get(T(UInt::type()), 4, false));
}

Type *Half::type()
{
	return T(llvm::Type::getInt16Ty(*jit->context));
}

bool HasRcpApprox()
{
#if defined(__i386__) || defined(__x86_64__)
	return true;
#else
	return false;
#endif
}

RValue<Float4> RcpApprox(RValue<Float4> x, bool exactAtPow2)
{
#if defined(__i386__) || defined(__x86_64__)
	if(exactAtPow2)
	{
		// rcpps uses a piecewise-linear approximation which minimizes the relative error
		// but is not exact at power-of-two values. Rectify by multiplying by the inverse.
		return x86::rcpps(x) * Float4(1.0f / _mm_cvtss_f32(_mm_rcp_ss(_mm_set_ps1(1.0f))));
	}
	return x86::rcpps(x);
#else
	UNREACHABLE("RValue<Float4> RcpApprox() not available on this platform");
	return { 0.0f };
#endif
}

RValue<Float> RcpApprox(RValue<Float> x, bool exactAtPow2)
{
#if defined(__i386__) || defined(__x86_64__)
	if(exactAtPow2)
	{
		// rcpss uses a piecewise-linear approximation which minimizes the relative error
		// but is not exact at power-of-two values. Rectify by multiplying by the inverse.
		return x86::rcpss(x) * Float(1.0f / _mm_cvtss_f32(_mm_rcp_ss(_mm_set_ps1(1.0f))));
	}
	return x86::rcpss(x);
#else
	UNREACHABLE("RValue<Float4> RcpApprox() not available on this platform");
	return { 0.0f };
#endif
}

bool HasRcpSqrtApprox()
{
#if defined(__i386__) || defined(__x86_64__)
	return true;
#else
	return false;
#endif
}

RValue<Float4> RcpSqrtApprox(RValue<Float4> x)
{
#if defined(__i386__) || defined(__x86_64__)
	return x86::rsqrtps(x);
#else
	UNREACHABLE("RValue<Float4> RcpSqrtApprox() not available on this platform");
	return { 0.0f };
#endif
}

RValue<Float> RcpSqrtApprox(RValue<Float> x)
{
#if defined(__i386__) || defined(__x86_64__)
	return x86::rsqrtss(x);
#else
	UNREACHABLE("RValue<Float4> RcpSqrtApprox() not available on this platform");
	return { 0.0f };
#endif
}

RValue<Float> Sqrt(RValue<Float> x)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::sqrtss(x);
#else
	return As<Float>(V(lowerSQRT(V(x.value()))));
#endif
}

RValue<Float> Round(RValue<Float> x)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	if(CPUID::supportsSSE4_1())
	{
		return x86::roundss(x, 0);
	}
	else
	{
		return Float4(Round(Float4(x))).x;
	}
#else
	return RValue<Float>(V(lowerRound(V(x.value()))));
#endif
}

RValue<Float> Trunc(RValue<Float> x)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	if(CPUID::supportsSSE4_1())
	{
		return x86::roundss(x, 3);
	}
	else
	{
		return Float(Int(x));  // Rounded toward zero
	}
#else
	return RValue<Float>(V(lowerTrunc(V(x.value()))));
#endif
}

RValue<Float> Frac(RValue<Float> x)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	if(CPUID::supportsSSE4_1())
	{
		return x - x86::floorss(x);
	}
	else
	{
		return Float4(Frac(Float4(x))).x;
	}
#else
	// x - floor(x) can be 1.0 for very small negative x.
	// Clamp against the value just below 1.0.
	return Min(x - Floor(x), As<Float>(Int(0x3F7FFFFF)));
#endif
}

RValue<Float> Floor(RValue<Float> x)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	if(CPUID::supportsSSE4_1())
	{
		return x86::floorss(x);
	}
	else
	{
		return Float4(Floor(Float4(x))).x;
	}
#else
	return RValue<Float>(V(lowerFloor(V(x.value()))));
#endif
}

RValue<Float> Ceil(RValue<Float> x)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	if(CPUID::supportsSSE4_1())
	{
		return x86::ceilss(x);
	}
	else
#endif
	{
		return Float4(Ceil(Float4(x))).x;
	}
}

Type *Float::type()
{
	return T(llvm::Type::getFloatTy(*jit->context));
}

Type *Float2::type()
{
	return T(Type_v2f32);
}

Float4::Float4(RValue<Float> rhs)
    : XYZW(this)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *vector = loadValue();
	Value *insert = Nucleus::createInsertElement(vector, rhs.value(), 0);

	std::vector<int> swizzle = { 0, 0, 0, 0 };
	Value *replicate = Nucleus::createShuffleVector(insert, insert, swizzle);

	storeValue(replicate);
}

RValue<Float4> MulAdd(RValue<Float4> x, RValue<Float4> y, RValue<Float4> z)
{
	auto func = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::fmuladd, { T(Float4::type()) });
	return RValue<Float4>(V(jit->builder->CreateCall(func, { V(x.value()), V(y.value()), V(z.value()) })));
}

RValue<Float4> FMA(RValue<Float4> x, RValue<Float4> y, RValue<Float4> z)
{
	auto func = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::fma, { T(Float4::type()) });
	return RValue<Float4>(V(jit->builder->CreateCall(func, { V(x.value()), V(y.value()), V(z.value()) })));
}

RValue<Float4> Abs(RValue<Float4> x)
{
	auto func = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::fabs, { V(x.value())->getType() });
	return RValue<Float4>(V(jit->builder->CreateCall(func, V(x.value()))));
}

RValue<Float4> Max(RValue<Float4> x, RValue<Float4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::maxps(x, y);
#else
	return As<Float4>(V(lowerPFMINMAX(V(x.value()), V(y.value()), llvm::FCmpInst::FCMP_OGT)));
#endif
}

RValue<Float4> Min(RValue<Float4> x, RValue<Float4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::minps(x, y);
#else
	return As<Float4>(V(lowerPFMINMAX(V(x.value()), V(y.value()), llvm::FCmpInst::FCMP_OLT)));
#endif
}

RValue<Float4> Sqrt(RValue<Float4> x)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::sqrtps(x);
#else
	return As<Float4>(V(lowerSQRT(V(x.value()))));
#endif
}

RValue<Int> SignMask(RValue<Float4> x)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if defined(__i386__) || defined(__x86_64__)
	return x86::movmskps(x);
#else
	return As<Int>(V(lowerFPSignMask(V(x.value()), T(Int::type()))));
#endif
}

RValue<Int4> CmpEQ(RValue<Float4> x, RValue<Float4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	//	return As<Int4>(x86::cmpeqps(x, y));
	return RValue<Int4>(Nucleus::createSExt(Nucleus::createFCmpOEQ(x.value(), y.value()), Int4::type()));
}

RValue<Int4> CmpLT(RValue<Float4> x, RValue<Float4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	//	return As<Int4>(x86::cmpltps(x, y));
	return RValue<Int4>(Nucleus::createSExt(Nucleus::createFCmpOLT(x.value(), y.value()), Int4::type()));
}

RValue<Int4> CmpLE(RValue<Float4> x, RValue<Float4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	//	return As<Int4>(x86::cmpleps(x, y));
	return RValue<Int4>(Nucleus::createSExt(Nucleus::createFCmpOLE(x.value(), y.value()), Int4::type()));
}

RValue<Int4> CmpNEQ(RValue<Float4> x, RValue<Float4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	//	return As<Int4>(x86::cmpneqps(x, y));
	return RValue<Int4>(Nucleus::createSExt(Nucleus::createFCmpONE(x.value(), y.value()), Int4::type()));
}

RValue<Int4> CmpNLT(RValue<Float4> x, RValue<Float4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	//	return As<Int4>(x86::cmpnltps(x, y));
	return RValue<Int4>(Nucleus::createSExt(Nucleus::createFCmpOGE(x.value(), y.value()), Int4::type()));
}

RValue<Int4> CmpNLE(RValue<Float4> x, RValue<Float4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	//	return As<Int4>(x86::cmpnleps(x, y));
	return RValue<Int4>(Nucleus::createSExt(Nucleus::createFCmpOGT(x.value(), y.value()), Int4::type()));
}

RValue<Int4> CmpUEQ(RValue<Float4> x, RValue<Float4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<Int4>(Nucleus::createSExt(Nucleus::createFCmpUEQ(x.value(), y.value()), Int4::type()));
}

RValue<Int4> CmpULT(RValue<Float4> x, RValue<Float4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<Int4>(Nucleus::createSExt(Nucleus::createFCmpULT(x.value(), y.value()), Int4::type()));
}

RValue<Int4> CmpULE(RValue<Float4> x, RValue<Float4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<Int4>(Nucleus::createSExt(Nucleus::createFCmpULE(x.value(), y.value()), Int4::type()));
}

RValue<Int4> CmpUNEQ(RValue<Float4> x, RValue<Float4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<Int4>(Nucleus::createSExt(Nucleus::createFCmpUNE(x.value(), y.value()), Int4::type()));
}

RValue<Int4> CmpUNLT(RValue<Float4> x, RValue<Float4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<Int4>(Nucleus::createSExt(Nucleus::createFCmpUGE(x.value(), y.value()), Int4::type()));
}

RValue<Int4> CmpUNLE(RValue<Float4> x, RValue<Float4> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<Int4>(Nucleus::createSExt(Nucleus::createFCmpUGT(x.value(), y.value()), Int4::type()));
}

RValue<Float4> Round(RValue<Float4> x)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if(defined(__i386__) || defined(__x86_64__)) && !__has_feature(memory_sanitizer)
	if(CPUID::supportsSSE4_1())
	{
		return x86::roundps(x, 0);
	}
	else
	{
		return Float4(RoundInt(x));
	}
#else
	return RValue<Float4>(V(lowerRound(V(x.value()))));
#endif
}

RValue<Float4> Trunc(RValue<Float4> x)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if(defined(__i386__) || defined(__x86_64__)) && !__has_feature(memory_sanitizer)
	if(CPUID::supportsSSE4_1())
	{
		return x86::roundps(x, 3);
	}
	else
	{
		return Float4(Int4(x));
	}
#else
	return RValue<Float4>(V(lowerTrunc(V(x.value()))));
#endif
}

RValue<Float4> Frac(RValue<Float4> x)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Float4 frc;

#if(defined(__i386__) || defined(__x86_64__)) && !__has_feature(memory_sanitizer)
	if(CPUID::supportsSSE4_1())
	{
		frc = x - x86::floorps(x);
	}
	else
	{
		frc = x - Float4(Int4(x));  // Signed fractional part.

		frc += As<Float4>(As<Int4>(CmpNLE(Float4(0.0f), frc)) & As<Int4>(Float4(1.0f)));  // Add 1.0 if negative.
	}
#else
	frc = x - Floor(x);
#endif

	// x - floor(x) can be 1.0 for very small negative x.
	// Clamp against the value just below 1.0.
	return Min(frc, As<Float4>(Int4(0x3F7FFFFF)));
}

RValue<Float4> Floor(RValue<Float4> x)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if(defined(__i386__) || defined(__x86_64__)) && !__has_feature(memory_sanitizer)
	if(CPUID::supportsSSE4_1())
	{
		return x86::floorps(x);
	}
	else
	{
		return x - Frac(x);
	}
#else
	return RValue<Float4>(V(lowerFloor(V(x.value()))));
#endif
}

RValue<Float4> Ceil(RValue<Float4> x)
{
	RR_DEBUG_INFO_UPDATE_LOC();
#if(defined(__i386__) || defined(__x86_64__)) && !__has_feature(memory_sanitizer)
	if(CPUID::supportsSSE4_1())
	{
		return x86::ceilps(x);
	}
	else
#endif
	{
		return -Floor(-x);
	}
}

RValue<UInt> Ctlz(RValue<UInt> v, bool isZeroUndef)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	auto func = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::ctlz, { T(UInt::type()) });
	return RValue<UInt>(V(jit->builder->CreateCall(func, { V(v.value()),
	                                                       isZeroUndef ? llvm::ConstantInt::getTrue(*jit->context) : llvm::ConstantInt::getFalse(*jit->context) })));
}

RValue<UInt4> Ctlz(RValue<UInt4> v, bool isZeroUndef)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	auto func = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::ctlz, { T(UInt4::type()) });
	return RValue<UInt4>(V(jit->builder->CreateCall(func, { V(v.value()),
	                                                        isZeroUndef ? llvm::ConstantInt::getTrue(*jit->context) : llvm::ConstantInt::getFalse(*jit->context) })));
}

RValue<UInt> Cttz(RValue<UInt> v, bool isZeroUndef)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	auto func = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::cttz, { T(UInt::type()) });
	return RValue<UInt>(V(jit->builder->CreateCall(func, { V(v.value()),
	                                                       isZeroUndef ? llvm::ConstantInt::getTrue(*jit->context) : llvm::ConstantInt::getFalse(*jit->context) })));
}

RValue<UInt4> Cttz(RValue<UInt4> v, bool isZeroUndef)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	auto func = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::cttz, { T(UInt4::type()) });
	return RValue<UInt4>(V(jit->builder->CreateCall(func, { V(v.value()),
	                                                        isZeroUndef ? llvm::ConstantInt::getTrue(*jit->context) : llvm::ConstantInt::getFalse(*jit->context) })));
}

RValue<Int> MinAtomic(RValue<Pointer<Int>> x, RValue<Int> y, std::memory_order memoryOrder)
{
	return RValue<Int>(Nucleus::createAtomicMin(x.value(), y.value(), memoryOrder));
}

RValue<UInt> MinAtomic(RValue<Pointer<UInt>> x, RValue<UInt> y, std::memory_order memoryOrder)
{
	return RValue<UInt>(Nucleus::createAtomicUMin(x.value(), y.value(), memoryOrder));
}

RValue<Int> MaxAtomic(RValue<Pointer<Int>> x, RValue<Int> y, std::memory_order memoryOrder)
{
	return RValue<Int>(Nucleus::createAtomicMax(x.value(), y.value(), memoryOrder));
}

RValue<UInt> MaxAtomic(RValue<Pointer<UInt>> x, RValue<UInt> y, std::memory_order memoryOrder)
{
	return RValue<UInt>(Nucleus::createAtomicUMax(x.value(), y.value(), memoryOrder));
}

Type *Float4::type()
{
	return T(llvm::VectorType::get(T(Float::type()), 4, false));
}

RValue<Long> Ticks()
{
	RR_DEBUG_INFO_UPDATE_LOC();
	llvm::Function *rdtsc = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::readcyclecounter);

	return RValue<Long>(V(jit->builder->CreateCall(rdtsc)));
}

RValue<Pointer<Byte>> ConstantPointer(const void *ptr)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	// Note: this should work for 32-bit pointers as well because 'inttoptr'
	// is defined to truncate (and zero extend) if necessary.
	auto ptrAsInt = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*jit->context), reinterpret_cast<uintptr_t>(ptr));
	return RValue<Pointer<Byte>>(V(jit->builder->CreateIntToPtr(ptrAsInt, T(Pointer<Byte>::type()))));
}

RValue<Pointer<Byte>> ConstantData(const void *data, size_t size)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	auto str = ::std::string(reinterpret_cast<const char *>(data), size);
	auto ptr = jit->builder->CreateGlobalStringPtr(str);
	return RValue<Pointer<Byte>>(V(ptr));
}

Value *Call(RValue<Pointer<Byte>> fptr, Type *retTy, std::initializer_list<Value *> args, std::initializer_list<Type *> argTys)
{
	// If this is a MemorySanitizer build, but Reactor routine instrumentation is not enabled,
	// mark all call arguments as initialized by calling __msan_unpoison_param().
	if(__has_feature(memory_sanitizer) && !jit->msanInstrumentation)
	{
		// void __msan_unpoison_param(size_t n)
		auto voidTy = llvm::Type::getVoidTy(*jit->context);
		auto sizetTy = llvm::IntegerType::get(*jit->context, sizeof(size_t) * 8);
		auto funcTy = llvm::FunctionType::get(voidTy, { sizetTy }, false);
		auto func = jit->module->getOrInsertFunction("__msan_unpoison_param", funcTy);

		jit->builder->CreateCall(func, { llvm::ConstantInt::get(sizetTy, args.size()) });
	}

	RR_DEBUG_INFO_UPDATE_LOC();
	llvm::SmallVector<llvm::Type *, 8> paramTys;
	for(auto ty : argTys) { paramTys.push_back(T(ty)); }
	auto funcTy = llvm::FunctionType::get(T(retTy), paramTys, false);

	auto funcPtrTy = funcTy->getPointerTo();
	auto funcPtr = jit->builder->CreatePointerCast(V(fptr.value()), funcPtrTy);

	llvm::SmallVector<llvm::Value *, 8> arguments;
	for(auto arg : args) { arguments.push_back(V(arg)); }
	return V(jit->builder->CreateCall(funcTy, funcPtr, arguments));
}

void Breakpoint()
{
	RR_DEBUG_INFO_UPDATE_LOC();
	llvm::Function *debugtrap = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::debugtrap);

	jit->builder->CreateCall(debugtrap);
}

}  // namespace rr

namespace rr {

#if defined(__i386__) || defined(__x86_64__)
namespace x86 {

// Differs from IRBuilder<>::CreateUnaryIntrinsic() in that it only accepts native instruction intrinsics which have
// implicit types, such as 'x86_sse_rcp_ps' operating on v4f32, while 'sqrt' requires explicitly specifying the operand type.
static Value *createInstruction(llvm::Intrinsic::ID id, Value *x)
{
	llvm::Function *intrinsic = llvm::Intrinsic::getDeclaration(jit->module.get(), id);

	return V(jit->builder->CreateCall(intrinsic, V(x)));
}

// Differs from IRBuilder<>::CreateBinaryIntrinsic() in that it only accepts native instruction intrinsics which have
// implicit types, such as 'x86_sse_max_ps' operating on v4f32, while 'sadd_sat' requires explicitly specifying the operand types.
static Value *createInstruction(llvm::Intrinsic::ID id, Value *x, Value *y)
{
	llvm::Function *intrinsic = llvm::Intrinsic::getDeclaration(jit->module.get(), id);

	return V(jit->builder->CreateCall(intrinsic, { V(x), V(y) }));
}

RValue<Int> cvtss2si(RValue<Float> val)
{
	Float4 vector;
	vector.x = val;

	return RValue<Int>(createInstruction(llvm::Intrinsic::x86_sse_cvtss2si, RValue<Float4>(vector).value()));
}

RValue<Int4> cvtps2dq(RValue<Float4> val)
{
	ASSERT(!__has_feature(memory_sanitizer));  // TODO(b/172238865): Not correctly instrumented by MemorySanitizer.

	return RValue<Int4>(createInstruction(llvm::Intrinsic::x86_sse2_cvtps2dq, val.value()));
}

RValue<Float> rcpss(RValue<Float> val)
{
	Value *vector = Nucleus::createInsertElement(V(llvm::UndefValue::get(T(Float4::type()))), val.value(), 0);

	return RValue<Float>(Nucleus::createExtractElement(createInstruction(llvm::Intrinsic::x86_sse_rcp_ss, vector), Float::type(), 0));
}

RValue<Float> sqrtss(RValue<Float> val)
{
	return RValue<Float>(V(jit->builder->CreateUnaryIntrinsic(llvm::Intrinsic::sqrt, V(val.value()))));
}

RValue<Float> rsqrtss(RValue<Float> val)
{
	Value *vector = Nucleus::createInsertElement(V(llvm::UndefValue::get(T(Float4::type()))), val.value(), 0);

	return RValue<Float>(Nucleus::createExtractElement(createInstruction(llvm::Intrinsic::x86_sse_rsqrt_ss, vector), Float::type(), 0));
}

RValue<Float4> rcpps(RValue<Float4> val)
{
	return RValue<Float4>(createInstruction(llvm::Intrinsic::x86_sse_rcp_ps, val.value()));
}

RValue<Float4> sqrtps(RValue<Float4> val)
{
	return RValue<Float4>(V(jit->builder->CreateUnaryIntrinsic(llvm::Intrinsic::sqrt, V(val.value()))));
}

RValue<Float4> rsqrtps(RValue<Float4> val)
{
	return RValue<Float4>(createInstruction(llvm::Intrinsic::x86_sse_rsqrt_ps, val.value()));
}

RValue<Float4> maxps(RValue<Float4> x, RValue<Float4> y)
{
	return RValue<Float4>(createInstruction(llvm::Intrinsic::x86_sse_max_ps, x.value(), y.value()));
}

RValue<Float4> minps(RValue<Float4> x, RValue<Float4> y)
{
	return RValue<Float4>(createInstruction(llvm::Intrinsic::x86_sse_min_ps, x.value(), y.value()));
}

RValue<Float> roundss(RValue<Float> val, unsigned char imm)
{
	llvm::Function *roundss = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::x86_sse41_round_ss);

	Value *undef = V(llvm::UndefValue::get(T(Float4::type())));
	Value *vector = Nucleus::createInsertElement(undef, val.value(), 0);

	return RValue<Float>(Nucleus::createExtractElement(V(jit->builder->CreateCall(roundss, { V(undef), V(vector), V(Nucleus::createConstantInt(imm)) })), Float::type(), 0));
}

RValue<Float> floorss(RValue<Float> val)
{
	return roundss(val, 1);
}

RValue<Float> ceilss(RValue<Float> val)
{
	return roundss(val, 2);
}

RValue<Float4> roundps(RValue<Float4> val, unsigned char imm)
{
	ASSERT(!__has_feature(memory_sanitizer));  // TODO(b/172238865): Not correctly instrumented by MemorySanitizer.

	return RValue<Float4>(createInstruction(llvm::Intrinsic::x86_sse41_round_ps, val.value(), Nucleus::createConstantInt(imm)));
}

RValue<Float4> floorps(RValue<Float4> val)
{
	return roundps(val, 1);
}

RValue<Float4> ceilps(RValue<Float4> val)
{
	return roundps(val, 2);
}

RValue<Short4> paddsw(RValue<Short4> x, RValue<Short4> y)
{
	return As<Short4>(V(lowerPSADDSAT(V(x.value()), V(y.value()))));
}

RValue<Short4> psubsw(RValue<Short4> x, RValue<Short4> y)
{
	return As<Short4>(V(lowerPSSUBSAT(V(x.value()), V(y.value()))));
}

RValue<UShort4> paddusw(RValue<UShort4> x, RValue<UShort4> y)
{
	return As<UShort4>(V(lowerPUADDSAT(V(x.value()), V(y.value()))));
}

RValue<UShort4> psubusw(RValue<UShort4> x, RValue<UShort4> y)
{
	return As<UShort4>(V(lowerPUSUBSAT(V(x.value()), V(y.value()))));
}

RValue<SByte8> paddsb(RValue<SByte8> x, RValue<SByte8> y)
{
	return As<SByte8>(V(lowerPSADDSAT(V(x.value()), V(y.value()))));
}

RValue<SByte8> psubsb(RValue<SByte8> x, RValue<SByte8> y)
{
	return As<SByte8>(V(lowerPSSUBSAT(V(x.value()), V(y.value()))));
}

RValue<Byte8> paddusb(RValue<Byte8> x, RValue<Byte8> y)
{
	return As<Byte8>(V(lowerPUADDSAT(V(x.value()), V(y.value()))));
}

RValue<Byte8> psubusb(RValue<Byte8> x, RValue<Byte8> y)
{
	return As<Byte8>(V(lowerPUSUBSAT(V(x.value()), V(y.value()))));
}

RValue<UShort4> pavgw(RValue<UShort4> x, RValue<UShort4> y)
{
	return As<UShort4>(V(lowerPAVG(V(x.value()), V(y.value()))));
}

RValue<Short4> pmaxsw(RValue<Short4> x, RValue<Short4> y)
{
	return As<Short4>(V(lowerPMINMAX(V(x.value()), V(y.value()), llvm::ICmpInst::ICMP_SGT)));
}

RValue<Short4> pminsw(RValue<Short4> x, RValue<Short4> y)
{
	return As<Short4>(V(lowerPMINMAX(V(x.value()), V(y.value()), llvm::ICmpInst::ICMP_SLT)));
}

RValue<Short4> pcmpgtw(RValue<Short4> x, RValue<Short4> y)
{
	return As<Short4>(V(lowerPCMP(llvm::ICmpInst::ICMP_SGT, V(x.value()), V(y.value()), T(Short4::type()))));
}

RValue<Short4> pcmpeqw(RValue<Short4> x, RValue<Short4> y)
{
	return As<Short4>(V(lowerPCMP(llvm::ICmpInst::ICMP_EQ, V(x.value()), V(y.value()), T(Short4::type()))));
}

RValue<Byte8> pcmpgtb(RValue<SByte8> x, RValue<SByte8> y)
{
	return As<Byte8>(V(lowerPCMP(llvm::ICmpInst::ICMP_SGT, V(x.value()), V(y.value()), T(Byte8::type()))));
}

RValue<Byte8> pcmpeqb(RValue<Byte8> x, RValue<Byte8> y)
{
	return As<Byte8>(V(lowerPCMP(llvm::ICmpInst::ICMP_EQ, V(x.value()), V(y.value()), T(Byte8::type()))));
}

RValue<Short4> packssdw(RValue<Int2> x, RValue<Int2> y)
{
	return As<Short4>(createInstruction(llvm::Intrinsic::x86_sse2_packssdw_128, x.value(), y.value()));
}

RValue<Short8> packssdw(RValue<Int4> x, RValue<Int4> y)
{
	return RValue<Short8>(createInstruction(llvm::Intrinsic::x86_sse2_packssdw_128, x.value(), y.value()));
}

RValue<SByte8> packsswb(RValue<Short4> x, RValue<Short4> y)
{
	return As<SByte8>(createInstruction(llvm::Intrinsic::x86_sse2_packsswb_128, x.value(), y.value()));
}

RValue<Byte8> packuswb(RValue<Short4> x, RValue<Short4> y)
{
	return As<Byte8>(createInstruction(llvm::Intrinsic::x86_sse2_packuswb_128, x.value(), y.value()));
}

RValue<UShort8> packusdw(RValue<Int4> x, RValue<Int4> y)
{
	if(CPUID::supportsSSE4_1())
	{
		return RValue<UShort8>(createInstruction(llvm::Intrinsic::x86_sse41_packusdw, x.value(), y.value()));
	}
	else
	{
		RValue<Int4> bx = (x & ~(x >> 31)) - Int4(0x8000);
		RValue<Int4> by = (y & ~(y >> 31)) - Int4(0x8000);

		return As<UShort8>(packssdw(bx, by) + Short8(0x8000u));
	}
}

RValue<UShort4> psrlw(RValue<UShort4> x, unsigned char y)
{
	return As<UShort4>(createInstruction(llvm::Intrinsic::x86_sse2_psrli_w, x.value(), Nucleus::createConstantInt(y)));
}

RValue<UShort8> psrlw(RValue<UShort8> x, unsigned char y)
{
	return RValue<UShort8>(createInstruction(llvm::Intrinsic::x86_sse2_psrli_w, x.value(), Nucleus::createConstantInt(y)));
}

RValue<Short4> psraw(RValue<Short4> x, unsigned char y)
{
	return As<Short4>(createInstruction(llvm::Intrinsic::x86_sse2_psrai_w, x.value(), Nucleus::createConstantInt(y)));
}

RValue<Short8> psraw(RValue<Short8> x, unsigned char y)
{
	return RValue<Short8>(createInstruction(llvm::Intrinsic::x86_sse2_psrai_w, x.value(), Nucleus::createConstantInt(y)));
}

RValue<Short4> psllw(RValue<Short4> x, unsigned char y)
{
	return As<Short4>(createInstruction(llvm::Intrinsic::x86_sse2_pslli_w, x.value(), Nucleus::createConstantInt(y)));
}

RValue<Short8> psllw(RValue<Short8> x, unsigned char y)
{
	return RValue<Short8>(createInstruction(llvm::Intrinsic::x86_sse2_pslli_w, x.value(), Nucleus::createConstantInt(y)));
}

RValue<Int2> pslld(RValue<Int2> x, unsigned char y)
{
	return As<Int2>(createInstruction(llvm::Intrinsic::x86_sse2_pslli_d, x.value(), Nucleus::createConstantInt(y)));
}

RValue<Int4> pslld(RValue<Int4> x, unsigned char y)
{
	return RValue<Int4>(createInstruction(llvm::Intrinsic::x86_sse2_pslli_d, x.value(), Nucleus::createConstantInt(y)));
}

RValue<Int2> psrad(RValue<Int2> x, unsigned char y)
{
	return As<Int2>(createInstruction(llvm::Intrinsic::x86_sse2_psrai_d, x.value(), Nucleus::createConstantInt(y)));
}

RValue<Int4> psrad(RValue<Int4> x, unsigned char y)
{
	return RValue<Int4>(createInstruction(llvm::Intrinsic::x86_sse2_psrai_d, x.value(), Nucleus::createConstantInt(y)));
}

RValue<UInt2> psrld(RValue<UInt2> x, unsigned char y)
{
	return As<UInt2>(createInstruction(llvm::Intrinsic::x86_sse2_psrli_d, x.value(), Nucleus::createConstantInt(y)));
}

RValue<UInt4> psrld(RValue<UInt4> x, unsigned char y)
{
	return RValue<UInt4>(createInstruction(llvm::Intrinsic::x86_sse2_psrli_d, x.value(), Nucleus::createConstantInt(y)));
}

RValue<Int4> pmaxsd(RValue<Int4> x, RValue<Int4> y)
{
	return RValue<Int4>(V(lowerPMINMAX(V(x.value()), V(y.value()), llvm::ICmpInst::ICMP_SGT)));
}

RValue<Int4> pminsd(RValue<Int4> x, RValue<Int4> y)
{
	return RValue<Int4>(V(lowerPMINMAX(V(x.value()), V(y.value()), llvm::ICmpInst::ICMP_SLT)));
}

RValue<UInt4> pmaxud(RValue<UInt4> x, RValue<UInt4> y)
{
	return RValue<UInt4>(V(lowerPMINMAX(V(x.value()), V(y.value()), llvm::ICmpInst::ICMP_UGT)));
}

RValue<UInt4> pminud(RValue<UInt4> x, RValue<UInt4> y)
{
	return RValue<UInt4>(V(lowerPMINMAX(V(x.value()), V(y.value()), llvm::ICmpInst::ICMP_ULT)));
}

RValue<Short4> pmulhw(RValue<Short4> x, RValue<Short4> y)
{
	return As<Short4>(createInstruction(llvm::Intrinsic::x86_sse2_pmulh_w, x.value(), y.value()));
}

RValue<UShort4> pmulhuw(RValue<UShort4> x, RValue<UShort4> y)
{
	return As<UShort4>(createInstruction(llvm::Intrinsic::x86_sse2_pmulhu_w, x.value(), y.value()));
}

RValue<Int2> pmaddwd(RValue<Short4> x, RValue<Short4> y)
{
	return As<Int2>(createInstruction(llvm::Intrinsic::x86_sse2_pmadd_wd, x.value(), y.value()));
}

RValue<Short8> pmulhw(RValue<Short8> x, RValue<Short8> y)
{
	return RValue<Short8>(createInstruction(llvm::Intrinsic::x86_sse2_pmulh_w, x.value(), y.value()));
}

RValue<UShort8> pmulhuw(RValue<UShort8> x, RValue<UShort8> y)
{
	return RValue<UShort8>(createInstruction(llvm::Intrinsic::x86_sse2_pmulhu_w, x.value(), y.value()));
}

RValue<Int4> pmaddwd(RValue<Short8> x, RValue<Short8> y)
{
	return RValue<Int4>(createInstruction(llvm::Intrinsic::x86_sse2_pmadd_wd, x.value(), y.value()));
}

RValue<Int> movmskps(RValue<Float4> x)
{
	Value *v = x.value();

	// TODO(b/172238865): MemorySanitizer does not support movmsk instructions,
	// which makes it look at the entire 128-bit input for undefined bits. Mask off
	// just the sign bits to avoid false positives.
	if(__has_feature(memory_sanitizer))
	{
		v = As<Float4>(As<Int4>(v) & Int4(0x80000000u)).value();
	}

	return RValue<Int>(createInstruction(llvm::Intrinsic::x86_sse_movmsk_ps, v));
}

RValue<Int> pmovmskb(RValue<Byte8> x)
{
	Value *v = x.value();

	// TODO(b/172238865): MemorySanitizer does not support movmsk instructions,
	// which makes it look at the entire 128-bit input for undefined bits. Mask off
	// just the sign bits in the lower 64-bit vector to avoid false positives.
	if(__has_feature(memory_sanitizer))
	{
		v = As<Byte16>(As<Int4>(v) & Int4(0x80808080u, 0x80808080u, 0, 0)).value();
	}

	return RValue<Int>(createInstruction(llvm::Intrinsic::x86_sse2_pmovmskb_128, v)) & 0xFF;
}

}  // namespace x86
#endif  // defined(__i386__) || defined(__x86_64__)

#ifdef ENABLE_RR_PRINT
void VPrintf(const std::vector<Value *> &vals)
{
	auto i32Ty = llvm::Type::getInt32Ty(*jit->context);
	auto i8PtrTy = llvm::Type::getInt8PtrTy(*jit->context);
	auto funcTy = llvm::FunctionType::get(i32Ty, { i8PtrTy }, true);
	auto func = jit->module->getOrInsertFunction("rr::DebugPrintf", funcTy);
	jit->builder->CreateCall(func, V(vals));
}
#endif  // ENABLE_RR_PRINT

void Nop()
{
	auto voidTy = llvm::Type::getVoidTy(*jit->context);
	auto funcTy = llvm::FunctionType::get(voidTy, {}, false);
	auto func = jit->module->getOrInsertFunction("nop", funcTy);
	jit->builder->CreateCall(func);
}

void EmitDebugLocation()
{
#ifdef ENABLE_RR_DEBUG_INFO
	if(jit->debugInfo != nullptr)
	{
		jit->debugInfo->EmitLocation();
	}
#endif  // ENABLE_RR_DEBUG_INFO
}

void EmitDebugVariable(Value *value)
{
#ifdef ENABLE_RR_DEBUG_INFO
	if(jit->debugInfo != nullptr)
	{
		jit->debugInfo->EmitVariable(value);
	}
#endif  // ENABLE_RR_DEBUG_INFO
}

void FlushDebug()
{
#ifdef ENABLE_RR_DEBUG_INFO
	if(jit->debugInfo != nullptr)
	{
		jit->debugInfo->Flush();
	}
#endif  // ENABLE_RR_DEBUG_INFO
}

}  // namespace rr

// ------------------------------  Coroutines ------------------------------

namespace {

// Magic values retuned by llvm.coro.suspend.
// See: https://llvm.org/docs/Coroutines.html#llvm-coro-suspend-intrinsic
enum SuspendAction
{
	SuspendActionSuspend = -1,
	SuspendActionResume = 0,
	SuspendActionDestroy = 1
};

void promoteFunctionToCoroutine()
{
	ASSERT(jit->coroutine.id == nullptr);

	// Types
	auto voidTy = llvm::Type::getVoidTy(*jit->context);
	auto i1Ty = llvm::Type::getInt1Ty(*jit->context);
	auto i8Ty = llvm::Type::getInt8Ty(*jit->context);
	auto i32Ty = llvm::Type::getInt32Ty(*jit->context);
	auto i8PtrTy = llvm::Type::getInt8PtrTy(*jit->context);
	auto promiseTy = jit->coroutine.yieldType;
	auto promisePtrTy = promiseTy->getPointerTo();

	// LLVM intrinsics
	auto coro_id = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::coro_id);
	auto coro_size = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::coro_size, { i32Ty });
	auto coro_begin = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::coro_begin);
	auto coro_resume = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::coro_resume);
	auto coro_end = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::coro_end);
	auto coro_free = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::coro_free);
	auto coro_destroy = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::coro_destroy);
	auto coro_promise = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::coro_promise);
	auto coro_done = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::coro_done);
	auto coro_suspend = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::coro_suspend);

	auto allocFrameTy = llvm::FunctionType::get(i8PtrTy, { i32Ty }, false);
	auto allocFrame = jit->module->getOrInsertFunction("coroutine_alloc_frame", allocFrameTy);
	auto freeFrameTy = llvm::FunctionType::get(voidTy, { i8PtrTy }, false);
	auto freeFrame = jit->module->getOrInsertFunction("coroutine_free_frame", freeFrameTy);

	auto oldInsertionPoint = jit->builder->saveIP();

	// Build the coroutine_await() function:
	//
	//    bool coroutine_await(CoroutineHandle* handle, YieldType* out)
	//    {
	//        if(llvm.coro.done(handle))
	//        {
	//            return false;
	//        }
	//        else
	//        {
	//            *value = (T*)llvm.coro.promise(handle);
	//            llvm.coro.resume(handle);
	//            return true;
	//        }
	//    }
	//
	{
		auto args = jit->coroutine.await->arg_begin();
		auto handle = args++;
		auto outPtr = args++;
		jit->builder->SetInsertPoint(llvm::BasicBlock::Create(*jit->context, "co_await", jit->coroutine.await));
		auto doneBlock = llvm::BasicBlock::Create(*jit->context, "done", jit->coroutine.await);
		auto resumeBlock = llvm::BasicBlock::Create(*jit->context, "resume", jit->coroutine.await);

		auto done = jit->builder->CreateCall(coro_done, { handle }, "done");
		jit->builder->CreateCondBr(done, doneBlock, resumeBlock);

		jit->builder->SetInsertPoint(doneBlock);
		jit->builder->CreateRet(llvm::ConstantInt::getFalse(i1Ty));

		jit->builder->SetInsertPoint(resumeBlock);
		auto promiseAlignment = llvm::ConstantInt::get(i32Ty, 4);  // TODO: Get correct alignment.
		auto promisePtr = jit->builder->CreateCall(coro_promise, { handle, promiseAlignment, llvm::ConstantInt::get(i1Ty, 0) });
		auto promise = jit->builder->CreateLoad(promiseTy, jit->builder->CreatePointerCast(promisePtr, promisePtrTy));
		jit->builder->CreateStore(promise, outPtr);
		jit->builder->CreateCall(coro_resume, { handle });
		jit->builder->CreateRet(llvm::ConstantInt::getTrue(i1Ty));
	}

	// Build the coroutine_destroy() function:
	//
	//    void coroutine_destroy(CoroutineHandle* handle)
	//    {
	//        llvm.coro.destroy(handle);
	//    }
	//
	{
		auto handle = jit->coroutine.destroy->arg_begin();
		jit->builder->SetInsertPoint(llvm::BasicBlock::Create(*jit->context, "", jit->coroutine.destroy));
		jit->builder->CreateCall(coro_destroy, { handle });
		jit->builder->CreateRetVoid();
	}

	// Begin building the main coroutine_begin() function.
	//
	//    CoroutineHandle* coroutine_begin(<Arguments>)
	//    {
	//        YieldType promise;
	//        auto id = llvm.coro.id(0, &promise, nullptr, nullptr);
	//        void* frame = coroutine_alloc_frame(llvm.coro.size.i32());
	//        CoroutineHandle *handle = llvm.coro.begin(id, frame);
	//
	//        ... <REACTOR CODE> ...
	//
	//    end:
	//        SuspendAction action = llvm.coro.suspend(none, true /* final */);  // <-- RESUME POINT
	//        switch(action)
	//        {
	//        case SuspendActionResume:
	//            UNREACHABLE(); // Illegal to resume after final suspend.
	//        case SuspendActionDestroy:
	//            goto destroy;
	//        default: // (SuspendActionSuspend)
	//            goto suspend;
	//        }
	//
	//    destroy:
	//        coroutine_free_frame(llvm.coro.free(id, handle));
	//        goto suspend;
	//
	//    suspend:
	//        llvm.coro.end(handle, false);
	//        return handle;
	//    }
	//

#ifdef ENABLE_RR_DEBUG_INFO
	jit->debugInfo = std::make_unique<rr::DebugInfo>(jit->builder.get(), jit->context.get(), jit->module.get(), jit->function);
#endif  // ENABLE_RR_DEBUG_INFO

	jit->coroutine.suspendBlock = llvm::BasicBlock::Create(*jit->context, "suspend", jit->function);
	jit->coroutine.endBlock = llvm::BasicBlock::Create(*jit->context, "end", jit->function);
	jit->coroutine.destroyBlock = llvm::BasicBlock::Create(*jit->context, "destroy", jit->function);

	jit->builder->SetInsertPoint(jit->coroutine.entryBlock, jit->coroutine.entryBlock->begin());
	jit->coroutine.promise = jit->builder->CreateAlloca(promiseTy, nullptr, "promise");
	jit->coroutine.id = jit->builder->CreateCall(coro_id, {
	                                                          llvm::ConstantInt::get(i32Ty, 0),
	                                                          jit->builder->CreatePointerCast(jit->coroutine.promise, i8PtrTy),
	                                                          llvm::ConstantPointerNull::get(i8PtrTy),
	                                                          llvm::ConstantPointerNull::get(i8PtrTy),
	                                                      });
	auto size = jit->builder->CreateCall(coro_size, {});
	auto frame = jit->builder->CreateCall(allocFrame, { size });
	jit->coroutine.handle = jit->builder->CreateCall(coro_begin, { jit->coroutine.id, frame });

	// Build the suspend block
	jit->builder->SetInsertPoint(jit->coroutine.suspendBlock);
	jit->builder->CreateCall(coro_end, { jit->coroutine.handle, llvm::ConstantInt::get(i1Ty, 0) });
	jit->builder->CreateRet(jit->coroutine.handle);

	// Build the end block
	jit->builder->SetInsertPoint(jit->coroutine.endBlock);
	auto action = jit->builder->CreateCall(coro_suspend, {
	                                                         llvm::ConstantTokenNone::get(*jit->context),
	                                                         llvm::ConstantInt::get(i1Ty, 1),  // final: true
	                                                     });
	auto switch_ = jit->builder->CreateSwitch(action, jit->coroutine.suspendBlock, 3);
	// switch_->addCase(llvm::ConstantInt::get(i8Ty, SuspendActionResume), trapBlock); // TODO: Trap attempting to resume after final suspend
	switch_->addCase(llvm::ConstantInt::get(i8Ty, SuspendActionDestroy), jit->coroutine.destroyBlock);

	// Build the destroy block
	jit->builder->SetInsertPoint(jit->coroutine.destroyBlock);
	auto memory = jit->builder->CreateCall(coro_free, { jit->coroutine.id, jit->coroutine.handle });
	jit->builder->CreateCall(freeFrame, { memory });
	jit->builder->CreateBr(jit->coroutine.suspendBlock);

	// Switch back to original insert point to continue building the coroutine.
	jit->builder->restoreIP(oldInsertionPoint);
}

}  // anonymous namespace

namespace rr {

void Nucleus::createCoroutine(Type *YieldType, const std::vector<Type *> &Params)
{
	// Coroutines are initially created as a regular function.
	// Upon the first call to Yield(), the function is promoted to a true
	// coroutine.
	auto voidTy = llvm::Type::getVoidTy(*jit->context);
	auto i1Ty = llvm::Type::getInt1Ty(*jit->context);
	auto i8PtrTy = llvm::Type::getInt8PtrTy(*jit->context);
	auto handleTy = i8PtrTy;
	auto boolTy = i1Ty;
	auto promiseTy = T(YieldType);
	auto promisePtrTy = promiseTy->getPointerTo();

	jit->function = rr::createFunction("coroutine_begin", handleTy, T(Params));
#if LLVM_VERSION_MAJOR >= 16
	jit->function->setPresplitCoroutine();
#else
	jit->function->addFnAttr("coroutine.presplit", "0");
#endif
	jit->coroutine.await = rr::createFunction("coroutine_await", boolTy, { handleTy, promisePtrTy });
	jit->coroutine.destroy = rr::createFunction("coroutine_destroy", voidTy, { handleTy });
	jit->coroutine.yieldType = promiseTy;
	jit->coroutine.entryBlock = llvm::BasicBlock::Create(*jit->context, "function", jit->function);

	jit->builder->SetInsertPoint(jit->coroutine.entryBlock);
}

void Nucleus::yield(Value *val)
{
	if(jit->coroutine.id == nullptr)
	{
		// First call to yield().
		// Promote the function to a full coroutine.
		promoteFunctionToCoroutine();
		ASSERT(jit->coroutine.id != nullptr);
	}

	//      promise = val;
	//
	//      auto action = llvm.coro.suspend(none, false /* final */); // <-- RESUME POINT
	//      switch(action)
	//      {
	//      case SuspendActionResume:
	//          goto resume;
	//      case SuspendActionDestroy:
	//          goto destroy;
	//      default: // (SuspendActionSuspend)
	//          goto suspend;
	//      }
	//  resume:
	//

	RR_DEBUG_INFO_UPDATE_LOC();
	Variable::materializeAll();

	// Types
	auto i1Ty = llvm::Type::getInt1Ty(*jit->context);
	auto i8Ty = llvm::Type::getInt8Ty(*jit->context);

	// Intrinsics
	auto coro_suspend = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::coro_suspend);

	// Create a block to resume execution.
	auto resumeBlock = llvm::BasicBlock::Create(*jit->context, "resume", jit->function);

	// Store the promise (yield value)
	jit->builder->CreateStore(V(val), jit->coroutine.promise);
	auto action = jit->builder->CreateCall(coro_suspend, {
	                                                         llvm::ConstantTokenNone::get(*jit->context),
	                                                         llvm::ConstantInt::get(i1Ty, 0),  // final: true
	                                                     });
	auto switch_ = jit->builder->CreateSwitch(action, jit->coroutine.suspendBlock, 3);
	switch_->addCase(llvm::ConstantInt::get(i8Ty, SuspendActionResume), resumeBlock);
	switch_->addCase(llvm::ConstantInt::get(i8Ty, SuspendActionDestroy), jit->coroutine.destroyBlock);

	// Continue building in the resume block.
	jit->builder->SetInsertPoint(resumeBlock);
}

std::shared_ptr<Routine> Nucleus::acquireCoroutine(const char *name)
{
	if(jit->coroutine.id)
	{
		jit->builder->CreateBr(jit->coroutine.endBlock);
	}
	else
	{
		// Coroutine without a Yield acts as a regular function.
		// The 'coroutine_begin' function returns a nullptr for the coroutine
		// handle.
		jit->builder->CreateRet(llvm::Constant::getNullValue(jit->function->getReturnType()));
		// The 'coroutine_await' function always returns false (coroutine done).
		jit->builder->SetInsertPoint(llvm::BasicBlock::Create(*jit->context, "", jit->coroutine.await));
		jit->builder->CreateRet(llvm::Constant::getNullValue(jit->coroutine.await->getReturnType()));
		// The 'coroutine_destroy' does nothing, returns void.
		jit->builder->SetInsertPoint(llvm::BasicBlock::Create(*jit->context, "", jit->coroutine.destroy));
		jit->builder->CreateRetVoid();
	}

#ifdef ENABLE_RR_DEBUG_INFO
	if(jit->debugInfo != nullptr)
	{
		jit->debugInfo->Finalize();
	}
#endif  // ENABLE_RR_DEBUG_INFO

	if(false)
	{
		std::error_code error;
		llvm::raw_fd_ostream file(std::string(name) + "-llvm-dump-unopt.txt", error);
		jit->module->print(file, 0);
	}

	jit->runPasses();

	if(false)
	{
		std::error_code error;
		llvm::raw_fd_ostream file(std::string(name) + "-llvm-dump-opt.txt", error);
		jit->module->print(file, 0);
	}

	llvm::Function *funcs[Nucleus::CoroutineEntryCount];
	funcs[Nucleus::CoroutineEntryBegin] = jit->function;
	funcs[Nucleus::CoroutineEntryAwait] = jit->coroutine.await;
	funcs[Nucleus::CoroutineEntryDestroy] = jit->coroutine.destroy;

	auto routine = jit->acquireRoutine(name, funcs, Nucleus::CoroutineEntryCount);

	delete jit;
	jit = nullptr;

	return routine;
}

Nucleus::CoroutineHandle Nucleus::invokeCoroutineBegin(Routine &routine, std::function<Nucleus::CoroutineHandle()> func)
{
	return func();
}

SIMD::Int::Int(RValue<scalar::Int> rhs)
    : XYZW(this)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *vector = loadValue();
	Value *insert = Nucleus::createInsertElement(vector, rhs.value(), 0);

	std::vector<int> swizzle = { 0 };
	Value *replicate = Nucleus::createShuffleVector(insert, insert, swizzle);

	storeValue(replicate);
}

RValue<SIMD::Int> operator<<(RValue<SIMD::Int> lhs, unsigned char rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return As<SIMD::Int>(V(lowerVectorShl(V(lhs.value()), rhs)));
}

RValue<SIMD::Int> operator>>(RValue<SIMD::Int> lhs, unsigned char rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return As<SIMD::Int>(V(lowerVectorAShr(V(lhs.value()), rhs)));
}

RValue<SIMD::Int> CmpEQ(RValue<SIMD::Int> x, RValue<SIMD::Int> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::Int>(Nucleus::createSExt(Nucleus::createICmpEQ(x.value(), y.value()), SIMD::Int::type()));
}

RValue<SIMD::Int> CmpLT(RValue<SIMD::Int> x, RValue<SIMD::Int> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::Int>(Nucleus::createSExt(Nucleus::createICmpSLT(x.value(), y.value()), SIMD::Int::type()));
}

RValue<SIMD::Int> CmpLE(RValue<SIMD::Int> x, RValue<SIMD::Int> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::Int>(Nucleus::createSExt(Nucleus::createICmpSLE(x.value(), y.value()), SIMD::Int::type()));
}

RValue<SIMD::Int> CmpNEQ(RValue<SIMD::Int> x, RValue<SIMD::Int> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::Int>(Nucleus::createSExt(Nucleus::createICmpNE(x.value(), y.value()), SIMD::Int::type()));
}

RValue<SIMD::Int> CmpNLT(RValue<SIMD::Int> x, RValue<SIMD::Int> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::Int>(Nucleus::createSExt(Nucleus::createICmpSGE(x.value(), y.value()), SIMD::Int::type()));
}

RValue<SIMD::Int> CmpNLE(RValue<SIMD::Int> x, RValue<SIMD::Int> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::Int>(Nucleus::createSExt(Nucleus::createICmpSGT(x.value(), y.value()), SIMD::Int::type()));
}

RValue<SIMD::Int> Abs(RValue<SIMD::Int> x)
{
#if LLVM_VERSION_MAJOR >= 12
	auto func = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::abs, { V(x.value())->getType() });
	return RValue<SIMD::Int>(V(jit->builder->CreateCall(func, { V(x.value()), llvm::ConstantInt::getFalse(*jit->context) })));
#else
	auto negative = x >> 31;
	return (x ^ negative) - negative;
#endif
}

RValue<SIMD::Int> Max(RValue<SIMD::Int> x, RValue<SIMD::Int> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	RValue<SIMD::Int> greater = CmpNLE(x, y);
	return (x & greater) | (y & ~greater);
}

RValue<SIMD::Int> Min(RValue<SIMD::Int> x, RValue<SIMD::Int> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	RValue<SIMD::Int> less = CmpLT(x, y);
	return (x & less) | (y & ~less);
}

RValue<SIMD::Int> RoundInt(RValue<SIMD::Float> cast)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return As<SIMD::Int>(V(lowerRoundInt(V(cast.value()), T(SIMD::Int::type()))));
}

RValue<SIMD::Int> RoundIntClamped(RValue<SIMD::Float> cast)
{
	RR_DEBUG_INFO_UPDATE_LOC();

// TODO(b/165000222): Check if fptosi_sat produces optimal code for x86 and ARM.
#if defined(__arm__) || defined(__aarch64__)
	// ARM saturates to the largest positive or negative integer. Unit tests
	// verify that lowerRoundInt() behaves as desired.
	return As<SIMD::Int>(V(lowerRoundInt(V(cast.value()), T(SIMD::Int::type()))));
#elif LLVM_VERSION_MAJOR >= 14
	llvm::Value *rounded = lowerRound(V(cast.value()));
	llvm::Function *fptosi_sat = llvm::Intrinsic::getDeclaration(
	    jit->module.get(), llvm::Intrinsic::fptosi_sat, { T(SIMD::Int::type()), T(SIMD::Float::type()) });
	return RValue<SIMD::Int>(V(jit->builder->CreateCall(fptosi_sat, { rounded })));
#else
	RValue<SIMD::Float> clamped = Max(Min(cast, SIMD::Float(0x7FFFFF80)), SIMD::Float(static_cast<int>(0x80000000)));
	return As<SIMD::Int>(V(lowerRoundInt(V(clamped.value()), T(SIMD::Int::type()))));
#endif
}

RValue<Int4> Extract128(RValue<SIMD::Int> val, int i)
{
	llvm::Value *v128 = jit->builder->CreateBitCast(V(val.value()), llvm::FixedVectorType::get(llvm::IntegerType::get(*jit->context, 128), SIMD::Width / 4));

	return As<Int4>(V(jit->builder->CreateExtractElement(v128, i)));
}

RValue<SIMD::Int> Insert128(RValue<SIMD::Int> val, RValue<Int4> element, int i)
{
	llvm::Value *v128 = jit->builder->CreateBitCast(V(val.value()), llvm::FixedVectorType::get(llvm::IntegerType::get(*jit->context, 128), SIMD::Width / 4));
	llvm::Value *a = jit->builder->CreateBitCast(V(element.value()), llvm::IntegerType::get(*jit->context, 128));

	return As<SIMD::Int>(V(jit->builder->CreateInsertElement(v128, a, i)));
}

Type *SIMD::Int::type()
{
	return T(llvm::VectorType::get(T(scalar::Int::type()), SIMD::Width, false));
}

SIMD::UInt::UInt(RValue<SIMD::Float> cast)
    : XYZW(this)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *xyzw = Nucleus::createFPToUI(cast.value(), SIMD::UInt::type());
	storeValue(xyzw);
}

SIMD::UInt::UInt(RValue<scalar::UInt> rhs)
    : XYZW(this)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *vector = loadValue();
	Value *insert = Nucleus::createInsertElement(vector, rhs.value(), 0);

	std::vector<int> swizzle = { 0 };
	Value *replicate = Nucleus::createShuffleVector(insert, insert, swizzle);

	storeValue(replicate);
}

RValue<SIMD::UInt> operator<<(RValue<SIMD::UInt> lhs, unsigned char rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return As<SIMD::UInt>(V(lowerVectorShl(V(lhs.value()), rhs)));
}

RValue<SIMD::UInt> operator>>(RValue<SIMD::UInt> lhs, unsigned char rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return As<SIMD::UInt>(V(lowerVectorLShr(V(lhs.value()), rhs)));
}

RValue<SIMD::UInt> CmpEQ(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::UInt>(Nucleus::createSExt(Nucleus::createICmpEQ(x.value(), y.value()), SIMD::Int::type()));
}

RValue<SIMD::UInt> CmpLT(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::UInt>(Nucleus::createSExt(Nucleus::createICmpULT(x.value(), y.value()), SIMD::Int::type()));
}

RValue<SIMD::UInt> CmpLE(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::UInt>(Nucleus::createSExt(Nucleus::createICmpULE(x.value(), y.value()), SIMD::Int::type()));
}

RValue<SIMD::UInt> CmpNEQ(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::UInt>(Nucleus::createSExt(Nucleus::createICmpNE(x.value(), y.value()), SIMD::Int::type()));
}

RValue<SIMD::UInt> CmpNLT(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::UInt>(Nucleus::createSExt(Nucleus::createICmpUGE(x.value(), y.value()), SIMD::Int::type()));
}

RValue<SIMD::UInt> CmpNLE(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::UInt>(Nucleus::createSExt(Nucleus::createICmpUGT(x.value(), y.value()), SIMD::Int::type()));
}

RValue<SIMD::UInt> Max(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	RValue<SIMD::UInt> greater = CmpNLE(x, y);
	return (x & greater) | (y & ~greater);
}

RValue<SIMD::UInt> Min(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	RValue<SIMD::UInt> less = CmpLT(x, y);
	return (x & less) | (y & ~less);
}

RValue<UInt4> Extract128(RValue<SIMD::UInt> val, int i)
{
	llvm::Value *v128 = jit->builder->CreateBitCast(V(val.value()), llvm::FixedVectorType::get(llvm::IntegerType::get(*jit->context, 128), SIMD::Width / 4));

	return As<UInt4>(V(jit->builder->CreateExtractElement(v128, i)));
}

RValue<SIMD::UInt> Insert128(RValue<SIMD::UInt> val, RValue<UInt4> element, int i)
{
	llvm::Value *v128 = jit->builder->CreateBitCast(V(val.value()), llvm::FixedVectorType::get(llvm::IntegerType::get(*jit->context, 128), SIMD::Width / 4));
	llvm::Value *a = jit->builder->CreateBitCast(V(element.value()), llvm::IntegerType::get(*jit->context, 128));

	return As<SIMD::UInt>(V(jit->builder->CreateInsertElement(v128, a, i)));
}

Type *SIMD::UInt::type()
{
	return T(llvm::VectorType::get(T(scalar::UInt::type()), SIMD::Width, false));
}

SIMD::Float::Float(RValue<scalar::Float> rhs)
    : XYZW(this)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *vector = loadValue();
	Value *insert = Nucleus::createInsertElement(vector, rhs.value(), 0);

	std::vector<int> swizzle = { 0 };
	Value *replicate = Nucleus::createShuffleVector(insert, insert, swizzle);

	storeValue(replicate);
}

RValue<SIMD::Float> operator%(RValue<SIMD::Float> lhs, RValue<SIMD::Float> rhs)
{
	return RValue<SIMD::Float>(Nucleus::createFRem(lhs.value(), rhs.value()));
}

RValue<SIMD::Float> MulAdd(RValue<SIMD::Float> x, RValue<SIMD::Float> y, RValue<SIMD::Float> z)
{
	auto func = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::fmuladd, { T(SIMD::Float::type()) });
	return RValue<SIMD::Float>(V(jit->builder->CreateCall(func, { V(x.value()), V(y.value()), V(z.value()) })));
}

RValue<SIMD::Float> FMA(RValue<SIMD::Float> x, RValue<SIMD::Float> y, RValue<SIMD::Float> z)
{
	auto func = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::fma, { T(SIMD::Float::type()) });
	return RValue<SIMD::Float>(V(jit->builder->CreateCall(func, { V(x.value()), V(y.value()), V(z.value()) })));
}

RValue<SIMD::Float> Abs(RValue<SIMD::Float> x)
{
	auto func = llvm::Intrinsic::getDeclaration(jit->module.get(), llvm::Intrinsic::fabs, { V(x.value())->getType() });
	return RValue<SIMD::Float>(V(jit->builder->CreateCall(func, V(x.value()))));
}

RValue<SIMD::Float> Max(RValue<SIMD::Float> x, RValue<SIMD::Float> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return As<SIMD::Float>(V(lowerPFMINMAX(V(x.value()), V(y.value()), llvm::FCmpInst::FCMP_OGT)));
}

RValue<SIMD::Float> Min(RValue<SIMD::Float> x, RValue<SIMD::Float> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return As<SIMD::Float>(V(lowerPFMINMAX(V(x.value()), V(y.value()), llvm::FCmpInst::FCMP_OLT)));
}

RValue<SIMD::Float> Sqrt(RValue<SIMD::Float> x)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return As<SIMD::Float>(V(lowerSQRT(V(x.value()))));
}

RValue<SIMD::Int> CmpEQ(RValue<SIMD::Float> x, RValue<SIMD::Float> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::Int>(Nucleus::createSExt(Nucleus::createFCmpOEQ(x.value(), y.value()), SIMD::Int::type()));
}

RValue<SIMD::Int> CmpLT(RValue<SIMD::Float> x, RValue<SIMD::Float> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::Int>(Nucleus::createSExt(Nucleus::createFCmpOLT(x.value(), y.value()), SIMD::Int::type()));
}

RValue<SIMD::Int> CmpLE(RValue<SIMD::Float> x, RValue<SIMD::Float> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::Int>(Nucleus::createSExt(Nucleus::createFCmpOLE(x.value(), y.value()), SIMD::Int::type()));
}

RValue<SIMD::Int> CmpNEQ(RValue<SIMD::Float> x, RValue<SIMD::Float> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::Int>(Nucleus::createSExt(Nucleus::createFCmpONE(x.value(), y.value()), SIMD::Int::type()));
}

RValue<SIMD::Int> CmpNLT(RValue<SIMD::Float> x, RValue<SIMD::Float> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::Int>(Nucleus::createSExt(Nucleus::createFCmpOGE(x.value(), y.value()), SIMD::Int::type()));
}

RValue<SIMD::Int> CmpNLE(RValue<SIMD::Float> x, RValue<SIMD::Float> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::Int>(Nucleus::createSExt(Nucleus::createFCmpOGT(x.value(), y.value()), SIMD::Int::type()));
}

RValue<SIMD::Int> CmpUEQ(RValue<SIMD::Float> x, RValue<SIMD::Float> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::Int>(Nucleus::createSExt(Nucleus::createFCmpUEQ(x.value(), y.value()), SIMD::Int::type()));
}

RValue<SIMD::Int> CmpULT(RValue<SIMD::Float> x, RValue<SIMD::Float> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::Int>(Nucleus::createSExt(Nucleus::createFCmpULT(x.value(), y.value()), SIMD::Int::type()));
}

RValue<SIMD::Int> CmpULE(RValue<SIMD::Float> x, RValue<SIMD::Float> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::Int>(Nucleus::createSExt(Nucleus::createFCmpULE(x.value(), y.value()), SIMD::Int::type()));
}

RValue<SIMD::Int> CmpUNEQ(RValue<SIMD::Float> x, RValue<SIMD::Float> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::Int>(Nucleus::createSExt(Nucleus::createFCmpUNE(x.value(), y.value()), SIMD::Int::type()));
}

RValue<SIMD::Int> CmpUNLT(RValue<SIMD::Float> x, RValue<SIMD::Float> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::Int>(Nucleus::createSExt(Nucleus::createFCmpUGE(x.value(), y.value()), SIMD::Int::type()));
}

RValue<SIMD::Int> CmpUNLE(RValue<SIMD::Float> x, RValue<SIMD::Float> y)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::Int>(Nucleus::createSExt(Nucleus::createFCmpUGT(x.value(), y.value()), SIMD::Int::type()));
}

RValue<SIMD::Float> Round(RValue<SIMD::Float> x)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::Float>(V(lowerRound(V(x.value()))));
}

RValue<SIMD::Float> Trunc(RValue<SIMD::Float> x)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::Float>(V(lowerTrunc(V(x.value()))));
}

RValue<SIMD::Float> Frac(RValue<SIMD::Float> x)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	SIMD::Float frc = x - Floor(x);

	// x - floor(x) can be 1.0 for very small negative x.
	// Clamp against the value just below 1.0.
	return Min(frc, As<SIMD::Float>(SIMD::Int(0x3F7FFFFF)));
}

RValue<SIMD::Float> Floor(RValue<SIMD::Float> x)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<SIMD::Float>(V(lowerFloor(V(x.value()))));
}

RValue<SIMD::Float> Ceil(RValue<SIMD::Float> x)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return -Floor(-x);
}

RValue<Float4> Extract128(RValue<SIMD::Float> val, int i)
{
	llvm::Value *v128 = jit->builder->CreateBitCast(V(val.value()), llvm::FixedVectorType::get(llvm::IntegerType::get(*jit->context, 128), SIMD::Width / 4));

	return As<Float4>(V(jit->builder->CreateExtractElement(v128, i)));
}

RValue<SIMD::Float> Insert128(RValue<SIMD::Float> val, RValue<Float4> element, int i)
{
	llvm::Value *v128 = jit->builder->CreateBitCast(V(val.value()), llvm::FixedVectorType::get(llvm::IntegerType::get(*jit->context, 128), SIMD::Width / 4));
	llvm::Value *a = jit->builder->CreateBitCast(V(element.value()), llvm::IntegerType::get(*jit->context, 128));

	return As<SIMD::Float>(V(jit->builder->CreateInsertElement(v128, a, i)));
}

Type *SIMD::Float::type()
{
	return T(llvm::VectorType::get(T(scalar::Float::type()), SIMD::Width, false));
}

}  // namespace rr
