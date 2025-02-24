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

#include "LLVMReactor.hpp"

#include "Debug.hpp"
#include "ExecutableMemory.hpp"
#include "LLVMAsm.hpp"
#include "PragmaInternals.hpp"
#include "Routine.hpp"

// TODO(b/143539525): Eliminate when warning has been fixed.
#ifdef _MSC_VER
__pragma(warning(push))
    __pragma(warning(disable : 4146))  // unary minus operator applied to unsigned type, result still unsigned
#endif

// See https://groups.google.com/g/llvm-dev/c/CAE7Va57h2c/m/74ITeXFEAQAJ
// for information about `RTDyldObjectLinkingLayer` vs `ObjectLinkingLayer`.
// On RISC-V and LoongArch, only `ObjectLinkingLayer` is supported.
#if defined(__riscv) || defined(__loongarch__)
#define USE_LEGACY_OBJECT_LINKING_LAYER 0
#else
#define USE_LEGACY_OBJECT_LINKING_LAYER 1
#endif

#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"

#if USE_LEGACY_OBJECT_LINKING_LAYER
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#else
#include "llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h"
#endif
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Instrumentation/AddressSanitizer.h"
#include "llvm/Transforms/Instrumentation/MemorySanitizer.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

#if LLVM_VERSION_MAJOR >= 13  // New pass manager
#	include "llvm/IR/PassManager.h"
#	include "llvm/Passes/PassBuilder.h"
#	include "llvm/Transforms/Scalar/ADCE.h"
#	include "llvm/Transforms/Scalar/DeadStoreElimination.h"
#	include "llvm/Transforms/Scalar/EarlyCSE.h"
#	include "llvm/Transforms/Scalar/LICM.h"
#	include "llvm/Transforms/Scalar/Reassociate.h"
#	include "llvm/Transforms/Scalar/SCCP.h"
#	include "llvm/Transforms/Scalar/SROA.h"
#	include "llvm/Transforms/Scalar/SimplifyCFG.h"
#else  // Legacy pass manager
#	include "llvm/IR/LegacyPassManager.h"
#	include "llvm/Pass.h"
#	include "llvm/Transforms/Coroutines.h"
#	include "llvm/Transforms/IPO.h"
#endif

#ifdef _MSC_VER
    __pragma(warning(pop))
#endif

#if defined(__unix__) || defined(__APPLE__) || defined(__Fuchsia__)
#	define ADDRESS_SANITIZER_INSTRUMENTATION_SUPPORTED true
#	if __has_feature(memory_sanitizer) || __has_feature(address_sanitizer)
#		include <dlfcn.h>  // dlsym()
#	endif
#else
#	define ADDRESS_SANITIZER_INSTRUMENTATION_SUPPORTED false
#endif

#ifndef REACTOR_ASM_EMIT_DIR
#	define REACTOR_ASM_EMIT_DIR "./"
#endif

#if defined(_WIN64)
        extern "C" void __chkstk();
#elif defined(_WIN32)
extern "C" void _chkstk();
#endif

#ifdef __ARM_EABI__
extern "C" signed __aeabi_idivmod();
#endif

#if __has_feature(memory_sanitizer)
#	include "sanitizer/msan_interface.h"

// MemorySanitizer uses thread-local storage (TLS) data arrays for passing around
// the 'shadow' values of function arguments and return values. The LLVM JIT can't
// access TLS directly, but it calls __emutls_get_address() to obtain the address.
// Typically, it would be passed a pointer to an __emutls_control structure with a
// name starting with "__emutls_v." that represents the TLS. Both the address of
// __emutls_get_address and the __emutls_v. structures are provided to the JIT by
// the symbol resolver, which can be overridden.
// We take advantage of this by substituting __emutls_get_address() with our own
// implementation, namely rr::getTLSAddress(), and substituting the __emutls_v
// variables with rr::MSanTLS enums. getTLSAddress() can then provide the address
// of the real TLS variable corresponding to the enum, in statically compiled C++.

// Forward declare the real TLS variables used by MemorySanitizer. These are
// defined in llvm-project/compiler-rt/lib/msan/msan.cpp.
extern __thread unsigned long long __msan_param_tls[];
extern __thread unsigned int __msan_param_origin_tls[];
extern __thread unsigned long long __msan_retval_tls[];
extern __thread unsigned int __msan_retval_origin_tls;
extern __thread unsigned long long __msan_va_arg_tls[];
extern __thread unsigned int __msan_va_arg_origin_tls[];
extern __thread unsigned long long __msan_va_arg_overflow_size_tls;
extern __thread unsigned int __msan_origin_tls;

namespace rr {

enum class MSanTLS
{
	param = 1,             // __msan_param_tls
	param_origin,          //__msan_param_origin_tls
	retval,                // __msan_retval_tls
	retval_origin,         //__msan_retval_origin_tls
	va_arg,                // __msan_va_arg_tls
	va_arg_origin,         // __msan_va_arg_origin_tls
	va_arg_overflow_size,  // __msan_va_arg_overflow_size_tls
	origin,                //__msan_origin_tls
};

static void *getTLSAddress(void *control)
{
	auto tlsIndex = static_cast<MSanTLS>(reinterpret_cast<uintptr_t>(control));
	switch(tlsIndex)
	{
	case MSanTLS::param: return reinterpret_cast<void *>(&__msan_param_tls);
	case MSanTLS::param_origin: return reinterpret_cast<void *>(&__msan_param_origin_tls);
	case MSanTLS::retval: return reinterpret_cast<void *>(&__msan_retval_tls);
	case MSanTLS::retval_origin: return reinterpret_cast<void *>(&__msan_retval_origin_tls);
	case MSanTLS::va_arg: return reinterpret_cast<void *>(&__msan_va_arg_tls);
	case MSanTLS::va_arg_origin: return reinterpret_cast<void *>(&__msan_va_arg_origin_tls);
	case MSanTLS::va_arg_overflow_size: return reinterpret_cast<void *>(&__msan_va_arg_overflow_size_tls);
	case MSanTLS::origin: return reinterpret_cast<void *>(&__msan_origin_tls);

	default:
		UNSUPPORTED("MemorySanitizer used an unrecognized TLS variable: %d", static_cast<int>(tlsIndex));
		return nullptr;
	}
}

}  // namespace rr
#endif

