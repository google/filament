// Copyright 2024 The Dawn & Tint Authors
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

#ifndef SRC_UTILS_COMPILER_H_
#define SRC_UTILS_COMPILER_H_

// DAWN_COMPILER_IS(CLANG|GCC|MSVC): Compiler detection
//
// Note: clang masquerades as GCC on POSIX and as MSVC on Windows. It must be checked first.
#if defined(__clang__)
#define DAWN_COMPILER_IS_CLANG 1
#define DAWN_COMPILER_IS_GCC 0
#define DAWN_COMPILER_IS_MSVC 0
#elif defined(__GNUC__)
#define DAWN_COMPILER_IS_CLANG 0
#define DAWN_COMPILER_IS_GCC 1
#define DAWN_COMPILER_IS_MSVC 0
#elif defined(_MSC_VER)
#define DAWN_COMPILER_IS_CLANG 0
#define DAWN_COMPILER_IS_GCC 0
#define DAWN_COMPILER_IS_MSVC 1
#else
#error "Unsupported compiler"
#endif

// Use #if DAWN_COMPILER_IS(XXX) for compiler specific code.
// Do not use #ifdef or the naked macro DAWN_COMPILER_IS_XXX.
// This can help avoid common mistakes like not including "compiler.h" and falling into unwanted
// code block as usage of undefined macro "function" will be blocked by the compiler.
#define DAWN_COMPILER_IS(X) (1 == DAWN_COMPILER_IS_##X)

// DAWN_HAS_ATTRIBUTE
//
// A wrapper around `__has_attribute`. This test whether its operand is recognized by the compiler.
#if defined(__has_attribute)
#define DAWN_HAS_ATTRIBUTE(x) __has_attribute(x)
#else
#define DAWN_HAS_ATTRIBUTE(x) 0
#endif

// DAWN_HAS_CPP_ATTRIBUTE
//
// A wrapper around `__has_cpp_attribute`. This test whether its operand is recognized by the
// compiler.
#if defined(__has_cpp_attribute)
#define DAWN_HAS_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
#define DAWN_HAS_CPP_ATTRIBUTE(x) 0
#endif

// DAWN_LIKELY(EXPR)
//
// Where available, hints the compiler that the expression will be true to help it generate code
// that leads to better branch prediction.
#if DAWN_COMPILER_IS(GCC) || DAWN_COMPILER_IS(CLANG)
#define DAWN_LIKELY(x) __builtin_expect(!!(x), 1)
#else
#define DAWN_LIKELY(x) (x)
#endif

// DAWN_UNLIKELY(EXPR)
//
// Where available, hints the compiler that the expression will be false to help it generate code
// that leads to better branch prediction.
#if DAWN_COMPILER_IS(GCC) || DAWN_COMPILER_IS(CLANG)
#define DAWN_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define DAWN_UNLIKELY(x) (x)
#endif

// DAWN_ASAN_ENABLED()
//
// Checks whether ASan is enabled.
#if DAWN_COMPILER_IS(CLANG)
#define DAWN_ASAN_ENABLED() __has_feature(address_sanitizer)
#elif DAWN_COMPILER_IS(GCC) || DAWN_COMPILER_IS(MSVC)
#if defined(__SANITIZE_ADDRESS__)
#define DAWN_ASAN_ENABLED() 1
#else
#define DAWN_ASAN_ENABLED() 0
#endif
#endif

// DAWN_MSAN_ENABLED()
//
// Checks whether MSan is enabled.
#if DAWN_COMPILER_IS(CLANG)
#define DAWN_MSAN_ENABLED() __has_feature(memory_sanitizer)
#elif DAWN_COMPILER_IS(GCC) || DAWN_COMPILER_IS(MSVC)
#if defined(__SANITIZE_ADDRESS__)
#define DAWN_MSAN_ENABLED() 1
#else
#define DAWN_MSAN_ENABLED() 0
#endif
#endif

