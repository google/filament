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

// DAWN_BUILTIN_UNREACHABLE()
//
// Hints the compiler that a code path is unreachable.
#if DAWN_COMPILER_IS(MSVC)
#define DAWN_BUILTIN_UNREACHABLE() __assume(false)
#else
#define DAWN_BUILTIN_UNREACHABLE() __builtin_unreachable()
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

// DAWN_NO_SANITIZE(instrumentation)
//
// Annotate a function or a global variable declaration to specify that a particular instrumentation
// or set of instrumentations should not be applied.
#if DAWN_HAS_ATTRIBUTE(no_sanitize)
#define DAWN_NO_SANITIZE(instrumentation) __attribute__((no_sanitize(instrumentation)))
#else
#define DAWN_NO_SANITIZE(instrumentation)
#endif

#endif  // SRC_UTILS_COMPILER_H_