namespace {

// TODO(b/174587935): Eliminate command-line parsing.
bool parseCommandLineOptionsOnce(int argc, const char *const *argv)
{
	// Use a static immediately invoked lambda to make this thread safe
	static auto initialized = [=]() {
		return llvm::cl::ParseCommandLineOptions(argc, argv);
	}();

	return initialized;
}

// JITGlobals is a singleton that holds all the immutable machine specific
// information for the host device.
class JITGlobals
{
public:
	static JITGlobals *get();

	llvm::orc::JITTargetMachineBuilder getTargetMachineBuilder() const;
	const llvm::DataLayout &getDataLayout() const;
	const llvm::Triple &getTargetTriple() const;

private:
	JITGlobals(llvm::orc::JITTargetMachineBuilder &&jitTargetMachineBuilder, llvm::DataLayout &&dataLayout);

	static llvm::CodeGenOpt::Level toLLVM(int level);

	const llvm::orc::JITTargetMachineBuilder jitTargetMachineBuilder;
	const llvm::DataLayout dataLayout;
};

JITGlobals *JITGlobals::get()
{
	static JITGlobals instance = [] {
		const char *argv[] = {
			"Reactor",
#if defined(__i386__) || defined(__x86_64__)
			"-x86-asm-syntax=intel",  // Use Intel syntax rather than the default AT&T
#endif
#if LLVM_VERSION_MAJOR <= 12
			"-warn-stack-size=524288",  // Warn when a function uses more than 512 KiB of stack memory
#endif
		};

		parseCommandLineOptionsOnce(sizeof(argv) / sizeof(argv[0]), argv);

		llvm::InitializeNativeTarget();
		llvm::InitializeNativeTargetAsmPrinter();
		llvm::InitializeNativeTargetAsmParser();

		// TODO(b/171236524): JITTargetMachineBuilder::detectHost() currently uses the target triple of the host,
		// rather than a valid triple for the current process. Once fixed, we can use that function instead.
		llvm::orc::JITTargetMachineBuilder jitTargetMachineBuilder(llvm::Triple(LLVM_DEFAULT_TARGET_TRIPLE));

		// Retrieve host CPU name and sub-target features and add them to builder.
		// Relocation model, code model and codegen opt level are kept to default values.
		llvm::StringMap<bool> cpuFeatures;
		bool ok = llvm::sys::getHostCPUFeatures(cpuFeatures);

#if defined(__i386__) || defined(__x86_64__) || \
    (defined(__linux__) && (defined(__arm__) || defined(__aarch64__)))
		ASSERT_MSG(ok, "llvm::sys::getHostCPUFeatures returned false");
#else
		(void)ok;  // getHostCPUFeatures always returns false on other platforms
#endif
		for(auto &feature : cpuFeatures)
		{
			jitTargetMachineBuilder.getFeatures().AddFeature(feature.first(), feature.second);
		}

#if LLVM_VERSION_MAJOR >= 11 /* TODO(b/165000222): Unconditional after LLVM 11 upgrade */

#if defined(__riscv) && __riscv_xlen == 64
		// jitTargetMachineBuilder.getFeatures() on RISC-V does
		// not return the RISC-V CPU extensions, so they are
		// manually added.
		jitTargetMachineBuilder.getFeatures().AddFeature("+m");
		jitTargetMachineBuilder.getFeatures().AddFeature("+a");
		jitTargetMachineBuilder.getFeatures().AddFeature("+f");
		jitTargetMachineBuilder.getFeatures().AddFeature("+d");
		jitTargetMachineBuilder.getFeatures().AddFeature("+c");
		// The default code model is "Small".
		// On RISC-V, using the default code model results in an
		// "Unsupported riscv relocation" error.
		jitTargetMachineBuilder.setCodeModel(llvm::CodeModel::Medium);
#elif defined(__loongarch__)
		// jitTargetMachineBuilder.getFeatures() on LoongArch does
		// not return the LoongArch CPU extensions, so they are
		// manually added.
		jitTargetMachineBuilder.getFeatures().AddFeature("+d");
#endif

		jitTargetMachineBuilder.setCPU(std::string(llvm::sys::getHostCPUName()));
#else
		jitTargetMachineBuilder.setCPU(llvm::sys::getHostCPUName());
#endif

		// Reactor's MemorySanitizer support depends on intercepting __emutls_get_address calls.
#if LLVM_VERSION_MAJOR < 17
		ASSERT(!__has_feature(memory_sanitizer) || (jitTargetMachineBuilder.getOptions().ExplicitEmulatedTLS &&
		                                            jitTargetMachineBuilder.getOptions().EmulatedTLS));
#else
		ASSERT(!__has_feature(memory_sanitizer) || jitTargetMachineBuilder.getOptions().EmulatedTLS);
#endif

		auto dataLayout = jitTargetMachineBuilder.getDefaultDataLayoutForTarget();
		ASSERT_MSG(dataLayout, "JITTargetMachineBuilder::getDefaultDataLayoutForTarget() failed");

		return JITGlobals(std::move(jitTargetMachineBuilder), std::move(dataLayout.get()));
	}();

	return &instance;
}

llvm::orc::JITTargetMachineBuilder JITGlobals::getTargetMachineBuilder() const
{
	llvm::orc::JITTargetMachineBuilder out = jitTargetMachineBuilder;
	out.setCodeGenOptLevel(toLLVM(rr::getPragmaState(rr::OptimizationLevel)));

	return out;
}

const llvm::DataLayout &JITGlobals::getDataLayout() const
{
	return dataLayout;
}

const llvm::Triple &JITGlobals::getTargetTriple() const
{
	return jitTargetMachineBuilder.getTargetTriple();
}

JITGlobals::JITGlobals(llvm::orc::JITTargetMachineBuilder &&jitTargetMachineBuilder, llvm::DataLayout &&dataLayout)
    : jitTargetMachineBuilder(jitTargetMachineBuilder)
    , dataLayout(dataLayout)
{
}

llvm::CodeGenOpt::Level JITGlobals::toLLVM(int level)
{
	// TODO(b/173257647): MemorySanitizer instrumentation produces IR which takes
	// a lot longer to process by the machine code optimization passes. Disabling
	// them has a negligible effect on code quality but compiles much faster.
	if(__has_feature(memory_sanitizer))
	{
		return llvm::CodeGenOpt::None;
	}

	switch(level)
	{
	case 0: return llvm::CodeGenOpt::None;
	case 1: return llvm::CodeGenOpt::Less;
	case 2: return llvm::CodeGenOpt::Default;
	case 3: return llvm::CodeGenOpt::Aggressive;
	default: UNREACHABLE("Unknown Optimization Level %d", int(level));
	}

	return llvm::CodeGenOpt::Default;
}

class MemoryMapper final : public llvm::SectionMemoryManager::MemoryMapper
{
public:
	MemoryMapper() {}
	~MemoryMapper() final {}

