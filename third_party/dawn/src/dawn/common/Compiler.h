// Copyright 2017 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_COMMON_COMPILER_H_
#define SRC_DAWN_COMMON_COMPILER_H_

#include "src/utils/compiler.h"

// Defines macros for compiler-specific functionality

// DAWN_UNUSED_FUNC
//
// Prevents unused variable/expression warnings on FUNC.
#define DAWN_UNUSED_FUNC(FUNC) static_assert(sizeof(&FUNC) == sizeof(void (*)()))

// DAWN_FORCE_INLINE
//
// Annotate a function indicating it should really never be inline, even in debug mode.
#if DAWN_COMPILER_IS(CLANG) && defined(NDEBUG) && DAWN_HAS_CPP_ATTRIBUTE(clang::always_inline)
#define DAWN_FORCE_INLINE [[clang::always_inline]] inline
#elif DAWN_COMPILER_IS(GCC) && defined(NDEBUG) && DAWN_HAS_ATTRIBUTE(always_inline)
#define DAWN_FORCE_INLINE inline __attribute__((always_inline))
#elif DAWN_COMPILER_IS(MSVC) && defined(NDEBUG)
#define DAWN_FORCE_INLINE __forceinline
#else
#define DAWN_FORCE_INLINE inline
#endif

// DAWN_TRIVIAL_ABI
//
// Marks a type as being eligible for the "trivial" ABI despite having a
// non-trivial destructor or copy/move constructor. Such types can be relocated
// after construction by simply copying their memory, which makes them eligible
// to be passed in registers. The canonical example is std::unique_ptr.
//
// Use with caution; this has some subtle effects on constructor/destructor
// ordering and will be very incorrect if the type relies on its address
// remaining constant. When used as a function argument (by value), the value
// may be constructed in the caller's stack frame, passed in a register, and
// then used and destructed in the callee's stack frame. A similar thing can
// occur when values are returned.
//
// TRIVIAL_ABI is not needed for types which have a trivial destructor and
// copy/move constructors, such as base::TimeTicks and other POD.
//
// It is also not likely to be effective on types too large to be passed in one
// or two registers on typical target ABIs.
//
// See also:
//   https://clang.llvm.org/docs/AttributeReference.html#trivial-abi
//   https://libcxx.llvm.org/docs/DesignDocs/UniquePtrTrivialAbi.html
#if DAWN_COMPILER_IS(CLANG) && DAWN_HAS_ATTRIBUTE(trivial_abi)
#define DAWN_TRIVIAL_ABI [[clang::trivial_abi]]
#else
#define DAWN_TRIVIAL_ABI
#endif

// DAWN_CONSTINIT
//
// Requires constant initialization. See constinit in C++20. Allows to rely on a variable being
// initialized before execution, and not requiring a global constructor.
#if DAWN_HAS_ATTRIBUTE(require_constant_initialization)
#define DAWN_CONSTINIT __attribute__((require_constant_initialization))
#else
#define DAWN_CONSTINIT
#endif

// DAWN_GSL_POINTER
//
// In [lifetime safety], If you write a custom class wrapping a pointer, the [[gsl::Owner]] and
// [[gsl::Pointer]] can help the compiler to know if it acquired ownership over the pointee, or not.
// The compiler is then able to emit useful warning.
//
// [lifetime safety]: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1179r0.pdf
#if DAWN_COMPILER_IS(CLANG)
#define DAWN_GSL_POINTER [[gsl::Pointer]]
#else
#define DAWN_GSL_POINTER
#endif

// DAWN_CONSTEXPR_DTOR
//
// Constexpr destructors were introduced in C++20. Dawn's minimum supported C++ version is C++17.
#if defined(__cpp_constexpr) && __cpp_constexpr >= 201907L
#define DAWN_CONSTEXPR_DTOR constexpr
#else
#define DAWN_CONSTEXPR_DTOR
#endif

