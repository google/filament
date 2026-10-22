// Copyright 2015 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef BENCHMARK_MACROS_H_
#define BENCHMARK_MACROS_H_

#if defined(_MSC_VER)
#include <intrin.h>
#endif

#include <stdint.h>

#define BENCHMARK_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;                \
  TypeName& operator=(const TypeName&) = delete

#ifdef BENCHMARK_HAS_CXX17
#define BENCHMARK_UNUSED [[maybe_unused]]
#elif defined(__GNUC__) || defined(__clang__)
#define BENCHMARK_UNUSED __attribute__((unused))
#else
#define BENCHMARK_UNUSED
#endif

#if defined(__clang__)
#define BENCHMARK_DONT_OPTIMIZE __attribute__((optnone))
#elif defined(__GNUC__) || defined(__GNUG__)
#define BENCHMARK_DONT_OPTIMIZE __attribute__((optimize(0)))
#else
#define BENCHMARK_DONT_OPTIMIZE
#endif

#if defined(__GNUC__) || defined(__clang__)
#define BENCHMARK_ALWAYS_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER) && !defined(__clang__)
#define BENCHMARK_ALWAYS_INLINE __forceinline
#define __func__ __FUNCTION__
#else
#define BENCHMARK_ALWAYS_INLINE
#endif

#define BENCHMARK_INTERNAL_TOSTRING2(x) #x
#define BENCHMARK_INTERNAL_TOSTRING(x) BENCHMARK_INTERNAL_TOSTRING2(x)

#if (defined(__GNUC__) && !defined(__NVCC__) && !defined(__NVCOMPILER)) || \
    defined(__clang__)
#define BENCHMARK_BUILTIN_EXPECT(x, y) __builtin_expect(x, y)
#define BENCHMARK_DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
#define BENCHMARK_DISABLE_DEPRECATED_WARNING \
  _Pragma("GCC diagnostic push")             \
      _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#define BENCHMARK_RESTORE_DEPRECATED_WARNING _Pragma("GCC diagnostic pop")
#elif defined(__NVCOMPILER)
#define BENCHMARK_BUILTIN_EXPECT(x, y) __builtin_expect(x, y)
#define BENCHMARK_DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
#define BENCHMARK_DISABLE_DEPRECATED_WARNING \
  _Pragma("diagnostic push")                 \
      _Pragma("diag_suppress deprecated_entity_with_custom_message")
#define BENCHMARK_RESTORE_DEPRECATED_WARNING _Pragma("diagnostic pop")
#elif defined(_MSC_VER)
#define BENCHMARK_BUILTIN_EXPECT(x, y) x
#define BENCHMARK_DEPRECATED_MSG(msg) __declspec(deprecated(msg))
#define BENCHMARK_WARNING_MSG(msg)                           \
  __pragma(message(__FILE__ "(" BENCHMARK_INTERNAL_TOSTRING( \
      __LINE__) ") : warning note: " msg))
#define BENCHMARK_DISABLE_DEPRECATED_WARNING \
  __pragma(warning(push)) __pragma(warning(disable : 4996))
#define BENCHMARK_RESTORE_DEPRECATED_WARNING __pragma(warning(pop))
#else
#define BENCHMARK_BUILTIN_EXPECT(x, y) x
#define BENCHMARK_DEPRECATED_MSG(msg)
#define BENCHMARK_WARNING_MSG(msg)                           \
  __pragma(message(__FILE__ "(" BENCHMARK_INTERNAL_TOSTRING( \
      __LINE__) ") : warning note: " msg))
#define BENCHMARK_DISABLE_DEPRECATED_WARNING
#define BENCHMARK_RESTORE_DEPRECATED_WARNING
#endif

#if defined(__GNUC__) && !defined(__clang__)
#define BENCHMARK_GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if defined(__GNUC__) || __has_builtin(__builtin_unreachable)
#define BENCHMARK_UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER)
#define BENCHMARK_UNREACHABLE() __assume(false)
#else
#define BENCHMARK_UNREACHABLE() ((void)0)
#endif

#if defined(__GNUC__)
#if defined(__i386__) || defined(__x86_64__)
#define BENCHMARK_INTERNAL_CACHELINE_SIZE 64
#elif defined(__powerpc64__)
#define BENCHMARK_INTERNAL_CACHELINE_SIZE 128
#elif defined(__aarch64__)
#define BENCHMARK_INTERNAL_CACHELINE_SIZE 64
#elif defined(__arm__)
#if defined(__ARM_ARCH_5T__)
#define BENCHMARK_INTERNAL_CACHELINE_SIZE 32
#elif defined(__ARM_ARCH_7A__)
#define BENCHMARK_INTERNAL_CACHELINE_SIZE 64
#endif
#endif
#endif

#ifndef BENCHMARK_INTERNAL_CACHELINE_SIZE
#define BENCHMARK_INTERNAL_CACHELINE_SIZE 64
#endif

#if defined(__GNUC__)
#define BENCHMARK_INTERNAL_CACHELINE_ALIGNED \
  __attribute__((aligned(BENCHMARK_INTERNAL_CACHELINE_SIZE)))
#elif defined(_MSC_VER)
#define BENCHMARK_INTERNAL_CACHELINE_ALIGNED \
  __declspec(align(BENCHMARK_INTERNAL_CACHELINE_SIZE))
#else
#define BENCHMARK_INTERNAL_CACHELINE_ALIGNED
#endif

#endif  // BENCHMARK_MACROS_H_