	llvm::sys::MemoryBlock allocateMappedMemory(
	    llvm::SectionMemoryManager::AllocationPurpose purpose,
	    size_t numBytes, const llvm::sys::MemoryBlock *const nearBlock,
	    unsigned flags, std::error_code &errorCode) final
	{
		errorCode = std::error_code();

		// Round up numBytes to page size.
		size_t pageSize = rr::memoryPageSize();
		numBytes = (numBytes + pageSize - 1) & ~(pageSize - 1);

		bool need_exec =
		    purpose == llvm::SectionMemoryManager::AllocationPurpose::Code;
		void *addr = rr::allocateMemoryPages(
		    numBytes, flagsToPermissions(flags), need_exec);
		if(!addr)
			return llvm::sys::MemoryBlock();
		return llvm::sys::MemoryBlock(addr, numBytes);
	}

	std::error_code protectMappedMemory(const llvm::sys::MemoryBlock &block,
	                                    unsigned flags)
	{
		// Round down base address to align with a page boundary. This matches
		// DefaultMMapper behavior.
		void *addr = block.base();
		size_t size = block.allocatedSize();
		size_t pageSize = rr::memoryPageSize();
		addr = reinterpret_cast<void *>(
		    reinterpret_cast<uintptr_t>(addr) & ~(pageSize - 1));
		size += reinterpret_cast<uintptr_t>(block.base()) -
		        reinterpret_cast<uintptr_t>(addr);

		rr::protectMemoryPages(addr, size, flagsToPermissions(flags));
		return std::error_code();
	}