// DAWN_ATTRIBUTE_RETURNS_NONNULL
//
// Tells the compiler that a function never returns a null pointer. Sourced from Abseil's
// `attributes.h`.
#if DAWN_HAS_ATTRIBUTE(returns_nonnull)
#define DAWN_ATTRIBUTE_RETURNS_NONNULL __attribute__((returns_nonnull))
#else
#define DAWN_ATTRIBUTE_RETURNS_NONNULL
#endif

// DAWN_ENABLE_STRUCT_PADDING_WARNINGS
//
// Tells the compiler to emit a warning if the structure has any padding.
// This is helpful to avoid uninitialized bits in cache keys or other similar structures.
#if DAWN_COMPILER_IS(CLANG)
#define DAWN_ENABLE_STRUCT_PADDING_WARNINGS \
    _Pragma("clang diagnostic push") _Pragma("clang diagnostic error \"-Wpadded\"")
#define DAWN_DISABLE_STRUCT_PADDING_WARNINGS _Pragma("clang diagnostic pop")
#elif DAWN_COMPILER_IS(GCC)
#define DAWN_ENABLE_STRUCT_PADDING_WARNINGS \
    _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic error \"-Wpadded\"")
#define DAWN_DISABLE_STRUCT_PADDING_WARNINGS _Pragma("GCC diagnostic pop")
#elif DAWN_COMPILER_IS(MSVC)
#define DAWN_ENABLE_STRUCT_PADDING_WARNINGS __pragma(warning(push)) __pragma(warning(error : 4820))
#define DAWN_DISABLE_STRUCT_PADDING_WARNINGS __pragma(warning(pop))
#else
#define DAWN_ENABLE_STRUCT_PADDING_WARNINGS
#define DAWN_DISABLE_STRUCT_PADDING_WARNINGS
#endif

// DAWN_MUTEX_CAPABILITY
//
// Used when a class provides a mutex capability for thread-safety analysis.
//
// https://clang.llvm.org/docs/ThreadSafetyAnalysis.html#capability-string
#if DAWN_HAS_ATTRIBUTE(capability)
#define DAWN_MUTEX_CAPABILITY __attribute__((capability("mutex")))
#else
#define DAWN_MUTEX_CAPABILITY
#endif

// DAWN_SCOPED_LOCKABLE
//
// Used when a class implements RAII-style locking.
//
// https://clang.llvm.org/docs/ThreadSafetyAnalysis.html#scoped-capability
#if DAWN_HAS_ATTRIBUTE(scoped_lockable)
#define DAWN_SCOPED_LOCKABLE __attribute__((scoped_lockable))
#else
#define DAWN_SCOPED_LOCKABLE
#endif

// DAWN_EXCLUSIVE_LOCK_FUNCTION
//
// Used when a function acquires a lock. The lock must not be in the acquired state at the beginning
// of the function and must be in the acquired state at the end of the function.
//
// https://clang.llvm.org/docs/ThreadSafetyAnalysis.html#acquire-acquire-shared-release-release-shared-release-generic
#if DAWN_HAS_ATTRIBUTE(exclusive_lock_function)
#define DAWN_EXCLUSIVE_LOCK_FUNCTION __attribute__((exclusive_lock_function()))
#else
#define DAWN_EXCLUSIVE_LOCK_FUNCTION
#endif

// DAWN_UNLOCK_FUNCTION
//
// Used when a function releases a lock. The lock must be in the acquired state at the beginning of
// the function and must be in the released state at the end of the function.
//
// https://clang.llvm.org/docs/ThreadSafetyAnalysis.html#acquire-acquire-shared-release-release-shared-release-generic
#if DAWN_HAS_ATTRIBUTE(unlock_function)
#define DAWN_UNLOCK_FUNCTION __attribute__((unlock_function()))
#else
#define DAWN_UNLOCK_FUNCTION
#endif

#endif  // SRC_DAWN_COMMON_COMPILER_H_
