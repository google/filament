/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_UTILS_COMPILER_H
#define TNT_UTILS_COMPILER_H

// compatibility with non-clang compilers...
#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#ifndef __has_feature
#define __has_feature(x) 0
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if __has_attribute(visibility)
#    define UTILS_PUBLIC  __attribute__((visibility("default")))
#else
#    define UTILS_PUBLIC  
#endif

#if __has_attribute(deprecated)
#   define UTILS_DEPRECATED [[deprecated]]
#else
#   define UTILS_DEPRECATED
#endif

#if __has_attribute(packed)
#   define UTILS_PACKED __attribute__((packed))
#else
#   define UTILS_PACKED
#endif

#if __has_attribute(noreturn)
#    define UTILS_NORETURN __attribute__((noreturn))
#else
#    define UTILS_NORETURN
#endif

#if __has_attribute(fallthrough)
#   define UTILS_FALLTHROUGH [[fallthrough]]
#else
#   define UTILS_FALLTHROUGH
#endif

#if __has_attribute(visibility)
#    ifndef TNT_DEV
#        define UTILS_PRIVATE __attribute__((visibility("hidden")))
#    else
#        define UTILS_PRIVATE
#    endif
#else
#    define UTILS_PRIVATE
#endif

#define UTILS_NO_SANITIZE_THREAD
#if __has_feature(thread_sanitizer)
#undef UTILS_NO_SANITIZE_THREAD
#define UTILS_NO_SANITIZE_THREAD __attribute__((no_sanitize("thread")))
#endif

#define UTILS_HAS_SANITIZE_THREAD 0
#if __has_feature(thread_sanitizer) || defined(__SANITIZE_THREAD__)
#undef UTILS_HAS_SANITIZE_THREAD
#define UTILS_HAS_SANITIZE_THREAD 1
#endif

#define UTILS_HAS_SANITIZE_MEMORY 0
#if __has_feature(memory_sanitizer)
#undef UTILS_HAS_SANITIZE_MEMORY
#define UTILS_HAS_SANITIZE_MEMORY 1
#endif

/*
 * helps the compiler's optimizer predicting branches
 */
#if __has_builtin(__builtin_expect)
#   ifdef __cplusplus
#      define UTILS_LIKELY( exp )    (__builtin_expect( !!(exp), true ))
#      define UTILS_UNLIKELY( exp )  (__builtin_expect( !!(exp), false ))
#   else
#      define UTILS_LIKELY( exp )    (__builtin_expect( !!(exp), 1 ))
#      define UTILS_UNLIKELY( exp )  (__builtin_expect( !!(exp), 0 ))
#   endif
#else
#   define UTILS_LIKELY( exp )    (!!(exp))
#   define UTILS_UNLIKELY( exp )  (!!(exp))
#endif

#if __has_builtin(__builtin_expect_with_probability)
#   ifdef __cplusplus
#      define UTILS_VERY_LIKELY( exp )    (__builtin_expect_with_probability( !!(exp), true, 0.995 ))
#      define UTILS_VERY_UNLIKELY( exp )  (__builtin_expect_with_probability( !!(exp), false, 0.995 ))
#   else
#      define UTILS_VERY_LIKELY( exp )    (__builtin_expect_with_probability( !!(exp), 1, 0.995 ))
#      define UTILS_VERY_UNLIKELY( exp )  (__builtin_expect_with_probability( !!(exp), 0, 0.995 ))
#   endif
#else
#   define UTILS_VERY_LIKELY( exp )    (!!(exp))
#   define UTILS_VERY_UNLIKELY( exp )  (!!(exp))
#endif

#if __has_builtin(__builtin_prefetch)
#   define UTILS_PREFETCH( exp ) (__builtin_prefetch(exp))
#else
#   define UTILS_PREFETCH( exp )
#endif

#if __has_builtin(__builtin_assume)
#   define UTILS_ASSUME( exp ) (__builtin_assume(exp))
#else
#   define UTILS_ASSUME( exp )
#endif

#if (defined(__i386__) || defined(__x86_64__))
#   define UTILS_HAS_HYPER_THREADING 1      // on x86 we assume we have hyper-threading.
#else
#   define UTILS_HAS_HYPER_THREADING 0
#endif

#if defined(FILAMENT_SINGLE_THREADED)
#   define UTILS_HAS_THREADING 0
#elif defined(__EMSCRIPTEN__)
#   if defined(__EMSCRIPTEN_PTHREADS__) && defined(FILAMENT_WASM_THREADS)
#      define UTILS_HAS_THREADING 1
#   else
#      define UTILS_HAS_THREADING 0
#   endif
#else
#   define UTILS_HAS_THREADING 1
#endif

#if __has_attribute(noinline)
#define UTILS_NOINLINE __attribute__((noinline))
#else
#define UTILS_NOINLINE
#endif

#if __has_attribute(always_inline)
#define UTILS_ALWAYS_INLINE __attribute__((always_inline))
#else
#define UTILS_ALWAYS_INLINE
#endif