	std::error_code releaseMappedMemory(llvm::sys::MemoryBlock &block)
	{
		size_t size = block.allocatedSize();

		rr::deallocateMemoryPages(block.base(), size);
		return std::error_code();
	}

private:
	int flagsToPermissions(unsigned flags)
	{
		int result = 0;
		if(flags & llvm::sys::Memory::MF_READ)
		{
			result |= rr::PERMISSION_READ;
		}
		if(flags & llvm::sys::Memory::MF_WRITE)
		{
			result |= rr::PERMISSION_WRITE;
		}
		if(flags & llvm::sys::Memory::MF_EXEC)
		{
			result |= rr::PERMISSION_EXECUTE;
		}
		return result;
	}
};

template<typename T>
T alignUp(T val, T alignment)
{
	return alignment * ((val + alignment - 1) / alignment);
}

void *alignedAlloc(size_t size, size_t alignment)
{
	ASSERT(alignment < 256);
	auto allocation = new uint8_t[size + sizeof(uint8_t) + alignment];
	auto aligned = allocation;
	aligned += sizeof(uint8_t);                                                                       // Make space for the base-address offset.
	aligned = reinterpret_cast<uint8_t *>(alignUp(reinterpret_cast<uintptr_t>(aligned), alignment));  // align
	auto offset = static_cast<uint8_t>(aligned - allocation);
	aligned[-1] = offset;
	return aligned;
}

void alignedFree(void *ptr)
{
	auto aligned = reinterpret_cast<uint8_t *>(ptr);
	auto offset = aligned[-1];
	auto allocation = aligned - offset;
	delete[] allocation;
}

template<typename T>
static void atomicLoad(void *ptr, void *ret, llvm::AtomicOrdering ordering)
{
	*reinterpret_cast<T *>(ret) = std::atomic_load_explicit<T>(reinterpret_cast<std::atomic<T> *>(ptr), rr::atomicOrdering(ordering));
}

template<typename T>
static void atomicStore(void *ptr, void *val, llvm::AtomicOrdering ordering)
{
	std::atomic_store_explicit<T>(reinterpret_cast<std::atomic<T> *>(ptr), *reinterpret_cast<T *>(val), rr::atomicOrdering(ordering));
}

#ifdef __ANDROID__
template<typename F>
static uint32_t sync_fetch_and_op(uint32_t volatile *ptr, uint32_t val, F f)
{
	// Build an arbitrary op out of looped CAS
	for(;;)
	{
		uint32_t expected = *ptr;
		uint32_t desired = f(expected, val);

		if(expected == __sync_val_compare_and_swap_4(ptr, expected, desired))
		{
			return expected;
		}
	}
}
#endif

#if LLVM_VERSION_MAJOR >= 11 /* TODO(b/165000222): Unconditional after LLVM 11 upgrade */
class ExternalSymbolGenerator : public llvm::orc::DefinitionGenerator
#else
class ExternalSymbolGenerator : public llvm::orc::JITDylib::DefinitionGenerator
#endif
{
	struct Atomic
	{
		static void load(size_t size, void *ptr, void *ret, llvm::AtomicOrdering ordering)
		{
			switch(size)
			{
			case 1: atomicLoad<uint8_t>(ptr, ret, ordering); break;
			case 2: atomicLoad<uint16_t>(ptr, ret, ordering); break;
			case 4: atomicLoad<uint32_t>(ptr, ret, ordering); break;
			case 8: atomicLoad<uint64_t>(ptr, ret, ordering); break;
			default:
				UNIMPLEMENTED_NO_BUG("Atomic::load(size: %d)", int(size));
			}
		}
		static void store(size_t size, void *ptr, void *ret, llvm::AtomicOrdering ordering)
		{
			switch(size)
			{
			case 1: atomicStore<uint8_t>(ptr, ret, ordering); break;
			case 2: atomicStore<uint16_t>(ptr, ret, ordering); break;
			case 4: atomicStore<uint32_t>(ptr, ret, ordering); break;
			case 8: atomicStore<uint64_t>(ptr, ret, ordering); break;
			default:
				UNIMPLEMENTED_NO_BUG("Atomic::store(size: %d)", int(size));
			}
		}
	};

	static void nop() {}
	static void neverCalled() { UNREACHABLE("Should never be called"); }

	static void *coroutine_alloc_frame(size_t size) { return alignedAlloc(size, 16); }
	static void coroutine_free_frame(void *ptr) { alignedFree(ptr); }

#ifdef __ANDROID__
	// forwarders since we can't take address of builtins
	static void sync_synchronize() { __sync_synchronize(); }
	static uint32_t sync_fetch_and_add_4(uint32_t *ptr, uint32_t val) { return __sync_fetch_and_add_4(ptr, val); }
	static uint32_t sync_fetch_and_and_4(uint32_t *ptr, uint32_t val) { return __sync_fetch_and_and_4(ptr, val); }
	static uint32_t sync_fetch_and_or_4(uint32_t *ptr, uint32_t val) { return __sync_fetch_and_or_4(ptr, val); }
	static uint32_t sync_fetch_and_xor_4(uint32_t *ptr, uint32_t val) { return __sync_fetch_and_xor_4(ptr, val); }
	static uint32_t sync_fetch_and_sub_4(uint32_t *ptr, uint32_t val) { return __sync_fetch_and_sub_4(ptr, val); }
	static uint32_t sync_lock_test_and_set_4(uint32_t *ptr, uint32_t val) { return __sync_lock_test_and_set_4(ptr, val); }
	static uint32_t sync_val_compare_and_swap_4(uint32_t *ptr, uint32_t expected, uint32_t desired) { return __sync_val_compare_and_swap_4(ptr, expected, desired); }

	static uint32_t sync_fetch_and_max_4(uint32_t *ptr, uint32_t val)
	{
		return sync_fetch_and_op(ptr, val, [](int32_t a, int32_t b) { return std::max(a, b); });
	}
	static uint32_t sync_fetch_and_min_4(uint32_t *ptr, uint32_t val)
	{
		return sync_fetch_and_op(ptr, val, [](int32_t a, int32_t b) { return std::min(a, b); });
	}
	static uint32_t sync_fetch_and_umax_4(uint32_t *ptr, uint32_t val)
	{
		return sync_fetch_and_op(ptr, val, [](uint32_t a, uint32_t b) { return std::max(a, b); });
	}
	static uint32_t sync_fetch_and_umin_4(uint32_t *ptr, uint32_t val)
	{
		return sync_fetch_and_op(ptr, val, [](uint32_t a, uint32_t b) { return std::min(a, b); });
	}
#endif

	class Resolver
	{
	public:
		using FunctionMap = llvm::StringMap<void *>;

		FunctionMap functions;

		Resolver()
		{
#ifdef ENABLE_RR_PRINT
			functions.try_emplace("rr::DebugPrintf", reinterpret_cast<void *>(rr::DebugPrintf));
#endif
			functions.try_emplace("nop", reinterpret_cast<void *>(nop));
			functions.try_emplace("floorf", reinterpret_cast<void *>(floorf));
			functions.try_emplace("nearbyintf", reinterpret_cast<void *>(nearbyintf));
			functions.try_emplace("truncf", reinterpret_cast<void *>(truncf));
			functions.try_emplace("printf", reinterpret_cast<void *>(printf));
			functions.try_emplace("puts", reinterpret_cast<void *>(puts));
			functions.try_emplace("fmodf", reinterpret_cast<void *>(fmodf));

			functions.try_emplace("sinf", reinterpret_cast<void *>(sinf));
			functions.try_emplace("cosf", reinterpret_cast<void *>(cosf));
			functions.try_emplace("asinf", reinterpret_cast<void *>(asinf));
			functions.try_emplace("acosf", reinterpret_cast<void *>(acosf));
			functions.try_emplace("atanf", reinterpret_cast<void *>(atanf));
			functions.try_emplace("sinhf", reinterpret_cast<void *>(sinhf));
			functions.try_emplace("coshf", reinterpret_cast<void *>(coshf));
			functions.try_emplace("tanhf", reinterpret_cast<void *>(tanhf));
			functions.try_emplace("asinhf", reinterpret_cast<void *>(asinhf));
			functions.try_emplace("acoshf", reinterpret_cast<void *>(acoshf));
			functions.try_emplace("atanhf", reinterpret_cast<void *>(atanhf));
			functions.try_emplace("atan2f", reinterpret_cast<void *>(atan2f));
			functions.try_emplace("powf", reinterpret_cast<void *>(powf));
			functions.try_emplace("expf", reinterpret_cast<void *>(expf));
			functions.try_emplace("logf", reinterpret_cast<void *>(logf));
			functions.try_emplace("exp2f", reinterpret_cast<void *>(exp2f));
			functions.try_emplace("log2f", reinterpret_cast<void *>(log2f));
			functions.try_emplace("fmaf", reinterpret_cast<void *>(fmaf));

			functions.try_emplace("fmod", reinterpret_cast<void *>(static_cast<double (*)(double, double)>(fmod)));
			functions.try_emplace("sin", reinterpret_cast<void *>(static_cast<double (*)(double)>(sin)));
			functions.try_emplace("cos", reinterpret_cast<void *>(static_cast<double (*)(double)>(cos)));
			functions.try_emplace("asin", reinterpret_cast<void *>(static_cast<double (*)(double)>(asin)));
			functions.try_emplace("acos", reinterpret_cast<void *>(static_cast<double (*)(double)>(acos)));
			functions.try_emplace("atan", reinterpret_cast<void *>(static_cast<double (*)(double)>(atan)));
			functions.try_emplace("sinh", reinterpret_cast<void *>(static_cast<double (*)(double)>(sinh)));
			functions.try_emplace("cosh", reinterpret_cast<void *>(static_cast<double (*)(double)>(cosh)));
			functions.try_emplace("tanh", reinterpret_cast<void *>(static_cast<double (*)(double)>(tanh)));
			functions.try_emplace("asinh", reinterpret_cast<void *>(static_cast<double (*)(double)>(asinh)));
			functions.try_emplace("acosh", reinterpret_cast<void *>(static_cast<double (*)(double)>(acosh)));
			functions.try_emplace("atanh", reinterpret_cast<void *>(static_cast<double (*)(double)>(atanh)));
			functions.try_emplace("atan2", reinterpret_cast<void *>(static_cast<double (*)(double, double)>(atan2)));
			functions.try_emplace("pow", reinterpret_cast<void *>(static_cast<double (*)(double, double)>(pow)));
			functions.try_emplace("exp", reinterpret_cast<void *>(static_cast<double (*)(double)>(exp)));
			functions.try_emplace("log", reinterpret_cast<void *>(static_cast<double (*)(double)>(log)));
			functions.try_emplace("exp2", reinterpret_cast<void *>(static_cast<double (*)(double)>(exp2)));
			functions.try_emplace("log2", reinterpret_cast<void *>(static_cast<double (*)(double)>(log2)));

			functions.try_emplace("atomic_load", reinterpret_cast<void *>(Atomic::load));
			functions.try_emplace("atomic_store", reinterpret_cast<void *>(Atomic::store));

			// FIXME(b/119409619): use an allocator here so we can control all memory allocations
			functions.try_emplace("coroutine_alloc_frame", reinterpret_cast<void *>(coroutine_alloc_frame));
			functions.try_emplace("coroutine_free_frame", reinterpret_cast<void *>(coroutine_free_frame));

			functions.try_emplace("memset", reinterpret_cast<void *>(memset));

#ifdef __APPLE__
			functions.try_emplace("__sincosf_stret", reinterpret_cast<void *>(__sincosf_stret));
#elif defined(__linux__)
			functions.try_emplace("sincosf", reinterpret_cast<void *>(sincosf));
#elif defined(_WIN64)
			functions.try_emplace("__chkstk", reinterpret_cast<void *>(__chkstk));
#elif defined(_WIN32)
			functions.try_emplace("_chkstk", reinterpret_cast<void *>(_chkstk));
#endif

#ifdef __ARM_EABI__
			functions.try_emplace("__aeabi_idivmod", reinterpret_cast<void *>(__aeabi_idivmod));
#endif
#ifdef __ANDROID__
			functions.try_emplace("aeabi_unwind_cpp_pr0", reinterpret_cast<void *>(neverCalled));
			functions.try_emplace("sync_synchronize", reinterpret_cast<void *>(sync_synchronize));
			functions.try_emplace("sync_fetch_and_add_4", reinterpret_cast<void *>(sync_fetch_and_add_4));
			functions.try_emplace("sync_fetch_and_and_4", reinterpret_cast<void *>(sync_fetch_and_and_4));
			functions.try_emplace("sync_fetch_and_or_4", reinterpret_cast<void *>(sync_fetch_and_or_4));
			functions.try_emplace("sync_fetch_and_xor_4", reinterpret_cast<void *>(sync_fetch_and_xor_4));
			functions.try_emplace("sync_fetch_and_sub_4", reinterpret_cast<void *>(sync_fetch_and_sub_4));
			functions.try_emplace("sync_lock_test_and_set_4", reinterpret_cast<void *>(sync_lock_test_and_set_4));
			functions.try_emplace("sync_val_compare_and_swap_4", reinterpret_cast<void *>(sync_val_compare_and_swap_4));
			functions.try_emplace("sync_fetch_and_max_4", reinterpret_cast<void *>(sync_fetch_and_max_4));
			functions.try_emplace("sync_fetch_and_min_4", reinterpret_cast<void *>(sync_fetch_and_min_4));
			functions.try_emplace("sync_fetch_and_umax_4", reinterpret_cast<void *>(sync_fetch_and_umax_4));
			functions.try_emplace("sync_fetch_and_umin_4", reinterpret_cast<void *>(sync_fetch_and_umin_4));

#	if defined(__i386__)
			// TODO(b/172974501): Workaround for an x86-32 issue where an R_386_PC32 relocation is used
			// When calling a C function from Reactor code, who's address is not associated with any symbol
			// (since it's an absolute constant), but it still invokes the symbol resolver for "".
			functions.try_emplace("", nullptr);
#	endif
#endif
#if __has_feature(memory_sanitizer)
			functions.try_emplace("__emutls_get_address", reinterpret_cast<void *>(rr::getTLSAddress));
			functions.try_emplace("__emutls_v.__msan_param_tls", reinterpret_cast<void *>(static_cast<uintptr_t>(rr::MSanTLS::param)));
			functions.try_emplace("__emutls_v.__msan_param_origin_tls", reinterpret_cast<void *>(static_cast<uintptr_t>(rr::MSanTLS::param_origin)));
			functions.try_emplace("__emutls_v.__msan_retval_tls", reinterpret_cast<void *>(static_cast<uintptr_t>(rr::MSanTLS::retval)));
			functions.try_emplace("__emutls_v.__msan_retval_origin_tls", reinterpret_cast<void *>(static_cast<uintptr_t>(rr::MSanTLS::retval_origin)));
			functions.try_emplace("__emutls_v.__msan_va_arg_tls", reinterpret_cast<void *>(static_cast<uintptr_t>(rr::MSanTLS::va_arg)));
			functions.try_emplace("__emutls_v.__msan_va_arg_origin_tls", reinterpret_cast<void *>(static_cast<uintptr_t>(rr::MSanTLS::va_arg_origin)));
			functions.try_emplace("__emutls_v.__msan_va_arg_overflow_size_tls", reinterpret_cast<void *>(static_cast<uintptr_t>(rr::MSanTLS::va_arg_overflow_size)));
			functions.try_emplace("__emutls_v.__msan_origin_tls", reinterpret_cast<void *>(static_cast<uintptr_t>(rr::MSanTLS::origin)));

			functions.try_emplace("__msan_unpoison", reinterpret_cast<void *>(__msan_unpoison));
			functions.try_emplace("__msan_unpoison_param", reinterpret_cast<void *>(__msan_unpoison_param));
#endif
		}
	};

	llvm::Error tryToGenerate(
#if LLVM_VERSION_MAJOR >= 11 /* TODO(b/165000222): Unconditional after LLVM 11 upgrade */
	    llvm::orc::LookupState &state,
#endif
	    llvm::orc::LookupKind kind,
	    llvm::orc::JITDylib &dylib,
	    llvm::orc::JITDylibLookupFlags flags,
	    const llvm::orc::SymbolLookupSet &set) override
	{
		static Resolver resolver;

		llvm::orc::SymbolMap symbols;

#if !defined(NDEBUG) || defined(DCHECK_ALWAYS_ON)
		std::string missing;
#endif  // !defined(NDEBUG) || defined(DCHECK_ALWAYS_ON)

		for(auto symbol : set)
		{
			auto name = symbol.first;

#if defined(__APPLE__)
			// Trim the underscore from the start of the symbol. LLVM adds it for Mach-O mangling convention.
			ASSERT((*name)[0] == '_');
			auto unmangled = (*name).drop_front(1);
#else
			auto unmangled = *name;
#endif

#if LLVM_VERSION_MAJOR < 17
			auto toSymbol = [](void *ptr) {
				return llvm::JITEvaluatedSymbol(
				    static_cast<llvm::JITTargetAddress>(reinterpret_cast<uintptr_t>(ptr)),
				    llvm::JITSymbolFlags::Exported);
			};
#else
			auto toSymbol = [](void *ptr) {
				return llvm::orc::ExecutorSymbolDef{
					llvm::orc::ExecutorAddr(reinterpret_cast<uintptr_t>(ptr)),
					llvm::JITSymbolFlags::Exported,
				};
			};
#endif

			auto it = resolver.functions.find(unmangled.str());
			if(it != resolver.functions.end())
			{
				symbols[name] = toSymbol(it->second);
				continue;
			}

#if __has_feature(memory_sanitizer) || (__has_feature(address_sanitizer) && ADDRESS_SANITIZER_INSTRUMENTATION_SUPPORTED)
			// Sanitizers use a dynamically linked runtime. Instrumented routines reference some
			// symbols from this library. Look them up dynamically in the default namespace.
			// Note this approach should not be used for other symbols, since they might not be
			// visible (e.g. due to static linking), we may wish to provide an alternate
			// implementation, and/or it would be a security vulnerability.

			void *address = dlsym(RTLD_DEFAULT, unmangled.data());

			if(address)
			{
				symbols[name] = toSymbol(address);
				continue;
			}
#endif

#if !defined(NDEBUG) || defined(DCHECK_ALWAYS_ON)
			missing += (missing.empty() ? "'" : ", '") + unmangled.str() + "'";
#endif
		}

#if !defined(NDEBUG) || defined(DCHECK_ALWAYS_ON)
		// Missing functions will likely make the module fail in non-obvious ways.
		if(!missing.empty())
		{
			WARN("Missing external functions: %s", missing.c_str());
		}
#endif

		if(symbols.empty())
		{
			return llvm::Error::success();
		}

		return dylib.define(llvm::orc::absoluteSymbols(std::move(symbols)));
	}
};

// As we must support different LLVM versions, add a generic Unwrap for functions that return Expected<T> or the actual T.
// TODO(b/165000222): Remove after LLVM 11 upgrade
template<typename T>
T &Unwrap(llvm::Expected<T> &&v)
{
	assert(v);
	return v.get();
}
template<typename T>
T &Unwrap(T &&v)
{
	return v;
}

// Sets *fatal to true if a diagnostic is received which makes a routine invalid or unusable.
struct FatalDiagnosticsHandler : public llvm::DiagnosticHandler
{
	FatalDiagnosticsHandler(bool *fatal)
	    : fatal(fatal)
	{}