// DAWN_UBSAN_ENABLED()
//
// Checks whether the undefined behavior sanitizer is enabled.
#if DAWN_COMPILER_IS(CLANG)
#define DAWN_UBSAN_ENABLED() __has_feature(undefined_behavior_sanitizer)
#elif DAWN_COMPILER_IS(GCC) || DAWN_COMPILER_IS(MSVC)
#if defined(__SANITIZE_UNDEFINED__)
#define DAWN_UBSAN_ENABLED() 1
#else
#define DAWN_UBSAN_ENABLED() 0
#endif
#endif

// DAWN_NO_SANITIZE(instrumentation)
//
// Annotate a function or a global variable declaration to specify that a particular instrumentation
// or set of instrumentations should not be applied.
#if DAWN_HAS_ATTRIBUTE(no_sanitize)
#define DAWN_NO_SANITIZE(instrumentation) __attribute__((no_sanitize(instrumentation)))
#else
#define DAWN_NO_SANITIZE(instrumentation)
#endif

// DAWN_TRIVIAL_ABI
//
// Marks a type as being eligible for the "trivial" ABI despite having a non-trivial destructor or
// copy/move constructor. Such types can be relocated after construction by simply copying their
// memory, which makes them eligible to be passed in registers. The canonical example is
// std::unique_ptr.
//
// Use with caution; this has some subtle effects on constructor/destructor ordering and will be
// very incorrect if the type relies on its address remaining constant. When used as a function
// argument (by value), the value may be constructed in the caller's stack frame, passed in a
// register, and then used and destructed in the callee's stack frame. A similar thing can occur
// when values are returned.
//
// TRIVIAL_ABI is not needed for types which have a trivial destructor and copy/move constructors,
// such as dawn::TypedInteger and other POD.
//
// It is also not likely to be effective on types too large to be passed in one or two registers on
// typical target ABIs.
//
// See also:
//   https://clang.llvm.org/docs/AttributeReference.html#trivial-abi
//   https://libcxx.llvm.org/docs/DesignDocs/UniquePtrTrivialAbi.html
#if DAWN_COMPILER_IS(CLANG) && DAWN_HAS_ATTRIBUTE(trivial_abi)
#define DAWN_TRIVIAL_ABI [[clang::trivial_abi]]
#else
#define DAWN_TRIVIAL_ABI
#endif

// Annotates a function or class data member indicating it can lead to out-of-bounds accesses (OOB)
// if given incorrect inputs.
//
// For functions, this commonly includes functions which take pointers, sizes, iterators, sentinels,
// etc. and cannot fully check their preconditions (e.g. that the provided pointer actually points
// to an allocation of at least the provided size). Useful to diagnose potential misuse via
// `-Wunsafe-buffer-usage`, as well as to mark functions potentially in need of safer alternatives.
//
// For fields, this would be used to annotate both pointer and size fields that have not yet been
// converted to a span.
//
// All functions or fields annotated with this macro should come with a `// PRECONDITIONS: ` comment
// that explains what the caller must guarantee to ensure safe operation. Callers can then write
// `// SAFETY: ` comments explaining why the specific preconditions have been met.
//
// Ideally, unsafe functions should also be paired with a safer version, e.g. one that replaces
// pointer parameters with `span`s; otherwise, document safer replacement coding patterns callers
// can migrate to.
//
// Annotating a function `DAWN_UNSAFE_BUFFER_USAGE` means all call sites (that do not disable the
// warning) must wrap calls in `DAWN_UNSAFE_BUFFERS()`; see documentation there. Annotating a field
// `DAWN_UNSAFE_BUFFER_USAGE` means that `DAWN_UNSAFE_BUFFERS()` must wrap expressions that mutate
// of the field.
//
// See also:
//   https://chromium.googlesource.com/chromium/src/+/main/docs/unsafe_buffers.md
//   https://clang.llvm.org/docs/SafeBuffers.html
//   https://clang.llvm.org/docs/DiagnosticsReference.html#wunsafe-buffer-usage
//
// Usage:
// ```
//   // Calls to this function must be wrapped in `DAWN_UNSAFE_BUFFERS()`.
//   DAWN_UNSAFE_BUFFER_USAGE void Func(T* input, T* end);
//
//   struct S {
//     // Changing this pointer requires `DAWN_UNSAFE_BUFFERS()`.
//     DAWN_UNSAFE_BUFFER_USAGE int* p;
//   };
// ```
#if DAWN_HAS_CPP_ATTRIBUTE(clang::unsafe_buffer_usage)
#define DAWN_UNSAFE_BUFFER_USAGE [[clang::unsafe_buffer_usage]]
#else
#define DAWN_UNSAFE_BUFFER_USAGE
#endif