#if __has_attribute(pure)
#define UTILS_PURE __attribute__((pure))
#else
#define UTILS_PURE
#endif

#if __has_attribute(maybe_unused) || (defined(_MSC_VER) && _MSC_VER >= 1911)
#define UTILS_UNUSED [[maybe_unused]]
#define UTILS_UNUSED_IN_RELEASE [[maybe_unused]]
#elif __has_attribute(unused)
#define UTILS_UNUSED __attribute__((unused))
#define UTILS_UNUSED_IN_RELEASE __attribute__((unused))
#else
#define UTILS_UNUSED
#define UTILS_UNUSED_IN_RELEASE
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1900
#    define UTILS_RESTRICT __restrict
#elif (defined(__clang__) || defined(__GNUC__))
#    define UTILS_RESTRICT __restrict__
#else
#    define UTILS_RESTRICT
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1900
#   define UTILS_HAS_FEATURE_CXX_THREAD_LOCAL 1
#elif __has_feature(cxx_thread_local)
#   define UTILS_HAS_FEATURE_CXX_THREAD_LOCAL 1
#else
#   define UTILS_HAS_FEATURE_CXX_THREAD_LOCAL 0
#endif

#if defined(__clang__)
#define UTILS_NONNULL _Nonnull
#define UTILS_NULLABLE _Nullable
#else
#define UTILS_NONNULL
#define UTILS_NULLABLE
#endif

#if defined(_MSC_VER)
// MSVC does not support loop unrolling hints
#   define UTILS_UNROLL
#   define UTILS_NOUNROLL
#else
// C++11 allows pragmas to be specified as part of defines using the _Pragma syntax.
#   define UTILS_UNROLL _Pragma("unroll")
#   define UTILS_NOUNROLL _Pragma("nounroll")
#endif

#if __has_feature(cxx_rtti) || defined(_CPPRTTI)
#   define UTILS_HAS_RTTI 1
#else
#   define UTILS_HAS_RTTI 0
#endif

#ifdef __ARM_ACLE
#   include <arm_acle.h>
#   define UTILS_WAIT_FOR_INTERRUPT()   __wfi()
#   define UTILS_WAIT_FOR_EVENT()       __wfe()
#   define UTILS_BROADCAST_EVENT()      __sev()
#   define UTILS_SIGNAL_EVENT()         __sevl()
#   define UTILS_PAUSE()                __yield()
#   define UTILS_PREFETCHW(addr)        __pldx(1, 0, 0, addr)
#else // !__ARM_ACLE
#   if (defined(__i386__) || defined(__x86_64__))
#       define UTILS_X86_PAUSE              {__asm__ __volatile__( "rep; nop" : : : "memory" );}
#       define UTILS_WAIT_FOR_INTERRUPT()   UTILS_X86_PAUSE
#       define UTILS_WAIT_FOR_EVENT()       UTILS_X86_PAUSE
#       define UTILS_BROADCAST_EVENT()
#       define UTILS_SIGNAL_EVENT()
#       define UTILS_PAUSE()                UTILS_X86_PAUSE
#       define UTILS_PREFETCHW(addr)        UTILS_PREFETCH(addr)
#   else // !x86
#       define UTILS_WAIT_FOR_INTERRUPT()
#       define UTILS_WAIT_FOR_EVENT()
#       define UTILS_BROADCAST_EVENT()
#       define UTILS_SIGNAL_EVENT()
#       define UTILS_PAUSE()
#       define UTILS_PREFETCHW(addr)        UTILS_PREFETCH(addr)
#   endif // x86
#endif // __ARM_ACLE


// ssize_t is a POSIX type.
#if defined(WIN32) || defined(_WIN32)
#include <Basetsd.h>
typedef SSIZE_T ssize_t;
#endif

#ifdef _MSC_VER
#   define UTILS_EMPTY_BASES __declspec(empty_bases)
#else
#   define UTILS_EMPTY_BASES
#endif

#if defined(WIN32) || defined(_WIN32)
    #define IMPORTSYMB __declspec(dllimport)
#else
    #define IMPORTSYMB
#endif

#if defined(_MSC_VER) && !defined(__PRETTY_FUNCTION__)
#    define __PRETTY_FUNCTION__ __FUNCSIG__
#endif 


#if defined(_MSC_VER)
#   define UTILS_WARNING_PUSH _Pragma("warning( push )")
#   define UTILS_WARNING_POP _Pragma("warning( pop )")
#   define UTILS_WARNING_ENABLE_PADDED _Pragma("warning(1: 4324)")
#elif defined(__clang__)
#   define UTILS_WARNING_PUSH _Pragma("clang diagnostic push")
#   define UTILS_WARNING_POP  _Pragma("clang diagnostic pop")
#   define UTILS_WARNING_ENABLE_PADDED _Pragma("clang diagnostic warning \"-Wpadded\"")
#else
#   define UTILS_WARNING_PUSH
#   define UTILS_WARNING_POP
#   define UTILS_WARNING_ENABLE_PADDED
#endif

#endif // TNT_UTILS_COMPILER_H