	bool handleDiagnostics(const llvm::DiagnosticInfo &info) override
	{
		switch(info.getSeverity())
		{
		case llvm::DS_Error:
			ASSERT_MSG(false, "LLVM JIT compilation failure");
			*fatal = true;
			break;
		case llvm::DS_Warning:
			if(info.getKind() == llvm::DK_StackSize)
			{
				// Stack size limit exceeded
				*fatal = true;
			}
			break;
		case llvm::DS_Remark:
			break;
		case llvm::DS_Note:
			break;
		}

		return true;  // Diagnostic handled, don't let LLVM print it.
	}

	bool *fatal;
};

// JITRoutine is a rr::Routine that holds a LLVM JIT session, compiler and
// object layer as each routine may require different target machine
// settings and no Reactor routine directly links against another.
class JITRoutine : public rr::Routine
{
public:
	JITRoutine(
	    std::unique_ptr<llvm::Module> module,
	    std::unique_ptr<llvm::LLVMContext> context,
	    const char *name,
	    llvm::Function **funcs,
	    size_t count)
	    : name(name)
#if LLVM_VERSION_MAJOR >= 13
	    , session(std::move(Unwrap(llvm::orc::SelfExecutorProcessControl::Create())))
#endif
#if USE_LEGACY_OBJECT_LINKING_LAYER
	    , objectLayer(session, [this]() {
		    return std::make_unique<llvm::SectionMemoryManager>(&memoryMapper);
	    })
#else
	    , objectLayer(session, llvm::cantFail(llvm::jitlink::InProcessMemoryManager::Create()))
#endif
	    , addresses(count)
	{
		bool fatalCompileIssue = false;
		context->setDiagnosticHandler(std::make_unique<FatalDiagnosticsHandler>(&fatalCompileIssue), true);

#ifdef ENABLE_RR_DEBUG_INFO
		// TODO(b/165000222): Update this on next LLVM roll.
		// https://github.com/llvm/llvm-project/commit/98f2bb4461072347dcca7d2b1b9571b3a6525801
		// introduces RTDyldObjectLinkingLayer::registerJITEventListener().
		// The current API does not appear to have any way to bind the
		// rr::DebugInfo::NotifyFreeingObject event.
#	if LLVM_VERSION_MAJOR >= 12
		objectLayer.setNotifyLoaded([](llvm::orc::MaterializationResponsibility &R,
		                               const llvm::object::ObjectFile &obj,
		                               const llvm::RuntimeDyld::LoadedObjectInfo &l) {
			static std::atomic<uint64_t> unique_key{ 0 };
			rr::DebugInfo::NotifyObjectEmitted(unique_key++, obj, l);
		});
#	else
		objectLayer.setNotifyLoaded([](llvm::orc::VModuleKey,
		                               const llvm::object::ObjectFile &obj,
		                               const llvm::RuntimeDyld::LoadedObjectInfo &l) {
			static std::atomic<uint64_t> unique_key{ 0 };
			rr::DebugInfo::NotifyObjectEmitted(unique_key++, obj, l);
		});
#	endif
#endif  // ENABLE_RR_DEBUG_INFO

		if(JITGlobals::get()->getTargetTriple().isOSBinFormatCOFF())
		{
			// Hack to support symbol visibility in COFF.
			// Matches hack in llvm::orc::LLJIT::createObjectLinkingLayer().
			// See documentation on these functions for more detail.
			objectLayer.setOverrideObjectFlagsWithResponsibilityFlags(true);
			objectLayer.setAutoClaimResponsibilityForObjectSymbols(true);
		}

		llvm::SmallVector<llvm::orc::SymbolStringPtr, 8> functionNames(count);
		llvm::orc::MangleAndInterner mangle(session, JITGlobals::get()->getDataLayout());

		for(size_t i = 0; i < count; i++)
		{
			llvm::Function *func = funcs[i];

			if(!func->hasName())
			{
				func->setName("f" + llvm::Twine(i).str());
			}

			functionNames[i] = mangle(func->getName());
		}

#ifdef ENABLE_RR_EMIT_ASM_FILE
		const auto asmFilename = rr::AsmFile::generateFilename(REACTOR_ASM_EMIT_DIR, name);
		rr::AsmFile::emitAsmFile(asmFilename, JITGlobals::get()->getTargetMachineBuilder(), *module);
#endif

		// Once the module is passed to the compileLayer, the llvm::Functions are freed.
		// Make sure funcs are not referenced after this point.
		funcs = nullptr;

		llvm::orc::IRCompileLayer compileLayer(session, objectLayer, std::make_unique<llvm::orc::ConcurrentIRCompiler>(JITGlobals::get()->getTargetMachineBuilder()));
		llvm::orc::JITDylib &dylib(Unwrap(session.createJITDylib("<routine>")));
		dylib.addGenerator(std::make_unique<ExternalSymbolGenerator>());

		llvm::cantFail(compileLayer.add(dylib, llvm::orc::ThreadSafeModule(std::move(module), std::move(context))));

		// Resolve the function addresses.
		for(size_t i = 0; i < count; i++)
		{
			fatalCompileIssue = false;  // May be set to true by session.lookup()

			// This is where the actual compilation happens.
			auto symbol = session.lookup({ &dylib }, functionNames[i]);

			ASSERT_MSG(symbol, "Failed to lookup address of routine function %d: %s",
			           (int)i, llvm::toString(symbol.takeError()).c_str());

			if(fatalCompileIssue)
			{
				addresses[i] = nullptr;
			}
			else  // Successful compilation
			{
#if LLVM_VERSION_MAJOR < 17
				addresses[i] = reinterpret_cast<void *>(static_cast<intptr_t>(symbol->getAddress()));
#else
				addresses[i] = reinterpret_cast<void *>(static_cast<intptr_t>(symbol->getAddress().getValue()));
#endif
			}
		}

#ifdef ENABLE_RR_EMIT_ASM_FILE
		rr::AsmFile::fixupAsmFile(asmFilename, addresses);
#endif
	}