// Annotates code indicating that it should be permanently exempted from `-Wunsafe-buffer-usage`.
// For temporary cases such as migrating callers to safer patterns, use `DAWN_UNSAFE_TODO()`
// instead; see documentation there.
//
// All calls to functions annotated with `DAWN_UNSAFE_BUFFER_USAGE` must be marked with one of these
// two macros; they can also be used around pointer arithmetic, pointer subscripting, and the like.
//
// ** USE OF THIS MACRO SHOULD BE VERY RARE.** Using this macro indicates that the compiler cannot
// verify that the code avoids OOB, and manual review is required. Even with manual review, it's
// easy for assumptions to change and security bugs to creep in over time. Prefer safer patterns
// instead.
//
// Usage should wrap the minimum necessary code, and *must* include a `// SAFETY: ...` comment that
// explains how the code guarantees safety or meets the requirements of called
// `DAWN_UNSAFE_BUFFER_USAGE` functions. Guarantees must be manually verifiable using only local
// invariants. Valid invariants include:
// - Runtime conditions or `CHECK()`s nearby
// - Invariants guaranteed by types in the surrounding code
// - Invariants guaranteed by function calls in the surrounding code
// - Caller requirements, if the containing function is itself annotated with
//   `DAWN_UNSAFE_BUFFER_USAGE`; this is less safe and should be a last resort
//
// See also:
//   https://chromium.googlesource.com/chromium/src/+/main/docs/unsafe_buffers.md
//   https://clang.llvm.org/docs/SafeBuffers.html
//   https://clang.llvm.org/docs/DiagnosticsReference.html#wunsafe-buffer-usage
//
// Usage:
// ```
//   // The following call will not trigger a compiler warning even if `Func()`
//   // is annotated `DAWN_UNSAFE_BUFFER_USAGE`.
//   return DAWN_UNSAFE_BUFFERS(Func(input, end));
// ```
//
// Test for `__clang__` directly, as there's no `__has_pragma` or similar (see
// https://github.com/llvm/llvm-project/issues/51887).
#if DAWN_COMPILER_IS(CLANG)
// Disabling `clang-format` allows each `_Pragma` to be on its own line, as recommended by
// https://gcc.gnu.org/onlinedocs/cpp/Pragmas.html.
// clang-format off
// SAFETY: This is the definition of the macro and not an unsafe usage.
#define DAWN_UNSAFE_BUFFERS(...)             \
  _Pragma("clang unsafe_buffer_usage begin") \
  __VA_ARGS__                                \
  _Pragma("clang unsafe_buffer_usage end")
// clang-format on
#else
// SAFETY: This is the definition of the macro and not an unsafe usage.
#define DAWN_UNSAFE_BUFFERS(...) __VA_ARGS__
#endif

// Annotates code indicating that it should be temporarily exempted from `-Wunsafe-buffer-usage`.
// While this is functionally the same as `DAWN_UNSAFE_BUFFERS()`, semantically it indicates that
// this is for migration purposes, and should be cleaned up as soon as possible.
//
// Usage:
// ```
//   // The following call will not trigger a compiler warning even if `Func()`
//   // is annotated `DAWN_UNSAFE_BUFFER_USAGE`.
//   return DAWN_UNSAFE_TODO(Func(input, end));
// ```
// SAFETY: The macro is used to tag code that should be modified to use DAWN_UNSAFE_BUFFERS or
// ideally use safer ways to manipulate the buffer. It's an explicit opt-out of DAWN_UNSAFE_BUFFERS
// and doesn't require a SAFETY argument when used.
#define DAWN_UNSAFE_TODO(...) DAWN_UNSAFE_BUFFERS(__VA_ARGS__)

#endif  // SRC_UTILS_COMPILER_H_