	~JITRoutine()
	{
#if LLVM_VERSION_MAJOR >= 11 /* TODO(b/165000222): Unconditional after LLVM 11 upgrade */
		if(auto err = session.endSession())
		{
			session.reportError(std::move(err));
		}
#endif
	}

	const void *getEntry(int index) const override
	{
		return addresses[index];
	}

private:
	std::string name;
	llvm::orc::ExecutionSession session;
	MemoryMapper memoryMapper;
#if USE_LEGACY_OBJECT_LINKING_LAYER
	llvm::orc::RTDyldObjectLinkingLayer objectLayer;
#else
	llvm::orc::ObjectLinkingLayer objectLayer;
#endif
	std::vector<const void *> addresses;
};

}  // anonymous namespace

namespace rr {

JITBuilder::JITBuilder()
    : context(new llvm::LLVMContext())
    , module(new llvm::Module("", *context))
    , builder(new llvm::IRBuilder<>(*context))
{
	module->setTargetTriple(LLVM_DEFAULT_TARGET_TRIPLE);
	module->setDataLayout(JITGlobals::get()->getDataLayout());

	msanInstrumentation = getPragmaState(MemorySanitizerInstrumentation);
}

void JITBuilder::runPasses()
{
#if defined(ENABLE_RR_LLVM_IR_VERIFICATION) || !defined(NDEBUG)
	if(llvm::verifyModule(*module, &llvm::errs()))
	{
		llvm::report_fatal_error("Invalid LLVM module");
	}
#endif

	int optimizationLevel = getPragmaState(OptimizationLevel);

#ifdef ENABLE_RR_DEBUG_INFO
	if(debugInfo != nullptr)
	{
		optimizationLevel = 0;  // Don't optimize if we're generating debug info.
	}
#endif  // ENABLE_RR_DEBUG_INFO

#if LLVM_VERSION_MAJOR >= 13  // New pass manager
	llvm::LoopAnalysisManager lam;
	llvm::FunctionAnalysisManager fam;
	llvm::CGSCCAnalysisManager cgam;
	llvm::ModuleAnalysisManager mam;
	llvm::PassBuilder pb;

	pb.registerModuleAnalyses(mam);
	pb.registerCGSCCAnalyses(cgam);
	pb.registerFunctionAnalyses(fam);
	pb.registerLoopAnalyses(lam);
	pb.crossRegisterProxies(lam, fam, cgam, mam);

	llvm::ModulePassManager pm;
	llvm::FunctionPassManager fpm;

	if(coroutine.id)
	{
		// Adds mandatory coroutine transforms.
		pm = pb.buildO0DefaultPipeline(llvm::OptimizationLevel::O0);
	}

	if(optimizationLevel > 0)
	{
		fpm.addPass(llvm::SROAPass(llvm::SROAOptions::PreserveCFG));
		fpm.addPass(llvm::InstCombinePass());
	}

	if(!fpm.isEmpty())
	{
		pm.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(fpm)));
	}

	if(__has_feature(memory_sanitizer) && msanInstrumentation)
	{
		llvm::MemorySanitizerOptions msanOpts(0 /* TrackOrigins */, false /* Recover */, false /* Kernel */, true /* EagerChecks */);
		pm.addPass(llvm::MemorySanitizerPass(msanOpts));
	}

	if(__has_feature(address_sanitizer) && ADDRESS_SANITIZER_INSTRUMENTATION_SUPPORTED)
	{
		pm.addPass(llvm::AddressSanitizerPass(llvm::AddressSanitizerOptions{}));
	}

	pm.run(*module, mam);
#else  // Legacy pass manager
	llvm::legacy::PassManager passManager;

	if(coroutine.id)
	{
		// Run mandatory coroutine transforms.
		passManager.add(llvm::createCoroEarlyLegacyPass());
		passManager.add(llvm::createCoroSplitLegacyPass());
		passManager.add(llvm::createCoroElideLegacyPass());
		passManager.add(llvm::createBarrierNoopPass());
		passManager.add(llvm::createCoroCleanupLegacyPass());
	}

	if(optimizationLevel > 0)
	{
		passManager.add(llvm::createSROAPass());
		passManager.add(llvm::createInstructionCombiningPass());
	}

	if(__has_feature(memory_sanitizer) && msanInstrumentation)
	{
		llvm::MemorySanitizerOptions msanOpts(0 /* TrackOrigins */, false /* Recover */, false /* Kernel */);
		passManager.add(llvm::createMemorySanitizerLegacyPassPass(msanOpts));
	}

	if(__has_feature(address_sanitizer) && ADDRESS_SANITIZER_INSTRUMENTATION_SUPPORTED)
	{
		passManager.add(llvm::createAddressSanitizerFunctionPass());
	}

	passManager.run(*module);
#endif
}

std::shared_ptr<rr::Routine> JITBuilder::acquireRoutine(const char *name, llvm::Function **funcs, size_t count)
{
	ASSERT(module);
	return std::make_shared<JITRoutine>(std::move(module), std::move(context), name, funcs, count);
}

}  // namespace rr
